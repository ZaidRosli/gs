#include "token-list.h"

/* keyword list */
struct KEY key[KEYWORDSIZE] = {
  {"and", TAND},         {"array", TARRAY},     {"begin", TBEGIN},
  {"boolean", TBOOLEAN}, {"break", TBREAK},     {"call", TCALL},
  {"char", TCHAR},       {"div", TDIV},         {"do", TDO},
  {"else", TELSE},       {"end", TEND},         {"false", TFALSE},
  {"if", TIF},           {"integer", TINTEGER}, {"not", TNOT},
  {"of", TOF},           {"or", TOR},           {"procedure", TPROCEDURE},
  {"program", TPROGRAM}, {"read", TREAD},       {"readln", TREADLN},
  {"return", TRETURN},   {"then", TTHEN},       {"true", TTRUE},
  {"var", TVAR},         {"while", TWHILE},     {"write", TWRITE},
  {"writeln", TWRITELN}};

/* Token counter */
int numtoken[NUMOFTOKEN + 1];

/* string of each token */
char *tokenstr[NUMOFTOKEN + 1] = {
  "",        "NAME",    "program", "var",     "array",     "of",     
  "begin",   "end",     "if",      "then",    "else",      "procedure",
  "return",  "call",    "while",   "do",      "not",       "or",
  "div",     "and",     "char",    "integer", "boolean",   "readln",
  "writeln", "true",    "false",   "NUMBER",  "STRING",    "+",
  "-",       "*",       "=",       "<>",      "<",         "<=",
  ">",       ">=",      "(",       ")",       "[",         "]",
  ":=",      ".",       ",",       ":",       ";",         "read",   
  "write",   "break"};

int main(int nc, char *np[]) {
    int token = 0, i = 0;
    FILE *fp;
    char *temp;

    /* End without argument */
    if (nc < 2) {
        error("File name id not given.");
        return EXIT_FAILURE;
    }

    /* End if file can not be opened */
    if (init_scan(np[1], &fp) < 0) {
        sprintf(temp, "File %s can not open.", np[1]);
        error(temp);
        return EXIT_FAILURE;
    }

    /* Initialize the token count array and the identifier count structure */
    init_int_array(numtoken, NUMOFTOKEN + 1);
    init_idtab();

    /* Count tokens */
    while ((token = scan(fp)) >= 0) {
        numtoken[token]++;
        if (token == TNAME) {
            /* If it was a name token, the identifier also counts */
            id_countup(string_attr);
        }
    }

    end_scan(fp);

    /* Output the counted result */
    for (i = 1; i < NUMOFTOKEN + 1; i++) {
        if (numtoken[i] > 0) {
            printf("\t\"%s\" \t%d\n", tokenstr[i], numtoken[i]);
        }
    }

    /* Output the counted identifier */
    print_idtab();
    release_idtab();

    return EXIT_SUCCESS;
}

/* Display error message */
void error(char *mes) {
    fprintf(stderr, "\nline%d ERROR: %s\n", get_linenum(), mes);
}
