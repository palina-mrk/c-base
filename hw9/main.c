#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "makecrc.c"
#include <string.h>
#include <stdint.h>

#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)

int
main(int argc, char *argv[]){
  int fd;
  char *addr;
  ssize_t s, len;
  struct stat sb;
  unsigned char buf[1025];
  long long int i;
  uint32_t crc = ~0U;
  char filename[1024];

  if(argc != 2){
    printf("the program has one argument: path to file for checkihg\n");
    exit(1);
  } else if(strlen(argv[1]) > 1024){
    printf("too long path to the file\n");
    exit(1);
  }
  strcpy(filename, argv[1]);

  fd = open(filename, O_RDWR);
  if (fd == -1)
    handle_error("open");

  if (stat(filename, &sb) == -1) /* To obtain file size */
    handle_error("stat");

  addr = mmap(NULL, sb.st_size , PROT_READ
              , MAP_SHARED , fd, 0);
  if (addr == MAP_FAILED)
    handle_error("mmap");

  printf("length of file: %lld\n", sb.st_size);

  for(i = 0; i < sb.st_size; i++)
    crc = crc32_tab[ (crc ^ addr[i]) & 0xFF] ^ (crc >> 8); 

  printf("crc-sum: %u or 0x%X\n", (crc^~0U));
  munmap(addr, sb.st_size);
  close(fd);

  exit(EXIT_SUCCESS);
}

