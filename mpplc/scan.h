#ifndef SCAN_H
#define SCAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ERROR -1
#define NORMAL 0

#define MAXSTRSIZE 1024

/* Token */
#define TNAME 1       /* Name : Alphabet { Alphabet | Digit } */
#define TPROGRAM 2    /* program : Keyword */
#define TVAR 3        /* var : Keyword */
#define TARRAY 4      /* array : Keyword */
#define TOF 5         /* of : Keyword */
#define TBEGIN 6      /* begin : Keyword */
#define TEND 7        /* end : Keyword */
#define TIF 8         /* if : Keyword */
#define TTHEN 9       /* then : Keyword */
#define TELSE 10      /* else : Keyword */
#define TPROCEDURE 11 /* procedure : Keyword */
#define TRETURN 12    /* return : Keyword */
#define TCALL 13      /* call : Keyword */
#define TWHILE 14     /* while : Keyword */
#define TDO 15        /* do : Keyword */
#define TNOT 16       /* not : Keyword */
#define TOR 17        /* or : Keyword */
#define TDIV 18       /* div : Keyword */
#define TAND 19       /* and : Keyword */
#define TCHAR 20      /* char : Keyword */
#define TINTEGER 21   /* integer : Keyword */
#define TBOOLEAN 22   /* boolean : Keyword */
#define TREADLN 23    /* readln : Keyword */
#define TWRITELN 24   /* writeln : Keyword */
#define TTRUE 25      /* true : Keyword */
#define TFALSE 26     /* false : Keyword */
#define TNUMBER 27    /* unsigned integer */
#define TSTRING 28    /* String */
#define TPLUS 29      /* + : symbol */
#define TMINUS 30     /* - : symbol */
#define TSTAR 31      /* * : symbol */
#define TEQUAL 32     /* = : symbol */
#define TNOTEQ 33     /* <> : symbol */
#define TLE 34        /* < : symbol */
#define TLEEQ 35      /* <= : symbol */
#define TGR 36        /* > : symbol */
#define TGREQ 37      /* >= : symbol */
#define TLPAREN 38    /* ( : symbol */
#define TRPAREN 39    /* ) : symbol */
#define TLSQPAREN 40  /* [ : symbol */
#define TRSQPAREN 41  /* ] : symbol */
#define TASSIGN 42    /* := : symbol */
#define TDOT 43       /* . : symbol */
#define TCOMMA 44     /* , : symbol */
#define TCOLON 45     /* : : symbol */
#define TSEMI 46      /* ; : symbol */
#define TREAD 47      /* read : Keyword */
#define TWRITE 48     /* write : Keyword */
#define TBREAK 49     /* break : Keyword */

#define NUMOFTOKEN 49

#define KEYWORDSIZE 28

#define ERROR -1
#define NORMAL 0

/*TYPE*/
#define NUMOFTYPE 8
#define TPNONE 0
#define TPINT 1
#define TPCHAR 2
#define TPBOOL 3
#define TPARRAY 4
#define TPARRAYINT 5
#define TPARRAYCHAR 6
#define TPARRAYBOOL 7
#define TPPROC 8

/*! maximum number of an unsigned int */
#define MAX_NUM_ATTR 32767

#define KEYWORDSIZE 28

extern struct KEY
{
    char *keyword; /*! keyword strings */
    int keytoken;  /*! token codes for keywords */
} key[KEYWORDSIZE];

struct TYPE
{
    int ttype;
    int arraysize;
    struct TYPE *etp;
    struct TYPE *paratp;
};

struct LINE
{
    int reflinenum;         /* line number */
    struct LINE *nextlinep; /* pointer to next struct */
};

struct ID
{
    char *name;         /* name */
    char *procname;     /* procedure name within which this name is defined, NULL if global name */
    struct TYPE *itp;   /* Type for the name */
    int ispara;         /* 1:formal parameter, 0:else(variable) */
    int deflinenum;     /* Name defined line number */
    struct LINE *irefp; /* List of line numbers where the name was referenced */
    struct ID *nextp;   /* pointer next struct */
};

struct LITERAL
{
    char *label;           /* label */
    char *value;           /*strings or unsigned int */
    struct LITERAL *nextp; /* pointer next struct */
};

/*main.c */
extern char *tokenstr[NUMOFTOKEN + 1];
extern char *typestr[NUMOFTYPE + 1];
extern int token;
extern int error(char *mes);
extern struct ID *crtableroot;
extern struct ID *localidroot;

/*scan.c*/
extern FILE *fp;
extern int num_attr;
extern char string_attr[MAXSTRSIZE];
extern int init_scan(char *filename);
extern int scan(void);
extern int get_linenum(void);
extern void end_scan(void);

/*parser.c*/
extern int parse_program(void);
extern int in_subprogram_declaration;
extern int def_proc_name;
extern int is_formal_parameter;
extern struct ID *id_variable;

/*id_list.c*/
extern void set_proc_name(char *name);
extern int add_globalid_to_crtable(void);
extern void init_crtab(void);
extern void release_crtab(void);
extern int release_locals(void);
extern int id_register_without_type(char *name);
extern int id_register_as_type(struct TYPE **type);
extern struct TYPE *std_type(int type);
extern struct TYPE *array_type(int type);
extern int register_linenum(char *name);
extern struct ID *search_procedure(char *procname);
extern void print_tab(struct ID *root);
extern char current_proc[MAXSTRSIZE];

/*compiler_lib.c*/
extern void outlib();

/*gen_code.c */
extern FILE *out_fp;
extern int init_compiler(char *filename_mppl);
extern int end_compiler(void);
extern int gen_code_start(char *program_name);
extern int create_newlabel(char **out);
extern void gen_code_block_end(void);
extern void gen_code_procedure_def();
extern int compile_procedure_begin();
extern void compile_procedure_end();
extern void compile_assign(void);
extern void compile_if_condition(char *else_label);
extern void compile_else(char *if_end_label, char *else_label);
extern void compile_iteration_condition(char *bottom_label);
extern void compile_break(void);
extern void compile_return(void);
extern void compile_variable_declaration(char *variable_name, char *procname, struct TYPE **type);
extern void compile_variable_reference_rval(struct ID *referenced_variable);
extern void compile_variable_reference_lval(struct ID *referenced_variable);
extern void assemble_assign_real_param_to_address(void);
extern void gen_code_call(struct ID *id_procedure);
extern void gen_code_expression(int relational_operator_token);
extern void gen_code_minus_sign();
extern void gen_code_ADDA();
extern void gen_code_SUBA();
extern void gen_code_OR();
extern int gen_code_constant(int constant_value);
extern void gen_code_not_factor(void);
extern void gen_code_type(int to_type, int from_type);
extern void gen_code_MULA();
extern void gen_code_DIVA();
extern void gen_code_AND();
extern int compile_output_format_string(char *strings);
extern void compile_output_format_standard_type(int type, int num);
extern void compile_output_line();
extern void gen_code_read(int type);
extern void gen_code_read_line();
extern void compile_library();

/*literal_list.c */

extern struct LITERAL *literal_root;
extern struct LITERAL *while_end_literal_root;
extern void init_literal_list();
extern int add_literal(struct LITERAL **root, char *label, char *value);
extern void pop_while_literal_list(void);
extern void release_literal_lists(void);
extern void release_literal(struct LITERAL **root);
extern void gen_code_literals(void);

#endif