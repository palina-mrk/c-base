# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <syslog.h>
# include <fcntl.h>
# include <unistd.h>
# include <sys/resource.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <string.h>
# include <sys/un.h>
# include <sys/socket.h>
# include <sys/wait.h>

# define CONFIG "config.txt"
# define BUFFER 1024

void getfullpath(char* str, int maxlen);
void readnames(char* fname, char* sname, int maxlen);
void daemonize(void);
int create_socket(char* sockname, struct sockaddr_un* server);
void listen_socket(char* filename, int sock, FILE* f_log);


int main(int argc, char* argv[]){
  char filename[BUFFER];
  char sockname[BUFFER];
  char logname[BUFFER];
  int sock, len;
  struct sockaddr_un server;
  FILE* f_log = NULL;
  // получаем полный путь к текущей директории
  getfullpath(logname, BUFFER - 10);
  strcpy(filename, logname);
  strcpy(sockname, logname);
  // создаём логфайл
  len = strlen(logname);
  strcpy(logname + len, "socket.log");
  // читаем конфиг
  readnames(filename + len, sockname + len, BUFFER - len);

  // создаём сокет
  sock = create_socket(sockname, &server);

  if(argc == 1){
    daemonize();
    if(!(f_log = fopen(logname, "r")))
      perror("logfile was not opened; errors will not printed");
  }

  listen_socket(filename, sock, f_log);
  close(sock);
  unlink(sockname);
  if(f_log)
    fclose(f_log);
  return 0;
}

void daemonize(void){
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        perror("Ошибка fork()!\n");
        exit(1);
    } else if (pid > 0) {
        exit(0);
    } else if (setsid() < 0) {
        exit(1);
    }
    chdir("/");
    umask(0);
    close(0);
    close(1);
    close(2);
}


int create_socket(char* sockname, struct sockaddr_un* server){
  int sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0){
    perror("opening stream socket");
    exit (1);
  }
  server->sun_family = AF_UNIX;
  strcpy(server->sun_path, sockname);
  remove(sockname);
  if(bind(sock, (struct sockaddr*)server, sizeof(struct sockaddr_un))){
    perror("binding stream socket");
    exit(1);
  }
  return sock;
}

void listen_socket(char* filename, int sock, FILE* f_log){
  int msgsock, status;
  struct sockaddr_un server;
  struct stat st;
  char msg[BUFFER];
  pid_t pid;

  // слушаем сокет
  listen(sock, 5);
  for(;;){
    pid = fork();
    if (pid < 0) {
        f_log ? fputs("error in fork()!\n",f_log) 
              : perror("error in fork()!");
        exit(1);
    } else if (pid > 0) {
        wait(&status);
        continue;
    } else {
      msgsock = accept(sock, 0, 0);
      if(msgsock == -1){
        f_log ? fputs("error: accept\n",f_log) 
              :  perror("accept");
        exit(1);
      } else if (!stat(filename, &st)){
        bzero(msg, BUFFER); 
        sprintf(msg, "size of file %s is: %d bytes\n"
                      , filename, st.st_size);
        send(msgsock, msg, strlen(msg),0 );
        close(msgsock);
        exit(0);
      } else {
        sprintf(msg, "the file path invalid or file %s does not exist!" 
                      , filename);
        send(msgsock, msg, strlen(msg),0 );
        close(msgsock);
        exit(1);
      }
    }
  }
  close(sock);
}

void readnames(char* fname, char* sname, int maxlen){
  FILE* fp = fopen(CONFIG, "r");
  int i, d; 
  char c;
  if(fp == NULL){
    perror("Error: config file could'n be opened");
    exit(1);
  }
  i = 0;
  while((d = getc(fp)) != EOF){
    if(d == '\n' || i == maxlen)
      break;
    sname[i++] = (char)d;
  }
  if(d == EOF || i == maxlen || i == 0){
    perror("Error: invalid config file");
    exit(1);
  }
  sname[i] = '\0';
  i = 0;
  while((d = getc(fp)) != EOF){
    if(d == '\n' || i == maxlen)
      break;
    fname[i++] = (char)d;
  }
  if(i == maxlen || i == 0){
    perror("Error: invalid config file");
    exit(1);
  }
  fname[i] = '\0';

  fclose(fp);
}

void getfullpath(char* str, int maxlen){
  int len;
  if(!getcwd(str, maxlen)){
    perror("Error: path to current directory wasn't read");
    exit(1);
  }
  len = strlen(str);
  str[len] = str[0]; // копируем обратный слэш в конец строки)))
  str[++len] = '\0';
}
