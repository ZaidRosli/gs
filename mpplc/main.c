#include <stdio.h>
#include "scan.h"
#include "token.h"

int token;
int parsed;
static char *file_name;
#ifndef PATH_MAX
#define PATH_MAX 512
#endif

int main(int nc, char *np[]) {

    if (nc < 2) {
        fprintf(stderr, "Error: File name not provided.\n");
        return EXIT_FAILURE;
    }

    file_name = np[1];

    /* Initialize scanner with the provided file name */
    if (init_scan(file_name) < 0) {
        fprintf(stderr, "Error: Cannot open file %s.\n", file_name);
        return EXIT_FAILURE;
    }

    /* Initialize compiler with the provided file name */
    if (init_compiler(file_name) < 0) {
        fprintf(stderr, "Error: Cannot open file %s.\n", file_name);
        return EXIT_FAILURE;
    }

    /* Initialize cross-reference table */
    init_crtab();
    /* Initialize literal list */
    init_literal_list();

    /* Scan the first token */
    token = scan();
    /* Parse the program */
    parsed = parse_program();

    /* Finalize the compiler */
    if (end_compiler() < 0) {
        error("Error closing compiler");
        fprintf(stderr, "Error: Cannot close compiler file.\n");
        return EXIT_FAILURE;
    }

    fflush(stdout);

    /* Release resources for cross-reference table */
    release_crtab();
    /* Release resources for literal lists */
    release_literal_lists();
    return parsed;

    // Cleanup
    end_scan();
    fclose(out_fp);
}

/*Chatgpt try fix*/
/*
int main(int nc, char *np[]) {
    int ret;
    if (nc < 2 || nc > 3) {
        fprintf(stderr, "Error: File name not provided.\n");
        return 1;
    }

    // Convert input filename to its absolute path
    char fullpath[PATH_MAX];
    // Get absolute path even if input is already absolute
    if (np[1][0] == '/') {
        // Input is already absolute path
        strncpy(fullpath, np[1], PATH_MAX - 1);
        fullpath[PATH_MAX - 1] = '\0';
    } else {
        // Convert relative path to absolute path
        if (!realpath(np[1], fullpath)) {
            fprintf(stderr, "Error: Invalid path %s\n", np[1]);
            return 1;
        }
    }

    // Check file extension
    char *dot = strrchr(fullpath, '.');
    if (!dot || strcmp(dot, ".mpl") != 0) {
        fprintf(stderr, "Error: Input file must have .mpl extension\n");
        return 1;
    }

    // Create output filename (replace .mpl with .csl)
    char outfile[MAXSTRSIZE];
    strncpy(outfile, fullpath, dot - fullpath);
    outfile[dot - fullpath] = '\0';
    strcat(outfile, ".csl");

    // Open output file
    out_fp = fopen(outfile, "w");
    if (!out_fp) {
        fprintf(stderr, "Error: Cannot create output file %s\n", outfile);
        return 1;
    }

    /* Initialize scanner with the provided file name */
/*
    if (init_scan(np[1]) < 0) {
        fclose(out_fp);
        return 1;
    }
*/
    //init_parser();
    /* Initialize cross-reference table */
    //init_crtab();
    /* Initialize literal list */
    //init_literal_list();

    /* Scan the first token */
    //token = scan();
    /* Parse the program */
    //ret = parse_program();
    //if (ret == 0) {
        /* Generate the end of the program */
        //gen_code_block_end();
        /* Output the library */
        //outlib();
 //   }
    /* Finalize the compiler */
    //if (end_compiler() < 0) {
       // error("Error closing compiler");
       // fprintf(stderr, "Error: Cannot close compiler file.\n");
        //return EXIT_FAILURE;
  //  }
  //  fflush(stdout);

    /* Release resources for cross-reference table */
  //  release_crtab();
    /* Release resources for literal lists */
   // release_literal_lists();
   // return ret;
//}
/*Chatgpt try fix*/ 


/* Print error message with line number */
int error(char *mes) {
    fprintf(stderr, "Line: %4d ERROR: %s\n", get_linenum(), mes);
    return ERROR;
}
