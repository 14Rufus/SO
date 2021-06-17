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
#include <signal.h>

typedef struct listaT {
  int pid;
  int pidCliente;
  int numeroTarefa;
  char* tarefa;
  struct listaT *prox;
} *ListaT;

typedef struct listaF {
  int max;
  int curr;
  char* filtro;
  char* executavel;
  int *pids;
  struct listaF *prox;
} *ListaF;

ListaF listaFiltros = NULL;
ListaT pendentes = NULL;
ListaT execucao = NULL;
int numeroTarefa = 1;
int comandoSize = 0;
int fd_fifo_server_client;
int fd_fifo_client_server;


char* itoa(int i){
    char const digit[] = "0123456789";
    int p = 0;
    char *b = malloc(sizeof(char) * 10);
    int shifter = i;

    do{
        p++;
        shifter = shifter/10;
    }while(shifter);

    b[p] = '\0';

    do{
        p--;
        b[p] = digit[i%10];
        i = i/10;
    }while(i);

    return b;
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
    comandoSize = nrComandos;
    return comando;
}

void help(int fdwr) {
    char help[] = "./aurras status\n./aurras transform input-filename output-filename filter-id-1 filter-id-2 ...\n";
    write(fdwr, help, sizeof(help));
}

ListaT adicionaTarefa(int pid, int pidC, int numeroTarefa, char* tarefa,ListaT l) {
  ListaT aux = l;
  ListaT new = malloc(sizeof(struct listaT));
  new->pid = pid;
  new->pidCliente = pidC;
  new->numeroTarefa = numeroTarefa;
  new->tarefa = malloc(sizeof(char) * strlen(tarefa));
  strcpy(new->tarefa,tarefa);
  new->prox = NULL;

  if (!l) {
    return new;
  }

  for (; aux->prox; aux = aux->prox);

  aux->prox = new;

  return l;
}

ListaF adicionaFiltro(int max, char* filtro, char* executavel,ListaF l) {
  ListaF aux = l;
  ListaF new = malloc(sizeof(struct listaF));
  new->max = max;
  new->curr = 0;
  new->filtro = malloc(sizeof(char) * strlen(filtro));
  new->executavel = malloc(sizeof(char) * strlen(executavel));
  new->pids = malloc(sizeof(int)*max);
  strcpy(new->filtro,filtro);
  strcpy(new->executavel,executavel);
  new->prox = NULL;

  if (!l) {
    return new;
  }

  for (; aux->prox; aux = aux->prox);

  aux->prox = new;

  return l;
}

ListaT removeTarefa(int pid, ListaT l) {
  if (!l) 
    return NULL;

  ListaT aux = l;
  if(aux->pid == pid){
    l = l->prox;
    free(aux);
  } else {
    ListaT ant = l;
    aux = aux->prox;

    if(!aux)
      return l;

    for (; aux->prox && aux->pid != pid; aux = aux->prox) ant = ant->prox;

    if (aux->prox) {
      ant->prox = aux->prox;
      aux->prox = NULL;
      free(aux);
    }
    else {
      if (pid) {
        ant->prox = NULL;
        free(aux);
      }
    }
  }

  return l;
}

int getClientefromPid(int pid, ListaT l) {
  if(!l)
    return -1;

  ListaT aux = l;
  for(; aux; aux = aux->prox) {

    if (aux->pid == pid) {
      return aux->pidCliente;
    }
  }

  return -1;
}

char* getExecfromFiltro(char* filtro, ListaF l) {
  if(!l)
    return NULL;

  ListaF aux = l;
  for(; aux; aux = aux->prox) {

    if (!strcmp(aux->filtro,filtro)) {
      return aux->executavel;
    }
  }

  return NULL;
}

void printLista(ListaT l, int fd) {
  if(!l)
    write(fd, "Não há tarefas em execução\n", 31);

  else{
    ListaT aux = l;
    int primeiro = 0;
    char buffer[1024];
    for (; aux; aux = aux->prox) {
      if (primeiro == 0) {
        strcpy(buffer,"task #");
        primeiro++;

      } else
        strcat(buffer,"task #");

      strcat(buffer,itoa(aux->numeroTarefa));
      strcat(buffer,": ");
      strcat(buffer,aux->tarefa);
      strcat(buffer, "\n");
    }
    write(fd, buffer, strlen(buffer));
  }
}

void printListaFiltros(ListaF l, int fd) {
  if(!l)
    write(fd, "Não há filtros\n", 28);

  else{
    ListaF aux = l;
    int primeiro = 0;
    char buffer[1024];
    for (; aux; aux = aux->prox) {
      if (primeiro == 0) {
        strcpy(buffer,"filter ");
        primeiro++;

      } else
        strcat(buffer,"filter ");

      strcat(buffer,aux->filtro);
      strcat(buffer,": ");
      strcat(buffer,itoa(aux->curr));
      strcat(buffer,"/");
      strcat(buffer,itoa(aux->max));
      strcat(buffer, " (running/max)\n");
    }
    write(fd, buffer, strlen(buffer));
  }
}

int filtroDisponivel(ListaF l, char* filtro) {
    ListaF aux = l;
    int res = 0;
    for (; aux && strcmp(aux->filtro,filtro); aux = aux->prox);

    if(aux && (aux->curr == aux->max))
      res++;

    return res;
}

void filtroInc(ListaF l, char* filtro,int pid) {
    ListaF aux = l;
    for (; aux && strcmp(aux->filtro,filtro); aux = aux->prox);

    if(aux && (aux->curr < aux->max)) {
      aux->pids[aux->curr] = pid;
      aux->curr++;
    }
}

void filtroDec(ListaF l, int pid) {
    ListaF aux = l;
    for (; aux; aux = aux->prox)  {
        for(int i = 0; i < aux-> curr;i++)
            if(aux->pids[i] == pid)
                aux->curr--;
    }
}

ssize_t readln (int fd, char *buffer, size_t size) {

    int resultado = 0, i = 0;

    ssize_t read_bytes = 0;

    while ((resultado = read (fd, &buffer[i], 1)) > 0 && i < size) {
        if (buffer[i] == '\n') {
            i += resultado;
            return i;
        }

        i += resultado;
    }

    return i;
}

void lerConfig(char* file) {
  char *buffer = malloc(1024 * sizeof(char));
  char *filtro;
  char *exec;
  char *max;
  int fd_config = open(file, O_RDONLY);
  int i = 0,n,o = 0,i2 = 0;
  char* token;


  while ((n = readln(fd_config,buffer,1024 * sizeof(char))) > 0) {
    token = strtok(buffer," ");
    filtro = strdup(token);
    token = strtok(NULL," ");
    exec = strdup(token);
    token = strtok(NULL," ");
    max = strdup(token);

//   printf("%s %s %s\n", filtro,exec,max);
  listaFiltros = adicionaFiltro(atoi(max),filtro,exec,listaFiltros);
  }
}

void sigChild_handler(int signum) {
  int r;
  int pid = wait(&r);
  r = WEXITSTATUS(r);
  int p = getClientefromPid(pid,execucao);

  if(p > 0)
    kill(p,SIGINT);

  if (pid > 0) 
    filtroDec(listaFiltros,pid);

  //VERIFICAR E EXECUTAR TAREFAS PENDENTES----------------------
}

int executar(char* input,char* output,char** filtros,int numFiltros) {
//   printf("%s %s %d\n", input,output,numFiltros);
  for (int i = 0; i < numFiltros; ++i)
  {
    // printf("%s\n", filtros[i]);
  }

  int fd_output = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0600);
  int fd_input = open(input, O_RDONLY);
  char* path = malloc(1024 * sizeof(char));
  strcpy(path,"aurrasd-filters/");

//   printf("%d %d\n", fd_output, fd_input);

    int n = numFiltros;                         // número de comandos == número de filhos
    int p[n-1][2];                              // -> matriz com os fd's dos pipes
    int status[n];                              // -> array que guarda o return dos filhos


    if(numFiltros == 1) {
      filtros[0] = strcat(path,filtros[0]);
      dup2(fd_input,0);
      close(fd_input);
      dup2(fd_output,1);
      close(fd_output);

      execlp(filtros[0],filtros[0],NULL);

      return 0;
    }


    // criar os pipes conforme o número de comandos
    for (int i = 0; i < n-1; i++) {
        if (pipe(p[i]) == -1) {
            perror("Pipe não foi criado");
            return -1;
        }
    }

    // criar processos filhos para executar cada um dos comandos
    for (int i = 0; i < n; i++) {
      filtros[i] = strcat(path,filtros[i]);

        if (i == 0) {
            switch(fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho 0

                    dup2(fd_input,0);
                    close(p[i][0]);

                    dup2(p[i][1],1);
                    close(p[i][1]);

                    execlp(filtros[i],filtros[i],NULL);

                    _exit(0);
                default:
                    close(p[i][1]);
            }
        }
        else if (i == n-1) {
            switch(fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho n-1

                    dup2(p[i-1][0],0);
                    close(p[i-1][0]);

                    dup2(fd_output,1);
                    close(p[i][1]);

                    execlp(filtros[i],filtros[i],NULL);

                    _exit(0);
                default:
                    close(p[i-1][0]);
            }
        }
        else {
            switch(fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho i

                    dup2(p[i-1][0],0);
                    close(p[i-1][0]);

                    dup2(p[i][1],1);
                    close(p[i][1]);


                    execlp(filtros[i],filtros[i],NULL);

                    _exit(0);
                default:
                    close(p[i-1][0]);
                    close(p[i][1]);
            }
        }
        // printf("%s", filtros[i]);
        memset(path, 0, 30);
        strcpy(path,"aurrasd-filters/"); // /bin
    }

    for (int i = 0; i < n; i++)
    {
        wait(&status[i]);

        if (WIFEXITED(status[i])) {
            // printf("[PAI]: filho terminou com %d\n", WEXITSTATUS(status[i]));
        }
    }
    return 1;
}

int main (int argc, char** argv){
    int fd_fifo_client_server;
    int fd_fifo_server_client,n,pid;
    char *buffer = malloc(1024 * sizeof(char));
    //char comand[2][100];
    char **comando;

    mkfifo("client_server_pipe",0666);
    mkfifo("server_client_pipe",0666);

    lerConfig(argv[1]);

    signal(SIGCHLD,sigChild_handler);


    while(1) {
        while ((fd_fifo_client_server = open("client_server_pipe", O_RDONLY)) == -1) {
            perror("Error opening fifo1\n");
        }

        n = read(fd_fifo_client_server,buffer,sizeof(char) * 1024);
        close(fd_fifo_client_server);

            comandoSize = 0;
            comando = separaString(buffer);

            if(!strcmp(comando[0], "status")) {
                fd_fifo_server_client = open("server_client_pipe", O_WRONLY);
                printLista(pendentes,fd_fifo_server_client);
                printListaFiltros(listaFiltros,fd_fifo_server_client);
                close(fd_fifo_server_client);
            } 
            else if(!strcmp(comando[0], "info")) {
                fd_fifo_server_client = open("server_client_pipe", O_WRONLY);
                help(fd_fifo_server_client);
                close(fd_fifo_server_client);

            }
            else if(!strcmp(comando[0], "transform") && comandoSize > 4 && comandoSize < 10) {
                write(fd_fifo_server_client, "1", sizeof(char));
                int res=0;
                for(int t = 3;t < comandoSize; t++){
                    res += filtroDisponivel(listaFiltros,comando[t]);
                }

                char**filtros = NULL;
                char * input = comando[1];
                char * output = comando[2];


                char* tarefa = malloc(1024 * sizeof(char));
                strcpy(tarefa,comando[0]);
                int i = 1;
                while(i < comandoSize - 1) {
                    if(i > 2) {
                      filtros = (char**) realloc(filtros, (i + 1) * sizeof(char*));
                      filtros[i - 3] = strdup(getExecfromFiltro(comando[i],listaFiltros));
                    }

                    strcat(tarefa," ");
                    strcat(tarefa,comando[i]);
                    i++;
                }


                if(res != 0) {
                    kill(atoi(comando[comandoSize - 1]),SIGUSR2);
                    pendentes = adicionaTarefa(pid,atoi(comando[comandoSize - 1]),numeroTarefa,tarefa,pendentes);
                }
                else {

                //executar comando
                    if(!(pid = fork())){
                        kill(atoi(comando[comandoSize - 1]),SIGUSR1);
                        //EXECUTAR TAREFA
                        executar(input,output,filtros,comandoSize - 4);
                        //sleep(3);
                        _exit(0);
                    } else {
                        execucao = adicionaTarefa(pid,atoi(comando[comandoSize - 1]),numeroTarefa,tarefa,execucao);
                        numeroTarefa++;

                        for(int t = 3;t < comandoSize; t++)
                            filtroInc(listaFiltros,comando[t],pid);
                    }
                }
            }
            else {
                printf("Comando invalido\n");
            }

            memset(buffer, 0, 1024);
            int r = 0;
            while(r < comandoSize) {
                memset(comando[r], 0, sizeof(comando[r]));
                r++;
            }
    }



    return 0;

}
