#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

# define BUFSIZE 10204
# define CUT { exit(EXIT_FAILURE); }


int strings_counter(char* addr, long long int size, int* maxlen);

void map_file(char* path);

void url_ref_traf_init(char* addr
                      , long long int size
                      , char** urls
                      , char** refs
                      , int* bytes
                      , int count
                      , int maxlen);

void sum_url_trafics(char** urls, int* bytes, int count,
                     char** url_summed, int* sum_trafic, int* length);


int
main(int argc, char *argv[]){

  map_file(argv[1]);
  return 0;
}


void map_file(char* path){
  char* addr;
  int fd, max_strlen, str_count;
  struct stat sb;
  char** urls, **refs;
  int* trafics;


  fd = open(path, O_RDWR);
  if (fd == -1){
    printf("opening file error, file: %s\n", path);
    CUT 
  }
  if (stat(path, &sb) == -1){ /* To obtain file size */
    printf("error of initializing struct stat for file %s\n", path);
    CUT
  }

  addr = mmap(NULL, sb.st_size , PROT_READ
                    ,MAP_SHARED , fd, 0);
  if (addr == MAP_FAILED){
    printf("error of mapping memory by mmap for file %s\n", path);
    CUT
  }

  printf("length of file: %lld\n", sb.st_size);

  str_count = strings_counter(addr, sb.st_size, &max_strlen);
  urls = calloc(sizeof(char*), str_count + 1);
  refs = calloc(sizeof(char*), str_count + 1);
  trafics = calloc(sizeof(int), str_count + 1);
  url_ref_traf_init(addr, sb.st_size, urls, refs, trafics, str_count, max_strlen);

  munmap(addr, sb.st_size);
  close(fd);
}

int strings_counter(char* addr, long long int size, int* maxlen){
  int counter, curlen;
  long long int i;
  
  counter = 0;
  *maxlen = 0;
  curlen = 0;
  i = -1;
  while(++i < size){
    if(addr[i] == '\n'){
      ++counter;
      if(*maxlen < curlen)
        *maxlen = curlen;
      curlen = 0;
    } else
      ++curlen;
  }
  
  if(addr[i - 1] != '\n')
    ++counter;
  return counter;
}

void url_ref_traf_init(char* addr
                      , long long int size
                      , char** urls
                      , char** refs
                      , int* bytes
                      , int count
                      , int maxlen){
  char* buff, *end = addr + size - 10;
  char* tmp = malloc(maxlen + 1);
  int i = 0, spaces = 0, offset = 0, w_begin;
  printf("%d srts, maxlen: %d\n", count, maxlen);

  buff = addr;
  i = 0;
  do{
    if(i)
      while(buff[offset++] != '\n')
        {};

    buff += offset;
    
    spaces = 0;
    offset = 0;
    // отсчитываем 6 пробелов
    while (spaces < 6 && buff[offset] != '\n')
      spaces += (buff[offset++] == ' ');
    if(buff[offset] == '\n')
      continue;

    w_begin = offset;
    // доходим до след. пробела
    while (spaces < 7 && buff[offset] != '\n')
      spaces += (buff[offset++] == ' ');
    if(buff[offset] == '\n')
      continue;
    
    // считываем 7-е поле в url
    urls[i] = calloc(sizeof(char), offset - w_begin);
    strncpy(urls[i],&buff[w_begin], offset - w_begin - 1);

    // считываем 9-е поле в bytes
    while (spaces < 9 && buff[offset] != '\n')
      spaces += (buff[offset++] == ' ');
    if(buff[offset] == '\n')
      continue;
    w_begin = offset;
    while (spaces < 10 && buff[offset] != '\n')
      spaces += (buff[offset++] == ' ');
    if(buff[offset] == '\n')
      continue;
    strncpy(tmp, &buff[w_begin], offset - w_begin - 1);
    tmp[offset - w_begin] = '\0';
    bytes[i] = atoi(tmp);

    w_begin = offset + 1;
    while (spaces < 11 && buff[offset] != '\n')
      spaces += (buff[offset++] == ' ');
    if(buff[offset] == '\n')
      continue;
    
    refs[i] = calloc(sizeof(char),  offset - w_begin - 2);
    strncpy(refs[i], &buff[w_begin], offset - w_begin - 2);
    //printf("%d:%s:%d:%s\n",i, refs[i], bytes[i], urls[i]);
  } while (++i < count);
  
 /* if(i < count){
   printf("i: %d, count: %d\n", i, count);
  }
    
  i = 0;
  while(i < count){
    printf("%s\n%d\n%s\n", refs[i], bytes[i], urls[i]);
    i++;
  }
*/
  free(tmp);
  return;
}

int are_equal(char* str1, char* str2){
  int i = 0;
  if(str1 == NULL || str2 == NULL)
    return (str1 == str2);
  while(str1[i] && str2[i])
    if(str1[i] != str2[i])
      return 0;
    else 
      ++i;

  return (str1[i] == str2[i]);
}

void sum_url_trafics(char** urls, int* bytes, int count, 
                     char** url_summed, int* sum_trafic, int* len){
  int next = 0, i = 0, i1;
  while(i < count){
    i1 = 0;
    while(i1 < next){
      if(are_equal(url_summed[i1], urls[i])){
        sum_trafic[i1] += bytes[i];
        break;
      }
      ++i1;
    }
    if(i1 == next){
      sum_trafic[i1] = bytes[i];
      url_summed[i1] = urls[i];
      ++next;
    }
    ++i;
  }
  *len = next;
}

