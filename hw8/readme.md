## Домашнее задание №8

- файл конфигурации: config.txt, содержит имя сокета, который будет создан в текущей директории, и имя файла, размер которого нужно выводить
- код - целиком в файле main.c
- следим за файлом file.txt, создаём сокет size.socket
- если сокет существует, он будет удалён и создан заново
- запуск ./a.out с любыми аргументами - без демонизации, без аргументов - с демонизацией. 

- компиляция и запуск:

```
gcc main.c
./a.out    # с демонизацией
./a.out j  # без демонизации
```

- проверка (если режим - без демонизации, то проверку нужно проводить из другого терминала):

```
ncat -i 1 -U size.socket
```



