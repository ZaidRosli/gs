/*gen_code.c*/

#include "scan.h"

/*! maximum length of a label */
#define LABEL_SIZE 128
char buffer[2048];

/*! File pointer for the output file */
FILE *out_fp;
/*! Counter for the number of labels created */
int label_counter = 0;
/* Unique label number */
int labelnum = 1;

/* Initialize the output file (convert filename to .csl) */
int init_compiler(char *filename_mppl) {
    char *filename = strtok(filename_mppl, ".");
    filename = strtok(filename_mppl, ".");
    char filename_csl[128];
    sprintf(filename_csl, "%s.csl", filename);   /* Convert filename.mppl to filename.csl */

    if ((out_fp = fopen(filename_csl, "w")) == NULL) {
        error("Unable to open output file");
        return -1;
    }

    return 0;
}

/* Close the output file */
int end_compiler(void) {
    if (fclose(out_fp) == EOF) {
        error("Unable to close output file");
        return -1;
    }
    return 0;
}

/* Get a unique label number */
int get_label_num(void){
    static int label_counter = 1;
    return label_counter++;
}

/* Output a label in CASL II format */
void gen_label(int lablenum){
    fprintf(out_fp, "L%04d\n", labelnum);
}

/* Generate a basic CASL II instruction with opcode and operand */
void gen_code(char *opc, char *opr){
    fprintf(out_fp, "\t%-4s\t%s\n", opc , opr);
}

/* Generate instructions that involve labels */
void gen_code_label(char *opc, char *opr, int label){
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%sL%04d", opr, label);
    gen_code(opc, buffer);
}
/* Generates the start of the program */
int gen_code_start(char *program_name) {
    fprintf(out_fp, "%%%s \tSTART\tL0001\n", program_name);
    return 0;
}

/* Generates a new unique label */
int create_newlabel(char **out) {
    char *new_label;

    /* Allocate memory for the new label */
    if ((new_label = (char *)malloc(sizeof(char) * (LABEL_SIZE + 1))) == NULL) {
        return error("Memory allocation failed in create_newlabel\n");
    }

    label_counter++;

    /* Format the new label */
    sprintf(new_label, "L%04d", label_counter);
    *out = new_label;

    return 0;
}

/* Marks the end of a code block */
void gen_code_block_end(void) {
    gen_code("RET", "");
}

/* Output the procedure definition label */
void gen_code_procedure_def(void) {
    fprintf(out_fp, "$%s\n", current_proc);
}

/* Initialize the procedure compilation */
int compile_procedure_begin(void) {
    struct ID *p_id;
    struct ID *p_id_list = NULL;

    /* Reverse the order of the parameters */
    p_id = localidroot;
    if (p_id != NULL) {
        while (p_id != NULL && p_id->ispara == 1) {
            struct ID *p_id_temp = NULL;
            /* Allocate memory for a temporary ID */
            if ((p_id_temp = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
                return error("Memory allocation failed for struct ID in compile_procedure_begin\n");
            }
            p_id_temp->name = p_id->name;
            p_id_temp->nextp = p_id_list;
            p_id_list = p_id_temp;
            p_id = p_id->nextp;
        }
    } else {
        /* If the procedure does not have parameters, do nothing */
        return 0;
    }

    gen_code("POP", "gr2"),
    /* Assign values to parameters */
    p_id = p_id_list;
    while (p_id != NULL) {
        gen_code("POP", "gr1");

        snprintf(buffer, sizeof(buffer), "$$%s%%%s", p_id->name, current_proc);
        gen_code("ST", buffer);
        p_id = p_id->nextp;
    }

    /* Push return address */
    gen_code("PUSH", "0,gr2");
    return 0;
}
/* End the procedure compilation */
void compile_procedure_end() {
    gen_code("RET", "");
}

/* Declare a variable */
void compile_variable_declaration(char *variable_name, char *procname, struct TYPE **type) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "$%s", variable_name);
    if (procname != NULL) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%%%s", procname);
    }
    gen_code(buffer, "");

    if ((*type)->ttype & TPARRAY) {
        snprintf(buffer, sizeof(buffer), "DS %d", (*type)->arraysize);
    } else {
        snprintf(buffer, sizeof(buffer), "DC 0");
    }
    gen_code("", buffer);
}
void compile_variable_reference_lval(struct ID *referenced_variable) {
    char operand[64];

    if (referenced_variable->ispara) {
        /* If the variable is a parameter, include the procedure name in the reference */
        snprintf(operand, sizeof(operand), "$$%s%%%s", referenced_variable->name, referenced_variable->procname);
        gen_code("LD", operand);
    } else if (referenced_variable->procname != NULL) {
        /* If the variable is local to a procedure */
        snprintf(operand, sizeof(operand), "$$%s%%%s", referenced_variable->name, referenced_variable->procname);
        gen_code("LAD", operand);
    } else {
        /* If the variable is global */
        snprintf(operand, sizeof(operand), "$$%s", referenced_variable->name);
        gen_code("LAD", operand);
    }

    if (referenced_variable->itp->ttype & TPARRAY) {
        /* If the variable is an array */
        gen_code("POP", "gr2");  // Pop the index into gr2

        /* Check for out-of-bounds array access */
        snprintf(operand, sizeof(operand), "%d", referenced_variable->itp->arraysize - 1);
        gen_code("LAD", "gr3, \t");   // Load the maximum index into gr3
        gen_code("", operand);
        gen_code("CPA", "gr2, \tgr3"); // Compare the index in gr2 with the maximum index in gr3
        gen_code_label("JPL", "", 0);  // Jump to error if the index is out of range

        /* Compute the effective address */
        gen_code("ADDA", "gr1, \tgr2");  // Add the index to the base address
        gen_code_label("JOV", "", 0);    // Jump to overflow error if overflow occurs
    }
}
void compile_variable_reference_rval(struct ID *referenced_variable) {
    if (referenced_variable->itp->ttype & TPARRAY) {
        compile_variable_reference_lval(referenced_variable); /* Get the address of the array element */
        gen_code("POP", "gr1");
        gen_code("LD", "gr1, 0, gr1"); /* Load the value from the address */
    } else {
        if (referenced_variable->ispara) {
            /* If the variable is a parameter, include the procedure name in the reference */
            snprintf(buffer, sizeof(buffer), "$%s%%%s", referenced_variable->name, referenced_variable->procname);
            gen_code("LD", buffer);
            gen_code("LD", "gr1, 0, gr1"); /* Load the value from the address */
        } else if (referenced_variable->procname != NULL) {
            /* If the variable is local to a procedure */
            snprintf(buffer, sizeof(buffer), "$%s%%%s", referenced_variable->name, referenced_variable->procname);
            gen_code("LD", buffer);
        } else {
            /* If the variable is global */
            snprintf(buffer, sizeof(buffer), "$%s", referenced_variable->name);
            gen_code("LD", buffer);
        }
    }

    gen_code("PUSH", "0, gr1");
}
/* Assign a real parameter to an address */
void assemble_assign_real_param_to_address(void) {
    char *label = NULL;
    create_newlabel(&label);
    add_literal(&literal_root, label, "0");
    snprintf(buffer, sizeof(buffer), "gr2, %s", label);
    gen_code("LAD", buffer);
    gen_code("POP", "gr1");
    gen_code("ST", "gr1, 0, gr2");
    gen_code("PUSH", "0, gr2");
}

/* Assign a value to a variable */
void compile_assign(void) {
    gen_code("POP", "gr2");
    gen_code("POP", "gr1");
    gen_code("ST", "gr2, 0, gr1");
}
/* Compile the if condition and generate the jump to else label if condition is false */
void compile_if_condition(char *else_label) {
    gen_code("POP", "gr1");
    gen_code("CPA", "gr1, gr0");
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "JZE %s", else_label);
    gen_code("", buffer);
}

/* Compile the else part and generate the jump to the end of if statement */
void compile_else(char *if_end_label, char *else_label) {
    gen_code("JUMP", if_end_label);
    fprintf(out_fp, "%s\n", else_label);
}

/* Compile the iteration condition and generate the jump to bottom label if condition is false */
void compile_iteration_condition(char *bottom_label) {
    gen_code("POP", "gr1");
    gen_code("CPA", "gr1, gr0");
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "JZE %s", bottom_label);
    gen_code("", buffer);
}

/* Compile the break statement and generate the jump to the end of the loop */
void compile_break(void) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "JUMP %s", while_end_literal_root->label);
    gen_code("", buffer);
}
/* Handles the return statement */
void compile_return(void) {
    if (in_subprogram_declaration) {
        gen_code("RET", "");
    } else {
        gen_code("SVC", "0");
    }
}

/* Generates code to call a procedure */
void gen_code_call(struct ID *id_procedure) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "$%s", id_procedure->name);
    gen_code("CALL", buffer);
}

/* Generates code for relational expressions */
void gen_code_expression(int relational_operator_token) {
    char *jmp_true_label = NULL;
    char *jmp_false_label = NULL;
    create_newlabel(&jmp_true_label);
    create_newlabel(&jmp_false_label);

    gen_code("POP", "gr2");
    gen_code("POP", "gr1");
    gen_code("CPA", "gr1, gr2");

    switch (relational_operator_token) {
        case TEQUAL: /* == */
            gen_code("JZE", jmp_true_label);
            break;
        case TNOTEQ: /* != */
            gen_code("JNZ", jmp_true_label);
            break;
        case TLE: /* < */
            gen_code("JMI", jmp_true_label);
            break;
        case TLEEQ: /* <= */
            gen_code("JMI", jmp_true_label);
            gen_code("JZE", jmp_true_label);
            break;
        case TGR: /* > */
            gen_code("JPL", jmp_true_label);
            break;
        case TGREQ: /* >= */
            gen_code("JPL", jmp_true_label);
            gen_code("JZE", jmp_true_label);
            break;
    }

    gen_code("LD", "gr1, gr0"); /* return 0 */
    gen_code("PUSH", "0, gr1");
    gen_code("JUMP", jmp_false_label);

    gen_code("", jmp_true_label);
    gen_code("LAD", "gr1, 1"); /* return 1 */
    gen_code("PUSH", "0, gr1");
    gen_code("", jmp_false_label);
}
/* Generates code for unary minus operation */
void gen_code_minus_sign() {
    gen_code("LAD", "gr1, -1");
    gen_code("PUSH", "0, gr1");
    gen_code_MULA();
}

/* Generates code for addition operation */
void gen_code_ADDA() {
    gen_code("POP", "gr2");
    gen_code("POP", "gr1");
    gen_code("ADDA", "gr1, gr2");
    gen_code("JOV", "EOVF");
    gen_code("PUSH", "0, gr1");
}

/* Generates code for subtraction operation */
void gen_code_SUBA() {
    gen_code("POP", "gr2");
    gen_code("POP", "gr1");
    gen_code("SUBA", "gr1, gr2");
    gen_code("JOV", "EOVF");
    gen_code("PUSH", "0, gr1");
}

/* Generates code for logical OR operation */
void gen_code_OR() {
    gen_code("POP", "gr2");
    gen_code("POP", "gr1");
    gen_code("OR", "gr1, gr2");
    gen_code("PUSH", "0, gr1");
}
/* Generates code to load a constant value into a register and push it onto the stack */
int gen_code_constant(int value) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "LAD \tgr1, \t%d", value);
    gen_code(buffer, "");
    gen_code("PUSH", "0, gr1");
    return 0;
}

/* Generates code for the logical NOT operation on a factor */
void gen_code_not_factor(void) {
    char *jmp_zero_label = NULL;
    char *jmp_not_end_label = NULL;
    create_newlabel(&jmp_zero_label);
    create_newlabel(&jmp_not_end_label);

    gen_code("POP", "gr1"); /* Pop the factor value into gr1 */
    gen_code("CPA", "gr1, gr0"); /* Compare the factor value with 0 */
    gen_code("JNZ", jmp_zero_label); /* If the value is not zero, jump to jmp_zero_label */
    gen_code("LAD", "gr1, 1"); /* Load 1 into gr1 (true) */
    gen_code("PUSH", "0, gr1"); /* Push the result onto the stack */
    gen_code("JUMP", jmp_not_end_label); /* Jump to the end label */

    gen_code("", jmp_zero_label);
    gen_code("LD", "gr1, gr0"); /* Load 0 into gr1 (false) */
    gen_code("PUSH", "0, gr1"); /* Push the result onto the stack */
    gen_code("", jmp_not_end_label);
}

void gen_code_type(int to_type, int from_type) {
    if (from_type == TPINT) {
        if (to_type == TPINT) {
            /* No conversion needed */
        } else if (to_type == TPBOOL) {
            char *jmp_true_label = NULL;
            char *jmp_cast_end_label = NULL;
            create_newlabel(&jmp_true_label);
            create_newlabel(&jmp_cast_end_label);
            gen_code("POP", "gr1");
            gen_code("CPA", "gr1, gr0");
            gen_code_label("JNZ", "", atoi(jmp_true_label)); /* If value != 0, set to true (1) */
            gen_code("LD", "gr1, gr0");          /* Set to false (0) */
            gen_code("PUSH", "0, gr1");
            gen_code_label("JUMP", "", atoi(jmp_cast_end_label));

            gen_code("", jmp_true_label);
            gen_code("LAD", "gr1, 1"); /* Set to true (1) */
            gen_code("PUSH", "0, gr1");
            gen_code("", jmp_cast_end_label);
        } else if (to_type == TPCHAR) {
            gen_code("POP", "gr1"); /* Get the integer value */
            gen_code("LAD", "gr2, #007F");
            gen_code("AND", "gr1, gr2"); /* Mask to 7-bit ASCII */
            gen_code("PUSH", "0, gr1");
        }
    } else if (from_type == TPBOOL) {
        /* No conversion needed */
    } else if (from_type == TPCHAR) {
        if (to_type == TPBOOL) {
            char *jmp_true_label = NULL;
            char *jmp_cast_end_label = NULL;
            create_newlabel(&jmp_true_label);
            create_newlabel(&jmp_cast_end_label);

            gen_code("POP", "gr1"); /* Get the character value */
            gen_code("CPA", "gr1, gr0");
            gen_code_label("JNZ", "", atoi(jmp_true_label)); /* If value != 0, set to true (1) */
            gen_code("LD", "gr1, gr0");          /* Set to false (0) */
            gen_code("PUSH", "0, gr1");
            gen_code_label("JUMP", "", atoi(jmp_cast_end_label));

            gen_code("", jmp_true_label);
            gen_code("LAD", "gr1, 1"); /* Set to true (1) */
            gen_code("PUSH", "0, gr1");
            gen_code("", jmp_cast_end_label);
        } else if (to_type == TPINT || to_type == TPCHAR) {
            /* No conversion needed */
        }
    }
}
/* Multiply two values from the stack */
void gen_code_MULA() {
    gen_code("POP", "gr2");
    gen_code("POP", "gr1");
    gen_code("MULA", "gr1, gr2");
    gen_code("JOV", "EOVF"); /* Jump to overflow error if overflow occurs */
    gen_code("PUSH", "0, gr1");
}

/* Divide two values from the stack */
void gen_code_DIVA() {
    gen_code("POP", "gr2");
    gen_code("POP", "gr1");
    gen_code("DIVA", "gr1, gr2");
    gen_code("JOV", "EODIV"); /* Jump to division error if division by zero occurs */
    gen_code("PUSH", "0, gr1");
}

/* Perform logical AND on two values from the stack */
void gen_code_AND() {
    gen_code("POP", "gr2");
    gen_code("POP", "gr1");
    gen_code("AND", "gr1, gr2");
    gen_code("PUSH", "0, gr1");
}    

/* Compile output format for a string */
int compile_output_format_string(char *strings) {
    char *label;
    char *surrounded_strings;

    /* Allocate memory for the string with surrounding quotes */
    if ((surrounded_strings = (char *)malloc(sizeof(char) * strlen(strings) + 2)) == NULL) {
        return error("Memory allocation failed for string in compile_output_format_string.\n");
    }
    create_newlabel(&label);
    sprintf(surrounded_strings, "'%s'", strings);
    add_literal(&literal_root, label, surrounded_strings);

    /* Generate code to load the string label and call WRITESTR */
    snprintf(buffer, sizeof(buffer), "LAD \tgr1, %s", label);
    gen_code(buffer, "");
    gen_code("LAD", "gr2, 0");
    gen_code("CALL", "WRITESTR");
    return 0;
}

/* Generates code to output a standard type (int, char, bool) */
void compile_output_format_standard_type(int type, int num) {
    gen_code("LAD", "gr2, 0");

    switch (type) {
        case TPINT:
            gen_code("CALL", "WRITEINT"); /* Call WRITEINT to output an integer */
            break;
        case TPCHAR:
            gen_code("CALL", "WRITECHAR"); /* Call WRITECHAR to output a character */
            break;
        case TPBOOL:
            gen_code("CALL", "WRITEBOOL"); /* Call WRITEBOOL to output a boolean */
            break;
    }
}
/* Generates code to output a newline */
void compile_output_line() {
    gen_code("CALL", "WRITELINE");
}

/* Generates code to read input based on the type */
void gen_code_read(int type) {
    switch (type) {
        case TPINT:
            gen_code("CALL", "READINT"); /* Read an integer */
            break;
        case TPCHAR:
            gen_code("CALL", "READCHAR"); /* Read a character */
            break;
    }
}

/* Generates code to read a line of input */
void gen_code_read_line() {
    gen_code("CALL", "READLINE");
}

