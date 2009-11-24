/** EXECUTOR version: 0.1.0                        *
 *            author: Cezary Bartoszuk             *
 *              mail: cbart@students.mimuw.edu.pl  *
 *     user@students: cb277617                     */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "./readwrite.h"
#include "./grammar.h"
#include "./err.h"

int main(int argc, char **argv) {
    Grammar g;
    char* grammar_path;
    char input[BUF_SIZE];
    size_t input_len;
    char production[BUF_SIZE];

    if(argc < 2)
        fatal("Executor was given too few arguments!\n");
    
    grammar_path = argv[1];

    if(0 != read_grammar(&g, grammar_path))
        fatal("Reading grammar failed!\n");

    while(-1 != (input_len = read_fd(0, input))) {
        if((0 == strncmp("*", input, BUF_SIZE))
           || (0 == strncmp("!", input, BUF_SIZE))) {
            if(-1 == write_fd(1, input))
                syserr(WRITE_ERR);
        }
        else if(0 == strncmp("#", input, BUF_SIZE)) {
            if(-1 == write_fd(1, "#"))
                syserr(WRITE_ERR);
            break;
        }
        else {
            if(!word_is_terminal(input, BUF_SIZE)) {
                switch(make_prod(&g, production, input, BUF_SIZE)) {
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
                if(-1 == write_fd(2, input))
                    syserr(WRITE_ERR);
                if(-1 == write_fd(1, "*"))
                    syserr(WRITE_ERR);
            }
        }
    }

    return 0;

}
