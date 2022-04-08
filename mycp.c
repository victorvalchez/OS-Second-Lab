// Internal command: mycp.
if (strcmp(argv_execvp[0], "mycp") == 0) {
    // We ensure that we receive two arguments (<original_file> and <copied_file>) to perform the operation.
    if ((argv_execvp[1] != NULL) && (argv_execvp[2] != NULL)) {

        // We declare two descriptors (one for the origin and another one for the destination).
        // Each of them will store both the descriptor of the first and second file respectively given as arguments.
        int fd_in = open(argv_execvp[1], O_RDONLY, PERM);
        int fd_out = open(argv_execvp[2], O_TRUNC | O_WRONLY | O_CREAT, PERM);
        
        if (fd_in >= 0) {

            if (fd_out < 0) {
                if((write(1, "[ERROR] Error opening the copied file\n", strlen("[ERROR] Error opening the copied file\n"))) < 0){
                    perror("[ERROR] Error while writing\n");
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
                    perror("[ERROR] Error while writing\n");
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
            
            char str[100];
            snprintf(str, 100, "[OK] Copy has been successful between %s and %s\n", argv_execvp[1], argv_execvp[2]);
            write(1, str, strlen(str));
			
			/*
			ESTO FUNCIONA BIEN; YA LO HE PROBADO
			char str[80];
            snprintf(str, 80, "[OK] Copy has been successful between %s and %s\n", argv_execvp[1], argv_execvp[2]);
            write(1, str, 80);*/
        } 
        // If the original file has not been successfully opened, an error is raised.
        else {
            if((write(1, "[ERROR] Error opening the original file\n", strlen("[ERROR] Error opening the original file\n"))) < 0){
                perror("[ERROR] Error while writing\n");
                return(-1);
            }
        }
    }
    // If argument 1 or 2 of argv_execvp is Null, it means that the structure of the command introduced is not correct and an error is raised.
    else {
        if((write(1, "[ERROR] The structure of the command is mycp <original_file><copied_file>\n", strlen("[ERROR] The structure of the command is mycp <original_file><copied_file>\n"))) <0){
            perror("[ERROR] Error while writing\n");
            return -1;
        }
    }
}
