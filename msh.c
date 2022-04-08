// MSH main file.

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

// To perform operations with the environment variable mycalc.
#include <math.h>

/*
PERM 0644 constant:
- User can read and write.
- Group can read.
- Others can read.
*/
#define PERM 0644

// Constant for the size of the buffer.
#define BUFF_SIZE 512

// Maximum number of commands.
#define MAX_COMMANDS 8

// Files in case of redirection.
char filev[3][64];

// To store the execvp second parameter.
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("**** Exiting MSH **** \n");
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


// Main sheell loop. 
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

    // Variable to store the error message while writing.
    char writing_error[100] = "[ERROR] Error while writing\n";
    
	char ***argvv = NULL;
	int num_commands;

    // Environment variable for the internal command mycalc.
    int Acc = 0;


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
			
			for (int i = 0; i < command_counter; i++) {
                getCompleteCommand(argvv, i);
        	} 
			/* else {
                // print_command(argvv, filev, in_background);
			} */
		}

        // Internal command: mycalc.
        if (strcmp(argv_execvp[0], "mycalc") == 0) {
            
			// Error message for the structure of mycalc
			char error_structure[100] = "[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n";
            
			// We ensure that we receive three arguments (<operand_1>, <add/mod>, <operand_2>) to perform the operation.
            if ((argv_execvp[1] != NULL) && (argv_execvp[2] != NULL) && (argv_execvp[3] != NULL)) {
	
                // ADD --> Addition operation.
                if (strcmp(argv_execvp[2], "add") == 0) {
                    
                    // We separate the two operands into 2 integer variables (using atoi) by taking them from positions 1 and 3 from argv_execvp.
                    int operand1 = atoi(argv_execvp[1]);
                    int operand2 = atoi(argv_execvp[3]);
        
                    // Environment variable Acc which stores the addition of the results of the sums.
                    Acc = operand1 + operand2 + Acc;
        
                    // We define a buffer to keep track of the accumulated sums.
                    char buffer[20];
                    sprintf(buffer, "%d", Acc);
                    const char *p = buffer;
                    
                    // Environment variable Acc which stores the addition of the sums done.
                    if (setenv("Acc", p, 1) < 0) {
                        perror("[ERROR] Error in enviroment variable\n");
                        return(-1);
                    }
            
                    char ok_op[120];
                    snprintf(ok_op, 120, "[OK] %d + %d = %d; Acc %s\n", operand1, operand2, operand1 + operand2, getenv("Acc"));
        
                    // We check if there is any error while writing.
                    if((write(2, ok_op, strlen(ok_op))) < 0){
                        perror(writing_error);
                        return(-1);
                    }
                                 
                }
        
                // MOD --> Modulus operation.
                else if (strcmp(argv_execvp[2], "mod") == 0) {

                    // We separate the two operands into 2 integer variables (using atoi) by taking them from positions 1 and 3 from argv_execvp.
                    int operand1 = atoi(argv_execvp[1]);
                    int operand2 = atoi(argv_execvp[3]);
                    
                    char ok_op[120];
                    snprintf(ok_op, 120, "[OK] %d %% %d = %d; Quotient %d\n", operand1, operand2, operand1 % operand2, abs(floor(operand1 / operand2)));
        
                    // We check if there is any error while writing the message on screen.
                    if((write(2, ok_op, strlen(ok_op)))<0){
                        perror(writing_error);
                        return(-1);
                    }
            
                } 
        
                // If add nor mod are selected as operations, the format of the call is not correct and an error is shown.
                else {
                    // We check if there is any error while writing the ERROR message on screen.
                    if((write(1, error_structure, strlen(error_structure))) < 0) {
                        perror(writing_error);
                        return(-1);
                    }    
                }
            }
            
            // If some of the arguments is null, the format of the call is not correct and an error is shown.
            else {
                // We check if there is any error while writing the ERROR message on screen.
                if((write(1, error_structure, strlen(error_structure))) < 0) {
                    perror(writing_error);
                    return(-1);
                }
            }
        }


        // Internal command: mycp.
        else if (strcmp(argv_execvp[0], "mycp") == 0) {

            char error_structure[100] = "[ERROR] The structure of the command is mycp <original_file> <copied_file>\n";
            
            // We ensure that we receive two arguments (<original_file> and <copied_file>) to perform the operation.
            if ((argv_execvp[1] != NULL) && (argv_execvp[2] != NULL)) {
        
                // We declare two descriptors (one for the origin and another one for the destination).
                // Each of them will store both the descriptor of the first and second file respectively given as arguments.
                int fd_in = open(argv_execvp[1], O_RDONLY, PERM);
                int fd_out = open(argv_execvp[2], O_TRUNC | O_WRONLY | O_CREAT, PERM);

                // Variables to store different errors.
                char open_error[100] = "[ERROR] Error opening the original file\n";

                
                if (fd_in >= 0) {
        
                    if (fd_out < 0) {
                        if((write(1, "[ERROR] Error opening the copied file\n", strlen("[ERROR] Error opening the copied file\n"))) < 0){
                            perror(writing_error);
                            return(-1);
                        }
                    }
                    
                    // Declaration of variables to store the bytes read and written.
                    int nwrite, nread;
                    // We define the buffer of size 512 as stated in BUFF_SIZE constant.
                    char buffer[BUFF_SIZE];
        
                    // We repeat the operation until there are no bytes left to read in the origin file.
                    while ((nread = read(fd_in, buffer, BUFF_SIZE)) > 0) {
                        // If something has been read from the origin file, we enter the loop.
                        do {
                            // We save the number of bytes written on success.
                            nwrite = write(fd_out, buffer, nread);
                            // We check if there is any error while writing.
                            if (nwrite < 0) {
                                // We check if there is any error while closing the descriptor (fd_out).
                                if (close(fd_out) < 0) {
                                    perror("[ERROR] Error closing the copied file\n");
                                    return(-1);
                                }
                            perror(writing_error);
                            return(-1);
                            }
                            // We update the number of bytes left to be written into destination.
                            nread -= nwrite;
                        } while (nread > 0);
                    }
        
                    // We check that the number of bytes read 
                    if (nread < 0) {
                        perror("[ERROR] Error while reading the file\n");
                        if (close(fd_in) < 0) {
                            perror("[ERROR] Error closing the original file\n");
                        }
                      }
                    
                    // After copying everything we close the corresponding files and check for errors
                    if (close(fd_in) < 0) {
                        perror("[ERROR] Error closing the original file\n");
                    }
                      
                    if (close(fd_out) < 0) {
                        perror("[ERROR] Error closing the copied file\n");
                    }

                    /*
                    char ok_copy[100];
                    snprintf(ok_copy, 100, "[OK] Copy has been successful between %s and %s\n", argv_execvp[1], argv_execvp[2]);
                    write(1, ok_copy, strlen(ok_copy));
                    */

        			char ok_msg[120];
                    snprintf(ok_msg, 120, "[OK] Copy has been successful between %s and %s\n", argv_execvp[1], argv_execvp[2]);
                    write(1, ok_msg, 120);
                    
                } 
                // If the original file has not been successfully opened, an error is raised.
                else {
                    if((write(1, open_error, strlen(open_error))) < 0){
                        perror(writing_error);
                        return(-1);
                    }
                }
            }
            // If argument 1 or 2 of argv_execvp is Null, it means that the structure of the command introduced is not correct and an error is raised.
            else {
                if((write(1, error_structure, strlen(error_structure))) <0){
                    perror(writing_error);
                    return -1;
                }
            }
        }

		// PART 1 & 2 --> EXECUTION OF SIMPLE COMMANDS AND EXECUTION IN BACKGROUND.
		// To excute a single command through the child, not necessary to create pipes.
		else if (command_counter == 1) {
			int descfile = 0;	// New file descriptor for when opening the respective files.
            int pid = fork();
			// Create the child process and check the different creation cases.
			switch(pid) {
				// In case the creation was a failure.
				case -1:
                	perror("[ERROR] Process creation error");
                	return (-1);
				
				//In case the fork was ok, we receive 0 from the child's process.
				case 0:
					// To redirect the standard input.
					// We check that the redirected input file exists, so it does not contain a zero.
					if (strcmp(filev[0], "0") != 0) {
						// Close it.
						if((close(0)) <0) {
							perror("[ERROR] Closing STD_IN error");
							return (-1);
						}
						// And open the new one
						if ((descfile = open(filev[0], O_RDWR, PERM)) < 0) {
							perror("[ERROR] Error while opening new input file");
							return (-1);
						}
					}
					
					// To redirect the standard ouput
					// Same as before, to check if it exists
					if (strcmp(filev[1], "0") != 0) {
						// Close it
						if((close(1)) <0){
							perror("[ERROR] Closing STD_OUT error");
							return (-1);
						}
						// And open the new one
						if ((descfile = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, PERM)) < 0) {
							perror("[ERROR] Error while opening new ouput file");
							return (-1);
						}
					}

					// To redirect the standard error
					// Same as before, to check if it exists
					if (strcmp(filev[2], "0") != 0) {
                        
						// Check for errors while closing
						if((close(2)) <0){
							perror("[ERROR] Closing STD_ERR error");
							return (-1);
						}
						// And open the new one
						if ((descfile = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, PERM)) < 0) {
							perror("[ERROR] Error while opening new error file");
							return (-1);
						}
					}
					
					/* Now, make the child process exceute the command. argv_execvp contains the command and was obtained by means of getCompleteCommand, no need to put '&' before the second one as it is a character vector. It only takes [0] as this part is only for when we have 1 command. */
					// getCompleteCommand(argvv, i);
					if (execvp(argv_execvp[0], argv_execvp) < 0) {
						perror("[ERROR] There was an error executing command in the child");
						return (-1);
					}
					break;
				
				/* As fork() returns the child's PID for the parent and we do not now it, this is the default case, the parent process */
				default:
					// If the descriptor obtained is not equal to 0
					if (descfile) { // descfile != 0
						if ((close(descfile)) < 0){
							perror("[ERROR] Error while closing new file descriptor");
							return (-1);
						}
					}
                    if (in_background == 0) {
                      	while (wait(&status) > 0);
                      		if (status < 0) {
                        		perror("[ERROR] Error in child's execution");
								return (-1);
                      		}
                   	}	
					else {
						pid = getpid();
	                    printf("[%d]\n", pid);
					}
            }
		}
		// PART 3 --> EXECUTION OF MORE THAN 1 COMMAND.
		// If we receive more than 1 parameter we need to create pipes.
		else {
			int n_commands = command_counter;
            int fd[2];		// Pipe input and output descriptors.
            int pid;
            int descfile = 0;
			int desc_dup;

			// To make the standard input be desc_dup
			if ((desc_dup = dup(0)) < 0) {
				perror("[ERROR] Descriptor could not be duplicated");
				return (-1);
			}
			// For each parameter (command) passed we create a pipe to execute it.
            for (int i = 0; i < n_commands; i++) {
            // Unless it is the last command that we do not create it
            	if (i != n_commands - 1) {
                    if (pipe(fd) < 0) {
                        perror("[ERROR] Pipe creation error");
                        return (-1);
                    }
                }

				// Create a process for each loop iteration, that is to say, for each command.
                switch (pid = fork()) {
                    case -1:
                      	perror("[ERROR] Process creation error");
						// Close both read and write descriptors created by the pipe.
						if ((close(fd[0])) < 0) {
							perror("[ERROR] Error while closing input descriptor");
							return (-1);
							
						}
						if ((close(fd[1])) < 0) {
							perror("[ERROR] Error while closing output descriptor");
							return (-1);
						}
                    
					// Child process.
					case 0:
						// Check if the error redirection file exists and open it.
                      	if (strcmp(filev[2], "0") != 0) {
							if((close(2)) <0) {
								perror("[ERROR] Error while closing error file descriptor");
								return (-1);
							}

							if ((descfile = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, PERM)) < 0) {
								perror("[ERROR] Error while opening new error file");
								return (-1);
							}
                      	}
						//If it is the first iteration, and we have redirection to a new input file, we close the defaul and open this one.
                      	if (i == 0 && strcmp(filev[0], "0") != 0) {
							if((close(0)) <0){
								perror("[ERROR] Error while closing default input descriptor");
								return (-1);
							}
							if ((descfile = open(filev[0], O_RDWR, PERM)) < 0) {
								perror("[ERROR] Error while opening new input file");
								return (-1);
							}
                      	}
						// Otherwise we set the new input file as a duplicate from the previous.
						else {
							if((close(0)) <0) {
								perror("[ERROR] Error while closing the default input");
								return (-1);
							}
							// Duplicate the first duplicated descriptor.
							if (dup(desc_dup) < 0) {
								perror("[ERROR] Error while duplicating the new input");
								return (-1);
							}
							// Close that first duplicated descriptor to only get the input through this one.
							if((close(desc_dup)) <0){
								perror("[ERROR] Error while closing the old new input");
								return (-1);
							}
                      	}

						// Check if it is not the last process and close the default output.
                      	if (i != n_commands - 1) {
							if((close(1)) <0) {
								perror("[ERROR] Error while closing the output file");
								return (-1);
							}
							// Duplicate the pipe output, so that the next one can get it from it.
							if (dup(fd[1]) < 0) {
								perror("[ERROR] Error while duplicating the pipe's output");
								return (-1);
							}
							// Close the current pipe's input.
							if ((close(fd[0])) < 0) {
								perror("[ERROR] Error while closing the pipe's input");
								return (-1);
							}
							// Close the current pipe's output.
							if ((close(fd[1])) < 0) {
								perror("[ERROR] Error while closing the pipe's output");
								return (-1);
							}
                      	}
						// If it is the last one, check the output redirection file, close the old one and se it as the current one.
					  	else {
                        	if (strcmp(filev[1], "0") != 0) {
								if((close(1)) <0) {
									perror("[ERROR] Error while closing output file");
									return (-1);
								}

								if ((descfile = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, PERM)) < 0) {
									perror("[ERROR] Error while opening new output file.");
									return (-1);
								}
                        	}
                    	}
						
						// Check if it is executing in background and print on the screen the child's pid with the format specified.
                      	getCompleteCommand(argvv, i);
						if (in_background == 1) {
							pid = getpid();
	                      	printf("[%d]\n", pid);
	                    }
						// Execute the current command.
						if (execvp(argv_execvp[0], argv_execvp) < 0) {
							perror("[ERROR] Error while executing command.");
							return (-1);
							}
						break;
					
					// For the parent process (as it returns the child's pid).
                    default:
						// Close the current input descriptor
						if ((close(desc_dup)) < 0) {
							perror("[ERROR] Error while closing the input descriptor.");
							return (-1);
						}
						// If it is the last command
						if (i != n_commands - 1) {
							// Set the value of desc_dup as the new input
							if ((desc_dup = dup(fd[0])) < 0) {
								perror("[ERROR] Error while duplicating input descriptor.");
								return (-1);
							}
							// A VER SI BORRANDO ESTO FUNCIONA PORQUE NO LO ENTIENDO
							// Duplicate the old one 
							if (dup(fd[0]) < 0) {
								perror("[ERROR] Error al duplicar descriptor.\n");
								return (-1);
							}
							// Close the current output descriptor
							if ((close(fd[1])) < 0) {
								perror("[ERROR] Error while closing output descriptor.");
								return (-1);
							}
                      	}
				}			
			}
			// Close the redirection file
			if (descfile != 0) {
				if((close(descfile)) < 0) {
					perror("[ERROR] Error while closing redirection file descriptor.");
					return (-1);
				}
			}
            
			// Wait for each corresponding children to finish
            if (in_background == 0) {
                while (wait(&status) > 0) {
                    if (status < 0) {
                      	perror("[ERROR] Error in child's execution.\n");
						return (-1);
                    }
                }
            }
		}
    }
	return 0;
}
