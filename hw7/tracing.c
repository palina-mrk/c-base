 #include <execinfo.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <strings.h>
 #include <unistd.h>

 #define BT_BUF_SIZE 100

 char*
 myfunc3(const char* message)
 {
  int nptrs;
  void *buffer[BT_BUF_SIZE];
  char **strings;
  char* msg;
  int initlen = strlen(message);
  int msglen = initlen;

  nptrs = backtrace(buffer, BT_BUF_SIZE);

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL) {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }

  for (size_t j = 0; j < nptrs; j++)
    msglen += strlen(strings[j]);

  msg = calloc(sizeof(char), msglen + 1);
  strcpy(msg, message);

  for (size_t j = 0; j < nptrs; j++){
    strcpy(msg + initlen,strings[j]);
    initlen += strlen(strings[j]);
  }
  msg[initlen] = '\0';

  free(strings);
  return msg;
 }
/*
 static void  "static" means don't export the symbol... 
 myfunc2(void)
 {
 myfunc3();
 }

 void
 myfunc(int ncalls)
 {
 if (ncalls > 1)
 myfunc(ncalls - 1);
 else
 myfunc2();
 }

 int
 main(int argc, char *argv[])
 {

 if (argc != 2) {
 fprintf(stderr, "%s num-calls\n", argv[0]);
 exit(EXIT_FAILURE);
 }

 myfunc(atoi(argv[1]));
 exit(EXIT_SUCCESS);
 }
*/
