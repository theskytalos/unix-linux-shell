#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <filehandler.h>

#define MAX_COMMAND_SIZE 512

void ReadFile(char* FilePathName, char*** Commands, int* CommandsSize)
{
	FILE* pFile = NULL;

	char LINE[MAX_COMMAND_SIZE];
	int LineCounter = 0;

	pFile = fopen(FilePathName, "r");

	if (pFile == NULL)
	{
		printf("Não foi possível abrir o arquivo de entrada.\n");
		exit(EXIT_FAILURE);
	}

	while (fgets(LINE, MAX_COMMAND_SIZE, pFile))
	{
		(*Commands) = (char**) realloc((*Commands), (LineCounter + 1) * sizeof(char**));
		(*Commands)[LineCounter] = (char*) malloc((strlen(LINE) + 1) * sizeof(char*));
		strcpy((*Commands)[LineCounter], LINE);

		// Checa se há um \n para ser removido
		if ((*Commands)[LineCounter][strlen((*Commands)[LineCounter]) - 1] == '\n')
			(*Commands)[LineCounter][strlen((*Commands)[LineCounter]) - 1] = '\0'; // Remove o \n do último caractere do comando

		LineCounter++;
	}

	*CommandsSize = LineCounter;

	fclose(pFile);
}

_Bool CheckFileExistence(char* FilePathName)
{
	if (access(FilePathName, F_OK) != -1)
		return TRUE;
	else
		return FALSE;
}
