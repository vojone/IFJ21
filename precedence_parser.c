/******************************************************************************
 *                                  IFJ21
 *                            precedence_parser.c
 * 
 *          Authors: Vojtěch Dvořák (xdvora3o), Juraj Dědič (xdedic07)
 *              Purpose:  Implementation of precedence parsing (PP)
 * 
 *                       Last change: 25. 11. 2021
 *****************************************************************************/

#include "precedence_parser.h"

DSTACK(expr_el_t, pp, fprintf(stderr," %d", s->data[i].type)) /**< Operations with stack, that saves elements from expr. */

/**
 * @brief Determines if is token separator, that can be used in expressions
 */ 
bool is_allowed_separator(scanner_t *sc, token_t *token) {
    return token->token_type == SEPARATOR && 
           (str_cmp(get_attr(token, sc), ")") == 0 || 
           str_cmp(get_attr(token, sc), "(") == 0);
}

/**
 * @brief Determines whether is token nil keyword or not
 */ 
bool is_nil(scanner_t *sc, token_t *token) {
    return token->token_type == KEYWORD && 
           (str_cmp(get_attr(token, sc), "nil") == 0);
}

/**
 * @brief Determines if operand is zero
 */ 
bool is_zero(scanner_t *sc, token_t *token) {
    char *value = get_attr(token, sc);
    if(token->token_type == INTEGER || token->token_type == NUMBER) {
        for(int i = 0; i < strlen(value); i++) {
            if(value[i] != '0' && value[i] != '.') {
                return false;
            }
        }
    }
    else if(token->token_type == STRING) {
        for(int i = 0; i < strlen(value); i++) {
            if(value[i] != '"') {
                return false;
            }
        }
    }
    else {
        return false;
    }

    return true;
}

/**
 * @brief Resolves if token can be part of expression
 */ 
bool is_EOE(scanner_t *sc, token_t *token) {
    token_type_t type = token->token_type;

    if(type == IDENTIFIER || type == OPERATOR ||
       is_allowed_separator(sc, token) || type == STRING ||
       type == NUMBER || type == INTEGER || is_nil(sc, token)) {
        return false;
    }
    else {
        return true;
    }
}

/**
 * @brief Tries to determine if current token is unary or binary minus operator (due to last_token)
 */ 
bool is_unary_minus(tok_buffer_t *tok_b) {
    return  tok_b->last.token_type == UNKNOWN || 
            str_cmp(get_attr(&tok_b->last, tok_b->scanner), "(") == 0 ||
            tok_b->last.token_type == OPERATOR;
}


/**
 * @brief Resolves operator type
 */ 
grm_sym_type_t operator_type(char first_c, char sec_c, tok_buffer_t *tok_b) {
    switch (first_c)
    {
        case '#':
            return HASH;
        case '*':
            return MULT;
        case '-':
            if(is_unary_minus(tok_b)) {
                return MINUS;
            }
            return SUB;
        case '/':
            if(sec_c == '/') {
                return INT_DIV;
            }
            return DIV;
        case '+':
            return ADD;
        case '<':
            if(sec_c == '=') {
                return LTE;
            }
            return LT;
        case '>':
            if(sec_c == '=') {
                return GTE;
            }
            return GT;
        case '=':
            if(sec_c == '=') {
                return EQ;
            }

            return UNDEFINED;
        case '~':
            return NOTEQ;
        case '(':
            return L_PAR;
        case ')':
            return R_PAR;
        case '.':
            return CONCAT;
        default:
            return UNDEFINED;
    }   //switch(first_ch)
}

/**
 * @brief Transforms token to symbol used in precedence parser (see expr_el_t in .h)
 */ 
int tok_to_type(tok_buffer_t *tok_b) {
    token_t token = tok_b->current;

    if(token.token_type == IDENTIFIER || token.token_type == NUMBER ||
       token.token_type == STRING || token.token_type == INTEGER || 
       is_nil(tok_b->scanner, &tok_b->current)) {
           return OPERAND;
    }
    else if(token.token_type == OPERATOR || token.token_type == SEPARATOR) {

        int char_num = 0;
        char first_ch = (get_attr(&token, tok_b->scanner))[char_num++],
        next_ch = (get_attr(&token, tok_b->scanner))[char_num];

        return operator_type(first_ch, next_ch, tok_b);
    }

    return UNDEFINED;
}


/**
 * @brief Makes dynamic string from given character
 */ 
void make_type_str(string_t *dst, char type_c) {
    str_clear(dst);
    app_char(type_c, dst);
}


/**
 * @brief Returns primary type of return types (first return type)
 * @note When we use function in expression we will work only with primary return type in the rest of expr.
 */ 
sym_dtype_t prim_type(string_t *type_string) {
    return char_to_dtype(to_str(type_string)[0]);
}


/**
 * @brief Returns true if two types are compatible
 */ 
bool is_compatible_type_c(string_t *dtypes, char type_char) {
    return (prim_type(dtypes) == char_to_dtype(type_char) ||
            (prim_type(dtypes) == INT && char_to_dtype(type_char) == NUM) ||
            (char_to_dtype(type_char) == INT && prim_type(dtypes) == NUM));
}

/**
 * @brief Checks if expected token type end token type of fiven token are equal
 */
bool is_tok_type(token_type_t exp_type, token_t *t) {
    return t->token_type == exp_type;
}


/**
 * @brief Checks whether expected token attribute end token attribute of fiven token are equal
 */
bool is_tok_attr(char *exp_attr, token_t *t, tok_buffer_t *tok_b) {
    return str_cmp(get_attr(t, tok_b->scanner), exp_attr) == 0;
}


/**
 * @brief Prints error msg to stderr
 */ 
void fcall_sem_error(tok_buffer_t *tok_b, token_t *func_id, char *msg) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSemantic error:\033[0m ", 
            tok_b->scanner->cursor_pos[ROW], tok_b->scanner->cursor_pos[COL]);

    fprintf(stderr, "Bad function call of \033[1;33m%s\033[0m! ", get_attr(func_id, tok_b->scanner));
    fprintf(stderr, "%s\n", msg);
}


/**
 * @brief Prints error msg to stderr
 */ 
void fcall_syn_error(tok_buffer_t *tok_b, token_t *func_id, char *msg) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSyntax error:\033[0m ", 
            tok_b->scanner->cursor_pos[ROW], tok_b->scanner->cursor_pos[COL]);

    fprintf(stderr, "In function call of \033[1;33m%s\033[0m! ", get_attr(func_id, tok_b->scanner));
    fprintf(stderr, "%s\n", msg);
}


/**
 * @brief Parses arguments in function call in expression
 */ 
int argument_parser(token_t *func_id, char *params_s, 
                    symbol_tables_t *syms, tok_buffer_t *tok_b) {

    size_t argument_cnt = 0;             
    bool closing_bracket = false;
    bool is_variadic = (params_s[0] == '%') ? true : false; //In our case variadic means - with variable AMOUNT and TYPES of arguments
    while(!closing_bracket) {
        token_aging(tok_b);

        token_t t = lookahead(tok_b->scanner);
        if(t.token_type == ERROR_TYPE) {
            return LEXICAL_ERROR;
        }
        else if(!is_EOE(tok_b->scanner, &t) && !is_tok_attr(")", &t, tok_b)) {
            
            string_t ret_type;
            str_init(&ret_type);
        
            int expr_retval = parse_expression(tok_b->scanner, syms, &ret_type);
            if(expr_retval != EXPRESSION_SUCCESS) {
                str_dtor(&ret_type);
                return -expr_retval; /**< Negative return code means "Propagate it, but don't write err msg "*/
            }

            if(!is_variadic) {
                if(!is_compatible_type_c(&ret_type, params_s[argument_cnt])) {
                    fcall_sem_error(tok_b, func_id, "Bad data types of arguments!");
                    str_dtor(&ret_type);
                    return SEMANTIC_ERROR_PARAMETERS_EXPR;
                }
                else {
                    //Parameter is ok
                }
            }

            str_dtor(&ret_type);

            argument_cnt++;
        }
        else if(is_tok_type(SEPARATOR, &t) && is_tok_attr(")", &t, tok_b)) {
            token_aging(tok_b);
            closing_bracket = true;
            continue;
        }
        else {
            fcall_syn_error(tok_b, func_id, "Missing ')' after function arguments!\n");
            return SYNTAX_ERROR_IN_EXPR;
        }

        //Check if there will be next argument
        t = lookahead(tok_b->scanner);
        if(t.token_type == ERROR_TYPE) {
            return LEXICAL_ERROR;
        }
        else if(is_tok_type(SEPARATOR, &t) && is_tok_attr(",", &t, tok_b)) {
            if(argument_cnt + 1 > strlen(params_s) && !is_variadic) { //Function needs less arguments
                fcall_sem_error(tok_b, func_id, "Too many arguments!");
                return SEMANTIC_ERROR_PARAMETERS_EXPR;
            }
            else {
                //Ok
            }
        }
        else if(is_tok_type(SEPARATOR, &t) && is_tok_attr(")", &t, tok_b)) {
            closing_bracket = true;
            if((argument_cnt < strlen(params_s)) && !is_variadic) { //Function needs more arguments
                fcall_sem_error(tok_b, func_id, "Missing arguments!");
                return SEMANTIC_ERROR_PARAMETERS_EXPR;
            }
        }
        else {
            fcall_syn_error(tok_b, func_id, "Missing ',' or ')' between arguments!\n");
            return SYNTAX_ERROR_IN_EXPR;
        }
    }
    if(argument_cnt == 0 && strlen(params_s) > 0) { //Function needs arguments but there aren't any
        fcall_sem_error(tok_b, func_id, "Missing arguments!");
        return SEMANTIC_ERROR_PARAMETERS_EXPR;
    }


    return EXPRESSION_SUCCESS;
}


/**
 * @brief Parses function call inside expression
 * @return EXPRESSION SUCCESS if wverything was ok
 */ 
int fcall_parser(tree_node_t *symbol, 
                 symbol_tables_t *syms, 
                 tok_buffer_t *tok_b) {

    token_t func_id = tok_b->current;
    char *params_s = to_str(&symbol->data.params);

    token_aging(tok_b);

    token_t t = lookahead(tok_b->scanner);
    if(t.token_type == ERROR_TYPE) {
        return LEXICAL_ERROR;
    }
    else if(!is_tok_type(SEPARATOR, &t) || !is_tok_attr("(", &t, tok_b)) {
        tok_b->current = t;
        fcall_syn_error(tok_b, &func_id, "Missing '(' after function indentifier!\n");
        return SYNTAX_ERROR_IN_EXPR;
    }
    else {
        int retval = argument_parser(&func_id, params_s, syms, tok_b);

        if(retval != EXPRESSION_SUCCESS) {
            return retval;
        }
    }

    return EXPRESSION_SUCCESS;
}


/**
 * @brief Processes identifier got on input
 */
int process_identifier(expr_el_t *result, 
                       tok_buffer_t *tok_b,
                       symbol_tables_t *syms) { 

    char *id_name = get_attr(&(tok_b->current), tok_b->scanner);

    tree_node_t *symbol;
    symbol = search_in_tables(&syms->symtab_st, &syms->symtab, id_name);

    if(symbol == NULL) {
        //Check if it is builtin function
        check_builtin(id_name, &syms->global);

        symbol = search(&syms->global, id_name);
        if(symbol == NULL) { //Symbol was not found in variables nor in global table with functions
            return UNDECLARED_IDENTIFIER;
        }
        else {
            str_cpy((char **)&result->value, id_name, strlen(id_name));

            int retval = fcall_parser(symbol, syms, tok_b); //Process function call and arguments
            if(retval != EXPRESSION_SUCCESS) {
                return retval;
            }

            //Function was succesfully called
            cpy_strings(&result->dtype, &(symbol->data.ret_types));

            return EXPRESSION_SUCCESS;
        }
    }
    else {
        char type_c = dtype_to_char(symbol->data.dtype);
        make_type_str(&result->dtype, type_c);

        return EXPRESSION_SUCCESS;
    }
}


/**
 * @brief Creates expression element from current and last token
 */ 
int from_input_token(expr_el_t *result, 
                     tok_buffer_t *tok_b,
                     symbol_tables_t *syms,
                     bool *was_operand) {

    result->type = tok_to_type(tok_b);
    result->value = NULL;
    result->is_zero = false;
    str_init(&result->dtype);
    int retval = EXPRESSION_SUCCESS;
    switch (tok_b->current.token_type)
    {
    case INTEGER:
        make_type_str(&result->dtype, 'i');
        *was_operand = true;
        break;
    case NUMBER:
        make_type_str(&result->dtype, 'n');
        *was_operand = true;
        break;
    case STRING:
        make_type_str(&result->dtype, 's');
        *was_operand = true;
        break;
    case IDENTIFIER:
        if(!*was_operand) {
            retval = process_identifier(result, tok_b, syms);
            if(retval != EXPRESSION_SUCCESS) {
                return retval;
            }
            else {
                *was_operand = true;
            }
        }

        break;
    default:
        if(is_nil(tok_b->scanner, &tok_b->current)) {
            make_type_str(&result->dtype, 'z');
        }
        else {
             make_type_str(&result->dtype, ' ');
        }

        *was_operand = false;

        break;
    }

    if(is_zero(tok_b->scanner, &tok_b->current) && PREVENT_ZERO_DIV) {
        result->is_zero = true;
    }
    if(result->value == NULL) {
        char * curr_val = get_attr(&(tok_b->current), tok_b->scanner);
        str_cpy((char **)&result->value, curr_val, strlen(curr_val));
    }

    return EXPRESSION_SUCCESS;
}


/**
 * @brief Creates stop symbol and initializes it
 */ 
expr_el_t stop_symbol() {
    expr_el_t stop_symbol = {
        .type = STOP_SYM, 
        .dtype = {
            .alloc_size = 0, 
            .length = 0, 
            .str = NULL
        }, 
        .value = NULL, 
        .is_zero = false
    };

    return stop_symbol;
}

/**
 * @brief Creates precedence sign due to given parameter and initializes it
 */ 
expr_el_t prec_sign(char sign) {
    expr_el_t precedence_sign = {
        .dtype = {
            .alloc_size = 0, 
            .length = 0, 
            .str = NULL
        }
    };

    switch(sign) {
        case '<':
            precedence_sign.type = '<';
            break;
        case '>':
            precedence_sign.type = '>';
            break;
        case '=':
            precedence_sign.type = '=';
            break;
        default:
            precedence_sign.type = '<';
            break;
    }

    precedence_sign.value = NULL;
    precedence_sign.is_zero = false;

    return precedence_sign;
}

/**
 * @brief Contains precedence table of operators in IFJ21
 */ 
char get_precedence(expr_el_t on_stack_top, expr_el_t on_input) {
    static char precedence_table[TERM_NUM][TERM_NUM] = {
    //   #    _   *   /   //  +   -  ..   <  <=  >  >=  ==  ~=   (   )   i   $
/*#*/  {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'}, 
/*_*/  {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/***/  {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*/*/  {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*//*/ {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*+*/  {'<','<','<','<','<','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*-*/  {'<','<','<','<','<','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*..*/ {'<','<','<','<','<','<','<','>','>','>','>','>','>','>','<','>','<','>'},
/*<*/  {'<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*<=*/ {'<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*>*/  {'<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*>=*/ {'<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*==*/ {'<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*~=*/ {'<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*(*/  {'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','=','<',' '},
/*)*/  {'>','>','>','>','>','>','>','>','>','>','>','>','>','>',' ','>',' ','>'},
/*i*/  {'>','>','>','>','>','>','>','>','>','>','>','>','>','>',' ','>',' ','>'},
/*$*/  {'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<',' ','<',' '}
    };

    if(on_stack_top.type >= TERM_NUM || on_input.type >= TERM_NUM) {
        return ' ';
    }

    return precedence_table[on_stack_top.type][on_input.type];
}

/**
 * @brief Additional function to get real expression element from enum type
 */ 
char *to_char_sequence(expr_el_t expression_element) {
    static char * cher_seq[] = {
        "#", "_", "*", "/", "//", "+", "-", "..", "<", "<=", 
        ">", ">=", "==", "~=", "(", ")", "i", "$", "E", NULL
    };

    return cher_seq[expression_element.type];
}

/**
 * @brief Contains rules of reduction on top of the pp_stack, operand possibilities and return data type of reduction
 * @note For explaning possible operands is used specific string:
 *       | separates possibilities for operands
 *       * every data type
 *       n number
 *       i integer
 *       s string
 *       z nil
 *       b bool
 *       ! inicates that cannot be zero
 *       ( types symbols after it defines type for resting operators (integer and number are compatible)
 *       ) returns from "must_be" mode to normal mode 
 *       Example: (nis|nis = First and sec operand can be number/integer/string and if it's e. g. string second must be string too
 */ 
expr_rule_t *get_rule(unsigned int index) {
    if(index >= REDUCTION_RULES_NUM) { //Safety check
        return NULL;
    }

    static expr_rule_t rules[REDUCTION_RULES_NUM] = {
        {"(E)", "*", ORIGIN, FIRST ,NULL,NULL},
        {"i", "*", ORIGIN, FIRST, NULL,NULL},
        {"E+E", "ni|ni", ORIGIN, ALL, "\"+\" expects number/integer as operands", generate_operation_add},
        {"E-E", "ni|ni", ORIGIN, ALL, "\"-\" expects number/integer as operands", generate_operation_sub},
        {"E*E", "ni|ni", ORIGIN, ONE, "\"*\" expects number/integer as operands", generate_operation_mul},
        {"E/E", "ni|!ni", ORIGIN, FIRST, "\"/\" expects number/integer as operands", generate_operation_div},
        {"E//E", "ni|!ni", INT, FIRST, "\"//\" expects number/integer as operands", generate_operation_idiv},
        {"_E", "ni", ORIGIN, FIRST, "Unary minus expects number/integer as operands", NULL},
        {"#E", "s", INT, NONE, "Only string can be operand of \"#\"", generate_operation_strlen},
        {"E<E", "(nis|nis", BOOL, NONE, "Incompatible operands of \"<\"", generate_operation_lt},
        {"E>E", "(nis|nis", BOOL, NONE, "Incompatible operands of \">\"", generate_operation_gt},
        {"E<=E", "(nis|nis", BOOL, NONE, "Incompatible operands of \"<=\"", generate_operation_lte},
        {"E>=E", "(nis|nis", BOOL, NONE, "Incompatible operands of \">=\"", generate_operation_gte},
        {"E==E", "z(nis|nis)z", BOOL, NONE, "Incompatible operands of \"==\"", generate_operation_eq},
        {"E~=E", "z(nis|nis)z", BOOL, NONE, "Incompatible operands of \"~=\"", generate_operation_eq},
        {"E..E", "s|s", STR, NONE, "Operation \"..\" needs strings as operands", generate_operation_concat},
    };

    return &(rules[index]);
}

/**
 * @brief Pops operand stack if it is not empty
 */ 
expr_el_t safe_op_pop(bool *cur_ok, int *check_result, pp_stack_t *op_stack) {
    if(!pp_is_empty(op_stack)) {
        *cur_ok = false;
        return pp_pop(op_stack);
    }
    else {
        *cur_ok = false;
        *check_result = INTERNAL_ERROR;
        return stop_symbol();
    }
}

/**
 * @brief Returns true if two types are compatible
 */ 
bool is_compatible(string_t *dtypes1, string_t *dtypes2) {
    return ((dstring_cmp(dtypes1, dtypes2) == 0) ||
            (prim_type(dtypes1) == INT && prim_type(dtypes2) == NUM) ||
            (prim_type(dtypes2) == INT && prim_type(dtypes1) == NUM));
}

/**
 * @brief Resolves which data type attribut should have newly created nonterminal on stack
 */ 
void resolve_res_type(string_t *res, expr_rule_t *rule, 
                      expr_el_t cur_op, bool cur_ok) {

    if(prim_type(res) == UNDEFINED) {
        if(cur_ok) {
            cpy_strings(res, &cur_op.dtype);
        }
    }
    
    if(rule->return_type != ORIGIN) {
        str_clear(res);
        app_char(dtype_to_char(rule->return_type), res);
    }
    else if(prim_type(&(cur_op.dtype)) == NUM && prim_type(res) == INT) {
        str_clear(res);
        app_char(dtype_to_char(NUM), res);
    }
}



int get_tcheck_ret(expr_el_t *current_operand) {
    int result;
    if(prim_type(&current_operand->dtype) == NIL && PREVENT_NIL) {
        result = NIL_ERROR;
    }
    else {
        result = SEM_ERROR_IN_EXPR;
    }

    return result;
}

/**
 * @brief Performs type checking when precedence parser reducing the top of the stack
 * @note Type check is based on rules writen in get_rule() ( @see get_rule())
 */ 
int type_check(pp_stack_t op_stack, expr_rule_t *rule, string_t *res_type) {

    expr_el_t current = pp_pop(&op_stack);
    bool is_curr_ok = false, must_be_flag = false;
    int result = EXPRESSION_SUCCESS;

    string_t tmp_res_type;
    str_init(&tmp_res_type);
    app_char(dtype_to_char(rule->return_type), &tmp_res_type);

    string_t must_be;
    str_init(&must_be);
    app_char(' ', &must_be);

    char c;
    for(int i = 0; (c = rule->operator_types[i]) != '\0' && !result; i++) {
        if(c == '|') { /**< Next operator symbol */
            if(!is_curr_ok) { /**< Specifier for current operator was not found -> error */
                result = get_tcheck_ret(&current);
                break;
            }
            else { /**< Specifier was found -> next check operator*/
                current = safe_op_pop(&is_curr_ok, &result, &op_stack);
            }
        }
        else if(c == '(') { /**< Sets type checcker to must_be mode */
            if(!is_curr_ok) {
                must_be_flag = true;
            }
        }
        else if(c == ')') { /**< Unsets must_be mode */
            must_be_flag = false;
        }
        else if(c == '!') {
            if(current.is_zero) {
                result = DIV_BY_ZERO;
                break;
            }
        }
        else { /**< Current char is type specifier */
            if(must_be_flag) { /**< Must_be mode is set */
                if(prim_type(&must_be) == UNDEFINED && 
                   prim_type(&(current.dtype)) == char_to_dtype(c)) {

                    cpy_strings(&must_be, &(current.dtype));
                }

                if(is_compatible(&(current.dtype), &must_be)) { /**< Type of current operator is compatible with type of earlier operator */
                    is_curr_ok = true;
                }
            }
            else { /**< Normal mode */
                if(c == '*' || prim_type(&(current.dtype)) == char_to_dtype(c)) {
                    is_curr_ok = true; //Operand type corresponds with operator specifier
                }
            }
        }

        resolve_res_type(&tmp_res_type, rule, current, is_curr_ok);
    }

    // Specifier string ended -> if specifier for last operator was not found -> error
    result = (!is_curr_ok && c == '\0') ?  get_tcheck_ret(&current) : result;

    str_dtor(&must_be);
    cpy_strings(res_type, &tmp_res_type);
    str_dtor(&tmp_res_type);

    return result;
}

/**
 * @brief Gets from top of the stack sequence that will be reduced
 */ 
void get_str_to_reduction(pp_stack_t *s, pp_stack_t *op, string_t *to_be_red) {
    expr_el_t from_top;
    while((from_top = pp_top(s)).type != STOP_SYM) {
        if(from_top.type == '<') {
            pp_pop(s);
            break;
        }

        if(from_top.type == NON_TERM || from_top.type == OPERAND) {
            pp_push(op, from_top);
        }

        char *char_seq = to_char_sequence(from_top);
        prep_str(to_be_red, char_seq);
        pp_pop(s);
    }

    //fprintf(stderr, "To be reduced: %s\n", to_be_red->str);
}


void print_operands(pp_stack_t *ops, symtabs_stack_t *sym_stack, 
                    symtab_t *symtab) {

    while(!pp_is_empty(ops)) {
        expr_el_t cur = pp_pop(ops);
        //fprintf(stderr, "Operand: %s (is zero: %d, data_type: %s)\n", (char *)cur.value, cur.is_zero, to_str(&cur.dtype));

        tree_node_t *symbol = search_in_tables(sym_stack, symtab, (char *)cur.value);
        if(symbol) {
            //fprintf(stderr, "Operand name: %s\n", to_str(&symbol->data.name));
        }
    }
}

/**
 * @brief Determines if result of reduction will be zero value (in some cases it is possible to find out it if is)
 */ 
bool resolve_res_zero(pp_stack_t operands, expr_rule_t *rule) {
    switch(rule->zero_prop)
    {
    case NONE:
        return false;
        break;
    case FIRST:
        if(!pp_is_empty(&operands) && pp_pop(&operands).is_zero) { //Check if first operand is zero if it is -> result is zero
            return true;
        }
        break;
    case SECOND:
        if(!pp_is_empty(&operands)) {
            pp_pop(&operands);
            if(!pp_is_empty(&operands) && pp_pop(&operands).is_zero) {
                return true;
            }
        }
        break;
    case ALL:
        while(!pp_is_empty(&operands)) {
            if(!pp_pop(&operands).is_zero) {
                return false;
            }
        }

        return true;
        break;
    case ONE:
        while(!pp_is_empty(&operands)) {
            if(pp_pop(&operands).is_zero) {
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

/**
 * @brief Creates non-terminal (E) expression symbol (there is only one non-terminal in rules)
 */ 
expr_el_t non_term(string_t *data_type, bool is_zero) {
    expr_el_t non_terminal;
    non_terminal.type = NON_TERM;

    str_init(&(non_terminal.dtype));
    cpy_strings(&(non_terminal.dtype), data_type);

    non_terminal.value = "NONTERM";
    non_terminal.is_zero = is_zero;

    return non_terminal;
}

/**
 * @brief Makes reduction of top of the main stack including type check and resolving errors
 * @param st Main stack where reductions are performed
 * @param ops Stack with operands
 * @param rule Rule containing information about result type and operand data types
 * @param err_m Pointer will be set to the string that explains semantic error in expression (if occured)
 */ 
//TODO pushing i only works for static values
//TODO we need to know wheter it is static value or a variable 
int reduce(pp_stack_t *st, pp_stack_t ops, 
           symbol_tables_t *syms,
           expr_rule_t *rule,
           string_t *result_type,
           bool will_be_zero,
           pp_stack_t *garbage) {
    
    if(strcmp(rule->right_side, "i") == 0) {
        expr_el_t element_terminal = pp_top(&ops);
        tree_node_t *res = search_in_tables(&syms->symtab_st, &syms->symtab, element_terminal.value);
        if(res == NULL) {
            sym_dtype_t dtype = char_to_dtype(to_str(&element_terminal.dtype)[0]);
            //We are pushing a static value
            generate_value_push(VAL, dtype, element_terminal.value);
        }
        else{
            //We are pushing variable
            //fprintf(stderr,"Pushing variable %s to stack\n", res->data.name.str);
            generate_value_push(VAR, res->data.dtype , res->data.name.str);
        }
    }

    //Generate operation code
    if(rule->generator_function != NULL)
        rule->generator_function();

    print_operands(&ops, &syms->symtab_st, &syms->symtab); /**< It will be probably substituted for code generating */

    if(!pp_push(st, non_term(result_type, will_be_zero))) { /**< Make non terminal at the top of main stack */
        return INTERNAL_ERROR;
    }
    if(!pp_push(garbage, pp_top(st))) { /**< Add nonterminal to garbage collector stack */
        return INTERNAL_ERROR;
    }

    return EXPRESSION_SUCCESS;
}

/**
 * @brief Tries to reduce top of stack to non_terminal due to rules in get_rule()
 */ 
int reduce_top(pp_stack_t *s, symbol_tables_t *symtabs,
               char ** failed_op_msg, string_t *ret_types, 
               pp_stack_t *garbage) {

    string_t to_be_reduced;
    str_init(&to_be_reduced);

    pp_stack_t operands;
    if(!pp_stack_init(&operands)) {
        return INTERNAL_ERROR;
    }

    get_str_to_reduction(s, &operands, &to_be_reduced); /**< Takes top of the stack and creates substitutable string from it*/
    
    expr_rule_t *rule;
    int retval = EXPRESSION_FAILURE; /**< If rule is not found it is invalid operation -> return EXPR_FAILURE */
    for(int i = 0; (rule = get_rule(i)); i++) {
        if(str_cmp(to_str(&to_be_reduced), rule->right_side) == 0) {
            int t_check_res = type_check(operands, rule, ret_types);

            //fprintf(stderr, "Ret. types: %s Retval:%d\n", to_str(ret_types), t_check_res);

            if(t_check_res != EXPRESSION_SUCCESS) { /**< Type check was not succesfull */
                *failed_op_msg = rule->error_message;
                retval = t_check_res;
            }
            else {
                bool will_be_zero = resolve_res_zero(operands, rule);
                retval = reduce(s, operands, symtabs, 
                                rule, ret_types, will_be_zero,
                                garbage);
            }

        }
    }

    pp_stack_dtor(&operands);
    str_dtor(&to_be_reduced);

    return retval;
}


/**
 * @brief Gets symbol from input (if it is valid as expression element otherwise is set to STOP_SYM)
 * @note Symbol on input adds to garbage stack to be freed at the end of expression parsing
 */ 
int get_input_symbol(bool stop_flag, expr_el_t *on_input, 
                     tok_buffer_t *t_buff, symbol_tables_t *symtabs,
                     pp_stack_t *garbage_stack, bool *was_operand) {

    int retval = EXPRESSION_SUCCESS;
    if(stop_flag || is_EOE(t_buff->scanner, &(t_buff->current))) {
        *on_input = stop_symbol();
    }
    else {
        retval = from_input_token(on_input, t_buff, symtabs, was_operand);
        pp_push(garbage_stack, *on_input);
    }

    return retval;
}

/**
 * @brief Gets first nonterminal from top of the stack (There can't be sequence of them) 
 */ 
void get_top_symbol(expr_el_t *on_top, pp_stack_t *stack) {
    expr_el_t on_top_tmp = pp_top(stack);
    expr_el_t tmp;

    if(on_top_tmp.type == NON_TERM) { //Ignore nonterminal if there is 
        tmp = pp_pop(stack);
        on_top_tmp = pp_top(stack);
        pp_push(stack, tmp);
    }

    *on_top = on_top_tmp;
}

/**
 * @brief Frees all resources (expecially dynamic allocated memory) of PP
 */ 
void free_everything(pp_stack_t *stack, pp_stack_t *garbage) {
    while(!pp_is_empty(garbage))
    {
        expr_el_t current_el = pp_pop(garbage);
        str_dtor(&(current_el.dtype));

        if(current_el.type != NON_TERM) {
            free(current_el.value);
        }
    }
    
    pp_stack_dtor(garbage);
    pp_stack_dtor(stack);
}


void token_aging(tok_buffer_t *token_buffer) {
    token_buffer->last = token_buffer->current;
    token_buffer->current = get_next_token(token_buffer->scanner);
}

/**
 * @brief Does necessary actions when stack top has lower precedence than input
 */ 
void has_lower_prec(pp_stack_t *stack, expr_el_t on_input) {
    expr_el_t top = pp_top(stack);
    if(top.type == NON_TERM) {
        pp_pop(stack);
        pp_push(stack, prec_sign('<'));
        pp_push(stack, top);
    }
    else {
        pp_push(stack, prec_sign('<'));
    }

    pp_push(stack, on_input);
}


/**
 * @brief Prints error message due to return value
 * @note Also can corrects return code to expected value
 */ 
void print_err_message(int *return_value, 
                       tok_buffer_t *token_buffer, 
                       char **err_m) {

    pos_t r = token_buffer->scanner->cursor_pos[ROW];
    pos_t c = token_buffer->scanner->cursor_pos[COL];
    char * attr = get_attr(&(token_buffer->current), token_buffer->scanner);
    char * lattr = get_attr(&(token_buffer->last), token_buffer->scanner);

    switch(*return_value)
    {
    case LEXICAL_ERROR:
        break;

    case SEM_ERROR_IN_EXPR:
        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSemantic error:\033[0m ", r, c);
        fprintf(stderr, "Bad data types in expression!\n");
        if(*err_m) {
            fprintf(stderr, "\t| \033[0;33m%s\033[0m\n", *err_m);
        }

        break;

    case UNDECLARED_IDENTIFIER:
        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSemantic error:\033[0m ", r, c);
        fprintf(stderr, "Undeclared identifier \"\033[1;33m%s\033[0m\"!\n", attr);
        break;

    case NIL_ERROR:
        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSemantic (nil) error:\033[0m ", r, c);
        fprintf(stderr, "Cannot use nil in expression like this!\n");
        break;
    
    case DIV_BY_ZERO:
        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mDivision by zero:\033[0m ", r, c);
        fprintf(stderr, "Cannot divide by zero!\n");
        break;

    case EXPRESSION_FAILURE:
        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSyntax error:\033[0m ", r, c);
        fprintf(stderr, "Invalid combination of tokens in expression! (\"\033[1;33m%s%s\033[0m\")\n", lattr, attr);
        *return_value = SYNTAX_ERROR_IN_EXPR;
        break;

    case MISSING_EXPRESSION:
        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSyntax error:\033[0m ", r, c);
        fprintf(stderr, "Expected expression, but it was not found! (found \"\033[1;33m%s\033[0m\" instead)\n", attr);
        *return_value = SYNTAX_ERROR_IN_EXPR;
        break;

    default:
        if(*return_value < 0) { //Indicates that only return code should be propagated without any msg
            *return_value = -(*return_value);
        }
        break;
    }
}


/**
 * @brief Inits all parts of auxiliary structure tok_buffer_t
 */ 
void prepare_buffer(scanner_t *sc, tok_buffer_t *tok_b) {
    tok_b->scanner = sc;
    token_init(&(tok_b->current));
    token_init(&(tok_b->last));
}

/**
 * @brief Prepare necessary stacks before their usage in precedence parser
 */ 
int prepare_stacks(pp_stack_t *main_stack, pp_stack_t *garbage_stack) {
    if(!pp_stack_init(main_stack) || !pp_stack_init(garbage_stack)) {
        return INTERNAL_ERROR;
    }

    if(!pp_push(main_stack, stop_symbol())) {
        return INTERNAL_ERROR;
    }

    return EXPRESSION_SUCCESS;
}


int parse_expression(scanner_t *sc, symbol_tables_t *s, string_t *dtypes) {
    int retval = EXPRESSION_SUCCESS;
    char *failed_op_msg = NULL;
    
    tok_buffer_t tok_buff;
    pp_stack_t stack;
    pp_stack_t garbage;
    
    prepare_buffer(sc, &tok_buff);
    retval = prepare_stacks(&stack, &garbage);
    if(retval != EXPRESSION_SUCCESS) {
        return retval;
    }
    
    bool stop_flag = false, empty_expr = true, 
         empty_cycle = false, was_operand = false;
    while(retval == EXPRESSION_SUCCESS) { //Main cycle
        tok_buff.current = lookahead(sc);

        if(tok_buff.current.token_type == ERROR_TYPE) {
            retval = LEXICAL_ERROR;
            break;
        }

        expr_el_t on_input, on_top;
        retval = get_input_symbol(stop_flag, &on_input, &tok_buff, 
                                  s, &garbage, &was_operand);

        if(retval != EXPRESSION_SUCCESS) {
            break;
        }

        get_top_symbol(&on_top, &stack);

        /*There is end of the expression on input and stop symbol at the top of the stack*/
        if(on_top.type == STOP_SYM && on_input.type == STOP_SYM) {
            retval = empty_expr ? MISSING_EXPRESSION : EXPRESSION_SUCCESS;
            break;
        }

        char precedence = get_precedence(on_top, on_input);
        //fprintf(stderr, "%s: %c %d(%d) %d(%d) Stop flag: %d\n", get_attr(&tok_buff.current, sc), precedence, on_top.type, on_top.is_zero, on_input.type, on_input.is_zero, stop_flag);
        if(precedence == '=') {
            if(!pp_push(&stack, on_input)) {
                retval = INTERNAL_ERROR;
            }

            token_aging(&tok_buff); /**< Make last token from current token */
            empty_cycle = false;
        }
        else if(precedence == '<') {  /**< Put input symbol to the top of the stack*/
            has_lower_prec(&stack, on_input);
            token_aging(&tok_buff);
            empty_cycle = false;
        }
        else if(precedence == '>') { /**< Basicaly, reduct while you can't put input symbol to the top of the stack*/
            retval = reduce_top(&stack, s, &failed_op_msg, dtypes, &garbage);
            empty_cycle = false;
        }
        else {
            stop_flag = true;

            if(stop_flag && empty_cycle) {
                retval = EXPRESSION_FAILURE;
                break;
            }

            empty_cycle = true;
        }

        empty_expr = false;
    }

    print_err_message(&retval, &tok_buff, &failed_op_msg);
    free_everything(&stack, &garbage);

    //token_t next = lookahead(sc); fprintf(stderr, "%s\n", get_attr(&next, sc));
    return retval;
}

/***                     End of precedence_parser.c                        ***/
