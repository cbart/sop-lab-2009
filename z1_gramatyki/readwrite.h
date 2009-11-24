#ifndef _READWRITE_H_
#define _READWRITE_H_

#define BUF_SIZE 4096

/** reads from `fd` file descriptor and inserts read string to buffer;  *
 *  reads at most `BUF_SIZE` characters, if last character is `\n`      *
 *  exchanges it with `\0`;                                             */
size_t read_fd(int fd, char* buffer);

/** writes `buffer` string with `fd` as file descriptor index;  *
 *  writes at most `BUF_SIZE` characters;                       */
size_t write_fd(int fd, const char* buffer);

#endif
