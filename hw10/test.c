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
#include <limits.h>


# define DEBUG 1 
# define CUT { exit(EXIT_FAILURE); }


int strings_counter(char* addr, long long int size, int* maxlen);

// функция для полной обработки одного файла
char* map_file(char* path);

void url_ref_traf_init(char* addr
                      , long long int size
                      , char** urls
                      , int* url_lens
                      , char** refs
                      , int* ref_lens
                      , int* bytes
                      , int count
                      , int maxlen);

void url_ref_uninit(char** urls, char** refs, int count);

void refs_counter(char** refs, int* ref_lens, int count,
                  char** refs_counted, int* ref_count, int* counted_lens, 
                  int* len);
// возвращает сумму аргументов,
// по которому сортируем
long long int take_first_N(char** fields, int* sort_by,int r_num
                    , char** fieldsN, int* byN, int* N);
void sum_url_trafics(char** urls, int* url_lens,
                     int* bytes, int count,
                     char** url_summed, int* summed_lens,
                     int* sum_trafic, int* length);


int
main(int argc, char *argv[]){

  char* r1 = map_file(argv[1]);
  printf("%s", r1);
  free(r1);
  return 0;
}


char* map_file(char* path){
  char* addr;
  int fd, max_strlen, str_count, i;
  struct stat sb;
  char** urls, **refs;
  char **r_counted, **u_summed;
  int *r_count, r_num, *t_sums, u_num;
  int *ref_lens, *url_lens, *counted_lens;
  int* trafics;
  char *refs10[10], *urls10[10];
  int r_count10[10], u_trafic10[10], sum_trafic;
  long int all_bytes, all_refs;
  int N;
  char *result;
  int offset = 0;

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

  str_count = strings_counter(addr, sb.st_size, &max_strlen);
  result = calloc(sizeof(char), max_strlen*20 + 20000);
  offset += sprintf(result + offset, "results for file: %s\n", path);
  urls = calloc(sizeof(char*), str_count + 1);
  refs = calloc(sizeof(char*), str_count + 1);
  ref_lens = calloc(sizeof(int), str_count + 1);
  counted_lens = calloc(sizeof(int), str_count + 1);
  url_lens = calloc(sizeof(int), str_count + 1);
  trafics = calloc(sizeof(int), str_count + 1);
  url_ref_traf_init(addr, sb.st_size, urls, url_lens,
                                      refs, ref_lens, 
                                      trafics, str_count, max_strlen);
  r_counted = calloc(sizeof(char*), str_count + 1);
  r_count = calloc(sizeof(int), str_count + 1); 
  refs_counter(refs, ref_lens, str_count
              ,r_counted, counted_lens, r_count, &r_num);
  N = 10;
  all_refs = take_first_N(r_counted, r_count, r_num
                 , refs10, r_count10, &N);
#if(DEBUG)
  i = -1;
  offset += 
        sprintf(result + offset,
                "count of all references:%lld\n",all_refs);
  if(N < 10)
   offset +=
        sprintf(result + offset, 
                "count of different references is only %d\n", N);  
  offset += 
        sprintf(result + offset, "%d mostly frequent refs:\n", N);
  while(++i < N){
    offset += sprintf(result + offset, "№%d:\n   reference:%s"
               "\n       count:%d\n", i + 1, refs10[i], r_count10[i]);
  }
#endif
  u_summed = calloc(sizeof(char*), str_count + 1);
  t_sums = calloc(sizeof(int), str_count + 1); 
  sum_url_trafics(urls, url_lens,
                  trafics , str_count,
                  u_summed, counted_lens,
                  t_sums, &u_num);
  N = 10;
  all_bytes = take_first_N(u_summed, t_sums, u_num
                 , urls10, u_trafic10, &N);
  
#if(DEBUG)
  i = -1;
  offset += sprintf(result + offset, 
                    "all sended traffic:%lld bytes\n",all_bytes);
  if(N < 10)
    offset += 
          sprintf(result + offset, 
                  "count of different urls is only %d\n", N);  
  offset += 
          sprintf(result + offset,
                  "%d urls with maximal traffices:\n", N);
  while(++i < N){
    offset += 
          sprintf(result + offset,
                  "№%d:\n      url:%s,"
                      "\n  traffic:%d\n", i+1, urls10[i], u_trafic10[i]);
  }
#endif
  
  url_ref_uninit(urls, refs, str_count);
  free(urls);
  free(refs);
  free(ref_lens);
  free(counted_lens);
  free(url_lens);
  free(trafics);
  free(r_counted);
  free(r_count);
  munmap(addr, sb.st_size);
  close(fd);
  return result;
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
                      , int* url_lens
                      , char** refs
                      , int* ref_lens
                      , int* bytes
                      , int count
                      , int maxlen){
  char* buff, *end = addr + size - 10;
  char* tmp = malloc(maxlen + 1);
  int i, spaces, offset, w_begin;
  //printf("%d srts, maxlen: %d\n", count, maxlen);

  buff = addr;
  offset = -1;
  i = 0;
  do{  // buff[offset] = '\n'
    buff = buff + (offset + 1);

    //buff = начало очередной строчки
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
    url_lens[i] = offset - w_begin;

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
    ref_lens[i] = offset - w_begin - 2;
    if(i + 1 != count)
      while(buff[offset++] != '\n');
  } while (++i < count);
  
  free(tmp);
  return;
}

int are_equal(char* str1, char* str2){
  int i = 0;
  if(str1 == NULL || str2 == NULL)
    return (str1 == str2);
  while((str1[i] != '\0')  && (str2[i] != '\0'))
    if(str1[i] != str2[i])
      return 0;
    else 
      ++i;

  return (str1[i] == str2[i]);
}

void refs_counter(char** refs, int* ref_lens,  int count, 
                  char** refs_counted, int* counted_lens, 
                  int* ref_count, int* len) {

  int i = 0, ref_i = 0, numref_i = 0;
  while(ref_i < count){  
                         // ищем refs[ref_i] в массиве 
                         // refs_counted[0]..[numref_i - 1]
    i = 0;
    while(i < numref_i){
      if( ref_lens[ref_i] == counted_lens[i] 
         && are_equal(refs_counted[i], refs[ref_i])){
        ++ref_count[i];
        break;
      }
      ++i;
    }
    if(i == numref_i) {  // если не нашли
      refs_counted[numref_i] = refs[ref_i];
      counted_lens[numref_i] = ref_lens[ref_i]; 
      ref_count[numref_i] = 1;
      ++numref_i;
    }
    ++ref_i;
  }
  *len = numref_i;
}


void sum_url_trafics(char** urls, int* url_lens,
                     int* bytes, int count,
                     char** url_summed, int* summed_lens,
                     int* sum_trafic, int* len){
  int i = 0, url_i = 0, sum_i = 0;
  //printf("%d strs\n", count);
  while(url_i < count){
    i = 0;
    while(i < sum_i){
      if(url_lens[url_i] == summed_lens[i] 
         && are_equal(url_summed[i], urls[url_i])){
        sum_trafic[i] += bytes[url_i];
        break;
      }
      ++i;
    }
    if(i == sum_i) {
      url_summed[sum_i] = urls[url_i];
      summed_lens[sum_i] = url_lens[url_i]; 
      sum_trafic[sum_i] = bytes[url_i];
      ++sum_i;
    }
    ++url_i;
  //  printf("%d ",url_i);
  }
  *len = sum_i;
                     
  //i = -1;
  //while(++i < *len)
  //  printf("%d, url:%s\ncount:%d\n",i, url_summed[i], sum_trafic[i]);
}

// byN - отсортирован по убыванию
void insert_into(char* field, int by,
                 char** fieldsN, int* byN, int N){
  int i = N, j = N;
  if(by <= byN[N - 1])
    return;

  while(--i)
    if(by <= byN[i - 1])
      break;
  
  // на i-е место вставляем
  // с N-1 до i+1 сдвигаем
  while(--j > i){
    byN[j] = byN[j - 1];
    fieldsN[j] = fieldsN[j - 1];
  }

  fieldsN[i] = field;
  byN[i] = by;
}


long long int take_first_N(char** fields, int* sort_by, int r_num
                         , char** fieldsN, int* byN, int* N){
  long long int sum = 0;
  int i = -1;
  while(++i < *N){
    fieldsN[i] = NULL;
    byN[i] = INT_MIN;
  }
  i = -1;

  while(++i < r_num){
    sum += (long long int)sort_by[i];
    insert_into(fields[i], sort_by[i],
                fieldsN, byN, *N);
  }
  if(i < *N)
    *N = i;
  return sum;
}


void url_ref_uninit(char** urls, char** refs, int count){
  int i = -1;
  while(++i < count){
    if(urls[i])
      free(urls[i]);
    if(refs[i])
      free(refs[i]);
  }
}
