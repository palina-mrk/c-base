# include <stdio.h>
# include "logger.c"
# include "tracing.c"

# define FALSE 0
# define TRUE 1
static char log_file_name[kMaxFileNameLen + 1];

void start_logging(const char* filename){
  int i = 0;
  logger_initFileLogger(filename, 0, 1);
  do {
    log_file_name[i] = filename[i];
  } while(log_file_name[i++]);
}
int are_equal(char* str1, char* str2){
  while (*str1 && *str2){
    if(*str1 != *str2)
      return FALSE;
    ++str1;
    ++str2;
  }
  if (*str1 || *str2)
    return FALSE;
  return TRUE;
}

void print_message(char* level, char* message, int line){
  LogLevel llevel;
  char* msg = NULL;
  
  if(are_equal(level,"error")){
    llevel = LogLevel_ERROR;
    msg = myfunc3(message);
  } else {
    msg = message;
    if(are_equal(level,"warning"))
      llevel = LogLevel_WARN;
    else if(are_equal(level,"info"))
      llevel = LogLevel_INFO;
    else if(are_equal(level,"debug"))
      llevel = LogLevel_DEBUG;
    else 
      llevel = LogLevel_TRACE;
  }

  logger_setLevel(llevel);
  logger_log(llevel, log_file_name, line, msg);
  if(msg != message)
    free(msg);
}
