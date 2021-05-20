#define _GNU_SOURCE
#include <sys/types.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

using std::hex;
using std::cerr;
using std::string;
using std::ios;
using std::endl;

#define gettid() syscall(__NR_gettid)

/*
Struct
*/

struct malloc_list_node {
    struct malloc_list_node* prev;
    struct malloc_list_node* next;
    size_t size;
};

/*
Global Variables
*/

int total_pointer = 0;
size_t total_used_size = 0;
struct malloc_list_node* head = NULL; 
size_t HEAD_SIZE = sizeof(struct malloc_list_node);

/*
Functions
*/

void Insert_Node(struct malloc_list_node *);
void Remove_Node(struct malloc_list_node *);
void Print_Node_Action(int insert, size_t size);

static void * (*real_malloc)(size_t size) = NULL;
static void (*real_free)(void *ptr) = NULL;
static void * (*leak_real_calloc)(size_t nmemb, size_t size) = NULL;

/* tmp memory management */

static int tmp_buffer[1024 * 1024]; /* increase this if necessary */
static int *tmp_buf_pos = tmp_buffer;
static void *tmp_malloc(size_t size)
{
    size = (size + 7) / 8 * 8;  // 我认为是对齐。能够对齐到最小的8的倍数
    if (size > sizeof(tmp_buffer) - (tmp_buf_pos - tmp_buffer)) {   // tmp_buffer没有空间了
        //abort();
    }
    void *p = tmp_buf_pos;  //从上一个分配到的地方开始分配
    tmp_buf_pos += size;    //移动起始位置的指针
    return p;
}


// static void *tmp_calloc(size_t n, size_t size)
// {
//     void *p = tmp_malloc(n * size);
//     bzero(p, n * size);
//     return p;
// }
// static void *tmp_realloc(void *oldp, size_t size)
// {
//     void *newp = tmp_malloc(size);
//     memcpy(newp, oldp, size);
//     return newp;
// }

static int tmp_free(void *p)  //   tmp_buffer ptr <= p <= ptr+size
{
    return (p >= (void *)tmp_buffer) && (p <= (void *)tmp_buffer + sizeof(tmp_buffer));
}

static void __attribute__((constructor)) calledFirst();
static void calledFirst()
{
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_free = dlsym(RTLD_NEXT, "free");
}

void * malloc(size_t size) {
    // use dlsym to find next malloc in dynamic libraries, ie. malloc in std library
    if (real_malloc == NULL) {  // if real_malloc is null, use tmp_malloc
        return tmp_malloc(size);
    }

    // Added Part.
    size_t real_size = size + HEAD_SIZE;
    struct malloc_list_node* ptr = (struct malloc_list_node * )real_malloc(real_size);
    void * user_ptr = (char*) ptr + HEAD_SIZE;

    ptr->size = size;
    Insert_Node(ptr);
    total_used_size += size;

    // 在glibc2.23版本中不能直接使用printf，因为printf会调用malloc，形成递归调用
    // 在glibc 2.17中测试是可行的。
    // printf ("%lu malloc (%u) == 0x%08x\n", gettid(), (unsigned int) size, p);

    // 使用write是为了解决递归调用的问题
    char buffer[100];
    (void) sprintf(buffer, "%lu malloc (%u) == %p \n\n", gettid(), (unsigned int)size, user_ptr);
    TraceFile << buffer << endl;
    // (void) write(1, buffer, strlen(buffer));

    return user_ptr;
    //Added Part.
}


void free(void *usr_ptr) {
    if (usr_ptr == NULL) {
        return;
    }

    if (tmp_free(usr_ptr)) {  // 
        return;
    }

    struct malloc_list_node* ptr = (struct malloc_list_node*)((char*)usr_ptr - HEAD_SIZE);
    Remove_Node(ptr);
    total_used_size -= ptr->size;

    real_free(ptr);

    // 输出结果，使用write防止递归调用
    char buffer[100];
    (void) sprintf(buffer, "%lu free == %p\n\n", gettid(), usr_ptr);
    (void) write(1, buffer, strlen(buffer));
}
/*
void *calloc(size_t nmemb, size_t size)
{
    if (real_calloc == NULL) {
        return tmp_calloc(nmemb, size);
    }
    void *p = real_calloc(nmemb, size);
    
    return p;
}
*/

void Insert_Node(struct malloc_list_node* cur ) {
    cur -> prev = NULL;
    if (head == NULL) {
        cur -> next = NULL;
    }
    else {
        cur->next = head;
        head->prev = cur;
    }
    head = cur;
    total_pointer += 1;
    Print_Node_Action(2, cur->size);
}

void Remove_Node(struct malloc_list_node* cur ) {
    if (cur->next != NULL) {
        cur->next->prev = cur->prev;
    }
    if (cur->prev != NULL) {
        cur->prev->next = cur->next;
    }
    if (head == cur) {
        head = head->next;
    }
    cur->next = NULL;
    cur->prev = NULL;

    total_pointer -= 1;
    Print_Node_Action(1, cur->size);
}

void Print_Node_Action(int mode, size_t size) {
    char buffer[100];
    if (mode == 2) {
        (void) sprintf(buffer, "Insert node with size = %u, Used_size = %lu\n", (unsigned int)size, total_used_size + size);
    }
    else if (mode == 1) {
        (void) sprintf(buffer, "Remove node with size = %u\nNumber of unfreed pointer is %u, unfreed size = %lu\n", 
            (unsigned int)size, total_pointer, total_used_size - size);
    }
    // else if (mode == 0) {
    //     struct malloc_list_node* cur = head;
    //     int cnt = 0;
    //     while (cur != NULL) {
    //         cnt++;
    //         cur = cur->next;
    //     } 
    //     (void) sprintf(buffer, "Number of unfreed pointer is %u, unfreed size = %lu\n", total_pointer, total_used_size;
    // }
    (void) write(1, buffer, strlen(buffer));
}
