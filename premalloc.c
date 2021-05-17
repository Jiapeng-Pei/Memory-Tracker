#define _GNU_SOURCE
#include <sys/types.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>


#define gettid() syscall(__NR_gettid)

static void * (*real_malloc)(size_t size) = NULL;
static void (*real_free)(void *ptr) = NULL;
static void *(*leak_real_calloc)(size_t nmemb, size_t size) = NULL;

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
/*
static void *tmp_calloc(size_t n, size_t size)
{
    void *p = tmp_malloc(n * size);
    bzero(p, n * size);
    return p;
}
static void *tmp_realloc(void *oldp, size_t size)
{
    void *newp = tmp_malloc(size);
    memcpy(newp, oldp, size);
    return newp;
}*/
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

    void * p = real_malloc(size);

    // 在glibc2.23版本中不能直接使用printf，因为printf会调用malloc，形成递归调用
    // 在glibc 2.17中测试是可行的。
    //printf ("%lu malloc (%u) == 0x%08x\n", gettid(), (unsigned int) size, p);

    // 使用write是为了解决递归调用的问题
    char buffer[100];
    (void) sprintf(buffer, "%lu malloc (%u) == %p\n", gettid(), (unsigned int)size, p);
    (void) write(1, buffer, strlen(buffer));
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
    // 输出结果，使用write防止递归调用
    char buffer[100];
    (void) sprintf(buffer, "%lu free == %p\n", gettid(), ptr);
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
