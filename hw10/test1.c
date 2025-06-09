#define _GNU_SOURCE
           #define _FILE_OFFSET_BITS 64
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
#include <dirent.h>
#include <pthread.h>


int strings_counter(char* addr, long long int size, int* maxlen);

// функция для полной обработки одного файла
char* map_file(int fd);

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

void* fcn(void* arg);

int
main(int argc, char *argv[]){
  pthread_t *p_arr;
  void* tmp;
  char /****p_arg,*/ ***p_ans;
  char **f_names, **f_ans;
  int **p_arg;
  struct dirent **all_names; 
  int i, j, k;
  int N; // number of threads
  int m; // number of files
  FILE *fp;
  int dirfd;
  if(argc != 3){
    printf("using: %s <directory> <number of threads>\n",argv[0]);
    exit(1);
  }

  N = atoi(argv[2]);
  m = scandir(argv[1], &all_names, 0, 0);
  if(N <= 0){
    printf("error of reading number of threads:\n"
           "N = %d, but it must be int > 0\n", N);
    exit(1);
  }
  if(m < 0){
    printf("error of reading the directory %s\n", argv[1]);
    exit(1);
  }
  f_names = calloc(sizeof(char*), m);
  if(!f_names){
    printf("memory allocation error\n");
    exit(1);
  }
  j = -1;
  i = -1;
  while(++i < m){
    if(all_names[i]->d_type != DT_REG){
      free(all_names[i]);
      continue;
    }
    ++j;
    f_names[j] = calloc(sizeof(char), strlen(all_names[i]->d_name) + 2);
    strcpy(f_names[j], all_names[i]->d_name);
    free(all_names[i]);
  }
  free(all_names);
  m = j+1;
  if(m <= 0){
    printf("error: no regular files in the directory %s\n", argv[1]);
    free(f_names);
    exit(1);
  }
  if(N > m)
    N = m;
  p_arr = calloc(sizeof(pthread_t), N);
  p_arg = calloc(sizeof(int*), N);
  p_ans = calloc(sizeof(char**), N);
  f_ans = calloc(sizeof(char*), m);
  if(!p_arr || !p_arg || !p_ans || !f_ans){
    printf("memory allocation error\n");
    exit(1);
  }

  dirfd = open(argv[1], O_PATH);
  if(!dirfd){
    printf("error of opening the directory %s\n", argv[1]);
    exit(1);
  }
  i = -1;
  while(++i < N){
    p_arg[i] = calloc(sizeof(int), (m - i - 1)/N + 2);
    if(!p_arg[i]){
      printf("memory allocation error\n");
      exit(1);
    }
    k = i;
    while(k < m){
      p_arg[i][k/N] = openat(dirfd, f_names[k], O_RDONLY | O_LARGEFILE);
      if(p_arg[i][k/N] < 0){
        printf("file opening error, function: open(), file: %s\n", f_names[k]);
        exit(1);
      }
      k += N;
    }
    p_arg[i][k/N] = -1;
  }
 
  i = -1;
  while(++i < N){
    k = pthread_create(&p_arr[i], NULL, fcn, (void*)&p_arg[i]); //char**
    if(k){
      printf("error in pthread_create()\n");
      exit(1);
    }
  }

  i = -1;
  while(++i < N){
    pthread_join(p_arr[i], (void**)&p_ans[i]);
    k = -1;
    while(p_ans[i][++k]){
      f_ans[k*N + i] = p_ans[i][k];
      close(p_arg[i][k/N]);
    }
  }
  close(dirfd);

  fp = fopen("reply.txt","w");
  if(!fp){
    printf("error: opening file to write the response is failed\n");
    exit(1);
  }

  i = -1;
  while(++i < m){
    fprintf(fp,"****************response for file %s\n",f_names[i]);
    fprintf(fp,"%s", f_ans[i]);
  }

  i = -1;
  while(++i < m){
    free(f_names[i]);
    free(f_ans[i]);
  }
  i = -1;
  while(++i < N){
    free(p_arg[i]);
    free(p_ans[i]);
  }
  free(p_arr);
  free(p_arg);
  free(p_ans);
  free(f_ans);
  free(f_names);
  fclose(fp);
  return 0;
}

void* fcn (void* arg){
  int N, i;
  char **ans;
  N = 0;
  if (arg && ((int**)arg)[0])
    while(((int**)arg)[0][N] >= 0)
      ++N; 

  if(!N)
    pthread_exit((void*)NULL);

  ans = calloc(sizeof(char*), N + 1);
  if(!ans){
    printf("memory allocation error\n");
    pthread_exit(NULL);
  }

  i = -1;
  while(++i < N)
    ans[i] = map_file(((int**)arg)[0][i]);

  ans[N] = NULL;

  pthread_exit( (void*)ans);
 // return ans;
}


char* map_file(int fd){
  char* addr;
  int  max_strlen, str_count, i;
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
  int offset = 0;
  char *err_str, *result;

  err_str = calloc(sizeof(char), 1000);
  if(!err_str){
    printf("memory allocation error\n");
    pthread_exit(NULL);
  }/*
  fd = open(path, O_RDONLY);
  if (fd == -1){
    sprintf(err_str,"opening file error, file: %s\n", path);
    return err_str;
  }*/
  if (fstat(fd, &sb) == -1){ /* To obtain file size */
    sprintf(err_str,"error of initializing file struct\n");
    return err_str;
  }

  addr = mmap(NULL, sb.st_size , PROT_READ
                    ,MAP_SHARED , fd, 0);
  if (addr == MAP_FAILED){
    sprintf(err_str,"error of mapping memory by mmap\n");
    return err_str;
  }

  str_count = strings_counter(addr, sb.st_size, &max_strlen);
  urls = calloc(sizeof(char*), str_count + 1);
  refs = calloc(sizeof(char*), str_count + 1);
  ref_lens = calloc(sizeof(int), str_count + 1);
  counted_lens = calloc(sizeof(int), str_count + 1);
  url_lens = calloc(sizeof(int), str_count + 1);
  trafics = calloc(sizeof(int), str_count + 1);
  r_counted = calloc(sizeof(char*), str_count + 1);
  r_count = calloc(sizeof(int), str_count + 1); 
  u_summed = calloc(sizeof(char*), str_count + 1);
  t_sums = calloc(sizeof(int), str_count + 1); 
  result = calloc(sizeof(char), 20*max_strlen + 500);
  if(!result || !urls || !refs || !ref_lens 
      || !counted_lens || !url_lens || !trafics
      || !r_counted || !r_count){
    free(urls);
    free(refs);
    free(ref_lens);
    free(counted_lens);
    free(url_lens);
    free(trafics);
    free(r_counted);
    free(r_count);
    free(result);
    munmap(addr, sb.st_size);
    close(fd);
    sprintf(err_str,"error of memory allocation");
    return err_str;
  }

  url_ref_traf_init(addr, sb.st_size, urls, url_lens,
                                      refs, ref_lens, 
                                      trafics, str_count, max_strlen);
  refs_counter(refs, ref_lens, str_count
              ,r_counted, counted_lens, r_count, &r_num);
  N = 10;
  all_refs = take_first_N(r_counted, r_count, r_num
                 , refs10, r_count10, &N);

/*  offset += sprintf(result + offset, "**********response for file %s:\n", path);*/  
  offset += sprintf(result + offset,
                    "sum of all reference frequences: %lld\n",all_refs);
  if(N < 10)
    offset += sprintf(result + offset,
                    "number of all references less than 10,\n");

  offset += sprintf( result + offset,
                    "%d mostly frequently occuring references:\n", N);
  i = -1;
  while(++i < N)
    offset += sprintf(result + offset, 
                      "            №%d:"
                    "\n   reference:%s,"
                    "\n      occurs %d times\n", i, refs10[i], r_count10[i]);

  sum_url_trafics(urls, url_lens,
                  trafics , str_count,
                  u_summed, counted_lens,
                  t_sums, &u_num);
  N = 10;
  all_bytes = take_first_N(u_summed, t_sums, u_num
                          , urls10, u_trafic10, &N);
  
  i = -1;
  offset += sprintf(result + offset, 
                    "sum of all sended traffic:%lld\n",all_bytes);
  if(N < 10)
    offset += sprintf(result + offset,
                    "number of all urls is less than 10,\n");

  offset += sprintf( result + offset,
                    "%d urls with the largest traffic:\n", N);
  i = -1;
  while(++i < N)
    offset += sprintf(result + offset, 
                      "            №%d:"
                    "\n            url:%s,"
                    "\n     was sended %d bytes\n"
                     , i, urls10[i], u_trafic10[i]);
  
  
  url_ref_uninit(urls, refs, str_count);
  free(urls);
  free(refs);
  free(ref_lens);
  free(counted_lens);
  free(url_lens);
  free(trafics);
  free(r_counted);
  free(r_count);
  free(err_str);
  munmap(addr, sb.st_size);
  //close(fd);
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
  if(!tmp){
    printf("memory allocation error\n");
    pthread_exit(NULL);
  }
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
 // printf("sum: %ld, r_num:%d\n", sum, r_num);

  while(++i < r_num){
    sum += (long long int)sort_by[i];
    insert_into(fields[i], sort_by[i],
                fieldsN, byN, *N);
  }
  if(i < *N)
    *N = i;
 /* printf("sum: %ld\n", sum);
  i = -1;
  while(++i < N)
    printf("%d: field:%s, by:%d\n", i, fields[i], byN[i]);
 */
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
