#define _GNU_SOURCE
#include <sys/types.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>


#define gettid() syscall(__NR_gettid)


struct file_node {
    struct file_node* prev;
    struct file_node* next;
    char * pathname;
    FILE * file_ptr;
};

int file_total_pointer = 0;
struct file_node* head = NULL; 


static void * (*real_malloc)(size_t size) = NULL;
static void (*real_free)(void *ptr) = NULL;
static void *(*leak_real_calloc)(size_t nmemb, size_t size) = NULL;
FILE *(*real_fopen)(const char *pathname, const char *mode) = NULL;
int (*real_fclose)(FILE *file) = NULL;
void Insert_File_Node(struct file_node* cur); 
void Remove_File_Node(struct file_node* cur);
struct file_node * Find_File_node(FILE * file);
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

static int tmp_free(void *p)  //   tmp_buffer ptr <= p <= ptr+size
{
    return (p >= (void *)tmp_buffer) && (p <= (void *)tmp_buffer + sizeof(tmp_buffer));
}




static void __attribute__((constructor)) calledFirst();
static void calledFirst()
{
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_free = dlsym(RTLD_NEXT, "free");
    real_fopen = dlsym(RTLD_NEXT, "fopen");
    real_fclose = dlsym(RTLD_NEXT, "fclose");
}


void * malloc(size_t size) {
    if (real_malloc == NULL) {  
        return tmp_malloc(size);
    }

    void * p = real_malloc(size);
    // char buffer[100];
    // (void) sprintf(buffer, "%lu malloc (%u) == %p\n", gettid(), (unsigned int)size, p);
    // (void) write(1, buffer, strlen(buffer));
    
    return p;
}


void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    if (tmp_free(ptr)) {  // 
        return;
    }

    real_free(ptr);
    
    // char buffer[100];
    // (void) sprintf(buffer, "%lu free == %p\n", gettid(), ptr);
    // (void) write(1, buffer, strlen(buffer));
}

FILE *fopen(const char *pathname, const char *mode) {
    char buffer[100];
    (void) sprintf(buffer, "called fopen(%s, %s)\n", pathname, mode);
    (void) write(1, buffer, strlen(buffer));
    if (!real_fopen) {
        real_fopen = dlsym(RTLD_NEXT, "fopen");
    }
    FILE * res = real_fopen(pathname, mode);
    return res;
}


int fclose(FILE *file) {
    if (!real_fclose) {
        real_fclose = dlsym(RTLD_NEXT, "fclose");
    }


    int MAXSIZE = 0xFFF;
    char proclnk[0xFFF];
    char filename[0xFFF];
    int fno;
    ssize_t r;
    int res;

    if (file != NULL)
    {
        fno = fileno(file);
        sprintf(proclnk, "/proc/self/fd/%d", fno);
        r = readlink(proclnk, filename, MAXSIZE);
        if (r < 0)
        {
            printf("failed to readlink\n");
            exit(1);
        }
        filename[r] = '\0';
        
        char buffer[100];
       (void) sprintf(buffer, "filename is %s\n", filename);
       (void) write(1, buffer, strlen(buffer));
        res = real_fclose(file);
    }
    
    
    return res;
}
