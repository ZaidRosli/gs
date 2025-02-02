//scan.c//

#include <ctype.h> 
#include "scan.h"
#include <stdbool.h>

#define MAXNUM 32767
FILE *fp; 
char string_attr[MAXSTRSIZE];  
int num_attr;  
char cbuf = '\0';  // buffer for current read char
int line_num;  // current line number 

// function declarations
int check_keyword(const char *token_str);
int process_identifier(const char *token_str);
int process_string(void);
int process_symbol(char *token_str);
int process_number(const char *token_str);
int skip_over(void);
int check_token_size(int length);
int get_linenum(void);

/* initialize scan  */
int init_scan(char *filename) {
    fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1; // unable to open file
    }
    line_num = 1; // initialize line number to start of file
    cbuf = (char) fgetc(fp);
    return 0;
}

/*get current line number*/
int get_linenum(void){
    return line_num;
}

/* Check if the character is a symbol */
bool is_symbol(char c){
    const char symbols[] = {'+', '-', '*', '=', '<', '>', '(', ')', '[', ']', ':', '.', ',', ';'};
    // Check if character is in symbols array
    for (size_t i = 0; i < sizeof(symbols); i++) {
        if (c == symbols[i]) {
            return true; // Character is a symbol
        }
    }
    return false; // Not a symbol
}

/* Scan and process tokens */
int scan(void) {
    char buffer[MAXSTRSIZE];
    int i = 0;

    memset(buffer, '\0', MAXSTRSIZE);

    // Skip whitespace (space, tab, newline) and comments ({comment}, /*comment*/)
    while (skip_over()) {
        if (cbuf == EOF) {
            // End of file reached
            return -1;
        }
    }

    // Handle symbols
    if (is_symbol(cbuf)){
        buffer[0] = cbuf; // Store symbol in buffer
        cbuf = (char) fgetc(fp);
        return process_symbol(buffer);
    }

    switch (cbuf) {
        case '\'':
            return process_string(); // Handle string

        // Keywords, identifiers, numbers
        default:
            if (isalpha(cbuf)) {  // Keyword or identifier
                buffer[0] = cbuf;
                for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
                    if (check_token_size(i) == -1) return -1;
                    if (isalpha(cbuf) || isdigit(cbuf)) {
                        buffer[i] = cbuf;
                    } else {
                        break;
                    }
                }
                int temp = check_keyword(buffer);
                if (temp != -1){
                    return temp; // Return keyword token
                } else {
                    return process_identifier(buffer); // Return identifier token
                }
            }

            if (isdigit(cbuf)) {  // Number
                buffer[0] = cbuf;
                for (i = 1; (cbuf = (char) fgetc(fp)) != EOF; i++) {
                    if (check_token_size(i) == -1) return -1;
                    if (isdigit(cbuf)) {
                        buffer[i] = cbuf;
                    } else {
                        break;
                    }
                }
                return process_number(buffer); // Return number token
            }
            error("Unexpected token encountered");
            return -1;
    }
}

/* Check if token matches with any keyword */
int check_keyword(const char *token_str) {
    for (int i = 0; i < KEYWORDSIZE; i++) {
        if (strcmp(token_str, key[i].keyword) == 0) {
            return key[i].keytoken; // Return the keyword token
        }
    }
    return -1; // No match found
}

/* Process and return identifier token */
int process_identifier(const char *token_str) {
    strncpy(string_attr, token_str, MAXSTRSIZE - 1);
    string_attr[MAXSTRSIZE - 1] = '\0';
    return TNAME;  // return identifier token 
}

/* Skip whitespace and comments */
int skip_over(void) {
    while (1) {
        // Skip whitespace characters
        while (isspace(cbuf)) {
            if (cbuf == '\n') line_num++; 
            cbuf = (char) fgetc(fp);
        }
        // Skip single-line comments ({comment})
        if (cbuf == '{') {
            while (cbuf != '}' && cbuf != EOF) {
                cbuf = (char) fgetc(fp);
                if (cbuf == '\n') line_num++;
            }
            if (cbuf == '}') cbuf = (char) fgetc(fp);
            continue;
        }

        // Skip single-line comments (//)
        if (cbuf == '/') {
            cbuf = (char) fgetc(fp);
            if (cbuf == '/') {
                while (cbuf != '\n' && cbuf != EOF) cbuf = (char) fgetc(fp);
                line_num++;
                cbuf = (char) fgetc(fp);
                continue;
            }

            // Skip multi-line comments (/*comment*/)
            if (cbuf == '*') {
                while (1) {
                    cbuf = (char) fgetc(fp);
                    if (cbuf == '*' && (cbuf = (char) fgetc(fp)) == '/') {
                        cbuf = (char) fgetc(fp);
                        break;
                    }
                    if (cbuf == '\n') line_num++;
                    
                    if (cbuf == EOF) {
                        printf("Unterminated multi-line comment at line %d. Skipping...\n", line_num);
                        return 1;
                    }
                }
                continue;
            } else {
                break;  // Not a comment
            }
        }
        break;
    }
    if (cbuf == EOF) {
        return 1;
    } else {
        return 0; // Continue scanning
    }
}

/* Handle string literals */
int process_string(void) {
    int i = 0;
    char tempbuf[MAXSTRSIZE];

    while ((cbuf = fgetc(fp)) != EOF) {
        if (check_token_size(i + 1) == -1) return -1;
        if (cbuf == '\'') {
            cbuf = fgetc(fp);
            if (cbuf != '\'') {
                tempbuf[i] = '\0';
                strncpy(string_attr, tempbuf, MAXSTRSIZE);
                return TSTRING;
            }
            tempbuf[i++] = '\'';  // Handle escaped single quotes
        } else {
            tempbuf[i++] = cbuf;
        }
    }   
    error("String not terminated properly");
    return -1;
}

/* Process numeric literals */
int process_number(const char *token_str) {
    long value = strtol(token_str, NULL, 10);
    if (value <= MAXNUM) {
        num_attr = (int) value;
    } else {
        error("Number exceeds maximum allowed value");
        return -1;
    }
        return TNUMBER;
    }
/* Process symbols and return corresponding token */
int process_symbol(char *token_str) {
    switch (token_str[0]) {
        case '+': return TPLUS;
        case '-': return TMINUS;
        case '*': return TSTAR;
        case '=': return TEQUAL;        
        case '<':
            if (cbuf == '>') { cbuf = fgetc(fp); return TNOTEQ; } // Not equal
            if (cbuf == '=') { cbuf = fgetc(fp); return TLEEQ; }  // Less than or equal
            return TLE; // Less than
        case '>':
            if (cbuf == '=') { cbuf = fgetc(fp); return TGREQ; }  // Greater than or equal
            return TGR; // Greater than
        case ':':
            if (cbuf == '=') { cbuf = fgetc(fp); return TASSIGN; } // Assignment
            return TCOLON; // Colon
        case '(': return TLPAREN; // Left parenthesis
        case ')': return TRPAREN; // Right parenthesis
        case '[': return TLSQPAREN; // Left square bracket
        case ']': return TRSQPAREN; // Right square bracket
        case '.': return TDOT; // Dot
        case ',': return TCOMMA; // Comma
        case ';': return TSEMI; // Semicolon
        default:
            error("Unrecognized symbol encountered.");
            return -1;
    }
}

/* Check if token length exceeds maximum allowed size */
int check_token_size(int length) {
    if (length >= MAXSTRSIZE) {
        error("Token size exceeds the maximum allowed length.");
        return -1;
    }
    return 1;
}

/* Close the file and end the scanning process */
void end_scan(void) {
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
}
