#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#define TRUE 1
#define FALSE 0

void ReadFile(char*, char***, int*);
void WriteFile(char*, char*);
_Bool CheckFileExistence(char*);
_Bool ReadEntryFile(char*, char**);

#endif
