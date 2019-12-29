#ifndef __STRING_HELPER_H__
#define __STRING_HELPER_H__

#define TRUE 1
#define FALSE 0

#define END_SHELL 	"fim\n"
#define ENTRY_FILE 	"<="
#define OUT_FILE 	"=>"
#define OUT_TO_ENTRY	"|"
#define SEPARATOR	" "
#define BACKGROUND	'&'

void ExplodeCommand(char*, char***, int*, char*);
_Bool CheckForAsync(char**);
_Bool CheckForEnd(char*);
_Bool CheckForPipeFile_Out(char***, int*, char**);
_Bool CheckForPipeFile_In(char***, int*, char**);
void AddNull(char***, int*);
int CheckCommand(char**, int);

#endif
