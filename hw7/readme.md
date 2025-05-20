## Домашнее задание №7

- интерфейс библиотеки логирования - в файле work_with_logs.c, использует функции из файлов logger.h, logger.c и tracing.c
- интерфейс: 

```
void start_logging(char* filename);
void print_message(char* level, char* message, int);
```

- демонстрация работы библиотеки - в файле test.c:

```
gcc test.c
./a.out
```

- библиотеку подключаем, используя `#include "work_with_logs.c";` 
- для инициализации файла именем filename в качестве логфайла используем `start_logging(filename);`
- для записи сообщения используем `print_message(level, message, __LINE__);`, где level - одна из строк "debug", "warning", "info", "error", message - сообщение, которое будет записано в логфайл.
- демонстрируется запись логов в файл log1.txt
