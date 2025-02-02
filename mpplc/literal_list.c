#include "scan.h"

/* Pointers to the start of the literal lists */
struct LITERAL *literal_root;
struct LITERAL *while_end_literal_root;

/* Initialize both literal lists to be empty */
void init_literal_list() {
    literal_root = NULL;
    while_end_literal_root = NULL;
}

int add_literal(struct LITERAL **root, char *label, char *value) {
    struct LITERAL *new_literal;

    /* Allocate memory for a new literal node */
    if ((new_literal = (struct LITERAL *)malloc(sizeof(struct LITERAL))) == NULL) {
        return error("Memory allocation failed for new literal.\n");
    }
    /* Set the label and value for the new literal node */
    new_literal->label = label;
    new_literal->value = value;
    /* Insert the new literal node at the beginning of the list */
    new_literal->nextp = *root;
    *root = new_literal;

    return 0;
}

/* Remove the top literal from the while_end_literal_root list */
void pop_while_literal_list(void) {
    struct LITERAL *p_literal;
    if (while_end_literal_root == NULL) {
        return;
    }
    p_literal = while_end_literal_root->nextp;
    free(while_end_literal_root);
    while_end_literal_root = p_literal;
}

/* Release all literals from both literal lists */
void release_literal_lists(void) {
    release_literal(&literal_root);
    release_literal(&while_end_literal_root);
}

/* Release all literals from a given literal list */
void release_literal(struct LITERAL **root) {
    struct LITERAL *p_literal = *root;
    while (p_literal != NULL) {
        struct LITERAL *next_p = p_literal->nextp;
        free(p_literal);
        p_literal = next_p;
    }
    *root = NULL;
}

/* Generate code for all literals in the literal_root list */
void gen_code_literals(void) {
    struct LITERAL *p_literal = literal_root;
    while (p_literal != NULL) {
        fprintf(out_fp, "%s \tDC \t%s\n", p_literal->label, p_literal->value);
        p_literal = p_literal->nextp;
    }
}