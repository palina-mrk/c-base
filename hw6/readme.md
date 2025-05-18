## Домашнее задание №6

- исходный код в файле test.c
- компиляция:

```
gcc $( pkg-config --cflags gtk4 ) -o test test.c $( pkg-config --libs gtk4 ) -g
```

- результат работы:

![01](./pic0.png)
![02](./pic1.png)
