# Unix/Linux shell

### Introduction

This project was made to the Operating Systems class lectured by Dr. Rafael Sachetto Oliveira in 2019/1 period at the Federal University of São João del-Rei.

The main objective of the project was to gain experience dealing with processes and communication between them by using pipes (syscall).

So it should be developed an Unix/Linux shell command interpreter capable of launching processes and chain the communication between them (creating a pair series of producer/consumer via pipes). 

The program should be made entirely and only in standard C language.

### Compilation & Running

No installation nor any dependency is needed, besides using an Unix/Linux platform and `make` program to compile.

To compile the program, inside the folder of the project, just type in your terminal ($ represents the shell):

```
$ make
```

Then to run, type:

```
$ ./shellso
```

### Usage Overview

There are some differences requested in the project specification to the original shell, those are:

Command | Effect
------------ | -------------
fim | Finishes the shell program.
\| | Gets the output of the left process and uses it as input to the right process.
& | This symbol used at the end of a line indicates that the commands should run in background.
=> | Connects a pipe to the standard out of a process and writes it to a file. File will be created if not exists.
<= | Connects a pipe to the standard in of a process, reading the data from a file. An error is printed if the file doesn't exists.


Disclaimer: The shell can also be finished by pressing Ctrl + D.
