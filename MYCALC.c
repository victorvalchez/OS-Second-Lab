// Internal command: mycalc.
if (strcmp(argv_execvp[0], "mycalc") == 0) {
    if (argv_execvp[1]!=NULL && argv_execvp[2]!=NULL && argv_execvp[3]!=NULL) {
        
        // Addition operation.
        if (strcmp(argv_execvp[2], "add") == 0) {
            int x = atoi(argv_execvp[1]);
            int y = atoi(argv_execvp[3]);
            Acu = Acu + x + y;
            char buf[20];
            sprintf(buf, "%d", Acu);
            const char *p = buf;
        
            // Environment variable Add which stores the addition of the sums done.
            if (setenv("Acc", p, 1) < 0) {
                perror("Error giving enviroment variable\n");
                goto error;
            }
    
            char str[100];
            snprintf(str, 100, "[OK] %d + %d = %d; Acc %s\n", x, y, x + y, getenv("Acc"));
      
            if((write(2, str, strlen(str)))<0){
                perror ("Error writing");
                goto error;
            }
                         
        } 

        //Operacion mod, en esta, no se cambia el modulo
        else if (strcmp(argv_execvp[2], "mod") == 0) {
            int x = atoi(argv_execvp[1]);
            int y = atoi(argv_execvp[3]);
            char str[100];
        
            snprintf(str, 100, "[OK] %d %% %d = %d * %d + %d\n", x, y, y, abs(floor(x / y)), x % y);
            if((write(2, str, strlen(str)))<0){
                perror ("Error writing");
                goto error;
            }
    
        } 
        
        else {
            if((write(1, "[ERROR] The structure of the command is <operand_1> <add/mod> <operand_2>", strlen("[ERROR] The structure of the command is <operand_1> <add/mod> <operando_2>"))) <0) {
                perror ("Error writing");
                goto error;
            }    
        }
    }
        
    else {
        if((write(1, "[ERROR] The structure of the command is <operand_1> <add/mod> <operand_2>", strlen("[ERROR] The structure of the command is <operand_1> <add/mod> <operando_2>"))) <0) {
            perror ("Error writing");
            goto error;
        }
    }
}