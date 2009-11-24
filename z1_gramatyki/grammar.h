#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#define MAX_PRODUCTIONS 10
#define MAX_PRODUCTION_LENGTH 10

/** Grammar;                                                 *
 *  an abstract type; should not be used directly, but only  *
 *  with functions specified below;                          */
typedef
  struct grammar {
    int productions_quantity;
    char productions[MAX_PRODUCTIONS][MAX_PRODUCTION_LENGTH + 1];
  } Grammar;

/** reads grammar from specified `path` to given grammar address;  */
int read_grammar(Grammar* g, const char* path);

/** indicates if `word` is terminal (has only terminal symbols);  *
 *  returns `1` if `word` is terminal and `0` in other case;      */
int word_is_terminal(const char* word, size_t max_size);

/** creates production from `old_prod` and inserts it in `new_prod`;  *
 *  max production size (with terminal 0) has to be lower or equal    *
 *  to `max_size`; uses grammar given with `g`;                       */
int make_prod
  (const Grammar* g, char* new_prod, const char* old_prod, size_t max_size);

#endif
