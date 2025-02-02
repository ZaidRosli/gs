//crossref.c//
#include <stdbool.h>
#include "scan.h"

// Function declarations
int parse_program(void);
int parse_block(void);
int parse_variable_declaration(void);
int parse_subprogram_declaration(void);
int parse_procedure_name(void);
int parse_variable_names(void);
int parse_type(void);
int parse_standard_type(void);
int parse_array_type(void);
int parse_formal_parameters(void);
int parse_variable(void);
int parse_expressions(void);
int parse_expression(int *is_expression_variable_only);
int parse_simple_expression(int *is_simple_expression_variable_only);
int parse_term(int *is_variable_only);
int parse_factor(int *is_variable);
int parse_constant(void);
int parse_statement(void);
int parse_compound_statement(void);
int parse_condition_statement(void);
int parse_iteration_statement(void);
int parse_call_statement(void);
int parse_assignment_statement(void);
int parse_input_statement(void);
int parse_output_statement(void);
int parse_output_format(void);

bool is_relational_operator(int c);

int empty_statement = 0;
int while_statement_level = 0;
int if_statement_level = 0;

int in_subprogram_declaration = 0;
int in_variable_declaration = 0;
int in_call_statement = 0;
int is_array_type = 0;
int is_formal_parameter = 0;
int def_proc_name = 0;

struct ID *id_procedure = NULL; // pointer to procedure name ID
struct ID *id_variable = NULL; 
struct ID *id_referenced_variable; // pointer to referenced variable ID

/* parse program ::= program name; block. */
int parse_program(void) {
    if (token != TPROGRAM) {
        return error("Expected 'program' keyword.");
    }
    token = scan();

    if (token != TNAME) {
        return error("Expected program name.");
    }

    if (gen_code_start(string_attr) == ERROR) {
        return ERROR;
    }

    token = scan();

    if (token != TSEMI) {
        return error("Expected ';' after program name.");
    }
    token = scan();

    if (parse_block() == ERROR) {
        return ERROR;
    }

    if (token != TDOT) {
        return error("Expected '.' at the end of the program.");
    }
    token = scan();

    gen_code_literals();
    outlib();

    return NORMAL;
}

/* parse block ::= {var_declaration | subprogram_declaration } compound_statement */
int parse_block(void) {
    char *L0001 = NULL;
    if (create_newlabel(&L0001) == ERROR) {
        return ERROR;
    }

    while (token == TVAR || token == TPROCEDURE) {
        if (token == TVAR) {
            /* parse variable declarations */
            if (parse_variable_declaration() == ERROR) {
                return ERROR;
            }
        } else {
            /* parse subprogram declarations */
            if (parse_subprogram_declaration() == ERROR) {
                return ERROR;
            }
        }
    }

    fprintf(out_fp, "%s\n", L0001);
    /* parse the main compound statement */
    if (parse_compound_statement() == ERROR) {
        return ERROR;
    }

    gen_code_block_end();

    return NORMAL;
}

/* parse variable declaration ::= "var" variable_names ":" type ";" { variable_names ":" type ";"} */
int parse_variable_declaration(void) {
    int is_first_variable_declaration = 1;
    if (token != TVAR) {
        return error("Expected 'var' keyword.");
    }

    token = scan();

    in_variable_declaration = true;

    while (token == TNAME) {
        if (parse_variable_names() == ERROR) {
            return ERROR;
        }

        if (token != TCOLON) {
            return error("Expected ':' after variable names.");
        }
        token = scan();

        if (parse_type() == ERROR) {
            return ERROR;
        }

        if (token != TSEMI) {
            return error("Expected ';' after type.");
        }
        token = scan();

        if (is_first_variable_declaration == 1) {
            is_first_variable_declaration = 0;
        }
    }

    in_variable_declaration = false;

    return NORMAL;
}

/* parse variable names ::= "name" {"," "name"} */
int parse_variable_names(void) {
    if (token != TNAME) {
        return error("Expected variable name.");
    }

    /* Register variable name if in variable declaration or formal parameter */
    if (in_variable_declaration || is_formal_parameter) {
        if (id_register_without_type(string_attr) == ERROR) {
            return ERROR;
        }
    }
    /* Reference variable name */
    else if (register_linenum(string_attr) == ERROR) {
        return ERROR;
    }

    token = scan();

    while (token == TCOMMA) {
        token = scan();

        if (token != TNAME) {
            return error("Expected variable name after ','.");
        }
        /* Register variable name if in variable declaration or formal parameter */
        if (in_variable_declaration || is_formal_parameter) {
            if (id_register_without_type(string_attr) == ERROR) {
                return ERROR;
            }
        }
        /* Reference variable name */
        else if (register_linenum(string_attr) == ERROR) {
            return ERROR;
        }

        token = scan();
    }
    return NORMAL;
}

/* parse type ::= standard_type | array_type */ 
int parse_type(void) {
    if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
        /* Parse standard type */
        if (parse_standard_type() == ERROR) {
            return error("Error parsing standard type.");
        }
    } else if (token == TARRAY) {
        if (is_formal_parameter) {
            return error("Array types are not allowed in formal parameters.");
        }
        is_array_type = true;
        /* Parse array type */
        if (parse_array_type() == ERROR) {
            return error("Error parsing array type.");
        }
        is_array_type = false;
    } else {
        return error("Invalid type.");
    }
    return NORMAL;
}

/* parse standard type ::= "integer" | "boolean" | "char" */
int parse_standard_type(void) {
    int standard_type = TPNONE;
    struct TYPE *type;
    if (token != TINTEGER && token != TBOOLEAN && token != TCHAR) {
        return error("Expected a standard type (integer, boolean, or char).");
    }

    /* Register type if in variable declaration or formal parameter */
    if (in_variable_declaration || is_formal_parameter) {
        if (is_array_type) {
            switch (token) {
                case TINTEGER:
                    type = array_type(TPARRAYINT);
                    break;
                case TBOOLEAN:
                    type = array_type(TPARRAYBOOL);
                    break;
                case TCHAR:
                    type = array_type(TPARRAYCHAR);
                    break;
            }
        } else {
            switch (token) {
                case TINTEGER:
                    type = std_type(TPINT);
                    break;
                case TBOOLEAN:
                    type = std_type(TPBOOL);
                    break;
                case TCHAR:
                    type = std_type(TPCHAR);
                    break;
            }
        }
        /* Check for multiple definition error */
        if (id_register_as_type(&type) == ERROR) {
            return ERROR;
        }
    }

    switch (token) {
        case TINTEGER:
            standard_type = TPINT;
            break;
        case TBOOLEAN:
            standard_type = TPBOOL;
            break;
        case TCHAR:
            standard_type = TPCHAR;
            break;
    }

    token = scan();
    return standard_type;
}
/* parse array type ::= "array" "[" number "]" "of" standard_type */
int parse_array_type(void) {
    if (token != TARRAY) {
        return error("Expected 'array' keyword.");
    }
    token = scan();

    if (token != TLSQPAREN) {
        return error("Expected '['.");
    }
    token = scan();

    if (token != TNUMBER) {
        return error("Expected a number.");
    }

    /* Check array size */
    if (in_variable_declaration) {
        /* Array size must be at least 1 */
        if (num_attr < 1) {
            return error("Array size must be at least 1.");
        }
    }

    token = scan();

    if (token != TRSQPAREN) {
        return error("Expected ']' after number.");
    }
    token = scan();

    if (token != TOF) {
        return error("Expected 'of' keyword.");
    }
    token = scan();

    if (parse_standard_type() == ERROR) {
        return ERROR;
    }
    return NORMAL;
}

/* parse subprogram declaration ::= procedure procedure_name [formal_parameters]; [variable_declaration] compound_statement ; */
int parse_subprogram_declaration(void) {
    if (token != TPROCEDURE) {
        return error("Expected 'procedure' keyword.");
    }

    token = scan();

    in_subprogram_declaration = true;
    def_proc_name = true;

    if (parse_procedure_name() == ERROR) {
        return ERROR;
    }

    def_proc_name = false;

    /* Parse formal parameters if present */
    if (token == TLPAREN && parse_formal_parameters() == ERROR) {
        return ERROR;
    }

    if (token != TSEMI) {
        return error("Expected ';' after formal parameters.");
    }
    token = scan();

    /* Parse variable declarations if present */
    if (token == TVAR) {
        if (parse_variable_declaration() == ERROR) {
            return ERROR;
        }
    }

    gen_code_procedure_def();
    compile_procedure_begin();

    /* Parse the main compound statement */
    if (parse_compound_statement() == ERROR) {
        return ERROR;
    }

    if (token != TSEMI) {
        return error("Expected ';' after compound statement.");
    }
    token = scan();

    in_subprogram_declaration = false;

    release_locals();

    compile_procedure_end();

    return NORMAL;
}

/* parse procedure name ::= "name" */
int parse_procedure_name(void) {
    if (token != TNAME) {
        return error("Procedure name expected.");
    }

    if (def_proc_name) {
        struct TYPE *type;
        id_register_without_type(string_attr); // Register new procedure name
        type = std_type(TPPROC);
        if (id_register_as_type(&type) == ERROR) { // Error if procedure name is already defined
            return ERROR;
        }

        set_proc_name(string_attr);
    } else {
        if (register_linenum(string_attr) == ERROR) {
            return ERROR;
        }
        id_procedure = search_procedure(string_attr); // Find the procedure by name
    }
    token = scan();

    return NORMAL;
}
/* parse formal parameters ::= "(" variable_names ":" type { ";" variable_names ":" type } ")" */
int parse_formal_parameters(void) {
    /* Check for '(' at the beginning */
    if (token != TLPAREN) {
        return error("Expected '(' to start formal parameters.");
    }
    token = scan();

    is_formal_parameter = true;

    /* Parse variable names */
    if (parse_variable_names() == ERROR) {
        return ERROR;
    }

    /* Check for ':' after variable names */
    if (token != TCOLON) {
        return error("Expected ':' after variable names.");
    }
    token = scan();

    /* Parse type */
    if (parse_type() == ERROR) return ERROR;

    /* Parse additional parameters separated by ';' */
    while (token == TSEMI) {
        token = scan();

        if (parse_variable_names() == ERROR) return ERROR;

        if (token != TCOLON) {
            return error("Expected ':' after variable names.");
        }
        token = scan();

        if (parse_type() == ERROR) return ERROR;
    }

    /* Check for ')' at the end */
    if (token != TRPAREN) {
        return error("Expected ')' to end formal parameters.");
    }
    token = scan();

    is_formal_parameter = false;

    return NORMAL;
}

/* parse compound statement ::= "begin" statement { ";" statement} "end" */
int parse_compound_statement(void) {
    /* Check for 'begin' keyword */
    if (token != TBEGIN) {
        return error("Expected 'begin' keyword.");
    }
    token = scan();

    /* Parse the first statement */
    if (parse_statement() == ERROR) {
        return ERROR;
    }

    /* Parse subsequent statements separated by ';' */
    while (token == TSEMI) {
        token = scan();

        if (parse_statement() == ERROR) {
            return ERROR;
        }
    }

    /* Check for 'end' keyword */
    if (token != TEND) {
        return error("Expected 'end' keyword.");
    }

    /* Reset empty statement flag */
    if (empty_statement) {
        empty_statement = 0;
    }
    token = scan();

    return NORMAL;
}

/* parse all types of statements */
int parse_statement(void) {
    switch (token) {
        case TNAME:
            if (parse_assignment_statement() == ERROR) return ERROR;
            break;
        case TIF:
            if (parse_condition_statement() == ERROR) return ERROR;
            break;
        case TWHILE:
            if (parse_iteration_statement() == ERROR) return ERROR;
            break;
        case TBREAK:
            if (while_statement_level == 0) return error("Break statement outside of while loop");
            compile_break();
            token = scan();
            break;
        case TCALL:
            if (parse_call_statement() == ERROR) return ERROR;
            break;
        case TRETURN:
            compile_return();
            token = scan();
            break;
        case TREAD:
        case TREADLN:
            if (parse_input_statement() == ERROR) return ERROR;
            break;
        case TWRITE:
        case TWRITELN:
            if (parse_output_statement() == ERROR) return ERROR;
            break;
        case TBEGIN:
            if (parse_compound_statement() == ERROR) return ERROR;
            break;
        default:
            /* empty statement */
            empty_statement = 1;
            break;
    }

    return NORMAL;
}

/* parse assignment statement ::= left_part ":=" expression */
int parse_assignment_statement(void) {
    int var_type = TPNONE;
    int exp_type = TPNONE;
    int is_expression_variable_only = 0;

    if ((var_type = parse_variable()) == ERROR) {
        return ERROR;
    }

    compile_variable_reference_lval(id_referenced_variable);

    if (token != TASSIGN) {
        return error("Expected ':='.");
    }
    token = scan();

    if ((exp_type = parse_expression(&is_expression_variable_only)) == ERROR) {
        return ERROR;
    }

    if (is_expression_variable_only) {
        compile_variable_reference_rval(id_referenced_variable);
    }

    if (var_type != exp_type) {
        return error("Type mismatch between variable and expression.");
    }

    compile_assign();

    return NORMAL;
}

/* parse condition statement ::= "if" expression "then" statement ["else" statement] */
int parse_condition_statement(void) {
    int exp_type = TPNONE;
    int is_expression_variable_only = 0;
    char *else_label = NULL;
    char *if_end_label = NULL;

    if (token != TIF) {
        return error("Expected 'if' keyword.");
    }
    token = scan();

    if ((exp_type = parse_expression(&is_expression_variable_only)) == ERROR) {
        return ERROR;
    }

    if (is_expression_variable_only) {
        compile_variable_reference_rval(id_referenced_variable);
    }

    if (exp_type != TPBOOL) {
        return error("Condition must be boolean.");
    }

    if (create_newlabel(&else_label) == ERROR) {
        return ERROR;
    }
    compile_if_condition(else_label);

    if (token != TTHEN) {
        return error("Expected 'then' keyword.");
    }
    token = scan();

    if (parse_statement() == ERROR) {
        return ERROR;
    }

    if (token == TELSE) {
        if (create_newlabel(&if_end_label) == ERROR) {
            return ERROR;
        }
        compile_else(if_end_label, else_label);
        token = scan();

        if (token != TIF) {
            if (parse_statement() == ERROR) {
                return ERROR;
            }
        } else {
            if (parse_statement() == ERROR) {
                return ERROR;
            }
        }
        fprintf(out_fp, "%s\n", if_end_label);
    } else {
        fprintf(out_fp, "%s\n", else_label);
    }
    return NORMAL;
}

/* parse iteration statement ::= "while" expression "do" statement */
int parse_iteration_statement(void) {
    int exp_type = TPNONE;
    int is_expression_variable_only = 0;
    char *iteration_top_label = NULL;
    char *iteration_bottom_label = NULL;

    create_newlabel(&iteration_top_label);
    fprintf(out_fp, "%s\n", iteration_top_label);
    create_newlabel(&iteration_bottom_label);
    add_literal(&while_end_literal_root, iteration_bottom_label, "0"); 

    if (token != TWHILE) {
        return error("Expected 'while' keyword.");
    }
    while_statement_level++;
    token = scan();

    if ((exp_type = parse_expression(&is_expression_variable_only)) == ERROR) {
        return ERROR;
    }

    if (exp_type != TPBOOL) {
        return error("Condition must be boolean.");
    }

    if (is_expression_variable_only) {
        /* condition needs right value */
        compile_variable_reference_rval(id_referenced_variable);
    }

    compile_iteration_condition(iteration_bottom_label);

    if (token != TDO) {
        return error("Expected 'do' keyword.");
    }
    token = scan();

    if (parse_statement() == ERROR) {
        return ERROR;
    }
    while_statement_level--;

    fprintf(out_fp, "\tJUMP \t%s\n", iteration_top_label);
    fprintf(out_fp, "%s\n", iteration_bottom_label);
    pop_while_literal_list();

    return NORMAL;
}

/* parse call statement ::= "call" procedure_name [ "(" expressions ")" ] */
int parse_call_statement(void) {
    if (token != TCALL) {
        return error("Expected 'call' keyword.");
    }
    token = scan();

    in_call_statement = true;
    if (parse_procedure_name() == ERROR) {
        return ERROR;
    }

    if (token == TLPAREN) {
        token = scan();

        if (parse_expressions() == ERROR) {
            return ERROR;
        }

        if (token != TRPAREN) {
            return error("Expected ')'.");
        }
        token = scan();
    } else {
        struct TYPE *para_type = id_procedure->itp->paratp;
        if (para_type != NULL) {
            return error("Too few arguments.");
        }
    }
    in_call_statement = false;

    gen_code_call(id_procedure);

    return NORMAL;
}

/* parse expressions ::= expression { "," expression } */
int parse_expressions(void) {
    int exp_type = TPNONE;
    int num_of_exp = 0;
    int is_expression_variable_only = 0;
    struct TYPE *para_type = id_procedure->itp->paratp;

    if ((exp_type = parse_expression(&is_expression_variable_only)) == ERROR) {
        return ERROR;
    }

    num_of_exp++;

    if (in_call_statement) {
        if (para_type == NULL) {
            return error("Procedure does not take any arguments.");
        }
        if (para_type->ttype != exp_type) {
            return error("Argument type mismatch.");
        }

        if (is_expression_variable_only) {
            compile_variable_reference_lval(id_referenced_variable); /* address */
        } else {
            assemble_assign_real_param_to_address();
        }
    }

    while (token == TCOMMA) {
        token = scan();

        if ((exp_type = parse_expression(&is_expression_variable_only)) == ERROR) {
            return ERROR;
        }

        num_of_exp++;
        if (in_call_statement) {
            para_type = para_type->paratp;
            if (para_type == NULL) {
                return error("Too many arguments.");
            }
            if (para_type->ttype != exp_type) {
                fprintf(stderr, "Argument %d type mismatch.", num_of_exp);
                return error("Argument type mismatch.");
            }

            if (is_expression_variable_only) {
                compile_variable_reference_lval(id_referenced_variable); /* address */
            } else {
                assemble_assign_real_param_to_address();
            }
        }
    }

    if (in_call_statement) {
        if (para_type->paratp != NULL) {
            return error("Too few arguments.");
        }
    }

    return NORMAL;
}

/* parse variable ::= variable_name [ "[" expression "]" ] */
int parse_variable(void) {
    int id_type = TPNONE;
    int is_expression_variable_only = 0;
    struct ID *id_array_variable = NULL;

    if (token != TNAME) {
        return error("Variable name expected.");
    }

    if ((id_type = register_linenum(string_attr)) == ERROR) {
        return ERROR;
    }

    id_referenced_variable = id_variable;

    token = scan();

    if (token == TLSQPAREN) {
        int exp_type = TPNONE;

        id_array_variable = id_variable;

        if (!(id_type & TPARRAY)) {
            fprintf(stderr, "%s is not an array type.", string_attr);
            return error("Variable is not an array type.");
        }

        /* variable is array type */
        token = scan();

        if ((exp_type = parse_expression(&is_expression_variable_only)) == ERROR) {
            return ERROR;
        } else if (exp_type != TPINT) {
            return error("Array index must be an integer.");
        }

        if (is_expression_variable_only) {
            /* index needs right value */
            compile_variable_reference_rval(id_referenced_variable);
        }

        if (token != TRSQPAREN) {
            return error("']' expected.");
        }
        token = scan();

        /* get the type of the array elements */
        switch (id_type) {
            case TPARRAYINT:
                id_type = TPINT;
                break;
            case TPARRAYCHAR:
                id_type = TPCHAR;
                break;
            case TPARRAYBOOL:
                id_type = TPBOOL;
                break;
        }

        id_referenced_variable = id_array_variable;
    }

    return id_type;
}

/* parse input statement ::= ("read" | "readln") [ "(" variable{ "," variable} ")" ] */
int parse_input_statement(void) {
    int read_token;

    if (token != TREAD && token != TREADLN) {
        return error("Expected 'read' or 'readln' keyword.");
    }
    read_token = token;

    token = scan();

    if (token == TLPAREN) {
        int var_type = TPNONE;
        token = scan();

        if ((var_type = parse_variable()) == ERROR) {
            return ERROR;
        }

        if (var_type != TPINT && var_type != TPCHAR) {
            return error("Variable must be of type integer or char.");
        }

        compile_variable_reference_lval(id_referenced_variable);
        gen_code_read(var_type);

        while (token == TCOMMA) {
            token = scan();

            if ((var_type = parse_variable()) == ERROR) {
                return ERROR;
            }

            if (var_type != TPINT && var_type != TPCHAR) {
                return error("Variable must be of type integer or char.");
            }

            compile_variable_reference_lval(id_referenced_variable);
            gen_code_read(var_type);
        }
        if (token != TRPAREN) {
            return error("Expected ')'.");
        }
        token = scan();
    }

    if (read_token == TREADLN) {
        gen_code_read_line();
    }

    return NORMAL;
}
/* parse output statement ::= ("write" | "writeln") [ "(" output_format { "," output_format } ")" ] */
int parse_output_statement(void) {
    int write_token;
    if (token != TWRITE && token != TWRITELN) {
        return error("Expected 'write' or 'writeln' keyword.");
    }
    write_token = token;
    // fprintf(stdout, "%s ", tokenstr[token]);
    token = scan();

    if (token == TLPAREN) {
        // fprintf(stdout, "%s ", tokenstr[token]);
        token = scan();

        if (parse_output_format() == ERROR) {
            return ERROR;
        }

        while (token == TCOMMA) {
            // fprintf(stdout, " %s ", tokenstr[token]);
            token = scan();

            if (parse_output_format() == ERROR) {
                return ERROR;
            }
        }

        if (token != TRPAREN) {
            return error("Expected ')'.");
        }
        // fprintf(stdout, " %s", tokenstr[token]);
        token = scan();
    }

    if (write_token == TWRITELN) {
        compile_output_line();
    }

    return NORMAL;
}

/* parse output format */
int parse_output_format(void) {
    int exp_type = TPNONE;
    int is_expression_variable_only = 0;

    /* Check if the token is a string with length greater than 1 */
    if (token == TSTRING && strlen(string_attr) > 1) {
        if (compile_output_format_string(string_attr) == ERROR) {
            return error("Failed to compile output format string.");
        }

        token = scan();
        return NORMAL;
    }

    switch (token) {
        case TPLUS:
        case TMINUS:
        case TNAME:
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TSTRING:
        case TLPAREN:
        case TNOT:
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            /* Parse the expression */
            if ((exp_type = parse_expression(&is_expression_variable_only)) == ERROR) {
                return error("Failed to parse expression.");
            }

            if (is_expression_variable_only) {
                compile_variable_reference_rval(id_referenced_variable);
            }

            if (exp_type & TPARRAY) {
                return error("Array type is not allowed in output format.");
            }

            /* Check for optional width specifier */
            if (token == TCOLON) {
                token = scan();

                if (token != TNUMBER) {
                    return error("Expected a number after ':'.");
                }
                token = scan();

                compile_output_format_standard_type(exp_type, num_attr);
            } else {
                compile_output_format_standard_type(exp_type, 0);
            }
            break;
        default:
            return error("Invalid output format.");
    }
    return NORMAL;
}

/* parse expression ::= simple_expression { relational_operator simple_expression } */
int parse_expression(int *is_expression_variable_only) {
    int exp_type1 = TPNONE;
    int is_simple_expression_variable_only = 0;
    *is_expression_variable_only = true;

    if ((exp_type1 = parse_simple_expression(&is_simple_expression_variable_only)) == ERROR) {
        return ERROR;
    }

    if (!is_simple_expression_variable_only) {
        *is_expression_variable_only = false;
    }

    while (is_relational_operator(token)) {
        int relational_operator_token = token;
        int exp_type2 = TPNONE;
        *is_expression_variable_only = false;

        if (is_simple_expression_variable_only) {
            compile_variable_reference_rval(id_referenced_variable);
        }

        token = scan();

        if ((exp_type2 = parse_simple_expression(&is_simple_expression_variable_only)) == ERROR) {
            return ERROR;
        }

        if (exp_type1 != exp_type2) {
            return error("Operand types do not match.");
        }

        /* The result of a relational operator is always boolean. */
        exp_type1 = TPBOOL;

        if (is_simple_expression_variable_only) {
            compile_variable_reference_rval(id_referenced_variable);
            is_simple_expression_variable_only = false;
        }

        gen_code_expression(relational_operator_token);
    }

    return exp_type1;
}

/* Check if the token is a relational operator */
bool is_relational_operator(int c) {
    switch (c) {
        case TEQUAL:
        case TNOTEQ:
        case TLE:
        case TLEEQ:
        case TGR:
        case TGREQ:
            return true;
        default:
            return false; // Not a relational operator
    }
}

/* parse simple expression ::= [ "+" | "-"] term { additive_operator term} */
int parse_simple_expression(int *is_simple_expression_variable_only) {
    int parsed_first_term = TPNONE;
    int parsed_second_term = TPNONE;
    int opr;
    int is_term_variable_only = 0;
    int sign_token = -1;
    *is_simple_expression_variable_only = true;

    /* Check for optional leading '+' or '-' */
    if (token == TPLUS || token == TMINUS) {
        *is_simple_expression_variable_only = false;
        parsed_first_term = TPINT;
        sign_token = token;

        token = scan();
    }

    /* Parse the first term */
    if ((parsed_second_term = parse_term(&is_term_variable_only)) == ERROR) {
        return ERROR;
    }

    /* Ensure the term type is integer if there is a leading '+' or '-' */
    if (parsed_first_term == TPINT && parsed_second_term != TPINT) {
        return error("Term must be an integer.");
    }
    parsed_first_term = parsed_second_term;

    /* If the term is a variable and there was a leading '+' or '-', get its value */
    if (is_term_variable_only && !(*is_simple_expression_variable_only)) {
        compile_variable_reference_rval(id_referenced_variable);
        is_term_variable_only = false;
    }

    /* Generate code for leading '-' */
    if (sign_token == TMINUS) {
        gen_code_minus_sign();
    }

    if (!is_term_variable_only) {
        *is_simple_expression_variable_only = false;
    }

    /* Parse additional terms with '+' or '-' or 'or' */
    while (token == TPLUS || token == TMINUS || token == TOR) {
        *is_simple_expression_variable_only = false;

        if (is_term_variable_only) {
            compile_variable_reference_rval(id_referenced_variable);
        }

        /* Ensure the operand types match the operator */
        if ((token == TPLUS || token == TMINUS) && parsed_first_term != TPINT) {
            return error("Operand must be an integer.");
        } else if (token == TOR && parsed_first_term != TPBOOL) {
            return error("Operand must be a boolean.");
        }
        opr = token;

        token = scan();

        if ((parsed_second_term = parse_term(&is_term_variable_only)) == ERROR) {
            return ERROR;
        }

        if (parsed_first_term == TPINT && parsed_second_term != TPINT) {
            return error("Operand must be an integer.");
        } else if (parsed_first_term == TPBOOL && parsed_second_term != TPBOOL) {
            return error("Operand must be a boolean.");
        }

        if (is_term_variable_only) {
            compile_variable_reference_rval(id_referenced_variable);
            is_term_variable_only = false;
        }

        /* Generate code for the operator */
        if (opr == TPLUS) {
            gen_code_ADDA();
        } else if (opr == TMINUS) {
            gen_code_SUBA();
        } else if (opr == TOR) {
            gen_code_OR();
        }
    }
    return parsed_first_term;
}

/* parse term ::= factor { multiplicative_operator factor } */
int parse_term(int *is_variable_only) {
    int parsed_first_term = TPNONE;
    int parsed_second_term = TPNONE;
    int opr;
    int is_variable = 0;
    *is_variable_only = true;

    /* Parse the first factor */
    if ((parsed_first_term = parse_factor(&is_variable)) == ERROR) {
        return ERROR;
    }

    if (!is_variable) {
        *is_variable_only = false;
    }

    /* Parse additional factors with '*' or '/' or 'and' */
    while (token == TSTAR || token == TDIV || token == TAND) {
        *is_variable_only = false;
        if (is_variable) {
            /* Load the value of the variable for calculation */
            compile_variable_reference_rval(id_referenced_variable);
        }

        /* Ensure the operand types match the operator */
        if ((token == TSTAR || token == TDIV) && parsed_first_term != TPINT) {
            return error("Operand must be an integer.");
        } else if (token == TAND && parsed_first_term != TPBOOL) {
            return error("Operand must be a boolean.");
        }
        opr = token;

        token = scan();

        /* Parse the next factor */
        if ((parsed_second_term = parse_factor(&is_variable)) == ERROR) {
            return ERROR;
        }

        if (parsed_first_term == TPINT && parsed_second_term != TPINT) {
            return error("Operand must be an integer.");
        } else if (parsed_first_term == TPBOOL && parsed_second_term != TPBOOL) {
            return error("Operand must be a boolean.");
        }

        if (is_variable) {
            /* Load the value of the variable for calculation */
            compile_variable_reference_rval(id_referenced_variable);
            is_variable = false;
        }

        /* Generate code for the operator */
        if (opr == TSTAR) {
            gen_code_MULA();
        } else if (opr == TDIV) {
            gen_code_DIVA();
        } else if (opr == TAND) {
            gen_code_AND();
        }
    }
    return parsed_first_term;
}

/* parse factor ::= variable | constant | "(" expression ")" | not factor | standard_type "(" expression ")" */
int parse_factor(int *is_variable) {
    int parsed_factor = TPNONE;
    int exp_type = TPNONE;
    int is_factor_var = 0;
    int is_expression_variable_only = 0;
    int parsed_token_type = 0;

    *is_variable = false;

    switch (token) {
        case TNAME:
            if ((parsed_factor = parse_variable()) == ERROR) {
                return ERROR;
            }
            *is_variable = true;
            break;
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TSTRING:
            if ((parsed_factor = parse_constant()) == ERROR) {
                return ERROR;
            }
            break;
        case TLPAREN:
            token = scan();

            if ((parsed_factor = parse_expression(&is_expression_variable_only)) == ERROR) {
                return ERROR;
            }
            /* (expression) is right value */
            if (is_expression_variable_only) {
                compile_variable_reference_rval(id_referenced_variable);
            }

            if (token != TRPAREN) {
                return error("Expected ')'.");
            }
            token = scan();
            break;
        case TNOT:
            token = scan();

            if ((parsed_factor = parse_factor(&is_factor_var)) == ERROR) {
                return ERROR;
            }
            if (parsed_factor != TPBOOL) {
                return error("Operand must be boolean.");
            }
            /* If factor is variable, get its value */
            if (is_factor_var) {
                compile_variable_reference_rval(id_referenced_variable);
            }
            gen_code_not_factor();
            break;
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            parsed_token_type = token;
            if ((parsed_factor = parse_standard_type()) == ERROR) {
                return ERROR;
            }

            if (token != TLPAREN) {
                return error("Expected '('.");
            }
            token = scan();

            if ((exp_type = parse_expression(&is_expression_variable_only)) == ERROR) {
                return ERROR;
            }

            if (is_expression_variable_only) {
                compile_variable_reference_rval(id_referenced_variable);
            }

            if (exp_type & TPARRAY) {
                return error("Type must be a standard type.");
            }

            if (token != TRPAREN) {
                return error("Expected ')'.");
            }

            gen_code_type(parsed_token_type, exp_type);

            token = scan();
            break;
        default:
            return error("Expected a factor.");
    }

    return parsed_factor;
}

/* parse constant ::= number | false | true | single character string */
int parse_constant(void) {
    int parsed_constant = NORMAL;
    int constant_value;

    switch (token) {
        case TNUMBER:
            parsed_constant = TPINT;
            constant_value = num_attr;
            break;
        case TFALSE:
        case TTRUE:
            parsed_constant = TPBOOL;
            constant_value = (token == TTRUE) ? 1 : 0;
            break;
        case TSTRING:
            if (strlen(string_attr) != 1) {
                return error("String constant length must be 1 character.");
            }
            parsed_constant = TPCHAR;
            constant_value = (int)string_attr[0];
            break;
        default:
            return error("Invalid constant.");
    }

    gen_code_constant(constant_value);

    token = scan();
    return parsed_constant;
}