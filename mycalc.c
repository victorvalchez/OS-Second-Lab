// Internal command: mycalc.
if (strcmp(argv_execvp[0], "mycalc") == 0) {
    // We ensure that we receive three arguments (<operand_1>, <add/mod>, <operand_2>) to perform the operation.
    if ((argv_execvp[1] != NULL) && (argv_execvp[2] != NULL) && (argv_execvp[3] != NULL)) {
        
        // ADD --> Addition operation.
        if (strcmp(argv_execvp[2], "add") == 0) {
            
            // We separate the two operands into 2 variables by taking them from positions 1 and 3 from argv_execvp.
            int operand1 = atoi(argv_execvp[1]);
            int operand2 = atoi(argv_execvp[3]);

            // Environment variable Acc which stores the addition of the results of the sums.
            Acc = operand1 + operand2 + Acc;

            // We define a buffer to keep track of the accumulated sums.
            char buffer[20];
            sprintf(buffer, "%d", Acc);
            const char *p = buffer;
            
            // Environment variable Add which stores the addition of the sums done.
            if (setenv("Acc", p, 1) < 0) {
                perror("[ERROR] Error in enviroment variable\n");
                return(-1);
            }
    
            char str[100];
            snprintf(str, 100, "[OK] %d + %d = %d; Acc %s\n", operand1, operand2, operand1 + operand2, getenv("Acc"));

            // We check if there is any error while writing.
            if((write(2, str, strlen(str))) < 0){
                perror("[ERROR] Error while writing\n");
                return(-1);
            }
                         
        }

        // MOD --> Modulus operation.
        else if (strcmp(argv_execvp[2], "mod") == 0) {
            int operand1 = atoi(argv_execvp[1]);
            int operand2 = atoi(argv_execvp[3]);
            char str[100];
        
            snprintf(str, 100, "[OK] %d %% %d = %d; Quotient %d\n", operand1, operand2, operand1 % operand2, abs(floor(operand1 / operand2)));

            // We check if there is any error while writing the message on screen.
            if((write(2, str, strlen(str)))<0){
                perror("[ERROR] Error while writing\n");
                return(-1);
            }
    
        } 

        // If add nor mod are selected as operations, the format of the call is not correct and an error is shown.
        else {
            // We check if there is any error while writing the ERROR message on screen.
            if((write(1, "[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n", strlen("[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n"))) < 0) {
                perror("Error while writing.");
                return(-1);
            }    
        }
    }
    
    // If some of the arguments is null, the format of the call is not correct and an error is shown.
    else {
        // We check if there is any error while writing the ERROR message on screen.
        if((write(1, "[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n", strlen("[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n"))) < 0) {
            perror("Error while writing.");
            return(-1);
        }
    }
}
