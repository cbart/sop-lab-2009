#define _GNU_SOURCE

#include <unistd.h>
#include <string.h>
#include "./readwrite.h"

size_t read_fd(int fd, char* buffer) {
  size_t input_len = read(fd, buffer, BUF_SIZE);
  if(input_len == -1)
    return -1;
  else
    if(buffer[input_len - 1] == '\n')
      buffer[input_len - 1] = '\0';
    else
      buffer[input_len] = '\0';
  return input_len;
}

size_t write_fd(int fd, const char* buffer) {
  size_t output_len = write(fd, buffer, strnlen(buffer, BUF_SIZE));
  return output_len;
}
