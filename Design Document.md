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

     A memory leak is any portion of an application which uses memory without eventually freeing it. An application that continually uses more memory without freeing any will eventually exhaust the pool of memory. When this happens, the application is likely to crash the next time it attempts to use more memory. For instance, in C/C++, a memory leak usually happens when a user invokes *new* but forgot to invoke the corresponding *delete*. In other environments, such as Java, the operating system allocates and deallocates memory automatically. While 



## Implementation







## Expected Goals





## Division of Labor