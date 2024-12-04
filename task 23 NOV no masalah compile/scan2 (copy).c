#include "pretty-print.h"

// Global Variables
char string_attr[MAXSTRSIZE];
int num_attr;          // Attribute for numbers
char cbuf = '\0';     // Current character buffer
int linenum = 0;      // Current line number

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
void skip_to_safe_point(FILE *fp) {
    fprintf(stderr, "line%d WARNING: Skipping to next safe point.\n", linenum);
    while (token != TSEMI && token != TEND && token != EOF) {
        fprintf(stderr, "Skipping token: %s (line %d)\n", tokenstr[token], linenum); // Debug output
        token = scan(fp);
    }
    if (token == TSEMI || token == TEND) {
        fprintf(stderr, "Reached safe point: %s (line %d)\n", tokenstr[token], linenum);
    }
}


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


/* Skip comments */
int skip_comment(FILE *fp, int sep_type) {
    int start_line = linenum;
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
        if (cbuf == '\n') {
            linenum++; // Only increment on actual newlines
        }
    }
    fprintf(stderr, "Unclosed comment started at line %d\n", start_line);
    return 1; // Handle unclosed comment
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
int identify_name(const char *tokenstr) {
    init_char_array(string_attr, MAXSTRSIZE);
    snprintf(string_attr, MAXSTRSIZE, "%s", tokenstr); // Copy token to string_attr
    return TNAME; // Return TNAME
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
        if (is_check_token_size(i + 1) == -1) return -1;
        if (cbuf == '\'') {
            if ((cbuf = (char) fgetc(fp)) != '\'') {
                init_char_array(string_attr, MAXSTRSIZE);
    // Validate and sanitize tempbuf before copying
	for (int j = 0; j < MAXSTRSIZE && tempbuf[j] != '\0'; j++) {
    	if ((unsigned char)tempbuf[j] > 127) { // Replace non-UTF-8 bytes
        	tempbuf[j] = '?';
    	}
	}
	snprintf(string_attr, MAXSTRSIZE, "%s", tempbuf); // Copy to string_attr

                return TSTRING; // Return TSTRING
            }
            tempbuf[i++] = '\''; // Handle escaped single quote
        } else {
            tempbuf[i] = cbuf;
        }
    }
    error("failed to reach string end."); // Handle error for unclosed string
    return -1;
}

/* ====================== Core Scanning and Parsing Functions ====================== */

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

