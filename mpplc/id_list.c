#include "scan.h"

/* Indent size for cross reference table */
/* Width of the Name column */
#define INDENT_SIZE_NAME 20
/* Width of the Type column */
#define INDENT_SIZE_TYPE 30
/* Width of the Def column */
#define INDENT_SIZE_DEF 4

/* Function prototypes */
static struct ID *find_id_in_table(struct ID **root, char *name, char *procname);
static int register_id_in_table(struct ID **root, char *name, char *procname, struct TYPE **type, int ispara, int deflinenum);
static int append_type_to_param_list(struct ID **root, char *procname, struct TYPE **type);
static int append_id_to_crtab(struct ID *root);
static void release_id_struct(struct ID **root);
static void release_type_struct(struct TYPE *root);

/* Pointers to the root of various symbol tables */
struct ID *globalidroot, *localidroot;
struct ID *crtabroot;
struct ID *id_without_type_root;
/* The name of the procedure currently being parsed */
char current_proc[MAXSTRSIZE];

/* Set the current procedure name */
void set_proc_name(char *name) {
    strncpy(current_proc, name, MAXSTRSIZE);
}

/* Add global IDs to the cross-reference table */
int add_globalid_to_crtab(void) {
    return append_id_to_crtab(globalidroot);
}

/* Initialize the cross-reference table */
void init_crtab() {
    globalidroot = NULL;
    localidroot = NULL;
    crtabroot = NULL;
    id_without_type_root = NULL;
    return;
}

/* Free all symbol tables and reinitialize the cross-reference table */
void release_crtab(void) {
    release_id_struct(&globalidroot);
    release_id_struct(&localidroot);
    release_id_struct(&crtabroot);
    release_id_struct(&id_without_type_root);

    init_crtab();
    return;
}

/* Free the local symbol table */
int release_locals(void) {
    release_id_struct(&localidroot);
    return 0;
}

/* Register an ID without type */
int id_register_without_type(char *name) {
    int ispara = is_formal_parameter;
    int deflinenum = get_linenum();
    if (in_subprogram_declaration) {
        /* Register ID without type in local scope */
        return register_id_in_table(&id_without_type_root, name, current_proc, NULL, ispara, deflinenum);
    } else {
        /* Register ID without type in global scope */
        return register_id_in_table(&id_without_type_root, name, NULL, NULL, ispara, deflinenum);
    }
}

/* Register an ID with a type */
int id_register_as_type(struct TYPE **type) {
    int ret, ret1;
    struct ID *p;

    if (type == NULL) {
        return error("Type structure is NULL");
    }

    for (p = id_without_type_root; p != NULL; p = p->nextp) {
        char *name = p->name;
        char *current_proc = p->procname;
        int ispara = p->ispara;
        int deflinenum = p->deflinenum;
        if (def_proc_name) {
            /* Register procedure name in global and cross-reference tables */
            ret = register_id_in_table(&globalidroot, name, NULL, type, ispara, deflinenum);
            ret1 = register_id_in_table(&crtabroot, name, NULL, type, ispara, deflinenum);
        } else if (in_subprogram_declaration) {
            /* Register local name and formal parameter in local and cross-reference tables */
            ret = register_id_in_table(&localidroot, name, current_proc, type, ispara, deflinenum);
            ret1 = register_id_in_table(&crtabroot, name, current_proc, type, ispara, deflinenum);
            compile_variable_declaration(name, current_proc, type);
        } else {
            /* Register global name in global and cross-reference tables */
            ret = register_id_in_table(&globalidroot, name, NULL, type, ispara, deflinenum);
            ret1 = register_id_in_table(&crtabroot, name, NULL, type, ispara, deflinenum);
            compile_variable_declaration(name, NULL, type);
        }
        if (ret == ERROR || ret1 == ERROR)
            return ERROR;

        /* Add type to the parameter list of a procedure if it is a formal parameter */
        if (is_formal_parameter) {
            append_type_to_param_list(&globalidroot, current_proc, type);
            append_type_to_param_list(&crtabroot, current_proc, type);
        }
    }
    release_id_struct(&id_without_type_root);
    free(*type);
    type = NULL;
    return 0;
}

/* Create a standard type */
struct TYPE *std_type(int type) {
    struct TYPE *p_type;
    /* Allocate memory for struct TYPE */
    if ((p_type = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
        error("Failed to allocate memory for struct TYPE in std_type");
        return NULL;
    }
    /* Set the type */
    p_type->ttype = type;
    p_type->arraysize = 0;
    p_type->etp = NULL;
    p_type->paratp = NULL;
    return p_type;
}

/* Create an array type */
struct TYPE *array_type(int type) {
    struct TYPE *p_type;
    struct TYPE *p_etp;
    /* Allocate memory for array type */
    if ((p_type = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
        error("Failed to allocate memory for array type");
        return (NULL);
    }
    /* Set array type */
    p_type->ttype = type;
    p_type->arraysize = num_attr;

    /* Allocate memory for element type */
    if ((p_etp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
        error("Failed to allocate memory for element type");
        return (NULL);
    }
    /* Set element type based on array type */
    switch (type) {
        case TPARRAYINT:
            p_etp->ttype = TPINT;
            break;
        case TPARRAYCHAR:
            p_etp->ttype = TPCHAR;
            break;
        case TPARRAYBOOL:
            p_etp->ttype = TPBOOL;
            break;
        default:
            fprintf(stderr, "[%d] is not a valid array type code.\n", type);
            error("Invalid array type code");
            return (NULL);
    }
    p_etp->arraysize = 0;
    p_etp->etp = NULL;
    p_etp->paratp = NULL;

    p_type->etp = p_etp;
    p_type->paratp = NULL;
    return p_type;
}

/* Register the line number where an ID is referenced */
int register_linenum(char *name) {
    struct ID *p_id;
    struct ID *p_crtab_id;
    struct LINE *p_line;
    struct LINE *p_crtab_line;
    struct LINE *p_line_tail;
    struct LINE *p_crtab_line_tail;
    int id_type;

    if (in_subprogram_declaration) {
        char *procname = current_proc;
        /* Search in local scope */
        if ((p_id = find_id_in_table(&localidroot, name, procname)) == NULL) {
            /* Search in global scope */
            if ((p_id = find_id_in_table(&globalidroot, name, NULL)) == NULL) {
                fprintf(stderr, "%s was not declared in this scope.", name);
                return error("Undefined name detected.");
            } else {
                /* ID found in global scope */
                p_crtab_id = find_id_in_table(&crtabroot, name, NULL);
            }
        } else {
            /* ID found in local scope */
            p_crtab_id = find_id_in_table(&crtabroot, name, procname);
        }

        /* Check for recursive call error */
        if (strcmp(name, procname) == 0 && p_id->itp->ttype == TPPROC) {
            fprintf(stderr, "%s is recursively called.", procname);
            return error("Recursive procedure call detected.");
        }
    } else {
        /* Search in global scope */
        if ((p_id = find_id_in_table(&globalidroot, name, NULL)) == NULL) {
            fprintf(stderr, "%s was not declared in this scope.", name);
            return error("Undefined name detected.");
        } else {
            p_crtab_id = find_id_in_table(&crtabroot, name, NULL);
        }
    }

    /* Get the referenced variable */
    id_variable = p_id;

    /* Get the type of the variable */
    id_type = p_id->itp->ttype;

    /* Allocate memory for a new LINE structure */
    if ((p_line = (struct LINE *)malloc(sizeof(struct LINE))) == NULL) {
        return error("Memory allocation failed for LINE.");
    }
    p_line->reflinenum = get_linenum();
    p_line->nextlinep = NULL;

    /* Move to the end of the list and register the line number */
    p_line_tail = p_id->irefp;
    if (p_line_tail == NULL) {
        p_id->irefp = p_line;
    } else {
        while (p_line_tail->nextlinep != NULL) {
            p_line_tail = p_line_tail->nextlinep;
        }
        p_line_tail->nextlinep = p_line;
    }

    /* Allocate memory for a new LINE structure for cross-reference table */
    if ((p_crtab_line = (struct LINE *)malloc(sizeof(struct LINE))) == NULL) {
        return error("Memory allocation failed for LINE in cross-reference table.");
    }
    p_crtab_line->reflinenum = get_linenum();
    p_crtab_line->nextlinep = NULL;

    /* Move to the end of the list and register the line number in cross-reference table */
    p_crtab_line_tail = p_crtab_id->irefp;
    if (p_crtab_line_tail == NULL) {
        p_crtab_id->irefp = p_crtab_line;
    } else {
        while (p_crtab_line_tail->nextlinep != NULL) {
            p_crtab_line_tail = p_crtab_line_tail->nextlinep;
        }
        p_crtab_line_tail->nextlinep = p_crtab_line;
    }

    return id_type;
}
/* Search for a procedure by name in the global symbol table */
struct ID *search_procedure(char *procname) {
    struct ID *p;
    p = find_id_in_table(&globalidroot, procname, NULL);
    return p;
}

/* Print the symbol table */
void print_tab(struct ID *root) {
    struct ID *p;
    struct LINE *q;

    fprintf(stdout, "--------------------------------------------------------------------------\n");
    fprintf(stdout, "%-*s", INDENT_SIZE_NAME, "Name");
    fprintf(stdout, "%-*s", INDENT_SIZE_TYPE, "Type");
    fprintf(stdout, "Def. | Ref.\n");
    for (p = root; p != NULL; p = p->nextp) {
        /* Print the name */
        if (p->procname != NULL) {
            char name_procname[INDENT_SIZE_NAME];
            sprintf(name_procname, "%s:%s", p->name, p->procname);
            fprintf(stdout, "%-*s", INDENT_SIZE_NAME, name_procname);
        } else {
            fprintf(stdout, "%-*s", INDENT_SIZE_NAME, p->name);
        }

        /* Print the type */
        if (p->itp->ttype == TPPROC) {
            struct TYPE *paratp;
            int remaining_space = INDENT_SIZE_TYPE;

            /* Print procedure type with parameters */
            remaining_space -= strlen(typestr[p->itp->ttype]) + 1;
            fprintf(stdout, "%s(", typestr[p->itp->ttype]);

            for (paratp = p->itp->paratp; paratp != NULL; paratp = paratp->paratp) {
                remaining_space -= strlen(typestr[paratp->ttype]);
                fprintf(stdout, "%s", typestr[paratp->ttype]);
                if (paratp->paratp != NULL) {
                    remaining_space -= 1;
                    fprintf(stdout, ",");
                }
            }
            remaining_space -= 1;
            fprintf(stdout, ")");
            fprintf(stdout, "%*s", 0 < remaining_space ? remaining_space : 0, " ");
        } else if (p->itp->ttype & TPARRAY) {
            /* Print array type */
            struct TYPE *p_type = p->itp;
            char str_array_of_type[INDENT_SIZE_TYPE];
            sprintf(str_array_of_type, "array[%d] of %s", p_type->arraysize, typestr[p_type->ttype]);
            fprintf(stdout, "%-*s", INDENT_SIZE_TYPE, str_array_of_type);
        } else {
            /* Print standard type */
            fprintf(stdout, "%-*s", INDENT_SIZE_TYPE, typestr[p->itp->ttype]);
        }

        /* Print definition line number */
        fprintf(stdout, "%*d", INDENT_SIZE_DEF, p->deflinenum);
        /* Separator */
        fprintf(stdout, " | ");
        /* Print reference line numbers */
        for (q = p->irefp; q != NULL; q = q->nextlinep) {
            fprintf(stdout, "%d", q->reflinenum);
            fprintf(stdout, "%s", q->nextlinep == NULL ? "" : ",");
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "--------------------------------------------------------------------------\n");
    return;
}
static struct ID *find_id_in_table(struct ID **root, char *name, char *procname) {
    struct ID *p;

    for (p = *root; p != NULL; p = p->nextp) {
        if (strcmp(name, p->name) == 0) {
            /* If both name and p->name are global (procname and p->procname are NULL) */
            if (procname == NULL && p->procname == NULL) {
                return (p);
            }
            /* If both name and p->name are local (procname and p->procname are not NULL) */
            else if (procname != NULL && p->procname != NULL && strcmp(procname, p->procname) == 0) {
                return (p);
            }
        }
    }
    return (NULL);
}

static int append_type_to_param_list(struct ID **root, char *procname, struct TYPE **type) {
    struct ID *p_id;
    struct TYPE *p_paratp;
    /* Search for the procedure name in the symbol table */
    if ((p_id = find_id_in_table(root, procname, NULL)) == NULL) {
        fprintf(stderr, "'%s' not found.", procname);
        return error("Procedure name not found.");
    }
    /* Move to the end of the parameter list */
    p_paratp = p_id->itp;
    while (p_paratp->paratp != NULL) {
        p_paratp = p_paratp->paratp;
    }
    /* Add the new type to the parameter list */
    if ((p_paratp->paratp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
        error("Memory allocation failed for new parameter type.");
        return ERROR;
    }
    p_paratp->paratp->ttype = (*type)->ttype;
    p_paratp->paratp->arraysize = (*type)->arraysize;

    /* If the parameter type is an array, allocate memory for the element type */
    if ((*type)->ttype & TPARRAY) {
        struct TYPE *p_etype;
        if ((p_etype = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
            return error("Memory allocation failed for array element type.");
        }
        p_etype->ttype = (*type)->ttype;
        p_etype->arraysize = (*type)->arraysize;
        /* Element type must be a standard type (etp == NULL) */
        p_etype->etp = NULL;
        /* Element type doesn't have a parameter list */
        p_etype->paratp = NULL;
        /* Register the element type */
        p_paratp->paratp->etp = p_etype;
    } else {
        /* Standard type doesn't have an element type */
        p_paratp->paratp->etp = NULL;
    }
    /* Next pointer of the parameter list is NULL */
    p_paratp->paratp->paratp = NULL;

    return 0;
}

static int register_id_in_table(struct ID **root, char *name, char *procname, struct TYPE **type, int ispara, int deflinenum) {
    struct ID *p_id;
    struct ID *p_id_tail;
    struct TYPE *p_type;
    char *p_name;
    char *p_procname;

    if ((p_id = find_id_in_table(root, name, procname)) != NULL) {
        fprintf(stderr, "multiple definition of '%s'.\n", name);
        return error("Multiple definition detected");
    }

    /* Allocate memory for struct ID */
    if ((p_id = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
        return error("Memory allocation failed for struct ID");
    }

    /* Allocate memory for ID name */
    if ((p_name = (char *)malloc(strlen(name) + 1)) == NULL) {
        return error("Memory allocation failed for ID name");
    }
    strcpy(p_name, name);
    p_id->name = p_name;

    /* Allocate memory for procedure name if it's a local ID */
    if (procname == NULL) {
        p_id->procname = NULL; /* Global ID */
    } else {
        if ((p_procname = (char *)malloc(strlen(procname) + 1)) == NULL) {
            return error("Memory allocation failed for procedure name");
        }
        strcpy(p_procname, procname);
        p_id->procname = p_procname;
    }

    /* Allocate memory for ID type if provided */
    if (type == NULL) {
        p_id->itp = NULL; /* ID without type */
    } else {
        if ((p_type = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
            return error("Memory allocation failed for struct TYPE");
        }
        p_type->ttype = (*type)->ttype;
        p_type->arraysize = (*type)->arraysize;

        /* Allocate memory for element type if ID type is an array */
        if ((*type)->ttype & TPARRAY) {
            struct TYPE *p_etype;
            if ((p_etype = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
                return error("Memory allocation failed for array element type");
            }
            p_etype->ttype = (*type)->ttype;
            p_etype->arraysize = (*type)->arraysize;
            p_etype->etp = NULL; /* Element type must be standard type */
            p_etype->paratp = NULL;
            p_type->etp = p_etype; /* Register element type */
        } else {
            p_type->etp = NULL; /* Standard type or procedure */
        }
        p_type->paratp = NULL;
        p_id->itp = p_type;
    }
    p_id->ispara = ispara;
    p_id->deflinenum = deflinenum;
    p_id->irefp = NULL;
    p_id->nextp = NULL;

    /* Add the new ID to the end of the list */
    p_id_tail = *root;
    if (p_id_tail == NULL) {
        *root = p_id;
    } else {
        while (p_id_tail->nextp != NULL) {
            p_id_tail = p_id_tail->nextp;
        }
        p_id_tail->nextp = p_id;
    }

    return 0;
}

static int append_id_to_crtab(struct ID *root) {
    struct ID *p;
    int ret;

    for (p = root; p != NULL; p = p->nextp) {
        char *name = p->name;
        char *current_proc = p->procname;
        int ispara = p->ispara;
        int deflinenum = p->deflinenum;
        struct TYPE *type = p->itp;
        ret = register_id_in_table(&crtabroot, name, current_proc, &type, ispara, deflinenum);
        if (ret == ERROR)
            return ERROR;
    }
    return 0;
}

/* Free the memory allocated for the ID structure */
static void release_id_struct(struct ID **root) {
    struct ID *p, *q;

    for (p = *root; p != NULL; p = q) {
        free(p->name); /* Free the memory allocated for the ID name */
        free(p->procname); /* Free the memory allocated for the procedure name */
        release_type_struct(p->itp); /* Free the memory allocated for the type structure */
        free(p->irefp); /* Free the memory allocated for the reference lines */
        q = p->nextp; /* Move to the next ID in the list */
        free(p); /* Free the memory allocated for the current ID */
    }
    *root = NULL; /* Set the root pointer to NULL */
    return;
}

static void release_type_struct(struct TYPE *root) {
    struct TYPE *p, *q;

    if (root == NULL) {
        return;
    }

    p = root;

    free(p->etp);
    for (p = root->paratp; p != NULL; p = q) {
        q = p->paratp;
        free(p);
    }
    root = NULL;
    return;
}