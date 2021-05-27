#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <cstdlib>
#include "time.h"

int main() {
    int* ptr1 = (int*)malloc(sizeof(int));
    int* ptr2 = new int;
    int* ptr3 = new int[10];
    clock_t start, finish; 
    long i = 10000000L; 
    start = clock();
    while (i--);
    finish = clock();
    printf("Time of loop: %f\n", (double)(finish - start));
    free(ptr1);
    delete ptr2;
	// int* ptr3 = (int*)malloc(sizeof(int) * 40);
	// int* ptr4 = (int*)malloc(sizeof(int) * 40);
	// int* ptr5 = (int*)malloc(sizeof(int) * 40);
	// int* ptr6 = (int*)malloc(sizeof(int) * 40);
	// int* ptr7 = (int*)malloc(sizeof(int) * 40);
    FILE* fp = fopen("../log.out", "r");
    int a = fgetc(fp);

    fclose(fp);

    FILE* fp1 = fopen("../output.log", "r");
    int b = fgetc(fp1);

    fclose(fp1);


    return 0;
}
