//servidor

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
//#include <errno.h>
#include <time.h>

//Lista listaFiltros = NULL;
//Lista pendentes = NULL;

void executarComando(char *comand){
  char* buffer[10];
  int i = 0,arg = 0,i2 = 0;

  for(;comand[i] != '\0';i++){
    if(i2 == 0)
      buffer[arg] = malloc(sizeof(char) * 30);

    if(comand[i] == ' '){
      buffer[arg][i2] = '\0';

      i2 = 0;arg++;
    } else {
      buffer[arg][i2] = comand[i];
      i2++;
    }
  }
  buffer[arg][i2] = '\0';

  buffer[arg + 1] = NULL;

  execvp(buffer[0],buffer);
}


char** separaString(char* buffer){
    char** comando = NULL;
    int nrComandos = 1;
    
    for (int i = 0; buffer[i]!='\0'; i++)
    {
        if (buffer[i] == ' ') nrComandos++;
    }
    
    comando = (char**) realloc(comando, (nrComandos + 1) * sizeof(char*));

    char* token = strtok(buffer," ");
    comando[0] = strdup(token);

    int nrComando = 1; 
    while ((token = strtok(NULL," ")) != NULL)
    {
        comando[nrComando] = strdup(token);
        nrComando++;
    }
    
    return comando;
}


void help(int fdwr) {
    char help[] = "./aurras status\n./aurras transform input-filename output-filename filter-id-1 filter-id-2 ...";
    write(fdwr, help, sizeof(help));
}


int main (int argc, char** argv){
    int fd_fifo_client_server;
    int fd_fifo_server_client,n;
    char *buffer = malloc(1024 * sizeof(char));
    char **comando;
    char comand[2][100];

    //lerConfig(argv[1]);


    while(1) {
        if ((fd_fifo_client_server = open("client_server_pipe", O_RDONLY)) == -1) {
            perror("Error opening fifo\n");
            return -1;
        }

        if((fd_fifo_server_client = open("server_client_pipe", O_WRONLY)) == -1){
            perror("Error opening fifo");
            return -1;
        }


        while((n = read(fd_fifo_client_server,buffer,sizeof(char) * 1024))>0){

            comando = separaString(buffer);
            printf("Teste %s\n", comando[1]);

            //printf("%s-----%s\n", comand[0],buffer);

            if(!strcmp(comando[0], "status")) {
                //print do status
                printf("STAAAAATTTUUUUSSSSS\n");
            } 
            else if(!strcmp(comando[0], "info")) {
                help(fd_fifo_server_client);
            }
            else {
                
                //executar comando
                // char* input;
                // char * output;
                // char ** filter;
                printf("Comando!!\n");
                // executarComando(comand[1]);
            }

            memset(buffer, 0, 1024);
            memset(comand[0], 0, 100);
            memset(comand[1], 0, 100);
        }
    }



    return 0;

}
