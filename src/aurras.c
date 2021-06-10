//cliente
// Created by faa on 09/06/21.
//

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
//#include <errno.h>
#include <time.h>






int main (int argc, char** argv){

    int arr[5];
    int inc=0;
    int fd_fifo_client_server;
    int fd_fifo_server_client;

    for (int i=0;i<5;i++){
        arr[i] = inc++;
    }

    //Open pipe for client-server comms
    if (mkfifo("client_server_pipe", 0666) == -1){
            perror("Não criou o fifo cli-serv\n");
            return 1;
    }

    if (mkfifo("server_client_pipe", 0666) == -1){
        perror("Não criou o fifo serv-cli\n");
        return 1;
    }


    fd_fifo_client_server = open ("client_server_pipe", O_WRONLY);
    if (fd_fifo_client_server == -1) return 1;


    for (int i=0; i<5; i++){
        if (write(fd_fifo_client_server, &arr[i], sizeof (int)) == -1){
            return 2;
        }
        printf("Cliente Escreveu %d\n", arr[i]);
    }


    // close (fd_fifo_client_server);

//-----------------------------------------------------------------------------------------------
    //Receiving from server
    
    fd_fifo_server_client = open("server_client_pipe", O_RDONLY);
    if (fd_fifo_server_client == -1) return 1;


    for (int i=0; i<5; i++){
        if (read (fd_fifo_server_client, &arr[i], sizeof (int )) == -1){
            return 2;
        }
        printf("Cliente Recebeu %d\n", arr[i]);
    }
    // close(fd_fifo_server_client);

    return 0;
}