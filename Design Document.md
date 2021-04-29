# Design Document of Process Memory Tracker

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

#### Real time statistics system process and thread memory usage：

use `/proc` file to check the memory use of process and thread and we can use ` std::ifstream` in c++ to read this file

##### 1. Get all processes' pid

​	use `std::filesystem` to check folders whose file name is a number in `/proc` , these files' name are all                 processes of the system

​	<img src="allpid.png" style="zoom:30%;" />

##### 2. Get memory usage of a certain process

​	read `/proc/pid/status` file to check information of memory usage

​	<img src="pmemusage.png" style="zoom: 33%;" />

##### 3. Get threads of a process

​	check folders in `/proc/pid/task` , folders are corresponding threads of the  process

​	<img src="tid.png" style="zoom: 50%;" />

##### 4. Get memory usage of a thread

​	read `proc/pid/task/tid/status` file to check the memory usage of a thread

##### 5. Show the information in realtime

​    repeat step 1 - step 4, every 5 million seconds then show the statistic result, in this way, it can achieve realtime statistic.



## Expected Goals

1. **Keep real-time statistics on the system process and its thread memory usage**.

   - Coding should be used to realize the statistics of memory usage information, and the memory statistics data are sorted and displayed in real time.

2. **Detection of the memory allocation and release in a process.**

   - Coding is employed to detect memory allocation and release in a specific process.
   - Coding is employed to monitor the allocation and release of the file handle in a specific process.

3. **Check whether there is a memory leak in a process.**

   - Record the process memory allocation and release, and confirm whether there is a leakage; If there is, point out the suspicious code.

   

## Division of Labor

​	The three expected goals will be solved serially. For each goal, we will generally follow the implementation route. If faced with obstacles, we'd like to attempt other approaches. To illustrate, ...