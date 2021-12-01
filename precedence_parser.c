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


bool is_allowed_separator(scanner_t *sc, token_t *token) {
    return token->token_type == SEPARATOR && 
           (str_cmp(get_attr(token, sc), ")") == 0 || 
           str_cmp(get_attr(token, sc), "(") == 0); //It must be left/right par
}


bool is_nil(scanner_t *sc, token_t *token) {
    return token->token_type == KEYWORD && 
           (str_cmp(get_attr(token, sc), "nil") == 0);
}


bool is_zero(scanner_t *sc, token_t *token) {
    char *value = get_attr(token, sc);
    if(token->token_type == INTEGER || token->token_type == NUMBER) {
        for(int i = 0; i < strlen(value); i++) {
            if(value[i] != '0' && value[i] != '.') { //It can be zero in number for
                return false;
            }
        }
    }
    else if(token->token_type == STRING) {
        for(int i = 0; i < strlen(value); i++) { //Or empty string
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


//Is end of expression?
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


bool is_unary_minus(tok_buffer_t *tok_b) {
    return  tok_b->last.token_type == UNKNOWN || //When it is start of epxression it is unary
            str_cmp(get_attr(&tok_b->last, tok_b->scanner), "(") == 0 || //After left par it is unary minus
            tok_b->last.token_type == OPERATOR; //When last token was operator
}


grm_sym_type_t operator_type(char first_c, char sec_c, tok_buffer_t *tok_b) {
    switch (first_c)
    {
        case '#':
            return HASH;
        case '*':
            return MULT;
        case '-':
            if(is_unary_minus(tok_b)) { //It can be unary or binary minus
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

            return UNDEFINED; //Assignment operator is not part of expression
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


int tok_to_type(tok_buffer_t *tok_b, bool *was_only_f_call) {
    token_t token = tok_b->current;

    if(token.token_type == IDENTIFIER || token.token_type == NUMBER ||
       token.token_type == STRING || token.token_type == INTEGER || 
       is_nil(tok_b->scanner, &tok_b->current)) { 
        
        //It is operand
        if(token.token_type != IDENTIFIER) {
            *was_only_f_call = false; //it can be still func call
        }

           return OPERAND;
    }
    else if(token.token_type == OPERATOR || token.token_type == SEPARATOR) {
        //It is operator
        int char_num = 0;
        char first_ch = (get_attr(&token, tok_b->scanner))[char_num++],
        next_ch = (get_attr(&token, tok_b->scanner))[char_num];

        if(token.token_type == OPERATOR) { 
            //It can be function call in parenthesis
            *was_only_f_call = false;
        }

        return operator_type(first_ch, next_ch, tok_b);
    }

    return UNDEFINED;
}


void make_type_str(string_t *dst, char type_c) {
    str_clear(dst); //First clear desetination string
    app_char(type_c, dst); //Then add type char
}


sym_dtype_t prim_type(string_t *type_string) {
    return char_to_dtype(to_str(type_string)[0]);
}


void int2num() {
    code_print("INT2FLOATS");
}


bool is_compatible_in_arg(char arg_type, string_t *dtypes) {
    if(prim_type(dtypes) == char_to_dtype(arg_type)) {
        return true;
    }
    else if(prim_type(dtypes) == NIL) { //If "rvalue" datatype is nil it is compatible with everything
        return true;
    }
    else if(prim_type(dtypes) == INT && char_to_dtype(arg_type) == NUM) {
        int2num();
        return true;
    }
    else {
        return false;
    }
}


bool is_tok_type(token_type_t exp_type, token_t *t) {
    return t->token_type == exp_type;
}



bool is_tok_attr(char *exp_attr, token_t *t, tok_buffer_t *tok_b) {
    return str_cmp(get_attr(t, tok_b->scanner), exp_attr) == 0;
}


void fcall_sem_error(tok_buffer_t *tok_b, token_t *func_id, char *msg) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSemantic error:\033[0m ", 
            tok_b->scanner->cursor_pos[ROW], tok_b->scanner->cursor_pos[COL]);

    fprintf(stderr, "Bad function call of \033[1;33m%s\033[0m! ", get_attr(func_id, tok_b->scanner));
    fprintf(stderr, "%s\n", msg);
}


void fcall_syn_error(tok_buffer_t *tok_b, token_t *func_id, char *msg) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSyntax error:\033[0m ", 
            tok_b->scanner->cursor_pos[ROW], tok_b->scanner->cursor_pos[COL]);

    fprintf(stderr, "In function call of \033[1;33m%s\033[0m! ", get_attr(func_id, tok_b->scanner));
    fprintf(stderr, "%s\n", msg);
}


int argument_parser(token_t *func_id, char *params_s, 
                    symbol_tables_t *syms, tok_buffer_t *tok_b) {

    size_t argument_cnt = 0;             
    bool closing_bracket = false;
    bool is_variadic = (params_s[0] == '%') ? true : false; //In our case variadic means - with variable AMOUNT and TYPES of arguments
    while(!closing_bracket) {
        if(token_aging(tok_b) != EXPRESSION_SUCCESS) {
            return LEXICAL_ERROR;
        }

        token_t t = lookahead(tok_b->scanner);
        if(t.token_type == ERROR_TYPE) {
            return LEXICAL_ERROR;
        }
        else if(!is_EOE(tok_b->scanner, &t) && !is_tok_attr(")", &t, tok_b)) {
            
            string_t ret_type;
            str_init(&ret_type);
        
            bool is_func_in_arg;
            int expr_retval = parse_expression(tok_b->scanner, syms, &ret_type, &is_func_in_arg);
            if(expr_retval != EXPRESSION_SUCCESS) {
                str_dtor(&ret_type);
                return -expr_retval; /**< Negative return code means "Propagate it, but don't write err msg "*/
            }

            if(!is_variadic) {
                if(!is_compatible_in_arg(params_s[argument_cnt], &ret_type)) { //Type check of epxression and argument and declared type
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
        else if(is_tok_type(SEPARATOR, &t) && is_tok_attr(")", &t, tok_b)) { //End of argument list
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


int fcall_parser(tree_node_t *symbol, 
                 symbol_tables_t *syms, 
                 tok_buffer_t *tok_b) {

    token_t func_id = tok_b->current;
    char *params_s = to_str(&symbol->data.params); //Getting pointer to string with parameter types

    if(token_aging(tok_b) != EXPRESSION_SUCCESS) {
        return LEXICAL_ERROR;
    }

    token_t t = lookahead(tok_b->scanner); //Finding argument list and '('
    if(t.token_type == ERROR_TYPE) {
        return LEXICAL_ERROR;
    }
    else if(!is_tok_type(SEPARATOR, &t) || !is_tok_attr("(", &t, tok_b)) { //Checking if there is '(' before arguments
        tok_b->current = t;
        fcall_syn_error(tok_b, &func_id, "Missing '(' after function indentifier!\n");
        return SYNTAX_ERROR_IN_EXPR;
    }
    else {
        int retval = argument_parser(&func_id, params_s, syms, tok_b); //Everything is ok, you can parse arguments

        if(retval != EXPRESSION_SUCCESS) {
            return retval;
        }
    }

    return EXPRESSION_SUCCESS;
}


/**
 * @brief Processes identifier got on input
 */
int process_identifier(p_parser_t *pparser, tok_buffer_t *t_buff, 
                       symbol_tables_t *syms, expr_el_t *result) { 

    char *id_name = get_attr(&(t_buff->current), t_buff->scanner);

    tree_node_t *symbol;
    symbol = search_in_tables(&syms->symtab_st, &syms->symtab, id_name); //Seaerching symbol in symbol tables with variables

    if(symbol == NULL) {
        //Check if it is builtin function
        check_builtin(id_name, &syms->global);

        symbol = search(&syms->global, id_name);
        if(symbol == NULL) { //Symbol was not found in variables nor in global table with functions
            return UNDECLARED_IDENTIFIER;
        }
        else {
            str_cpy((char **)&result->value, id_name, strlen(id_name));
            //Process function call and arguments
            int retval = fcall_parser(symbol, syms, t_buff);
            if(retval != EXPRESSION_SUCCESS) {
                return retval;
            }

            //Function was succesfully called
            cpy_strings(&result->dtype, &(symbol->data.ret_types), true);
            result->is_fcall = true;


            return EXPRESSION_SUCCESS;
        }
    }
    else {
        pparser->only_f_was_called = false;

        char type_c = dtype_to_char(symbol->data.dtype);
        make_type_str(&result->dtype, type_c);

        return EXPRESSION_SUCCESS;
    }
}


int from_input_token(p_parser_t *pparser, tok_buffer_t *t_buff, 
                     symbol_tables_t *syms, expr_el_t *result) {

    result->type = tok_to_type(t_buff, &(pparser->only_f_was_called));
    result->value = NULL;
    result->is_zero = false;
    result->is_fcall = false;
    str_init(&result->dtype);
    int retval = EXPRESSION_SUCCESS;
    //Resolving data type of symbol on input
    switch (t_buff->current.token_type)
    {
        case INTEGER:
            make_type_str(&result->dtype, 'i');
            pparser->was_operand = true;
            break;
        case NUMBER:
            make_type_str(&result->dtype, 'n');
            pparser->was_operand = true;
            break;
        case STRING:
            make_type_str(&result->dtype, 's');
            pparser->was_operand = true;
            break;
        case IDENTIFIER:
            if(!pparser->was_operand) {
                retval = process_identifier(pparser, t_buff, syms, result);
                if(retval != EXPRESSION_SUCCESS) { //Checking if processing identifier was succesfull or not
                    return retval;
                }
                else {
                    pparser->was_operand = true;
                }
            }

            break;
        default:
            if(is_nil(t_buff->scanner, &t_buff->current)) { //checking if it is nil type
                make_type_str(&result->dtype, 'z');
                pparser->was_operand = true;
            }
            else {
                make_type_str(&result->dtype, ' ');
                pparser->was_operand = false;
            }

            break;
    }

    //Resolving zero flag (to prevent division by zero)
    if(is_zero(t_buff->scanner, &t_buff->current) && PREVENT_ZERO_DIV) {
        result->is_zero = true;
    }
    //Making hard copy of token attribute
    if(result->value == NULL) {
        char * curr_val = get_attr(&(t_buff->current), t_buff->scanner);
        str_cpy((char **)&result->value, curr_val, strlen(curr_val));
    }

    return EXPRESSION_SUCCESS;
}


expr_el_t stop_symbol() {
    //Initialization of new stop symbol
    expr_el_t stop_symbol = {
        .type = STOP_SYM, 
        .dtype = {
            .alloc_size = 0, 
            .length = 0, 
            .str = NULL
        }, 
        .value = NULL, 
        .is_zero = false,
        .is_fcall = false
    };

    return stop_symbol;
}


expr_el_t prec_sign(char sign) {
    //Initialization of precedence sign symbol (due to given character)
    expr_el_t precedence_sign = {
        .dtype = {
            .alloc_size = 0, 
            .length = 0, 
            .str = NULL
        },
        .is_fcall = false
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

    if(on_stack_top.type >= TERM_NUM || on_input.type >= TERM_NUM) { //For safety
        return ' ';
    }

    return precedence_table[on_stack_top.type][on_input.type];
}


char *to_char_sequence(expr_el_t expression_element) {
    static char * cher_seq[] = {
        "#", "_", "*", "/", "//", "+", "-", "..", "<", "<=", 
        ">", ">=", "==", "~=", "(", ")", "i", "$", "E", NULL
    };

    return cher_seq[expression_element.type];
}

/**
 * @see presedence_parser.h (get_rule()) to learn meaning of rule parts and see the exmaples
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
        {"_E", "ni", ORIGIN, FIRST, "Unary minus expects number/integer as operands", generate_operation_unary_minus},
        {"#E", "s", INT, NONE, "Only string can be operand of \"#\"", generate_operation_strlen},
        {"E<E", "(nis|nis", BOOL, NONE, "Incompatible operands of \"<\"", generate_operation_lt},
        {"E>E", "(nis|nis", BOOL, NONE, "Incompatible operands of \">\"", generate_operation_gt},
        {"E<=E", "(nis|nis", BOOL, NONE, "Incompatible operands of \"<=\"", generate_operation_lte},
        {"E>=E", "(nis|nis", BOOL, NONE, "Incompatible operands of \">=\"", generate_operation_gte},
        {"E==E", "z(nis|nis)z", BOOL, NONE, "Incompatible operands of \"==\"", generate_operation_eq},
        {"E~=E", "z(nis|nis)z", BOOL, NONE, "Incompatible operands of \"~=\"", generate_operation_neq},
        {"E..E", "s|s", STR, NONE, "Operation \"..\" needs strings as operands", generate_operation_concat},
    };

    return &(rules[index]);
}


expr_el_t safe_op_pop(bool *cur_ok, int *check_result, pp_stack_t *op_stack) {
    //Perform check of stack emptyness before pop
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


bool is_compatible(string_t *dtypes1, string_t *dtypes2) {
    return ((dstring_cmp(dtypes1, dtypes2) == 0) || //Types are same
            (prim_type(dtypes1) == INT && prim_type(dtypes2) == NUM) || //Types are compatible (in expression)
            (prim_type(dtypes2) == INT && prim_type(dtypes1) == NUM));
}


void resolve_res_type(string_t *res, expr_rule_t *rule, 
                      expr_el_t cur_op, bool cur_ok) {

    if(prim_type(res) == UNDEFINED) { //If result type was not set yet set it current operand data type
        if(cur_ok) {
            cpy_strings(res, &cur_op.dtype, false);
        }
    }
    
    if(rule->return_type != ORIGIN) { //Rule has no influence to result data type
        str_clear(res);
        app_char(dtype_to_char(rule->return_type), res);
    }
    else if(prim_type(&(cur_op.dtype)) == NUM && prim_type(res) == INT) { //Implicit recasting of result type to number
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


int type_check(pp_stack_t op_stack, expr_rule_t *rule, string_t *res_type) {
    bool is_curr_ok = false, must_be_flag = false;
    int result = EXPRESSION_SUCCESS;
    expr_el_t current = safe_op_pop(&is_curr_ok, &result, &op_stack); //Getting first operand from operand stack

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

                    cpy_strings(&must_be, &(current.dtype), false);
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
    cpy_strings(res_type, &tmp_res_type, false);
    str_dtor(&tmp_res_type);

    return result;
}


void get_str_to_reduction(pp_stack_t *s, pp_stack_t *op, string_t *to_be_red) {
    expr_el_t from_top;
    while((from_top = pp_top(s)).type != STOP_SYM) {
        if(from_top.type == '<') { //If '<' is found, stop 
            pp_pop(s);
            break;
        }

        if(from_top.type == NON_TERM || from_top.type == OPERAND) {
            pp_push(op, from_top); //Filling operand stack (to determine result data type)
        }

        char *char_seq = to_char_sequence(from_top);
        prep_str(to_be_red, char_seq);
        pp_pop(s);
    }

    // fprintf(stderr, "To be reduced: %s\n", to_be_red->str);
}


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
            pp_pop(&operands); //Dont canre about first operand
            if(!pp_is_empty(&operands) && pp_pop(&operands).is_zero) { //Second must be zero
                return true;
            }
        }
        break;
    case ALL:
        while(!pp_is_empty(&operands)) {
            if(!pp_pop(&operands).is_zero) { //All of operands must be zero
                return false;
            }
        }

        return true;
        break;
    case ONE:
        while(!pp_is_empty(&operands)) { //If one operand is zero result is zero
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


expr_el_t non_term(string_t *data_type, bool is_zero) {
    expr_el_t non_terminal;
    non_terminal.type = NON_TERM;

    str_init(&(non_terminal.dtype));
    cpy_strings(&(non_terminal.dtype), data_type, false); //Data type of non-terminal (important for propagating datatype through expression)

    non_terminal.value = "NONTERM"; //Be carefull and DONT deallocate this value
    non_terminal.is_zero = is_zero;
    non_terminal.is_fcall = false;

    return non_terminal;
}


int reduce(p_parser_t *pparser, pp_stack_t ops, symbol_tables_t *syms, 
           expr_rule_t *rule, string_t *res_type) {

    //Todo fix function calls being generated as variables
    if(strcmp(rule->right_side, "i") == 0) {
        expr_el_t element_terminal = pp_top(&ops);
        tree_node_t *res = search_in_tables(&syms->symtab_st, &syms->symtab, element_terminal.value);

        if(element_terminal.is_fcall) {
            //Only function was called during reduction 

            //Code for function
            generate_call_function(element_terminal.value);
        }
        else if(res == NULL) {
            //We are pushing a static value

            char prim_dtype_c = to_str(&element_terminal.dtype)[0];
            sym_dtype_t dtype = char_to_dtype(prim_dtype_c);

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

    bool will_be_zero = resolve_res_zero(ops, rule);
    if(!pp_push(&pparser->stack, non_term(res_type, will_be_zero))) { /**< Make non terminal at the top of main stack (with corresponding zero flag)*/
        return INTERNAL_ERROR;
    }
    if(!pp_push(&pparser->garbage, pp_top(&pparser->stack))) { /**< Add nonterminal to garbage collector stack */
        return INTERNAL_ERROR;
    }

    return EXPRESSION_SUCCESS;
}


int reduce_top(p_parser_t *pparser, symbol_tables_t *syms,
               char ** failed_op_msg, string_t *ret_types) {

    string_t to_be_reduced;
    str_init(&to_be_reduced);

    pp_stack_t operands; //Initialization of auxiliary stack with operands
    if(!pp_stack_init(&operands)) {
        return INTERNAL_ERROR;
    }

    get_str_to_reduction(&(pparser->stack), &operands, &to_be_reduced); /**< Takes top of the stack and creates substitutable string from it*/

    //fprintf(stderr, "To be reduced: %s\n", to_str(&to_be_reduced));
    expr_rule_t *rule;
    int retval = EXPRESSION_FAILURE; /**< If rule is not found it is invalid operation -> return EXPR_FAILURE */
    for(int i = 0; (rule = get_rule(i)); i++) {
        if(str_cmp(to_str(&to_be_reduced), rule->right_side) == 0) {
            int t_check_res = type_check(operands, rule, ret_types); /**<Checking type compatibility and getting result data type */

            if(t_check_res != EXPRESSION_SUCCESS) { /**< Type check was not succesfull */
                *failed_op_msg = rule->error_message;
                retval = t_check_res;
            }
            else {
                retval = reduce(pparser, operands, syms, rule, ret_types);
            }

        }
    }

    pp_stack_dtor(&operands);
    str_dtor(&to_be_reduced);

    return retval;
}


int get_input_symbol(p_parser_t *pparser, tok_buffer_t *t_buff, 
                     symbol_tables_t *symtabs, expr_el_t *on_input) {

    int retval = EXPRESSION_SUCCESS;
    if(pparser->stop_flag || is_EOE(t_buff->scanner, &(t_buff->current))) {
        *on_input = stop_symbol();
    }
    else {
        retval = from_input_token(pparser, t_buff, symtabs, on_input);
        pp_push(&(pparser->garbage), *on_input);
    }

    return retval;
}


void get_top_symbol(expr_el_t *on_top, p_parser_t *pparser) {
    expr_el_t on_top_tmp = pp_top(&(pparser->stack));
    expr_el_t tmp;

    if(on_top_tmp.type == NON_TERM) { //Ignore nonterminal if there is 
        tmp = pp_pop(&(pparser->stack));
        on_top_tmp = pp_top(&(pparser->stack));
        pp_push(&(pparser->stack), tmp);
    }

    *on_top = on_top_tmp;
}


void free_everything(p_parser_t *pparser) {
    while(!pp_is_empty(&(pparser->garbage)))
    {
        expr_el_t current_el = pp_pop(&(pparser->garbage));
        str_dtor(&(current_el.dtype));
        //Freeing attributes of stored elements in garbage collector stack (if it is dynamicaly allocated)
        if(current_el.type != NON_TERM) {
            free(current_el.value);
        }
    }
    
    pp_stack_dtor(&(pparser->garbage));
    pp_stack_dtor(&(pparser->stack));
}


int token_aging(tok_buffer_t *token_buffer) {
    token_buffer->last = token_buffer->current; //Make current token older
    token_buffer->current = get_next_token(token_buffer->scanner);
    
    if(is_tok_type(ERROR_TYPE , &token_buffer->current)) {
        return LEXICAL_ERROR; //Check for lexical errors
    }

    return EXPRESSION_SUCCESS;
}


void has_lower_prec(pp_stack_t *stack, expr_el_t on_input) {
    //Just push '<' sign to stack and input symbol after it
    expr_el_t top = pp_top(stack);
    if(top.type == NON_TERM) {
        //If there is nonterminal push '<' after it
        pp_pop(stack);
        pp_push(stack, prec_sign('<'));
        pp_push(stack, top);
    }
    else {
        pp_push(stack, prec_sign('<'));
    }

    pp_push(stack, on_input);
}


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


void prepare_buffer(scanner_t *sc, tok_buffer_t *tok_b) {
    tok_b->scanner = sc;
    token_init(&(tok_b->current));
    token_init(&(tok_b->last));
}


int prepare_pp(p_parser_t *pp) {
    if(!pp_stack_init(&pp->stack) || !pp_stack_init(&pp->garbage)) {
        return INTERNAL_ERROR;
    }

    if(!pp_push(&pp->stack, stop_symbol())) {
        return INTERNAL_ERROR;
    }

    //Initiallization of flags
    pp->stop_flag = false;
    pp->empty_expr = true;
    pp->empty_cycle = false;
    pp->was_operand = false;
    //Presume that it is only function call (important for stack popping and return codes)
    pp->only_f_was_called = true;

    return EXPRESSION_SUCCESS;
}


int parse_expression(scanner_t *sc, symbol_tables_t *s, 
                     string_t *dtypes, bool *was_f_call) {

    int ret = EXPRESSION_SUCCESS; //Return value of precedence parsing
    char *failed_op_msg = NULL; //Pointer to error msg
    
    tok_buffer_t tok_buff;
    prepare_buffer(sc, &tok_buff);

    p_parser_t pparser;
    ret = prepare_pp(&pparser);
    if(ret != EXPRESSION_SUCCESS) {
        return ret;
    }
    
    while(ret == EXPRESSION_SUCCESS) { //Main cycle
        tok_buff.current = lookahead(sc);

        if(tok_buff.current.token_type == ERROR_TYPE) {
            ret = LEXICAL_ERROR;
            break;
        }

        expr_el_t on_input, on_top;
        ret = get_input_symbol(&pparser, &tok_buff, s, &on_input);
    
        if(ret != EXPRESSION_SUCCESS) {
            break;
        }

        get_top_symbol(&on_top, &pparser);

        /*** There is end of the expression on input and stop symbol at the top of the stack*/
        if(on_top.type == STOP_SYM && on_input.type == STOP_SYM) {
            ret = pparser.empty_expr ? MISSING_EXPRESSION : EXPRESSION_SUCCESS;
            break;
        }

        char precedence = get_precedence(on_top, on_input);
        //fprintf(stderr, "%s: %c %d(%d) %d(%d) Stop flag: %d\n", get_attr(&tok_buff.current, sc), precedence, on_top.type, on_top.is_zero, on_input.type, on_input.is_zero, stop_flag);
        if(precedence == '=') {
            if(!pp_push(&pparser.stack, on_input)) {
                ret = INTERNAL_ERROR;
            }

            ret = token_aging(&tok_buff); /**< Make last token from current token */
            pparser.empty_cycle = false;
        }
        else if(precedence == '<') {  /**< Put input symbol to the top of the stack*/
            has_lower_prec(&(pparser.stack), on_input);
            ret = token_aging(&tok_buff);
            pparser.empty_cycle = false;
        }
        else if(precedence == '>') { /**< Basicaly, reduct while you can't put input symbol to the top of the stack*/
            ret = reduce_top(&pparser, s, &failed_op_msg, dtypes);
            pparser.empty_cycle = false;
        }
        else {
            pparser.stop_flag = true;

            if(pparser.stop_flag && pparser.empty_cycle) {
                ret = EXPRESSION_FAILURE;
                break;
            }

            pparser.empty_cycle = true;
        }

        pparser.empty_expr = false;
    }

    *was_f_call = pparser.only_f_was_called;
    //Print error msg to terminal or adjust error code and free resources
    print_err_message(&ret, &tok_buff, &failed_op_msg);
    free_everything(&pparser);

    token_t next = lookahead(sc); fprintf(stderr, "REST: %s\n", get_attr(&next, sc));
    return ret;
}

/***                     End of precedence_parser.c                        ***/
