#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stringhelper.h>

void ExplodeCommand(char* Command, char*** CommandPieces, int* Size, char* Discriminator)
{
	int Counter = 0;

	// Divide o comando pelo critério estabelecido na variável Discriminator
	char *Token = strtok(Command, Discriminator);

	while (Token != NULL)
	{
		// Impede a criação de um espaço no vetor para '\n'
		if (strcmp(Token, "\n") != 0)
		{
			(*CommandPieces) = (char**) realloc((*CommandPieces), (Counter + 1) * sizeof(char*)); // Aloca mais um espaço no vetor de 'peças' do comando recebido
			(*CommandPieces)[Counter] = (char*) malloc((strlen(Token) + 1) * sizeof(char)); // Aloca no espaço novo o espaço para a nova 'peça'
			strcpy((*CommandPieces)[Counter], Token); // Copia o Token para o vetor de 'peças'
			Counter++;
		}

		Token = strtok(NULL, Discriminator);
	}

	if (Counter > 0)
		// Checa se há um \n para ser removido
		if ((*CommandPieces)[Counter - 1][strlen((*CommandPieces)[Counter - 1]) - 1] == '\n')
			(*CommandPieces)[Counter - 1][strlen((*CommandPieces)[Counter - 1]) - 1] = '\0'; // Remove o \n da última 'peça' do comando


	*Size = Counter;
}

int CheckFor(char** CommandPieces, int Size, char* LookFor)
{
	for (int i = 0; i < Size; i++)
		if (strcmp(CommandPieces[i], LookFor) == 0)
			return TRUE;

	return FALSE;
}

_Bool CheckForEnd(char* Command)
{
	if (strcmp(Command, END_SHELL) == 0)
		return TRUE;

	return FALSE;
}

_Bool CheckForAsync(char** Command)
{
	int SizeCommand = strlen((*Command));

	// Checa se o último caractere (desconsiderando \n) do comando é &, se for, o substitui por \0.
	if ((*Command)[SizeCommand - 2] == '&')
	{
		(*Command)[SizeCommand - 2] = '\0';
		return TRUE;
	}

	return FALSE;
}

_Bool CheckForPipeFile_Out(char*** CommandRipped, int* SizeCommandRipped, char** FileOut)
{
	for (int i = 0; i < (*SizeCommandRipped); i++)
	{
		// Checa pela presença da string '=>' no dado comando.
		if (strcmp((*CommandRipped)[i], OUT_FILE) == 0 && i != 0)
		{
			// Previne o acesso de uma posição inválida do vetor.
			if (i != (*SizeCommandRipped - 1))
			{
				// Aloca espaço na variável FileOut, e copia a string após '=>' para a mesma.
				(*FileOut) = (char*) malloc((strlen((*CommandRipped)[i + 1]) + 1) * sizeof(char));
				strcpy((*FileOut), (*CommandRipped)[i + 1]);

				// Diminui o tamanho do vetor, eliminando '=>' e o nome do arquivo.
				(*CommandRipped) = (char**) realloc((*CommandRipped), i * sizeof(char*));
				*SizeCommandRipped = i;

				return TRUE;
			}
			else
				return FALSE;
		}
	}

	return FALSE;
}

_Bool CheckForPipeFile_In(char*** CommandRipped, int* SizeCommandRipped, char** FileIn)
{
	for (int i = 0; i < (*SizeCommandRipped); i++)
	{
		// Checa pela presença da string '<=' no dado comando.
		if (strcmp((*CommandRipped)[i], ENTRY_FILE) == 0 && i != 0)
		{
			// Previne o acesso de uma posição inválida do vetor.
			if (i != (*SizeCommandRipped - 1))
			{
				// Aloca espaço no ponteiro FileIn, e copia a string após '<=' para o mesmo.
				(*FileIn) = (char*) malloc((strlen((*CommandRipped)[i + 1]) + 1) * sizeof(char));
				strcpy((*FileIn), (*CommandRipped)[i + 1]);

				// Diminui o tamanho do vetor, eliminando '<=' e o nome do arquivo.
				(*CommandRipped) = (char**) realloc((*CommandRipped), i * sizeof(char*));
				*SizeCommandRipped = i;

				return TRUE;
			}
			else
				return FALSE;
		}
	}

	return FALSE;
}

void AddNull(char*** CommandPieces, int* Size)
{
	// Aloca mais um espaço no 'argv', este sendo nulo, padrão da função execvp
	(*CommandPieces) = (char**) realloc((*CommandPieces), ((*Size) + 1) * sizeof(char*));
	(*CommandPieces)[(*Size)] = NULL;
	(*Size)++;
}
