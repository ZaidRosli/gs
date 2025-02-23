//pretty_print.c 44 passed

#include "pretty-print.h" //baru buat kat task2

/* Variable to store the token read by scan() */
int token;

int indent_level = 0;

/* Variable to store the existence of emptystatement */
int existence_empty_statement = 0;

int parse_program(FILE *fp) {
    // Expect 'program' keyword
    if (token != TPROGRAM) { 
        return error("Keyword 'program' is not found");
    }
    printf("%s ", tokenstr[token]); // Print "program"
    token = scan(fp); // Move to the next token

    // Expect program name
    if (token != TNAME) { 
        return error("Program name is not found");
    }
    printf("%s", string_attr); // Print program name
    token = scan(fp); // Move to the next token

    // Expect semicolon
    if (token != TSEMI) { 
        return error("Semicolon is not found");
    }
    printf("%s\n", tokenstr[token]); // Print ";" with a newline
    token = scan(fp); // Move to the next token

    // Parse the program block
    if (parse_block(fp) == ERROR) { 
        return ERROR; 
    }

    // Expect period
    if (token != TDOT) { 
        return error("Period is not found at the end of program");
    }
    printf("%s\n", tokenstr[token]); // Print "." with a newline

    return NORMAL;
}


int parse_block(FILE *fp) {
    //int initial_indent_level = indent_level; // Save the initial indent level

    while (token == TVAR || token == TPROCEDURE) {
        switch (token) {
            case TVAR:
                //print_indent(); // Align "var" with the current indent level
                if (parse_variable_declaration(fp) == ERROR) {
                    return error("Failed to parse variable declaration in block");
                }
                //indent_level = initial_indent_level; // Reset indent after variable declaration
                break;
            case TPROCEDURE:
                //print_indent(); // Align "procedure" with the current indent level
                if (parse_subprogram_declaration(fp) == ERROR) {
                    return error("Failed to parse subprogram declaration in block");
                }
                //indent_level = initial_indent_level; // Reset indent after subprogram declaration
                break;
            default:
                return error("Unexpected token in block");
        }
    }

    // Parse the compound statement at the end of the block
    if (parse_compound_statement(fp) == ERROR) {
        return error("Failed to parse compound statement in block");
    }

    return NORMAL;
}

int parse_variable_declaration(FILE *fp) {
    // Temporarily increase the indentation level for "var"
    indent_level++;
    // Print "var" at the current indentation level
    print_indent(indent_level);
    printf("%s\n", tokenstr[token]); // Print "var"
    token = scan(fp);

    // Parse the variable declarations
    indent_level++; // Increase indentation for variables
    while (token == TNAME) {
        print_indent(indent_level);
        printf("%s ", string_attr); // Print the first variable name
        token = scan(fp);

        // Handle multiple variables separated by commas
        while (token == TCOMMA) {
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if (token == TNAME) {
                printf("%s ", string_attr);
                token = scan(fp);
            } else {
                return error("Expected variable name after ','");
            }
        }

        // Expect and print ":"
        if (token == TCOLON) {
            printf("%s " ,tokenstr[token] );
            token = scan(fp);
        } else {
            return error("Expected ':' after variable names");
        }

        // Print the type
        if (parse_type(fp) == ERROR) {
            return error("Failed to parse type");
        }

        // Expect and print ";"
        if (token == TSEMI) {
            printf("%s\n", tokenstr[token]);
            token = scan(fp);
        } else {
            return error("Expected ';' at the end of variable declaration");
        }
    }
    indent_level-=2; // Restore indentation

    return NORMAL;
}


int parse_variable_names(FILE *fp) {
    // Ensure the first variable name exists and parse it
    if (parse_variable_name(fp) == ERROR) { 
        return error("Failed to parse the first variable name in the list");
    }

    // Handle additional variable names separated by commas
    while (token == TCOMMA) {
        //print_token(tokenstr[token], 0); // Print `,` without newline
        printf("%s", tokenstr[token]);
        token = scan(fp);

        // Ensure a variable name follows the comma
        if (token != TNAME) {
            return error("Expected a variable name after ','");
        }

        // Parse the next variable name
        if (parse_variable_name(fp) == ERROR) { 
            return error("Failed to parse a variable name after ','");
        }
    }

    return NORMAL; // Successfully parsed the variable names list
}

int parse_variable_name(FILE *fp) {
    // Ensure the current token is a valid name
    if (token != TNAME) { 
        return error("Expected a variable name, but not found");
    }

    // Sanitize and print the variable name
    for (int i = 0; string_attr[i] != '\0'; i++) {
        if ((unsigned char)string_attr[i] < 32 || (unsigned char)string_attr[i] > 126) {
            return error("Variable name contains invalid characters");
        }
    }
	printf("%s ",tokenstr[token]);
    //print_token(string_attr, 0); // Print the variable name without newline
    token = scan(fp); // Move to the next token

    return NORMAL; // Parsing succeeded
}


int parse_type(FILE *fp) {
    switch (token) {
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            // Parse a standard type
            if (parse_standard_type(fp) == ERROR) { 
                return error("Failed to parse standard type"); 
            }
            break;

        case TARRAY:
            // Parse an array type
            if (parse_array_type(fp) == ERROR) { 
                return error("Failed to parse array type"); 
            }
            break;

        default:
            // Provide detailed error information
            return error("Type is not found");
    }

    return NORMAL; // Successfully parsed the type
}

int parse_standard_type(FILE *fp) {
    switch (token) {
        case TINTEGER:
            printf("%s",tokenstr[token]);
            token = scan(fp); // Move to the next token
            return NORMAL;

        case TBOOLEAN:
        case TCHAR:
            // Print the standard type token with proper formatting
            //print_token(tokenstr[token], 0); 
            printf("%s ",tokenstr[token]);
            token = scan(fp); // Move to the next token
            return NORMAL;

        default:
            return error("Standard type is not found");
    }
}

int parse_array_type(FILE *fp) {
    // Parse "array"
    if (token != TARRAY) {
        return error("Expected 'array' keyword");
    }
    printf("%s",tokenstr[token]);
    //print_token(tokenstr[token], 0); // Print `array`
    token = scan(fp);

    // Parse "["
    if (token != TLSQPAREN) {
        return error("Expected '[' after 'array'");
    }
    printf("%s",tokenstr[token]);
    //print_token(tokenstr[token], 0); // Print `[`
    token = scan(fp);

    // Parse the array size (number)
    if (token != TNUMBER) {
        return error("Expected a number inside '[]' to specify array size");
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", num_attr); // Convert the number to a string
    printf("%s",tokenstr[token]);
    //print_token(buffer, 0); // Print the numeric value
    token = scan(fp);

    // Parse "]"
    if (token != TRSQPAREN) {
        return error("Expected ']' after array size");
    }
    printf("%s",tokenstr[token]);
    //print_token(tokenstr[token], 0); // Print `]`
    token = scan(fp);

    // Parse "of"
    if (token != TOF) {
        return error("Expected 'of' keyword after '[]'");
    }
    printf("%s",tokenstr[token]);
    //print_token(tokenstr[token], 0); // Print `of`
    token = scan(fp);

    // Parse the type following "of"
    if (parse_standard_type(fp) == ERROR) {
        return error("Failed to parse type after 'of'");
    }

    return NORMAL; // Successfully parsed the array type
}

int parse_subprogram_declaration(FILE *fp) {
    // Increment indent_level to apply indentation for the procedure
    indent_level++;

    // Print "procedure" at the current indentation level
    print_indent(indent_level); 
    printf("%s", tokenstr[token]); // Print "procedure"
    token = scan(fp);

    // Parse procedure name
    if (parse_procedure_name(fp) == ERROR) {
        return ERROR;
    }

    // Parse optional formal parameters
    if (token == TLPAREN) {
        if (parse_formal_parameters(fp) == ERROR) {
            return ERROR;
        }
    }

    // Expect and print ";"
    if (token == TSEMI) {
        printf("%s\n", tokenstr[token]); // Print semicolon and newline
        token = scan(fp);
    } else {
        return error("Expected ';' after procedure declaration");
    }

    // Parse optional variable declaration
    if (token == TVAR) {
        if (parse_variable_declaration(fp) == ERROR) {
            return error("Failed to parse variable declarations in procedure");
        }
    }

    // Parse procedure body (compound statement)
    if (parse_compound_statement(fp) == ERROR) {
        return error("Failed to parse procedure body");
    }

    // Expect and print the closing ";"
    if (token == TSEMI) {
        print_indent(indent_level - 1); // Align `;` with `procedure` keyword
        printf("%s\n", tokenstr[token]); // Print semicolon and newline
        token = scan(fp);
    } else {
        return error("Expected ';' after procedure body");
    }

    // Decrement indent_level after processing the procedure
    indent_level--;

    return NORMAL;
}




int parse_procedure_name(FILE *fp) {
    if (token != TNAME) {
        return error("Procedure name is not found");
    }

    // Print the procedure name
    printf("%s", tokenstr[token]); 

    // Move to the next token
    token = scan(fp);

    return NORMAL;
}

int parse_formal_parameters(FILE *fp) {
    if (token != TLPAREN) {
        return error("Symbol '(' is not found");
    }

    printf("%s",tokenstr[token]);
    //print_token(tokenstr[token], 0); // Print `(` without newline
    token = scan(fp);

    while (token == TNAME) {
        if (parse_variable_names(fp) == ERROR) { // Parse variable names
            return error("var_names missing");
        }

        if (token != TCOLON) {
            return error("Symbol ':' is not found");
        }
        printf("%s",tokenstr[token]);
        //print_token(tokenstr[token], 0); // Print `:` without newline
        token = scan(fp);

        if (parse_type(fp) == ERROR) { // Parse the type
            return error("type missing");
        }

        if (token == TSEMI) { // Check for additional parameter groups
            //print_token(tokenstr[token], 0); // Print `;` without newline
            printf("%s", tokenstr[token]);
            token = scan(fp);
        } else {
            break; // No more parameter groups
        }
    }

    if (token != TRPAREN) {
        return error("Symbol ')' is not found");
    }

    printf("%s", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int parse_compound_statement(FILE *fp) {
    if (token != TBEGIN) {
        return error("Expected 'begin'");
    }

    // Print "begin" at the current indentation level
    print_indent(indent_level);
    printf("%s\n", tokenstr[token]); // Print "begin"
    indent_level++; // Increase indentation for statements inside the block
    token = scan(fp);

    // Parse statements inside the block
    while (token != TEND && token != EOF) {
        // Print each statement with increased indentation
        print_indent(indent_level);
        if (parse_statement(fp) == ERROR) {
            return error("Failed to parse statement");
        }

        // Handle semicolon after each statement
        if (token == TSEMI) {
            printf("%s\n", tokenstr[token]); // Print `;` with a newline
            token = scan(fp);
        } else if (token != TEND) {
            return error("Expected ';' between statements");
        }
    }

    // Print "end" at the same level as "begin"
    if (token == TEND) {
    printf("\n");
        indent_level--; // Restore indentation for "end"
        print_indent(indent_level);
        printf("%s", tokenstr[token]); // Print "end"
        token = scan(fp);
    } else {
        return error("Expected 'end'");
    }

    return NORMAL;
}

int parse_statement(FILE *fp) {
    // Indent the current statement
    //print_indent(indent_level);

    switch (token) {
        case TNAME: // Assignment statement
            if (parse_assignment_statement(fp) == ERROR) {
                return error("Failed to parse assignment statement");
            }
            break;

        case TIF: // Conditional statement
            if (parse_condition_statement(fp) == ERROR) {
                return error("Failed to parse conditional statement");
            }
            break;

        case TWHILE: // Iteration statement
            if (parse_iteration_statement(fp) == ERROR) {
                return error("Failed to parse iteration statement");
            }
            break;

        case TBREAK: // Exit statement
            if (parse_exit_statement(fp) == ERROR) {
                return error("Failed to parse exit statement");
            }
            break;

        case TCALL: // Procedure call statement
            if (parse_call_statement(fp) == ERROR) {
                return error("Failed to parse procedure call statement");
            }
            break;

        case TREAD:
        case TREADLN: // Input statement
            if (parse_input_statement(fp) == ERROR) {
                return error("Failed to parse input statement");
            }
            break;

        case TWRITE:
        case TWRITELN: // Output statement
            if (parse_output_statement(fp) == ERROR) {
                return error("Failed to parse output statement");
            }
            break;

        case TRETURN: // Return statement
            if (parse_return_statement(fp) == ERROR) {
                return error("Failed to parse return statement");
            }
            break;

        case TBEGIN: // Compound statement
            if (parse_compound_statement(fp) == ERROR) {
                return error("Failed to parse compound statement");
            }
            break;
/*
        case TSEMI: // Empty statement (explicit)
            printf("%s\n", tokenstr[token]); // Print `;` with a newline
            token = scan(fp); // Move to the next token
            break;
*/
        default: // Implicit empty statement or unknown token
            return parse_empty_statement(fp); // Handles implicit empty statements
    }

    return NORMAL;
}

int parse_condition_statement(FILE *fp) {
    if (token != TIF) {
        return error("Expected 'if'");
    }

    // Print "if" and the condition
    print_indent(indent_level);
    printf("%s", tokenstr[token]); // Print "if"
    token = scan(fp);

    // Parse the condition expression
    if (parse_expression(fp) == ERROR) {
        return error("Failed to parse condition expression");
    }

    // Expect "then"
    if (token != TTHEN) {
        return error("Expected 'then'");
    }
    printf("%s\n", tokenstr[token]); // Print "then" with a newline
    token = scan(fp);

    // Parse the statement under "then"
    if (token == TBEGIN) {
        if (parse_compound_statement(fp) == ERROR) {
            return error("Failed to parse compound statement after 'then'");
        }
    } else {
        indent_level++; // Temporarily increase indentation
        print_indent(indent_level);
        if (parse_statement(fp) == ERROR) {
            return error("Failed to parse single-line statement after 'then'");
        }
        indent_level--; // Restore original indentation
    }

    // Handle "else" if present
    if (token == TELSE) {
        printf("\n"); // Add a newline before "else"
        print_indent(indent_level); // Align "else" with "if"
        printf("%s\n", tokenstr[token]); // Print "else"
        token = scan(fp);

        if (token == TBEGIN) {
            if (parse_compound_statement(fp) == ERROR) {
                return error("Failed to parse compound statement after 'else'");
            }
        } else {
            indent_level++; // Temporarily increase indentation
            print_indent(indent_level);
            if (parse_statement(fp) == ERROR) {
                return error("Failed to parse single-line statement after 'else'");
            }
            indent_level--; // Restore original indentation
        }
    }

    return NORMAL;
}

int parse_iteration_statement(FILE *fp) {
    if (token != TWHILE) {
        return error("Expected 'while'");
    }

    // Print "while" and the condition
    //print_indent(indent_level);
    printf("%s ", tokenstr[token]); // Print "while"
    token = scan(fp);

    // Parse the condition expression
    if (parse_expression(fp) == ERROR) {
        return error("Failed to parse condition expression");
    }

    // Expect "do"
    if (token != TDO) {
        return error("Expected 'do'");
    }
    printf(" %s\n", tokenstr[token]); // Print "do" with a newline
    token = scan(fp);

    // Increase the indent level temporarily for the statement under "do"
    int current_indent_level = indent_level; // Save the current level
    if (current_indent_level == 0) {
        current_indent_level = 1; // Ensure there's at least one level of indentation
    }
    indent_level++; // Temporarily increase indentation

    // Parse the statement or block under "do"
    if (token == TBEGIN) {
        // Parse the compound statement if present
        if (parse_compound_statement(fp) == ERROR) {
            return error("Failed to parse compound statement after 'do'");
        }
    } else {
        // Handle single-line statements with the adjusted indentation
        print_indent(indent_level);
        if (parse_statement(fp) == ERROR) {
            return error("Failed to parse single-line statement after 'do'");
        }
    }

    // Restore the original indentation level
    indent_level = current_indent_level;

    return NORMAL;
}



int parse_exit_statement(FILE *fp) {
    if (token != TBREAK) {
        return error("Keyword 'break' is not found");
    }

    // Print the "break" token
    printf("%s", tokenstr[token]);
    //print_token(tokenstr[token], 1);
    token = scan(fp); // Move to the next token

    return NORMAL;
}

int parse_call_statement(FILE *fp) {
    if (token != TCALL) {
        return error("Keyword 'call' is not found");
    }

    printf("%s", tokenstr[token]);
    //print_token(tokenstr[token], 0);
    token = scan(fp);

    if (parse_procedure_name(fp) == ERROR) {
        return error("proceure_name missing");
    }

    if (token == TLPAREN) {
       printf("%s", tokenstr[token]);
        //print_token(tokenstr[token], 0);
        token = scan(fp);

        if (parse_expressions(fp) == ERROR) {
            return error("exressions not found");
        }

        if (token != TRPAREN) {
            return error("Symbol ')' is not found");
        }

        printf("%s", tokenstr[token]);
        //print_token(tokenstr[token], 0);
        token = scan(fp);
    }

    return NORMAL;
}


int parse_expressions(FILE *fp) {
    // Parse the first expression
    if (parse_expression(fp) == ERROR) {
        return error("expression missing");
    }

    // Handle subsequent expressions separated by commas
    while (token == TCOMMA) {
        printf("%s ",tokenstr[token]);
//        print_token(tokenstr[token], 0); // Print `,` with a space
        token = scan(fp);

        if (parse_expression(fp) == ERROR) {
            return error("expression missing");
        }
    }

    return NORMAL;
}

int parse_return_statement(FILE *fp) {
    // Ensure the current token is 'return'
    if (token != TRETURN) {
        return error("Keyword 'return' is not found");
    }

    // Print the `return` keyword and move to the next line
    printf("%s\n",tokenstr[token]);
    //print_token(tokenstr[token], 1); // Use newline_after = 1 to add a newline
    token = scan(fp);               // Advance to the next token

    return NORMAL; // Parsing succeeded
}


int parse_assignment_statement(FILE *fp) {
    // Add indentation for the assignment statement
    //print_indent();

    // Parse the left-hand side of the assignment
    if (parse_left_part(fp) == ERROR) {
        return error("left_part missing");
    }

    // Ensure the assignment operator ':=' is present
    if (token != TASSIGN) {
        return error("Symbol ':=' is not found in assignment statement");
    }

    // Print the ':=' token and move to the next token
    printf("%s ",tokenstr[token]);
    //print_token(tokenstr[token], 0);
    token = scan(fp);

    // Parse the right-hand side (expression) of the assignment
    if (parse_expression(fp) == ERROR) {
        return error("expression missing");
    }

    // Add a newline after the entire statement for proper formatting
    //printf("\n");

    return NORMAL;
}


int parse_left_part(FILE *fp) {
    if (parse_variable(fp) == ERROR) { return error("variable missing"); }

    return NORMAL;
}

int parse_variable(FILE *fp) {
    // Parse the variable name
    if (parse_variable_name(fp) == ERROR) {
        return error("variable_name missing");
    }

    // Check for array indexing (e.g., variable[index])
    if (token == TLSQPAREN) {
        printf("%s",tokenstr[token]);
//        print_token(tokenstr[token], 0); // Print `[`
        token = scan(fp);

        // Parse the expression inside the brackets
        if (parse_expression(fp) == ERROR) {
            return error("expression missing");
        }

        // Ensure the closing bracket `]` is present
        if (token != TRSQPAREN) {
            return error("Symbol ']' is not found at the end of expression");
        }
        printf("%s",tokenstr[token]);
        //print_token(tokenstr[token], 0); // Print `]`
        token = scan(fp); // Move to the next token
    }

    return NORMAL; // Successfully parsed
}

int parse_expression(FILE *fp) {
    // Parse the first simple expression
    if (parse_simple_expression(fp) == ERROR) {
        return error("Failed to parse simple expression in expression");
    }

    // Loop to handle relational operators followed by simple expressions
    while (is_relational_operator(token)) {
        if (parse_relational_operator(fp) == ERROR) {
            return error("relational_operator missing");
        }

        if (parse_simple_expression(fp) == ERROR) {
            return error("Failed to parse simple expression after relational operator");
        }
    }

    return NORMAL; // Successfully parsed
}

int parse_simple_expression(FILE *fp) {
    // Handle optional unary operators
    if (token == TPLUS || token == TMINUS) {
        printf("%s ",tokenstr[token]);
        //print_token(tokenstr[token], 0); // Print `+` or `-` without a newline
        token = scan(fp);
    }

    // Parse the first term
    if (parse_term(fp) == ERROR) {
        return error("term missing");
    }

    // Handle { additive_operator term }
    while (token == TPLUS || token == TMINUS || token == TOR) {
        printf("%s ",tokenstr[token]);
        //print_token(tokenstr[token], 0); // Print the additive operator
        token = scan(fp);

        // Parse the next term
        if (parse_term(fp) == ERROR) {
            return error("term missing");
        }
    }

    return NORMAL; // Successfully parsed
}

int parse_term(FILE *fp) {
    // Grammar: 項 (term) ::= 因子 { 乗法演算子 因子 }

    // Parse the first factor
    if (parse_factor(fp) == ERROR) { 
        return error("factor missing"); 
    }

    // Handle the { 乗法演算子 因子 } part
    while (is_multiplicative_operator(token)) {
        // Parse the multiplicative operator
        if (parse_multiplicative_operator(fp) == ERROR) { 
            return error("multiplicative opr missing"); 
        }

        // Parse the next factor
        if (parse_factor(fp) == ERROR) { 
            return error("factor missing"); 
        }
    }

    return NORMAL; // Successfully parsed
}

int parse_factor(FILE *fp) {
    switch (token) {
        case TNAME: // Handle variables (変数)
            if (parse_variable(fp) == ERROR) {
                return error("variable missing");
            }
            break;

        case TNUMBER: // Handle constants (定数)
        case TFALSE:
        case TTRUE:
        case TSTRING:
            if (parse_constant(fp) == ERROR) { 
                return error("constant missing"); 
            }
            break;

        case TLPAREN: // Handle "(" 式 ")"
            printf("%s ",tokenstr[token]);
            //print_token(tokenstr[token], 0); // Print `(`
            token = scan(fp);

            if (parse_expression(fp) == ERROR) { 
                return error("expression missing"); 
            }

            if (token != TRPAREN) { 
                return error("Expected ')' but not found"); 
            }

            printf("%s",tokenstr[token]);
            //print_token(tokenstr[token], 0); // Print `)`
            token = scan(fp);
            break;

        case TNOT: // Handle "not" 因子
            printf("%s ",tokenstr[token]);
            //print_token(tokenstr[token], 0); // Print `not`
            token = scan(fp);

            if (parse_factor(fp) == ERROR) { 
                return error("factor missing"); 
            }
            break;

        case TINTEGER: // Handle "標準型 "(" 式 ")"
        case TBOOLEAN:
        case TCHAR:
            if (parse_standard_type(fp) == ERROR) { 
                return ERROR; 
            } // Print handled by parse_standard_type

            if (token != TLPAREN) { 
                return error("Expected '(' after standard type but not found"); 
            }
			printf("%s",tokenstr[token]);	
            //print_token(tokenstr[token], 0); // Print `(`
            token = scan(fp);

            if (parse_expression(fp) == ERROR) { 
                return error("expression missing"); 
            }

            if (token != TRPAREN) { 
                return error("Expected ')' but not found"); 
            }
			printf("%s",tokenstr[token]);
            //print_token(tokenstr[token], 0); // Print `)`
            token = scan(fp);
            break;

        default: // Handle missing factor
            return error("Factor is missing or invalid");
    }

    return NORMAL;
}


int parse_constant(FILE *fp) {
    switch (token) {
        case TNUMBER: // Numeric constants
            printf("%d", num_attr); // Print the numeric constant
            token = scan(fp); // Move to the next token
            break;

        case TFALSE: // Boolean false
        case TTRUE:  // Boolean true
            printf("%s",tokenstr[token]);
//            print_token(tokenstr[token], 0); // Print `true` or `false`
            token = scan(fp); // Move to the next token
            break;

        case TSTRING: // String constants
            printf("'%s'", string_attr); // Print the string constant
            token = scan(fp); // Move to the next token
            break;

        default: // If the token doesn't match any constant type
            return error("Constant is not found");
    }

    return NORMAL; // Successfully parsed the constant
}

int parse_multiplicative_operator(FILE *fp) {
    switch (token) {
        case TSTAR:
        case TDIV:
        case TAND:
            printf("%s", tokenstr[token]);
            //print_token(tokenstr[token], 0);
            token = scan(fp);
            break;
        default:
            return (error("Multiplicative operator is not found"));
    }

    return NORMAL;
}

int parse_additive_operator(FILE *fp) {
    switch (token) {
        case TPLUS:
           	printf("%s", tokenstr[token]);
            //print_token(tokenstr[token], 0);
            token = scan(fp);
            break;
        case TMINUS:
           	printf("%s", tokenstr[token]);
            //print_token(tokenstr[token], 0);
            token = scan(fp);
            break;
        case TOR:
            printf("%s", tokenstr[token]);
            //print_token(tokenstr[token], 0);
            token = scan(fp);
            break;
        default:
            return (error("Additive operator is not found"));
    }

    return NORMAL;
}

int parse_relational_operator(FILE *fp) {
    if (is_relational_operator(token)) {
        //print_token(tokenstr[token], 0);
        printf("%s ",tokenstr[token]);
        token = scan(fp);
        return NORMAL;
    }
    return error("Relational operator is not found");
}

int parse_input_statement(FILE *fp) {
    // Print `read` or `readln`
    //print_token(tokenstr[token], 0);
    printf("%s ",tokenstr[token]);
    token = scan(fp);

    // Check for optional parentheses
    if (token == TLPAREN) {
        //print_token(tokenstr[token], 0); // Print `(`
        printf("%s ",tokenstr[token]);
        token = scan(fp);

        // Parse the first variable
        if (parse_variable(fp) == ERROR) {
            return error("Expected a variable in input statement");
        }

        // Handle subsequent variables separated by commas
        while (token == TCOMMA) {
            //print_token(tokenstr[token], 0); // Print `,`
            printf("%s",tokenstr[token]);
            token = scan(fp);

            if (parse_variable(fp) == ERROR) {
                return error("Expected a variable after ',' in input statement");
            }
        }

        // Ensure the parentheses are closed
        if (token != TRPAREN) {
            return error("Expected ')' to close variable list in input statement");
        }
        //print_token(tokenstr[token], 0); // Print `)` 
        printf("%s",tokenstr[token]);
        token = scan(fp);
    }

    return NORMAL;
}

int parse_output_statement(FILE *fp) {
    // Handle `write` or `writeln`
    switch (token) {
        case TWRITE:
        case TWRITELN:
            //print_token(tokenstr[token], 0); // Print `write` or `writeln`
            printf("%s ",tokenstr[token]);
            token = scan(fp);
            break;
        default:
            return error("Expected 'write' or 'writeln' at the beginning of output statement");
    }

    // Check for optional parentheses
    if (token == TLPAREN) {
        printf("%s ",tokenstr[token]); // Directly print `(`
        token = scan(fp);

        // Parse the first output format
        if (parse_output_format(fp) == ERROR) {
            return error("Invalid output format in output statement");
        }

        // Handle subsequent output formats separated by commas
        while (token == TCOMMA) {
            //print_token(tokenstr[token], 0); // Print `,`
            printf("%s ",tokenstr[token]);
            token = scan(fp);

            if (parse_output_format(fp) == ERROR) {
                return error("Invalid output format after ',' in output statement");
            }
        }

        // Ensure the parentheses are closed
        if (token != TRPAREN) {
            return error("Expected ')' to close output statement");
        }
        printf("%s",tokenstr[token]); // Print `)`
        token = scan(fp);
    }

    return NORMAL; // Successfully parsed
}

int parse_output_format(FILE *fp) {
    if (token == TSTRING) { // Handle "文字列"
        if (strlen(string_attr) == 1) {
            return error("Output format string length cannot be 1 unless it is a constant from an expression");
        }

        // Wrap string in single quotes and print directly
        printf("'%s' ", string_attr); // Print the quoted string
        token = scan(fp); // Move to the next token
        return NORMAL;
    }

    // Handle "式 [ ':' 符号なし整数 ]"
    if (parse_expression(fp) == ERROR) {
        return error("Failed to parse expression in output format");
    }

    if (token == TCOLON) { // Handle optional ':'
        printf("%s", tokenstr[token]); // Print the colon directly
        token = scan(fp); // Move to the next token

        if (token != TNUMBER) {
            return error("Expected an unsigned integer after ':' in output format");
        }

        // Print the number directly
        printf("%d ", num_attr); // Convert number to string and print
        token = scan(fp); // Move to the next token
    }

    return NORMAL; // Successfully parsed
}

int parse_token(FILE *fp) {
    print_indent(indent_level);

    while (token != EOF) {
        printf("%s", string_attr); // Print the current token

        // Peek the next token to decide if a space is needed
        int next_token = peek_next_token(fp);

        // Add space unless the next token is a semicolon or EOF
        if (next_token != TSEMI && next_token != EOF) {
            printf(" "); // Add space only if the next token allows it
        }

        // Advance to the next token
        token = scan(fp);
    }

    return NORMAL;
}

// Function to peek at the next token without consuming it
int peek_next_token(FILE *fp) {
    long current_pos = ftell(fp); // Save the current file position
    int next_token = scan(fp);   // Get the next token
    fseek(fp, current_pos, SEEK_SET); // Restore the file position
    return next_token;
}


int parse_empty_statement(FILE *fp) {
    // An empty statement doesn't print anything or advance the token.
    return NORMAL;  // Indicate successful parsing of an empty statement
}


