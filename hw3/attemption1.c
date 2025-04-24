# include <stdio.h>
# include <stdlib.h>

int       p(long int x); // четно или нет?
void      m(long int* str); // распечатывает массив, пока не встретит 0
long int* f(long int* str); // формирует "задом наперёд" 
                            // массив из элементов str, 
                            // для которых p(str[i]) == 1

long int* adding_loop(long int* str, int len); 
// adding_loop формирует массив в куче, равный массиву data,
// только конечным элементом служит 0

int main(void){
  // так как внутри ассемблерного кода мы передвигаемся по массиву,
  // прибавляя к адресу каждый раз 8, то тип у нас long int (8 байт)
  long int data[6] = {4, 8, 15, 16, 23, 42};
  long int *rax, *res;
  int data_length  =  6;

  rax = adding_loop(data, data_length);
  m(rax);
  res = f(rax);
  m(res);

  free(rax);
  free(res);
  return 0;
}

int p(long int x){
  return x%2;
}
void m(long int* str){
  while(*str)
    printf("%ld ", *str++);
  printf("\n");
  return;
}
long int* f(long int* str){
  int cnt, i;
  long int* ans;

  // считаем кол-во элементов x из str, для которых p(x) == 1
  cnt = 0;
  i = -1;
  while(str[++i])
    cnt += p(str[i]);

  // записываем в ans все элементы x из str, для кот. p(x) == 1
  ans = malloc((cnt + 1)*sizeof(long int));
  ans[cnt] = 0;
  i = -1;
  while(str[++i])
    if(p(str[i]))
      ans[--cnt] = str[i];
  
  return ans;
}

long int* adding_loop(long int* str, int len){
  int i = -1;
  long int* ans = malloc((len + 1)*sizeof(long int));

  while (++i < len)
    ans[i] = str[i];
  ans[i] = 0;
  return ans;
}

