//cliente

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
    int fd_fifo_client_server;
    int fd_fifo_server_client;
    char* buffer = malloc(sizeof(char) * 1024);


    int i = 1;

    if(i < argc) {
        strcpy(buffer,argv[i]);
        i++;
    }

    while(i < argc) {
        strcat(buffer," ");
        strcat(buffer,argv[i]);
        i++;
    }


    if ((fd_fifo_client_server = open("client_server_pipe", O_WRONLY)) == -1) {
        perror("Error opening fifo\n");
        return -1;
    }

    if((fd_fifo_server_client = open("server_client_pipe", O_RDONLY)) == -1){
        perror("Error opening fifo");
        return -1;
     }

    if(argc == 1)
        strcpy(buffer,"info");


    write(fd_fifo_client_server,buffer,sizeof(char)*strlen(buffer));

    int n = 0;
    if((n = read(fd_fifo_server_client,buffer,1024 * sizeof(char))) > 0){
      write(1,buffer,n * sizeof(char));
    }

    return 0;
}


// ------------------------------------------------------------------------------------------------

//Separação de funcoes

//envia inputs da shell para o servidor

// void shellInputHandler(int fd_fifo_client_server){
//     write(1,"aurras$ ",strlen("aurras$ "));

//     char buffer[256]; char c;
//     int bytesread = 0;
//     while(read(0,&c,1) && c != '\n'){
//             if(c != '\"') buffer[bytesread++] = c; 
//     }
//     buffer[bytesread++] = '\0';
            
//     write(fd_fifo_client_server, buffer, bytesread);
// }


// // Receber feedback do servidor, e dar print no "stdout" da shell argus
 
// void shellOutputHandler(int fd_fifo_server_client){
//     int bytesread = 0;

//     char buffer2[512];

//     while((bytesread = read(fd_fifo_server_client, buffer2, 512)) > 0)
//                 write(1, buffer2, bytesread);
// }





// // Criação da shell
 
// int shellAssembler(int fd_fifo_client_server, int fd_fifo_server_client){
//     while (1){

//             if ((fd_fifo_client_server = open("canalClienteServidor", O_WRONLY)) == -1) {
//                 perror("Error opening fifo\n");
//                 return -1;
//             }

//             shellInputHandler(fd_fifo_client_server);
            
//             close(fd_fifo_client_server);

//             if((fd_fifo_server_client = open("canalServidorCliente", O_RDONLY)) == -1){
//                 perror("Error opening fifo");
//                 return -1;
//             }
            
//             shellOutputHandler(fd_fifo_server_client);
            
//             close(fd_fifo_server_client);
//         }
// }




// //envio de argumentos (info), separados por espaço, para o servidor
// void argumentSenderToServer (int fd_fifo_client_server, int argc, char** argv){

//     char info[512];
//     strcpy(info, argv[1]);
//     for(int i = 2; i < argc; i++){
//         strcat(info, " "); strcat(info, argv[i]); 
//     }

//     write(fd_fifo_client_server, info, strlen(info)+1);
// }


// //recebe o feedback do servidor através do fifo e apresenta no ecrã ao cliente

// void feedbackReceiverFromServer(int fd_fifo_server_client){

//     char buffer[512];
//     int bytesread;
//     while((bytesread = read(fd_fifo_server_client, buffer, 256)) > 0) write(1, buffer, bytesread);
// }




// //envia para o servidor e recebe resposta
// void senderToServer (int fd_fifo_client_server, int fd_fifo_server_client, int argc, char** argv){    

//     if ((fd_fifo_client_server = open("canalClienteServidor", O_WRONLY)) == -1) {
//             perror("Error opening fifo\n");
//             return -1;
//         }

//         argumentSenderToServer(fd_fifo_client_server, argc, argv);

//         close(fd_fifo_client_server);

//         if((fd_fifo_server_client = open("canalServidorCliente", O_RDONLY)) == -1){
//             perror("Error 2 opening fifo\n");
//             return -1;
//         }

//         feedbackReceiverFromServer(fd_fifo_server_client);

//         close(fd_fifo_server_client);

// }
