#ifndef AURRAS_H
#define AURRAS_H


/* AURRAS (Cliente) */

void shellInputHandler(int fd_fifo_client_server);
void shellOutputHandler(int fd_fifo_server_client);
int shellAssembler(int fd_fifo_client_server, int fd_fifo_server_client);
void argumentSenderToServer (int fd_fifo_client_server, int argc, char** argv);
void feedbackReceiverFromServer(int fd_fifo_server_client);
void senderToServer (int fd_fifo_client_server, int fd_fifo_server_client, int argc, char** argv);



/* AURRASD (Servidor) */



#endif