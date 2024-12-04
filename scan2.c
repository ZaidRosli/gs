//scan.c

#include "pretty-print.h"

// Global Variables
char string_attr[MAXSTRSIZE];
int num_attr;          // Attribute for numbers
char cbuf = '\0';     // Current character buffer
int linenum = 0;      // Current line number
//int indent_level = 0;
/* ====================== Initialization and Utility Functions ====================== */

/* Call before scanning, open the file to scan
 * Success: 0; Failed: -1 */
int init_scan(char *filename, FILE **fp) {
    *fp = fopen(filename, "r"); // Open the file for reading
    if (*fp == NULL) {
        return -1; // Return error if file cannot be opened
    }

    linenum = 1; // Initialize line number
    cbuf = (char) fgetc(*fp); // Read the first character
    return 0; // Successful initialization
}

/* Initialize char array with '\0' */
void init_char_array(char *array, int arraylength) {
    for (int i = 0; i < arraylength; i++) {
        array[i] = '\0'; // Set each character to null
    }
}

/* Initialize int type array with 0 */
void init_int_array(int *array, int arraylength) {
    for (int i = 0; i < arraylength; i++) {
        array[i] = 0; // Set each integer to zero
    }
}


/* ====================== Task2 ====================== */
/*
void print_indent() {
    for (int i = 0; i < indent_level; i++) {
    	//printf("Debug: i = %d, indent_level = %d\n", i, indent_level);
        printf("\t"); // 4 spaces per level
    }
}
*/

void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("    "); // Print 4 spaces for each indentation level
    }
}


void print_token(const char *token, int newline_after) {
    printf("%s", token);
    if (newline_after) {
        printf("\n");
    } else if (strcmp(token, ";") != 0 && strcmp(token, ".") != 0) {
        printf(" ");
    }
}

/*
void print_token(const char *token, int newline_after) {
    // Punctuation or symbols that do not need a preceding space
    if (strcmp(token, ";") == 0 || strcmp(token, ".") == 0 || strcmp(token, ",") == 0 || strcmp(token, ")") == 0) {
        printf("%s", token); // Print punctuation immediately after the last token
    }
    // Handle opening parentheses or other cases explicitly without space
    else if (strcmp(token, "(") == 0) {
        printf("%s", token);
    }
    // Regular tokens or keywords
    else {
        printf("%s", token); // Print the token
        if (newline_after == 0) {
            printf(" "); // Add space only when not followed by a newline
        }
    }

    // Handle newlines if requested
    if (newline_after) {
        printf("\n");
    }
}
*/

/* ====================== Helper Functions ====================== */

/* Check whether the character is alphabetic */
int is_check_alphabet(char c) {
    return ((c >= 0x41 && c <= 0x5a) || (c >= 0x61 && c <= 0x7a));
}

/* Confirm whether the character is a number */
int is_check_number(char c) {
    return (c >= 0x30 && c <= 0x39);
}

/* Confirm whether the character is a symbol */
int is_check_symbol(char c) {
    return ((c >= 0x28 && c <= 0x2e) || (c >= 0x3a && c <= 0x3e) ||
            c == 0x5b || c == 0x5d);
}

/* Check if the token size exceeds the maximum, return -1 if it does */
int is_check_token_size(int i) {
    if (i >= MAXSTRSIZE) {
        error("one token is too long.");
        return -1;
    }
    return 1; // Return success
}

int is_relational_operator(int token) {
    return (token == TEQUAL || token == TNOTEQ || token == TLE ||
            token == TLEEQ || token == TGR || token == TGREQ);
}

int is_multiplicative_operator(int token) {
    return (token == TSTAR || token == TDIV || token == TAND);
}


/* ====================== Separator and Comment Handling Functions ====================== */

/* Skip separators (whitespace, tabs, newlines, and comments) */

int skip_separator(char c, FILE *fp) {
    switch (c) {
        case '\t': // Tab
        case '\v': // Vertical tab
        case 0x20: // Space
            cbuf = (char) fgetc(fp); // Read next character
            return 1; // Return 1 for whitespace
        case '\r': // Carriage return
            cbuf = (char) fgetc(fp);
            // Only increment linenum if followed by '\n'
            if (cbuf == '\n') {
                cbuf = (char) fgetc(fp);
                linenum++;
            }
            return 1; // Handle but do not increment
        case '\n': // New line
            linenum++; // Increment line number
            cbuf = (char) fgetc(fp);
            return 1; // Return 1 for new line
        case '{': // Start of single-line comment
            return skip_comment(fp, 2);
        case '/': // Start of multi-line comment
            if ((cbuf = (char) fgetc(fp)) == '*') {
                return skip_comment(fp, 3);
            }
            error("Unexpected token after '/'.");
            return -1; // Return error
        default:
            return 0; // Not a separator
    }
}


/*
int skip_separator(char c, FILE *fp) {
    switch (c) {
        case '\t': // Tab
        case '\v': // Vertical tab
        case 0x20: // Space
            cbuf = (char) fgetc(fp); // Read next character
            return feof(fp) ? -1 : 1; // Return error if EOF is reached
        case '\r': // Carriage return
            cbuf = (char) fgetc(fp);
            if (cbuf == '\n') {
                cbuf = (char) fgetc(fp);
                linenum++;
            }
            return feof(fp) ? -1 : 1; // Handle EOF
        case '\n': // New line
            linenum++; // Increment line number
            cbuf = (char) fgetc(fp);
            return feof(fp) ? -1 : 1; // Handle EOF
        case '{': // Single-line comment
            return skip_comment(fp, 2);
        case '/': // Multi-line comment
            if ((cbuf = (char) fgetc(fp)) == '*') {
                return skip_comment(fp, 3);
            }
            error("Unexpected token after '/'.");
            return -1; // Return error
        default:
            return 0; // Not a separator
    }
}
*/

/* Skip comments */
int skip_comment(FILE *fp, int sep_type) {
    int start_line = linenum; // Track the starting line of the comment
    while ((cbuf = (char) fgetc(fp)) != EOF) {
        if (sep_type == 2 && cbuf == '}') {
            cbuf = (char) fgetc(fp);
            return 2; // End of single-line comment
        } else if (sep_type == 3 && cbuf == '*') {
            if ((cbuf = (char) fgetc(fp)) == '/') {
                cbuf = (char) fgetc(fp);
                return 3; // End of multi-line comment
            }
        }
        // Count newlines for linenum
        if (cbuf == '\r') {
            if ((cbuf = (char) fgetc(fp)) == '\n') {
                linenum++;
            } else {
                ungetc(cbuf, fp); // Put character back if not newline
                linenum++;
            }
        } else if (cbuf == '\n') {
            linenum++; // Increment line number
        }
    }

    // Print a message for unclosed comments instead of raising an error
    printf("This is an unclosed comment started at line %d\n", start_line);
    return 1; // Return a value indicating an unclosed comment
}
/* ====================== Token Identification Functions ====================== */

int identify_keyword(const char *tokenstr) {
    if (strlen(tokenstr) <= MAXKEYWORDLENGTH) {
        for (int i = 0; i < KEYWORDSIZE; i++) {
            if (strcmp(tokenstr, key[i].keyword) == 0) {
                return key[i].keytoken; // Return token code for keyword
            }
        }
    }
    return -1; // Return -1 if not found
}

/* Store name in string_attr and return TNAME */
/*
int identify_name(const char *tokenstr) {
    //printf("Debug: Identifying name '%s'\n", tokenstr);
    init_char_array(string_attr, MAXSTRSIZE);
    snprintf(string_attr, MAXSTRSIZE, "%s", tokenstr); // Copy token to string_attr
   // printf("Debug: Stored in string_attr '%s'\n", string_attr);
    return TNAME; // Return TNAME
}
*/

int identify_name(const char *name) {
    // Validate the input
    if (name == NULL || strlen(name) == 0) {
        fprintf(stderr, "Error: Invalid name input.\n");
        return ERROR;
    }

    // Copy the name into the TNAME slot in tokenstr
    snprintf(string_attr, MAXSTRSIZE, "%s", name); // Store the name in string_attr for backward compatibility
    tokenstr[TNAME] = strdup(name); // Dynamically store the name in tokenstr at the TNAME index

    // Return the TNAME token
    return TNAME;
}


/* Identify symbols and return their corresponding token code */
int identify_symbol(char *tokenstr, FILE *fp) {
    switch (tokenstr[0]) {
        case '(': return TLPAREN;
        case ')': return TRPAREN;
        case '*': return TSTAR;
        case '+': return TPLUS;
        case ',': return TCOMMA;
        case '-': return TMINUS;
        case '.': return TDOT;
        case ':':
            if (cbuf == '=') {
                cbuf = (char) fgetc(fp);
                return TASSIGN; // Return assignment token
            }
            return TCOLON;
        case ';': return TSEMI;
        case '<':
            if (cbuf == '>') { cbuf = (char) fgetc(fp); return TNOTEQ; }
            if (cbuf == '=') { cbuf = (char) fgetc(fp); return TLEEQ; }
            return TLE;
        case '=': return TEQUAL;
        case '>':
            if (cbuf == '=') { cbuf = (char) fgetc(fp); return TGREQ; }
            return TGR;
        case '[': return TLSQPAREN;
        case ']': return TRSQPAREN;
        default:
            error("failed to identify symbol.");
            return -1; // Return error if symbol is not recognized
    }
}

/* Store numbers in num_attr and return TNUMBER */
/*
int identify_number(const char *tokenstr) {
    long temp = strtol(tokenstr, NULL, 10); // Convert string to long
    if (temp <= 32767) {
   // printf("%ld" ,temp);
   printf("before:%d",num_attr);
        num_attr = (int) temp; // Store value in num_attr
         printf("someth:%d",num_attr);
    } else {
        error("number is bigger than 32767."); // Handle error for out-of-range number
        return -1;
    }
    //snprintf(num_attr, MAXSTRSIZE, "%s", tempbuf); 
    //printf("%ld" ,temp);
    return TNUMBER; // Return TNUMBER
}
*/
/* Store numbers in num_attr and return TNUMBER */
int identify_number(const char *tokenstr) {
    long temp = strtol(tokenstr, NULL, 10); // Convert string to long
    if (temp <= 32767) {
        num_attr = (int) temp; // Store value in num_attr
    } else {
        error("number is bigger than 32767."); // Handle error for out-of-range number
        return -1;
    }
    return TNUMBER; // Return TNUMBER
}


/* Store string in string_attr and return TSTRING */


int identify_string(FILE *fp) {
    char tempbuf[MAXSTRSIZE];
    int i = 0;

    for (i = 0; (cbuf = (char) fgetc(fp)) != EOF; i++) {
        if (i >= MAXSTRSIZE - 1) {
            error("String exceeds maximum length.");
            return -1;
        }

        if (cbuf == '\'') {
            if ((cbuf = (char) fgetc(fp)) != '\'') {
                tempbuf[i] = '\0'; // Null-terminate the string
                snprintf(string_attr, MAXSTRSIZE, "%s", tempbuf); // Copy to string_attr
                return TSTRING; // Return TSTRING token
            }
            tempbuf[i++] = '\''; // Handle escaped single quote
        } else if ((unsigned char)cbuf < 32 || (unsigned char)cbuf > 126) {
            error("Invalid character in string.");
            return -1;
        } else {
            tempbuf[i] = cbuf;
        }
    }

    error("Unclosed string literal.");
    return -1;
}

/* ====================== Core Scanning and Parsing Functions==parse==================== */
/*
int scan(FILE *fp) {
    char strbuf[MAXSTRSIZE];
    int i = 0, temp = 0, sep_type = 0;

    init_char_array(strbuf, MAXSTRSIZE); // Initialize strbuf
    
    while ((sep_type = skip_separator(cbuf, fp)) != 0) {
        if (sep_type == -1) return -1; // Return error if occurred
    }
    if (cbuf == EOF) return -1; // Check for EOF

    // Identify the type of token in cbuf
    if (is_check_symbol(cbuf)) {
        strbuf[0] = cbuf;
        cbuf = (char) fgetc(fp);
        return identify_symbol(strbuf, fp); // Identify and return symbol
    } else if (is_check_number(cbuf)) {
        strbuf[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            if (is_check_token_size(i) == -1) return -1;
            if (is_check_number(cbuf)) {
                strbuf[i] = cbuf;
            } else {
                break; // Break on non-number character
            }
        }
        return identify_number(strbuf); // Identify and return number
    } else if (cbuf == '\'') {
        return identify_string(fp); // Identify and return string
    } else if (is_check_alphabet(cbuf)) {
        strbuf[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            if (is_check_token_size(i) == -1) return -1;
            if (is_check_alphabet(cbuf) || is_check_number(cbuf)) {
                strbuf[i] = cbuf;
            } else {
                break; // Break on non-alphanumeric character
            }
        }
        if ((temp = identify_keyword(strbuf)) != -1) return temp; // Identify keyword
        return identify_name(strbuf); // Identify name
    }

    error("contain unexpected token."); // Handle unexpected token error
    return -1;
}
*/
int scan(FILE *fp) {
    char strbuf[MAXSTRSIZE];
    int i = 0, temp = 0, sep_type = 0;

    init_char_array(strbuf, MAXSTRSIZE); // Initialize strbuf
    while ((sep_type = skip_separator(cbuf, fp)) != 0) {
        if (sep_type == -1) return -1; // Return error if occurred
    }
    if (cbuf == EOF) return -1; // Check for EOF

    // Identify the type of token in cbuf
    if (is_check_symbol(cbuf)) {
        strbuf[0] = cbuf;
        cbuf = (char) fgetc(fp);
        return identify_symbol(strbuf, fp); // Identify and return symbol
    } else if (is_check_number(cbuf)) {
        strbuf[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            if (is_check_token_size(i) == -1) return -1;
            if (is_check_number(cbuf)) {
                strbuf[i] = cbuf;
            } else {
                break; // Break on non-number character
            }
        }
        return identify_number(strbuf); // Identify and return number
    } else if (cbuf == '\'') {
        return identify_string(fp); // Identify and return string
    } else if (is_check_alphabet(cbuf)) {
        strbuf[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
            if (is_check_token_size(i) == -1) return -1;
            if (is_check_alphabet(cbuf) || is_check_number(cbuf)) {
                strbuf[i] = cbuf;
            } else {
                break; // Break on non-alphanumeric character
            }
        }
        if ((temp = identify_keyword(strbuf)) != -1) return temp; // Identify keyword
        return identify_name(strbuf); // Identify name
    }

    error("contain unexpected token."); // Handle unexpected token error
    return -1;
}

/* ====================== Miscellaneous and Cleanup Functions ====================== */

/* Get the current line number */
int get_linenum(void) {
    return linenum; // Return current line number
}

/* Close the file after scanning */
void end_scan(FILE *fp) {
    if (fclose(fp) == EOF) {
        error("File cannot close."); // Handle file close error
    }
}
