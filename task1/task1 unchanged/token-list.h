/* token-list.h  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSTRSIZE 1024

/* Token */
#define	TNAME		1	/* Name : Alphabet { Alphabet | Digit } */
#define	TPROGRAM	2	/* program : Keyword */
#define	TVAR		3	/* var : Keyword */
#define	TARRAY		4	/* array : Keyword */
#define	TOF		5	/* of : Keyword */
#define	TBEGIN		6	/* begin : Keyword */
#define	TEND		7  	/* end : Keyword */
#define	TIF		8  	/* if : Keyword */
#define	TTHEN		9	/* then : Keyword */
#define	TELSE		10	/* else : Keyword */
#define	TPROCEDURE	11	/* procedure : Keyword */
#define	TRETURN		12	/* return : Keyword */
#define	TCALL		13	/* call : Keyword */
#define	TWHILE		14	/* while : Keyword */
#define	TDO		15 	/* do : Keyword */
#define	TNOT		16	/* not : Keyword */
#define	TOR		17	/* or : Keyword */
#define	TDIV		18 	/* div : Keyword */
#define	TAND		19 	/* and : Keyword */
#define	TCHAR		20	/* char : Keyword */
#define	TINTEGER	21	/* integer : Keyword */
#define	TBOOLEAN	22 	/* boolean : Keyword */
#define	TREADLN		23	/* readln : Keyword */
#define	TWRITELN	24	/* writeln : Keyword */
#define	TTRUE		25	/* true : Keyword */
#define	TFALSE		26	/* false : Keyword */
#define	TNUMBER		27	/* unsigned integer */
#define	TSTRING		28	/* String */
#define	TPLUS		29	/* + : symbol */
#define	TMINUS		30 	/* - : symbol */
#define	TSTAR		31 	/* * : symbol */
#define	TEQUAL		32 	/* = : symbol */
#define	TNOTEQ		33 	/* <> : symbol */
#define	TLE		34 	/* < : symbol */
#define	TLEEQ		35 	/* <= : symbol */
#define	TGR		36	/* > : symbol */
#define	TGREQ		37	/* >= : symbol */
#define	TLPAREN		38 	/* ( : symbol */
#define	TRPAREN		39 	/* ) : symbol */
#define	TLSQPAREN	40	/* [ : symbol */
#define	TRSQPAREN	41 	/* ] : symbol */
#define	TASSIGN		42	/* := : symbol */
#define	TDOT		43 	/* . : symbol */
#define	TCOMMA		44	/* , : symbol */
#define	TCOLON		45	/* : : symbol */
#define	TSEMI		46	/* ; : symbol */
#define	TREAD		47	/* read : Keyword */
#define	TWRITE		48	/* write : Keyword */
#define	TBREAK		49	/* break : Keyword */

#define NUMOFTOKEN	49

/* token-list.c */
#define KEYWORDSIZE    28
#define MAXKEYWORDLENGTH    9
#define S_ERROR  -1

extern struct KEY {
    char *keyword;
    int keytoken;
} key[KEYWORDSIZE];

extern void error(char *mes);

/* id-list.c */
extern void init_idtab();
extern struct ID *search_idtab(char *np);
extern void id_countup(char *np);
extern void print_idtab();
extern void release_idtab();

/* scan.c */
extern int init_scan(char *filename, FILE **fp);

extern int scan(FILE *fp);

extern void init_int_array(int *array, int arraylength);

extern void init_char_array(char *array, int arraylength);

extern int is_check_alphabet(char c);

extern int is_check_number(char c);

extern int is_check_symbol(char c);

extern int skip_separator(char c, FILE *fp);

extern int identify_keyword(const char *tokenstr);

extern int identify_name(const char *tokenstr);

extern int identify_number(const char *tokenstr);

extern int identify_symbol(char *tokenstr, FILE *fp);

extern int identify_string(FILE *fp);

extern int skip_comment(FILE *fp, int sep_type);

extern int num_attr;

extern char string_attr[MAXSTRSIZE];

extern char cbuf;

extern int linenum;

extern int get_linenum(void);

extern void end_scan(FILE *fp);

extern int is_check_token_size(int i);


