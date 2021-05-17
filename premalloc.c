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


static void __attribute__((constructor)) calledFirst();
static void calledFirst()
{
    printf("kkkkkkkkk\n");
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_free = dlsym(RTLD_NEXT, "free");
}


void * malloc(size_t size) {
    // use dlsym to find next malloc in dynamic libraries, ie. malloc in std library
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
    
    real_free(ptr);
    // 输出结果，使用write防止递归调用
    char buffer[100];
    (void) sprintf(buffer, "%lu free == %p\n", gettid(), ptr);
    (void) write(1, buffer, strlen(buffer));
}

