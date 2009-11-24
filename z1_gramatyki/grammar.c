#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "./grammar.h"
#include "./err.h"

const char NON_TERMINALS[] = "QWERTYUIOPASDFGHJKLZXCVBNM";

/** fgets which removes terminal \n; */
char* fgets_n(char* s, int size, FILE *stream) {
    size_t len;
    s = fgets(s, size, stream);
    if(s == NULL)
        return NULL;
    else {
        len = strnlen(s, size);
        if(s[len - 1] == '\n')
            s[len - 1] = '\0';
        return s;
    }
}

int read_grammar(Grammar* g, const char* path) {
    FILE *grammar_file = NULL;
    if((grammar_file = fopen(path, "r")) == NULL)
        syserr("Error in grammar module: cannot read grammar file.\n");
    for(g->productions_quantity = 0;
        NULL != fgets_n(g->productions[g->productions_quantity],
                        MAX_PRODUCTION_LENGTH,
                        grammar_file)
        && g->productions_quantity < 10;
        g->productions_quantity ++);
    if(0 != fclose(grammar_file))
        syserr("Error in grammar module: cannot close grammar file.\n");
    return 0;
}

int word_is_terminal(const char* word, size_t max_size) {
    if(word == NULL)
        fatal("NULL pointer in is_terminal!\n");
    return (NULL == strpbrk(word, NON_TERMINALS));
}

int shift_to_first_nonterm(const char** word, size_t max_size) {
    *word = strpbrk(*word, NON_TERMINALS);
    return 0;
}

int production(const char** new_prod, const Grammar* g, char non_terminal) {
    int i = 0;
    while(i < g->productions_quantity && g->productions[i][0] != non_terminal)
        i ++;
    if(g->productions[i][0] == non_terminal) {
        *new_prod = &(g->productions[i][1]);
        return 0;
    }
    else
        return -1; /* nie znaleziono produkcji; */
}

int make_prod
    (const Grammar* g, char* new_prod, const char* old_prod, size_t max_size) {
    const char* old_prod_ptr = old_prod;
    const char* prod;
    if(0 != shift_to_first_nonterm(&old_prod_ptr, max_size)) {
        strncpy(new_prod, old_prod, max_size);
        return 0;
    }
    new_prod[0] = '\0';
    strncpy(new_prod, old_prod, old_prod_ptr - old_prod);
    new_prod += old_prod_ptr - old_prod;
    new_prod[0] = '\0';
    new_prod -= old_prod_ptr - old_prod;
    if(0 != production(&prod, g, *old_prod_ptr))
        return -1;
    strcat(new_prod, prod);
    strcat(new_prod, old_prod_ptr + 1);
    return 0;
}
