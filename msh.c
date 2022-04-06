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

		//To excute a single command through the child, not necessary to create pipes
		if (command_counter == 1) {
            int pid = fork();
			switch(pid){
				case -1:
                	perror("Process creation error");
                	return (-1);
				
				//In case the fork was ok, we receive 0 from the child's process
				case 0:
					int check_file=0;
        			int stat;  //CAMBIAR NOMBRE DE ESTO; CUANDO SEPA QUE SIGNIFICA
			
					//To redirect the standard input
					if (strcmp(filev[0], "0") != 0) {
						if((close(0)) <0){
							perror("Closing STD_IN error");
						}

						if ((check_file = open(filev[0], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error while opening new input file\n");
						}
					}

					//To redirect the standard ouput
					if (strcmp(filev[1], "0") != 0) {
						if((close(1)) <0){
							perror("Closing STD_OUT error");
						}

						if ((check_file = open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error while opening new ouput file\n");
						}
					}

					//To redirect the standard error
					if (strcmp(filev[2], "0") != 0) {
						if((close(2)) <0){
							perror("Closing STD_ERR error");
						}

						if ((check_file = open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0644)) < 0) {
							perror("Error while opening new error file\n");
						}
					}
                
					//Make the child process exceute the command
					if (execvp(argv_execvp[0], argv_execvp) < 0) {
						perror("Error al ejecutar\n");
					}
             
					default: //Parent process
						if(check_file!=0){
							if((close(check_file)) <0){
								perror("Error al cerrar descriptor");
							}
						}
                    	if (!in_background) {
                      		while (wait(&stat) > 0);
                      		if (stat < 0) {
                        		perror("Error ejecucion hijo\n");
                      		}
                   		 }	
            }
		}

		//If we receive more than 1 parameter we need to create pipes
		else {
			int n_commands= command_counter;
            int fd[2];
            int pid, status2;
            int check_file=0;
			int in;


		}
    }
	
	return 0;
}