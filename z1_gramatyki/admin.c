/** ADMIN version: 0.1.0                        *
 *         author: Cezary Bartoszuk             *
 *           mail: cbart@students.mimuw.edu.pl  *
 *  user@students: cb277617                     */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "./err.h"
#include "./grammar.h"
#include "./readwrite.h"

/** procedure printing usage help; */
void print_help() {
    const char message[] =
        "Usage: admin EXECUTORS GRAMMAR START\n\
         \n\
         \tEXECUTORS - number (natural number) of executor processes.\n\
         \tGRAMMAR   - path to grammar file.\n\
         \tSTART     - start symbol.\n";
    write_fd(1, message);
}

int main(int argc, char **argv) {

    int child_status;           /* child return status; */
    int executors_quantity;     /* number of exCLS_ERRecutors to arise; */
    char* grammar_path;         /* file name of grammar; */
    char* start_symbol;         /* start symbol; */
    int input_pipe_dsc[2];      /* input pipe; */
    int output_pipe_dsc[2];     /* output pipe (a.b.); */
    int n;                      /* current process: 1..executors_quantity; */
    char buffer[BUF_SIZE];      /* string buffer; */
    char production[BUF_SIZE];
    Grammar g;

    /** argv[0] - administrator's name;  *
     *  argv[1] - quantity of executors; *
     *  argv[2] - grammar file name;     *
     *  argv[3] - start symbol;          */

    if(argc < 4) {
        /* if there's not enough arguments -> print help; */
        print_help();
        return 0;
    }

    executors_quantity = atoi(argv[1]);
    grammar_path = argv[2];
    start_symbol = argv[3];

    if(executors_quantity < 0) {
        write_fd(2, "Executors quantity must be an non negative integer!\n");
        return 0;
    }

    if(0 != read_grammar(&g, grammar_path))
        fatal("Reading grammar failed!\n");

    /* create administrator's output pipe; */
    if(pipe(output_pipe_dsc) == -1)
        syserr(PIPE_ERR);
    
    /* if executors_quantity > 0: */
    for(n = executors_quantity; n > 0; n --) {
        /* previous output pipe is current input pipe; */
        input_pipe_dsc[0] = output_pipe_dsc[0];
        input_pipe_dsc[1] = output_pipe_dsc[1];

        /* creating new output pipe; */
        if(pipe(output_pipe_dsc) == -1)
            syserr("Error while creating output pipe.\n");

        switch(fork()) {

            /* error in fork; */
            case -1:
                syserr("Error in fork.\n");

            /* child process - the executor; */
            case 0:
                if(n == executors_quantity)
                    if(close(input_pipe_dsc[1]) == -1)
                        syserr(CLS_ERR);
                /* closing stdin; */
                if(close(0) == -1)
                    syserr(CLS_ERR);
                /* copying input pipe out to stdin; */
                if(dup(input_pipe_dsc[0]) != 0)
                    syserr(DUP_ERR);
                /* closing input pipe out; */
                if(close(input_pipe_dsc[0]) == -1)
                    syserr(CLS_ERR);
                /* closing stdout; */
                if(close(1) == -1)
                    syserr(CLS_ERR);
                /* copying output pipe in to stdout; */
                if(dup(output_pipe_dsc[1]) != 1)
                    syserr(DUP_ERR);
                /* closing output pipe in; */
                if(close(output_pipe_dsc[1]) == -1)
                    syserr(CLS_ERR);
                /* closing output pipe out; */
                if(close(output_pipe_dsc[0]) == -1)
                    syserr(CLS_ERR);
                /* starting executor process with grammar file argument; */
                if(execl("./executor", "executor", grammar_path, (char *) 0)
                   == -1)
                    syserr(EXEC_ERR);

            /* parent process - the administrator; */
            default:
                /* ... ADMIN >=input_pipe=> EXECUTOR(n) >=output_pipe=> ... */
                if(n == executors_quantity) {
                    /* closing stdout; */
                    if(close(1) == -1)
                        syserr(CLS_ERR);     
                    /* copying input pipe in to stdout; */
                    if(dup(input_pipe_dsc[1]) != 1)
                        syserr(DUP_ERR);
                    /* closing input pipe in; */
                    if(close(input_pipe_dsc[1]) == -1)
                        syserr(CLS_ERR);
                }
                /* ... >=input_pipe=> EXECUTOR(1) >=output_pipe=> ADMIN ... */
                if(n == 1) {
                    /* closing stdin; */
                    if(close(0) == -1)
                        syserr(CLS_ERR);
                    /* copying output pipe in to stdin; */
                    if(dup(output_pipe_dsc[0]) != 0)
                        syserr(DUP_ERR);
                    /* closing output pipe out; */
                    if(close(output_pipe_dsc[0]) == -1)
                        syserr(CLS_ERR);
                }
                /* closing output pipe in; */
                if(close(output_pipe_dsc[1]) == -1)
                    syserr(CLS_ERR);
                /* closing input pipe out; */
                if(close(input_pipe_dsc[0]) == -1)
                    syserr(CLS_ERR);

        }  /* switch(fork()); */

    }  /* for n in 1..executors_quantity; */
    
    if(executors_quantity == 0) {
        /* looping stdout->pipe->stdin; */
        if(close(0) == -1)
            syserr(CLS_ERR);
        if(dup(output_pipe_dsc[0]) != 0)
            syserr(DUP_ERR);
        if(close(output_pipe_dsc[0]) == -1)
            syserr(CLS_ERR);
        if(close(1) == -1)
            syserr(CLS_ERR);
        if(dup(output_pipe_dsc[1]) != 1)
            syserr(DUP_ERR);
        if(close(output_pipe_dsc[1]) == -1)
            syserr(CLS_ERR);
    }  
    
    write_fd(1, start_symbol);

    /** admin is also an executor: */

    while(-1 != read_fd(0, buffer)) {
        if((0 == strncmp("*", buffer, BUF_SIZE))
           || (0 == strncmp("!", buffer, BUF_SIZE))) {
            if(-1 == write_fd(1, "#"))
                syserr(WRITE_ERR);
        }
        else if(0 == strncmp("#", buffer, BUF_SIZE))
            break;
        else {
            if(!word_is_terminal(buffer, BUF_SIZE)) {
                switch(make_prod(&g, production, buffer, BUF_SIZE)) {
                    case -1:  /* error; */
                        if(-1 == write_fd(1, "!"))
                            syserr(WRITE_ERR);
                        fatal("Error while making production.\n");
                    default:  /* production ok; */
                        if(-1 == write_fd(1, production))
                            syserr(WRITE_ERR);
                }  /* switch; */
            }
            else {  /* word_is_terminal(input, BUF_SIZE); */
                if(-1 == write_fd(2, buffer))
                    syserr(WRITE_ERR);
                if(-1 == write_fd(1, "#"))
                    syserr(WRITE_ERR);
            }
        }
    }

    for(n = 1; n <= executors_quantity; n++) {
        wait(&child_status);
        if(child_status != 0)
            fatal("Executor ended with status %d.\n", child_status);
    }
    
    return 0;

}
