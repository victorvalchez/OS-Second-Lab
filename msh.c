// MSH main file.
// Write your msh source code here.

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8


// files in case of redirection.
char filev[3][64];

//to store the execvp second parameter.
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Exiting MSH **** \n");
	//signal(SIGINT, siginthandler);
	exit(0);
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */

void getCompleteCommand(char*** argvv, int num_command) {
	// Reset first.
	for(int j = 0; j < 8; j++)
		argv_execvp[j] = NULL;

	int i = 0;
	for ( i = 0; argvv[num_command][i] != NULL; i++)
		argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{
	/**** Do not delete this code.****/
	int end = 0; 
	int executed_cmd_lines = -1;
	char *cmd_line = NULL;
	char *cmd_lines[10];

	if (!isatty(STDIN_FILENO)) {
		cmd_line = (char*)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF){
			if(strlen(cmd_line) <= 0) return 0;
			cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
			strcpy(cmd_lines[end], cmd_line);
			end++;
			fflush (stdin);
			fflush(stdout);
		}
	}

	/*********************************/

	char ***argvv = NULL;
	int num_commands;


	while (1) {
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		// Prompt 
		write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

		// Get command
		//********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
		executed_cmd_lines++;
		if( end != 0 && executed_cmd_lines < end) {
			command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
		}
		else if( end != 0 && executed_cmd_lines == end) {
			return 0;
		}
		else {
			command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
		}
		//************************************************************************************************


		/************************ STUDENTS CODE ********************************/
	    if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			}
			else {
				for (int i = 0; i < command_counter; i++) {
                  getCompleteCommand(argvv, i);
                }
			}
		}
		// PART 1,2 EXECUTION OF SIMPLE COMMANDS, AND EXECUTION IN BACKGROUND
		//To excute a single command through the child, not necessary to create pipes
		if (command_counter == 1) {
			int filehandle=0;
        	int stat;  //CAMBIAR NOMBRE DE ESTO; CUANDO SEPA QUE SIGNIFICA
            int pid = fork();
			switch(pid) {
				case -1:
                	perror("Process creation error");
                	return (-1);
				

				//In case the fork was ok, we receive 0 from the child's process
				case 0:
					//To redirect the standard input
					if (strcmp(filev[0], "0") != 0) {
						if((close(0)) <0){
							perror("Closing STD_IN error");
							return (-1);
						}

						if ((filehandle = open(filev[0], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error while opening new input file\n");
							return (-1);
						}
					}

					//To redirect the standard ouput
					if (strcmp(filev[1], "0") != 0) {
						if((close(1)) <0){
							perror("Closing STD_OUT error");
							return (-1);
						}

						if ((filehandle = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error while opening new ouput file\n");
							return (-1);
						}
					}

					//To redirect the standard error
					if (strcmp(filev[2], "0") != 0) {
						if((close(2)) <0){
							perror("Closing STD_ERR error");
							return (-1);
						}

						if ((filehandle = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error while opening new error file\n");
							return (-1);
						}
					}
					break;	// As it is not returning anything we have to put break;
					//Make the child process exceute the command
					if (execvp(argv_execvp[0], argv_execvp) < 0) {
						perror("Error al ejecutar\n");
						return (-1);
					}
             
				default: //Parent process
					if(filehandle!=0){
						if((close(filehandle)) <0){
							perror("Error al cerrar descriptor");
							return (-1);
						}
					}
                    if (!in_background) {
                      	while (wait(&stat) > 0) {
                      		if (stat < 0) {
                        		perror("Error ejecucion hijo\n");
								return (-1);
                      		}
						}
                   	}	
            }
		}
		// PART 3, EXECUTION OF MORE THAN 1 COMMAND
		//If we receive more than 1 parameter we need to create pipes
		else {
			int n_commands = command_counter;
            int fd[2];		//
            int pid, status2;
            int filehandle = 0;
			
			int desc_dup;

			// To make the standard input be desc_dup
			if ((desc_dup = dup(0)) < 0) {
				perror("Descriptor could not be duplicated\n");
				return (-1);
			}
			//For each parameter (command) passed we create a pipe to execute it
            for (int i = 0; i < n_commands; i++) {
            // Unless it is the last command that we do not create it
            	if (i != n_commands - 1) {
                    if (pipe(fd) < 0) {
                        perror("Pipe creation error\n");
                        return (-1);
                    }
                }

				/* se crea el proceso siguiente en la cadena */
                switch (pid = fork()) {
                    case -1:
                      	perror("Process creation error");
						// Close both read and write descriptors created by the pipe
						if((close(fd[0])) < 0) {
							perror("Error al cerrar descriptor");
							
						}
						if((close(fd[1])) < 0) {
							perror("Error al cerrar descriptor");
						}
                      	return (-1);
					// Child process
					case 0:
						// Cambiar orden de abrir los archivos empezar por 0
                      	if (strcmp(filev[2], "0") != 0) {
							if((close(2)) <0) {
								perror("Error al cerrar descriptor");
								return (-1);
							}

							if ((filehandle = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
								perror("Error al abrir fichero\n");
								return (-1);
							}
                      	}
						//If it is the first iteration, create the input file
                      	if (i == 0 && strcmp(filev[0], "0") != 0) {
							// Close the standard input
							if((close(0)) <0){
								perror("Error al cerrar descriptor");
								return (-1);
							}
							if ((filehandle = open(filev[0], O_RDWR, 0644)) < 0) {
								perror("Error al abrir fichero\n");
								return (-1);
							}
                      	}
						// Otherwise ... 
						else {
							if((close(0)) <0){
								perror("Error al cerrar descriptor");
								return (-1);
								}
								if (dup(desc_dup) < 0) {
									perror("Error al duplicar descriptor\n");
									return (-1);
								}
								if((close(desc_dup)) <0){
									perror("Error al cerrar descriptor");
									return (-1);
								}
                      	}

						// Check if it is the last process and close the default output
                      	if (i != n_commands - 1) {
							if((close(1)) <0) {
								perror("Error al cerrar descriptor");
								return (-1);
							}

							if (dup(fd[1]) < 0) {
								perror("Error al duplicar descriptor\n");
								return (-1);
							}
							if((close(fd[0])) <0) {
								perror("Error al cerrar descriptor");
								return (-1);
								}
							if((close(fd[1])) <0) {
								perror("Error al cerrar descriptor");
								return (-1);
							}
                      	} 
					  	else {
                        	if (strcmp(filev[1], "0") != 0) {
								if((close(1)) <0) {
									perror("Error al cerrar descriptor");
									return (-1);
								}

								if ((filehandle = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
									perror("Error al abrir fichero\n");
									return (-1);
								}
                        	}
                    	}

                      	getCompleteCommand(argvv, i);
							if (in_background) {
	                      		printf("[%d]\n", getpid());
	                    	}

							if (execvp(argv_execvp[0], argv_execvp) < 0) {
								perror("Error al ejecutar\n");
								return (-1);
							}
                     	
						break;
					// For the parent process (as it returns the child's PID )
                    default:
                      	// El padre le da el nuevo valor a in para que pueda utilizarlo el hijo a no ser que sea el ultimo proceso
						if((close(desc_dup)) <0){
							perror("Error al cerrar descriptor");
							return (-1);
						}                      
						if (i != n_commands - 1) {
							if ((desc_dup = dup(fd[0])) < 0) {
								perror("Error al duplicar descriptor\n");
								return (-1);
							}
							if (dup(fd[0]) < 0) {
								perror("Error al duplicar descriptor\n");
								return (-1);
							}
							if((close(fd[1])) <0) {
								perror("Error al cerrar descriptor");
								return (-1);
							}
                      	}
				}			
			}
			if(filehandle!=0){
				if((close(filehandle)) <0) {
					perror("Error al cerrar descriptor");
					return (-1);
				}
			}
            
			// Al terminar el bucle, el primer proceso espera al último, que irá despertando a todos
            if (!in_background) {
                while (wait(&status2) > 0) {
                    if (stat < 0) {
                      	perror("Error ejecucion hijo\n");
						return (-1);
                    }
				}
            }
		}
    }
	return 0;
}