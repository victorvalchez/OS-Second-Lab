//  MSH main file
// Write your msh source code here

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


// files in case of redirection
char filev[3][64];

//to store the execvp second parameter
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
	//reset first
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


	while (1) 
	{
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
		int command_type = -1;
		char* command_name ;
		char* mycalc = "mycalc";
		char* mycp = "mycp";
		int result = -1;
		
		

	   if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			}
			else {
				
				// -----------------------------------------------------------------
				// step 1 - determine the type process to do 
				// -----------------------------------------------------------------

				int command_type = -1;
				char* command_name = "";

				// printf("Redirect 1: %s - %d \n", filev[0], strcmp(filev[0], "0"));

				if (command_counter == 1) {
					if (strcmp(filev[0] , "0") + strcmp(filev[1], "0")  + strcmp(filev[2] , "0") == 0) {	// no redirections
						if (in_background == 0) {
							command_type = 1;
						}
						else {
							command_type = 2;
						}
					}
					else {
						command_type = 3;
					}
					// override type if command name is mycalc or mycp
					command_name = argvv[0][0];
					if (strcmp(command_name, mycalc) == 0 || strcmp(command_name, mycp) == 0) {
						command_type = 7;
					}
				}
				if (command_counter > 1) {
					if (strcmp(filev[0], "0") + strcmp(filev[1], "0") + strcmp(filev[2], "0") == 0) {	// no redirections
						if (in_background == 0) {
							command_type = 4;
						}
						else {
							command_type = 5;
						}
					}
					else {
						command_type = 6;
					}
				}

				// -----------------------------------------------------------------
				// step 2 - depending on command_type execute command(s)
				// -----------------------------------------------------------------
				switch (command_type) {
				case 1:
					// single command - no background
					printf("Executing command type. %n \n", command_type);
					result = execute_1(argv);					

					break;
				case 2:
					// single command - with background
					printf("Executing command type. %n \n", command_type);
					result = execute_2(argv);
					break;
				case 3:
					// single command - with redirection
					printf("number is equal to 100");
					break;
				case 4:
					// single command - with redirection
					printf("number is equal to 100");
					break;
				case 5:
					// single command - with redirection
					printf("number is equal to 100");
					break;
				case 6:
					// single command - with redirection
					printf("number is equal to 100");
					break;
				case 7:
					// user commands
					// printf("Executing command type. %n \n", command_type);
					result = execute_7(argv);
					break;
				default:
					printf("*** Error*** Invalid command type. %n \n", command_type);
				}
				print_command(argvv, filev, in_background);
				printf("Executing previous command with routine %n \n", command_type);


			}
		}
	}
	
	return 0;
}


/**
 * execute single command
 */
int execute_1(char** argv) {
	// char * arguments [ 3 ] = { "ls" , "-l" , "NULL" } ;
	int child_pid;
	int parent_pid;
	int child_status;
	const error = -1;


	child_pid = fork();

	switch (child_pid) {
	case -1:
		/* e r r o r */
		perror("Error in fork. \n");
		return -1;
	case 0:
		/* c h i l d */
		execvp(argv[0], argv);
		perror("Error in exec. Unknown command? \n");
		break;
	default:
		/* parent */
		do {
			parent_pid = wait(&child_status);
			if (parent_pid != child_pid) printf("process_terminated; %n\n", parent_pid);
		} while (parent_pid != child_pid);

		return child_status;
	}
	exit(0);
}


/**
 * execute single command in background
 * recreate the command including & and do a system call
 */
int execute_2(char** argv) {
	// char * arguments [ 3 ] = { "ls" , "-l" , "NULL" } ;
	char* command = "";
	char* background = " &";
	char* space = " ";
	int i = 0;

	// aqui tenemos que poner el código para unir el nombre del comando y los parámetros separados por blanco y al final un &
	do {
		strcat(command, argv[i]);
		strcat(command, space);
		i = i + 1;
	} while (argv[i] != NULL);

	strcat(command, background);

	system(command);
}

/**
 * execute user commands - mycalc and mycp
 * 
 */
int execute_7(char** argv) {
	// char * arguments [ 3 ] = { "ls" , "-l" , "NULL" } ;
	char* command = "";
	char* op1 = "";
	char* op2 = "";
	char* operation = "";
	

	command = argv[0];
	op1 = argv[1];
	operation = argv[2];
	op2 = argv[3];

}
