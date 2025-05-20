# include <stdio.h>
# include "work_with_logs.c"

int main(void){
  char filename[] = "log1.txt";
  start_logging(filename);
  print_message("debug", "first log", __LINE__);
  print_message("error", "second log", __LINE__);
  return 0;
}
