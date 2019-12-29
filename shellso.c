// Grupo: José Venâncio Junior, Lucas de Freitas Vidigal, Mariana Lellis
// Data Inicial: 15/03/2018
// Data Última Modificação: 12/04/2018
//
// ************* CHANGELOG *************
// >> Lendo comandos, e os executando, juntamente com seus respectivos parâmetros
// >> Implementação da leitura de comandos via arquivo pelo argv
// >> Comando 'fim' adicionado, interconexão de comandos via pipes '|'
// >> Caractere '&' reconhecido no final do comando para execução em segundo plano
// >> Reconhecimento de comandos de redirecionamento de entrada e saída padrão
// >> Implementação de redirecionamento de entrada e saída padrão
// >> SIGCHLD implementado para evitar criação de processos zumbis quando rodando comandos em segundo plano. Resolvido bug no malloc que não alocava espaço para '\0'.
// >> Trocado wait(NULL) por waitpid para evitar problemas de esperar processos anteriores que não eram para ser esperados. Troca no método de redirecionamento de entrada e saída padrão.
// >> Implementado o encerramento da shell com Ctrl+D

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <filehandler.h>
#include <stringhelper.h>

#define MAX_COMMAND_SIZE 512

void ExecCommand(char*);
void FreeArray(char***, int);
void FreeString(char**);

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		char Command[MAX_COMMAND_SIZE];

		while (TRUE)
		{
			printf("$ ");
			if (fgets(Command, MAX_COMMAND_SIZE, stdin) != NULL) // Lê o comando da entrada padrão e verifica se Ctrl+D é pressionado.
				ExecCommand(Command); // Passa o comando para a função principal
			else
				return EXIT_SUCCESS; // Caso o seja pressionado, fecha o shell.
		}
	}
	else if (argc == 2)
	{
		if (CheckFileExistence(argv[1]))
		{
			char** Commands = NULL;
			int CommandsSize;

			// Lê os comandos presentes no arquivo informado, e popula o vetor de strings Commands com os mesmos.
			ReadFile(argv[1], &Commands, &CommandsSize);

			// Executa todos comandos presentes no arquivo.
			for (int i = 0; i < CommandsSize; i++)
				ExecCommand(Commands[i]);

			// Uma vez executados todos comandos, o vetor de entrada é liberado da memória.
			FreeArray(&Commands, CommandsSize);
		}
		else
		{
			printf("O arquivo informado para execução de comandos não existe.\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		printf("Número de argumentos inválido.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// Função que executa um ou mais (interconectados via pipes) comandos.
void ExecCommand(char* Command)
{
	char** Commands = NULL;
	int SizeCommands = 0;

	// Checa se é o comando 'fim'.
	if (CheckForEnd(Command))
		exit(EXIT_SUCCESS);

	// Checa pela presença do & para execução em segundo plano.
	_Bool AsyncCommand = CheckForAsync(&Command);

	// Separa todos os comandos por |.
	ExplodeCommand(Command, &Commands, &SizeCommands, OUT_TO_ENTRY);

	int Pipe[2], AUX_Pipe[2];
	pipe(Pipe);
	pipe(AUX_Pipe);

	for (int i = 0; i < SizeCommands; i++)
	{
		// Quando há multiplexação de pipes, havendo a necessidade de escrever em um pipe já fechado, recria o pipe necessário.
		if (i > 1 && SizeCommands > 2)
			if (i % 2 == 0)
				pipe(Pipe);
			else
				pipe(AUX_Pipe);

		char** CommandRipped = NULL;
		int SizeCommandRipped = 0;
		char* FileIn = NULL;
		char* FileOut = NULL;
		_Bool FileInSet = FALSE;
		_Bool FileOutSet = FALSE;

		// Separa todos os componentes do comando por espaços e tabulações.
		ExplodeCommand(Commands[i], &CommandRipped, &SizeCommandRipped, SEPARATOR);

		if (CheckForPipeFile_Out(&CommandRipped, &SizeCommandRipped, &FileOut))
		{
			FileOutSet = TRUE;
			if (SizeCommands > 1 && i != (SizeCommands - 1))
			{
				printf("Os comandos de redirecionamento de saída devem ficar na última parcela do comando.\n");
				exit(EXIT_FAILURE);
			}
		}

		if (CheckForPipeFile_In(&CommandRipped, &SizeCommandRipped, &FileIn))
		{
			FileInSet = TRUE;
			if (SizeCommands > 1 && i != 0)
			{
				printf("Os comandos de redirecionamento de entrada devem ficar na primeira parcela do comando.\n");
				exit(EXIT_FAILURE);
			}
		}

		//Adiciona NULL no final do vetor de pedaços, padrão da syscall execvp.
		AddNull(&CommandRipped, &SizeCommandRipped);

		// Previnir é melhor que remediar... "vai que", né?
		if (CommandRipped != NULL)
		{
			pid_t pid;
			int CHKDup2_1 = 0;
			int CHKDup2_2 = 0;

			pid = fork();

			if (pid < 0)
				perror("Erro");
			else if (pid == 0) // Processo filho.
			{
				if (SizeCommands > 1)
				{
					if (i == 0)
					{
						if (FileInSet)
						{
							if (CheckFileExistence(FileIn))
							{
								int FileDescriptor = open(FileIn, O_RDONLY);

								if (FileDescriptor == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}

								CHKDup2_1 = dup2(FileDescriptor, STDIN_FILENO);

								if (CHKDup2_1 == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}
							}
							else
							{
								printf("O arquivo informado para redirecionamento de entrada padrão não existe.\n");
								exit(EXIT_FAILURE);
							}
						}

						close(Pipe[0]);
						CHKDup2_1 = dup2(Pipe[1], STDOUT_FILENO);

						if (CHKDup2_1 == -1)
						{
							perror("Erro");
							exit(EXIT_FAILURE);
						}
					}
					else if (i > 0 && i < (SizeCommands - 1))
					{
						// Necessidade de multiplexação de pipes.
						if (SizeCommands > 3)
						{
							if (i % 2 == 0)
							{
								CHKDup2_1 = dup2(AUX_Pipe[0], STDIN_FILENO);

								if (CHKDup2_1 == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}

								close(Pipe[0]);
								CHKDup2_2 = dup2(Pipe[1], STDOUT_FILENO);

								if (CHKDup2_2 == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}
							}
							else
							{
								CHKDup2_1 = dup2(Pipe[0], STDIN_FILENO);

								if (CHKDup2_1 == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}

								close(AUX_Pipe[0]);
								CHKDup2_2 = dup2(AUX_Pipe[1], STDOUT_FILENO);

								if (CHKDup2_2 == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}
							}
						}
						else
						{
							CHKDup2_1 = dup2(Pipe[0], STDIN_FILENO);

							if (CHKDup2_1 == -1)
							{
								perror("Erro");
								exit(EXIT_FAILURE);
							}

							close(AUX_Pipe[0]);
							CHKDup2_2 = dup2(AUX_Pipe[1], STDOUT_FILENO);

							if (CHKDup2_2 == -1)
							{
								perror("Erro");
								exit(EXIT_FAILURE);
							}
						}
					}
					else if (i > 0 && i == (SizeCommands - 1))
					{
						if (SizeCommands > 2)
						{
							if (i % 2 == 0)
							{
								CHKDup2_1 = dup2(AUX_Pipe[0], STDIN_FILENO);

								if (CHKDup2_1 == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}

								// Redirecionamento de saída padrão.
								if (FileOutSet)
								{
									// Abre o arquivo, caso o crie, cria-o com permissões de Leitura, Escrita, Execução para qualquer usuário.
									int FileDescriptor = open(FileOut, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

									if (FileDescriptor == -1)
									{
										perror("Erro");
										exit(EXIT_FAILURE);
									}

									CHKDup2_2 = dup2(FileDescriptor, STDOUT_FILENO);

									if (CHKDup2_2 == -1)
									{
										perror("Erro");
										exit(EXIT_FAILURE);
									}
								}
							}
							else
							{
								CHKDup2_1 = dup2(Pipe[0], STDIN_FILENO);

								if (CHKDup2_1 == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}

								// Redirecionamento de saída padrão.
								if (FileOutSet)
								{
									// Abre o arquivo, caso o crie, cria-o com permissões de Leitura, Escrita, Execução para qualquer usuário.
									int FileDescriptor = open(FileOut, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

									if (FileDescriptor == -1)
									{
										perror("Erro");
										exit(EXIT_FAILURE);
									}

									CHKDup2_2 = dup2(FileDescriptor, STDOUT_FILENO);

									if (CHKDup2_2 == -1)
									{
										perror("Erro");
										exit(EXIT_FAILURE);
									}
								}
							}
						}
						else
						{
							CHKDup2_1 = dup2(Pipe[0], STDIN_FILENO);
							if (CHKDup2_1 == -1)
							{
								perror("Erro");
								exit(EXIT_FAILURE);
							}

							// Redirecionamento de saída padrão.
							if (FileOutSet)
							{
								// Abre o arquivo, caso o crie, cria-o com permissões de Leitura, Escrita, Execução para qualquer usuário.
								int FileDescriptor = open(FileOut, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

								if (FileDescriptor == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}

								CHKDup2_2 = dup2(FileDescriptor, STDOUT_FILENO);

								if (FileDescriptor == -1)
								{
									perror("Erro");
									exit(EXIT_FAILURE);
								}
							}
						}
					}
				}
				else
				{
					if (FileInSet)
					{
						if (CheckFileExistence(FileIn))
						{
							int FileDescriptor = open(FileIn, O_RDONLY);

							if (FileDescriptor == -1)
							{
								perror("Erro");
								exit(EXIT_FAILURE);
							}

							CHKDup2_1 = dup2(FileDescriptor, STDIN_FILENO);

							if (CHKDup2_1 == -1)
							{
								perror("Erro");
								exit(EXIT_FAILURE);
							}

						}
						else
						{
							printf("O arquivo informado para redirecionamento de entrada padrão não existe.\n");
							exit(EXIT_FAILURE);
						}
					}

					if (FileOutSet)
					{
						// Abre o arquivo, caso o crie, cria-o com permissões de Leitura, Escrita, Execução para qualquer usuário.
						int FileDescriptor = open(FileOut, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

						if (FileDescriptor == -1)
						{
							perror("Erro");
							exit(EXIT_FAILURE);
						}

						CHKDup2_1 = dup2(FileDescriptor, STDOUT_FILENO);

						if (CHKDup2_1 == -1)
						{
							perror("Erro");
							exit(EXIT_FAILURE);
						}
					}
				}

				// Substitui a imagem do filho pelo comando fornecido.
				if (execvp(CommandRipped[0], CommandRipped) == -1)
				{
					// Se houver erro em algum comando, desaloca tudo e sai do loop, caso tenha mais de um comando.
					perror("Erro");

					if (FileInSet)
						FreeString(&FileIn);
					if (FileOutSet)
						FreeString(&FileOut);

					FreeArray(&CommandRipped, SizeCommandRipped);

					break;
				}
			}
			else if (pid > 0) // Processo pai.
			{
				if (SizeCommands > 1)
				{
					if (i == 0)
						close(Pipe[1]);
					else if (i > 0 && i < (SizeCommands - 1))
					{
						if (SizeCommands > 3)
						{
							if (i % 2 == 0)
							{
								close(AUX_Pipe[0]);
								close(Pipe[1]);
							}
							else
							{
								close(Pipe[0]);
								close(AUX_Pipe[1]);
							}
						}
						else
						{
							close(Pipe[0]);
							close(AUX_Pipe[1]);
						}
					}
					else if (i > 0 && i == (SizeCommands - 1))
					{
						if (SizeCommands > 2)
						{
							if (i % 2 == 0)
								close(AUX_Pipe[0]);
							else
								close(Pipe[0]);
						}
						else
							close(Pipe[0]);
					}
				}

				if (!AsyncCommand)
					wait(NULL); // Na ausência do símbolo '&' no final do comando, o processo pai trava esperando pelo término do filho.
				else
					signal(SIGCHLD, SIG_IGN); // Caso não seja usado o wait, SIGCHLD é usado para a prevenção de criação de processos zumbis.
			}
		}

		// Se houver, independente se usados ou não, arquivos para redirecionamento de entrada/saída padrão, estes são desalocados.
		if (FileInSet)
			FreeString(&FileIn);
		if (FileOutSet)
			FreeString(&FileOut);

		FreeArray(&CommandRipped, SizeCommandRipped); // Libera da memória os 'pedaços' do comando já executado.
	}

	FreeArray(&Commands, SizeCommands); // Uma vez executados todos os comandos, os libera da memória.
}

// Desaloca memória de um array de strings ou de uma matriz
void FreeArray(char*** Array, int Size)
{
	for (int i = 0; i < Size; i++)
		free((*Array)[i]);

	free(*Array);
}

// Desaloca memória de uma string
void FreeString(char** String)
{
	free(*String);
}
