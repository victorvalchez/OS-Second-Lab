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
			int descfile = 0;	// New file descriptor for when opening the respective files
            int pid = fork();
			// Create the child process and check the different creation cases
			switch(pid) {
				// In case the creation was a failure
				case -1:
                	perror("Process creation error");
                	return (-1);
				
				//In case the fork was ok, we receive 0 from the child's process
				case 0:
					// To redirect the standard input
					// We check that the redirected input file exists, so it does not contain a zero
					if (strcmp(filev[0], "0") != 0) {
						// Close it
						if((close(0)) <0){
							perror("Closing STD_IN error");
							return (-1);
						}
						// And open the new one
						if ((descfile = open(filev[0], O_RDWR, 0644)) < 0) {
							perror("Error while opening new input file");
							return (-1);
						}
					}
					
					// To redirect the standard ouput
					// Same as before, to check if it exists
					if (strcmp(filev[1], "0") != 0) {
						// Close it
						if((close(1)) <0){
							perror("Closing STD_OUT error");
							return (-1);
						}
						// And open the new one
						if ((descfile = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error while opening new ouput file");
							return (-1);
						}
					}

					// To redirect the standard error
					// Same as before, to check if it exists
					if (strcmp(filev[2], "0") != 0) {
						// Close it
						if((close(2)) <0){
							perror("Closing STD_ERR error");
							return (-1);
						}
						// And open the new one
						if ((descfile = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error while opening new error file");
							return (-1);
						}
					}
					
					/* Now, make the child process exceute the command. argv_execvp contains the command and was obtained by means of getCompleteCommand, no need to put '&' before the second one as it is a character vector. It only takes [0] as this part is only for when we have 1 command. */
					if (execvp(argv_execvp[0], argv_execvp) < 0) {
						perror("There was an error executing command in the child");
						return (-1);
					}
					break;
				
				/* As fork() returns the child's PID for the parent and we do not now it, this is the default case, the parent process */
				default:
					// If the descriptor obtained is not equal to 0
					if (descfile) { // descfile != 0
						if ((close(descfile)) < 0){
							perror("Error while closing new file descriptor");
							return (-1);
						}
					}
                    if (in_background == 0) {
                      	while (wait(&status) > 0);
                      		if (status < 0) {
                        		perror("Error ejecucion hijo\n");
								return (-1);
                      		}
                   	}	
            }
		}
		// PART 3, EXECUTION OF MORE THAN 1 COMMAND
		//If we receive more than 1 parameter we need to create pipes
		else {
			int n_commands = command_counter;
            int fd[2];		// Pipe input and output descriptors
            int pid;
            int descfile = 0;
			int desc_dup;

			// To make the standard input be desc_dup
			if ((desc_dup = dup(0)) < 0) {
				perror("Descriptor could not be duplicated");
				return (-1);
			}
			//For each parameter (command) passed we create a pipe to execute it
            for (int i = 0; i < n_commands; i++) {
            // Unless it is the last command that we do not create it
            	if (i != n_commands - 1) {
                    if (pipe(fd) < 0) {
                        perror("Pipe creation error");
                        return (-1);
                    }
                }

				// Create a process for each loop iteration, that is to say, for each command
                switch (pid = fork()) {
                    case -1:
                      	perror("Process creation error");
						// Close both read and write descriptors created by the pipe
						if ((close(fd[0])) < 0) {
							perror("Error while closing input descriptor");
							return (-1);
							
						}
						if ((close(fd[1])) < 0) {
							perror("Error while closing output descriptor");
							return (-1);
						}
                    
					// Child process
					case 0:
						// Check if the error redirection file exists and open it
                      	if (strcmp(filev[2], "0") != 0) {
							if((close(2)) <0) {
								perror("Error while closing error file descriptor");
								return (-1);
							}

							if ((descfile = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
								perror("Error while opening new error file");
								return (-1);
							}
                      	}
						//If it is the first iteration, and we have redirection to a new input file, we close the defaul and open this one
                      	if (i == 0 && strcmp(filev[0], "0") != 0) {
							if((close(0)) <0){
								perror("Error while closing default input descriptor");
								return (-1);
							}
							if ((descfile = open(filev[0], O_RDWR, 0644)) < 0) {
								perror("Error while opening new input file");
								return (-1);
							}
                      	}
						// Otherwise we set the new input file as a duplicate from the previous
						else {
							if((close(0)) <0) {
								perror("Error while closing the default input");
								return (-1);
							}
							// Duplicate de first duplicated descriptor
							if (dup(desc_dup) < 0) {
								perror("Error while duplicating the new input");
								return (-1);
							}
							// Close that first duplicated descriptor to only get the input through this one
							if((close(desc_dup)) <0){
								perror("Error while closing the old new input");
								return (-1);
							}
                      	}

						// Check if it is the last process and close the default output
                      	if (i != n_commands - 1) {
							if((close(1)) <0) {
								perror("Error while closing the output file");
								return (-1);
							}
							// Duplicate the pipe output, so that the next one can get it from it
							if (dup(fd[1]) < 0) {
								perror("Error while duplicating the pipe's output");
								return (-1);
							}
							// Close the current pipe's input
							if ((close(fd[0])) < 0) {
								perror("Error while closing the pipe's input");
								return (-1);
							}
							// Close the current pipe's output
							if ((close(fd[1])) < 0) {
								perror("Error while closing the pipe's output");
								return (-1);
							}
                      	}
						// If its not the last one, check the input redirection file, close the old one and se it as the current one
					  	else {
                        	if (strcmp(filev[1], "0") != 0) {
								if((close(1)) <0) {
									perror("Error while closing input file");
									return (-1);
								}

								if ((descfile = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
									perror("Error while opening new input file");
									return (-1);
								}
                        	}
                    	}
						
						// Check if it is executing in background and print on the screen the child's pid with the format specified
                      	getCompleteCommand(argvv, i);
							if (in_background != 0) {			
	                      		printf("[%d]\n", getpid());
	                    	}
							// Execute the current command
							if (execvp(argv_execvp[0], argv_execvp) < 0) {
								perror("Error while executing command");
								return (-1);
							}
						break;
					
					// For the parent process (as it returns the child's pid)
                    default:
                      	// El padre le da el nuevo valor a in para que pueda utilizarlo el hijo a no ser que sea el ultimo proceso
						// Close the current input descriptor
						if ((close(desc_dup)) < 0) {
							perror("Error while closing the input descriptor");
							return (-1);
						}
						// If it is the last command
						if (i != n_commands - 1) {
							// Set the value of desc_dup as the new input
							if ((desc_dup = dup(fd[0])) < 0) {
								perror("Error while duplicating input descriptor");
								return (-1);
							}
							// A VER SI BORRANDO ESTO FUNCIONA PORQUE NO LO ENTIENDO
							// Duplicate the old one 
							if (dup(fd[0]) < 0) {
								perror("Error al duplicar descriptor\n");
								return (-1);
							}
							// Close the current output descriptor
							if ((close(fd[1])) < 0) {
								perror("Error while closing output descriptor");
								return (-1);
							}
                      	}
				}			
			}
			// Close the redirection file
			if (descfile != 0) {
				if((close(descfile)) < 0) {
					perror("Error while closing redirection file descriptor");
					return (-1);
				}
			}
            
			// Wait for each corresponding children to finish
            if (in_background == 0) {
                while (wait(&status) > 0);
                    if (status < 0) {
                      	perror("Error ejecucion hijo\n");
						return (-1);
                    }
            }
		}
    }
	return 0;
}