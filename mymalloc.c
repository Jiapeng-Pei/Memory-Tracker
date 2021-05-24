#define _GNU_SOURCE
#include <sys/types.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <execinfo.h>
#include "time.h" 

#define gettid() syscall(__NR_gettid)

/*
Struct
*/

struct malloc_list_node {
    struct malloc_list_node* prev;
    struct malloc_list_node* next;
    clock_t create_time;
    char* info_str;
    size_t size;
};

/*
Global Variables
*/

int total_pointer = 0;
size_t total_used_size = 0;
struct malloc_list_node* head = NULL;
size_t HEAD_SIZE = sizeof(struct malloc_list_node);
double EXPIRE_TIME = 10000;
FILE *mem_fp = NULL;
FILE *handle_fp = NULL;

int trace_flag = 0;
int fopen_flag = 1;

/*
Functions
*/

void Insert_Node(struct malloc_list_node *);
void Remove_Node(struct malloc_list_node *);
void Print_Node_Action(int mode, size_t size, struct malloc_list_node* option);
void Check_Malloc_Time();
void Get_Trace_Pointer(struct malloc_list_node *);

static void * (*real_malloc)(size_t size) = NULL;
static void (*real_free)(void *ptr) = NULL;
static void * (*leak_real_calloc)(size_t nmemb, size_t size) = NULL;
FILE *(*real_fopen)(const char *pathname, const char *mode) = NULL;
int (*real_fclose)(FILE *file) = NULL;

/* 
tmp memory management 
*/

static int tmp_buffer[1024 * 1024]; /* increase this if necessary */
static int *tmp_buf_pos = tmp_buffer;
static void *tmp_malloc(size_t size)
{
    size = (size + 7) / 8 * 8;  // 我认为是对齐。能够对齐到最小的8的倍数
    if (size > sizeof(tmp_buffer) - (tmp_buf_pos - tmp_buffer)) {   // tmp_buffer没有空间了
        abort();
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
    if (fopen_flag) {
    	mem_fp = fopen("mem_info.log", "w+");
    	handle_fp = fopen("handle_info.log", "w+");
    	fopen_flag = 0;
    	char buffer[100];
    	(void) sprintf(buffer, "            Currnet thread's ID: %lu \n\n", gettid());
    	(void) write(mem_fp->_fileno, buffer, strlen(buffer));
    	(void) write(handle_fp->_fileno, buffer, strlen(buffer));
    }
    
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_free = dlsym(RTLD_NEXT, "free");
    real_fopen = dlsym(RTLD_NEXT, "fopen");
    real_fclose = dlsym(RTLD_NEXT, "fclose");
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
    ptr->create_time = clock(); 
    Get_Trace_Pointer(ptr);
    total_used_size += size;

    // 使用write是为了解决递归调用的问题
    char buffer[100];
    (void) sprintf(buffer, "malloc (%u), return %p \n", (unsigned int)size, user_ptr);
    (void) write(mem_fp->_fileno, buffer, strlen(buffer));

    Insert_Node(ptr);

    if(trace_flag != 1) {
        print_trace();
    }

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
    total_used_size -= ptr->size;

    // 输出结果，使用write防止递归调用
    char buffer[100];
    (void) sprintf(buffer, "free (%p)\n", usr_ptr);
    (void) write(mem_fp->_fileno, buffer, strlen(buffer));

    Check_Malloc_Time();
    Remove_Node(ptr);
    real_free(ptr);

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
    Print_Node_Action(2, cur->size, NULL);
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
    Print_Node_Action(1, cur->size, NULL);
}

void Print_Node_Action(int mode, size_t size, struct malloc_list_node* option) {
    char buffer[100];
    if (mode == 3) {
        //TODO, print malloc calling function string.
        (void) sprintf(buffer, "    %s\n", option->info_str);
    }
    else if (mode == 2) {
        (void) sprintf(buffer, "  --insert node with size = %u, Unfreed pointer: %u, Used_size = %lu\n\n", 
            (unsigned int)size, total_pointer, total_used_size);
    }
    else if (mode == 1) {
        (void) sprintf(buffer, "  --remove node with size = %u, Unfreed pointer: %u, Used size = %lu\n\n", 
            (unsigned int)size, total_pointer, total_used_size);
    }

    (void) write(mem_fp->_fileno, buffer, strlen(buffer));
}

void Check_Malloc_Time() {
    struct malloc_list_node* cur_ptr = head;
    clock_t cur_time = clock();
    int flag = 0;
    while (cur_ptr != NULL) {
        if (flag || (double)(cur_time - cur_ptr->create_time) > EXPIRE_TIME) {
            Print_Node_Action(3, 0, cur_ptr);
            flag = 1;
        }  
        cur_ptr = cur_ptr->next;    
    }
}

/*
FILE_HANDLE part
*/

FILE *fopen(const char *pathname, const char *mode) {
    if (!real_fopen) {
        real_fopen = dlsym(RTLD_NEXT, "fopen");
    }

    FILE * res = real_fopen(pathname, mode);
    if (!fopen_flag) {
    	char buffer[100];
    	(void) sprintf(buffer, "\nfopen(%s, %s)\n  return %p \n", pathname, mode, res);
    	(void) write(handle_fp->_fileno, buffer, strlen(buffer));
    }

    return res;
}


int fclose(FILE *file) {
    if (!real_fclose) {
        real_fclose = dlsym(RTLD_NEXT, "fclose");
    }

    int MAXSIZE = 100;
    char proclnk[100];
    char filename[100];
    int fno;
    ssize_t r;
    int res;

    if (file != NULL)
    {
        fno = fileno(file);
        sprintf(proclnk, "/proc/self/fd/%d", fno);
        r = readlink(proclnk, filename, MAXSIZE);
        // if (r < 0)
        // {
        //     printf("failed to readlink\n");
        // }
        filename[r] = '\0';
        
        char buffer[100];
        (void) sprintf(buffer, "\nfclose(%p) \n  filepath = %s\n", file, filename);
        (void) write(handle_fp->_fileno, buffer, strlen(buffer));
        res = real_fclose(file);
    }
    
    return res;
}

/*
Malloc call origin function part
*/

int has_dy = 0;
int has_static = 0;
int all_libcall = 0;

void
Get_Trace_Pointer (struct malloc_list_node* cur)
{
  void *array[10];
  char **strings;
  char res_string[1000] = "";
  char dy_func[100];

  int size, i;
  trace_flag = 1;
  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);
  trace_flag = 0;
  if (strings != NULL)
  {
    
    for (i = 0; i < size; i++){
        if(strstr(strings[i], ".so")){
            continue;
        }
        if(strstr(strings[i], "/lib")){
            continue;
        }
        char * left_p = strstr(strings[i], "(");
        if(*(left_p+1) != '+'){
            all_libcall = 0;
            has_dy = 1;

            strcpy(dy_func,strings[i]);
            strcat(dy_func,"\n");

            break;
        }
        else {
            has_static = 1;
            all_libcall = 0;
        }

    }
    if(all_libcall){
        strcat(res_string,"Library function leakage!\n");
    }
    if(has_static){
        strcat(res_string,"Static function leakage!\n");
    }
    if(has_dy){
        strcat(res_string,dy_func);
    }
  }
  cur->info_str = res_string;
}
