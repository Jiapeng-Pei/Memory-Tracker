# Design Document

Group member: 张沐阳， 陈浩， 裴嘉鹏

## Project Background and Description

- ### Project description

  This project requires developing a tool on the Linux platform, which can display the memory usage and the possible memory leakage problem of a specific process. The general requirement including aspects :

  - Statistics of processes and threads in operating system should be performed in real time.
  - Keep a real time Detection of certain process's memory leakage problem, such as its memory, file handle. The result should be displayed in real time or output to a file.

  Furthermore, the tool shouldn't cause too much effect on the system's performance, and it orients C/C++ programs.

- ### Background

  ​    Linux system needs to run at an optimal level, making the display of memory usage relevant information and monitoring memory statistics vital. Moreover, It is often essential to check memory usage and memory used per process so that resources do not fall short, since some unexpected behavior may at times be caused by system resource limitations. It may happen that the system will become very slow or even go down when there is a traffic spike. 

     A memory leak is any portion of an application which uses memory without eventually freeing it. An application that continually uses more memory without freeing any will eventually exhaust the pool of memory. When this happens, the application is likely to crash the next time it attempts to use more memory. For instance, in C/C++, a memory leak usually happens when a user invokes *new* but forgot to invoke the corresponding *delete*. In other environments, such as Java, the operating system allocates and deallocates memory automatically. While programmers are error prone to allocate and deallocate memory specifically, they control the computer's resources.
  
  #### Related Work
  
  - ##### Valgrind
  
    Valgrind is an instrumentation framework for building dynamic analysis tools. There are Valgrind tools that can automatically detect many memory management and threading bugs, and profile the programs in detail. One of Valgrind's tools is Memcheck, which is a memory error detector. It is able to detect memory leaks, using undefined values, incorrect freeing of heap memory, e.t.c.
  
  - ##### Dmalloc
  
    The debug memory allocation or dmalloc library has been designed as a drop in replacement for the system's malloc, realloc, calloc, free, and other memory management routines while providing powerful debugging facilities configurable at runtime. If any of the above debugging features detect an error, the library will try to recover.
  
  - ##### libleak
  
    This a lighter-weight tool compared to the two tools mentioned above. libleak detects memory leaks by hooking memory functions (e.g., malloc) by LD_PRELOAD. There is no need to modify or re-compile the target program, and the user can enable/disable the detection during target running. Moreover, there is less impact on performance compared with Valgrind.
  
    



## Implementation







## Expected Goals





## Division of Labor