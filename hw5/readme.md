## Домашнее задание №5

- патч - в файле difference.diff
- применение: 

```
patch < difference.diff
```
- если "could not find file to patch" - нужно ввести ./clib/deps/hash/hash.c

- до патча:

![01](./pic1.png)

- изменение - добавление трёх строк (`if(!ret) {free(key); и key= NULL; }` ) в файл clib/deps/hash/hash.c:

![02](./pic2.png)

- применение патча:

![04](./pic5.png)

- после патча:

![03](./pic3.png)
