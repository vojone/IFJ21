/******************************************************************************
 *                                  IFJ21
 *                            precedence_parser.c
 * 
 *          Authors: Vojtěch Dvořák (xdvora3o), Juraj Dědič (xdedic07)
 *              Purpose: Implementation of precedence parsing (PP)
 * 
 *                       Last change: 25. 11. 2021
 *****************************************************************************/

/**
 * @file precedence_parser.c
 * @brief Implementation of precedence parsing (PP)
 * @note For more documentation (especially function description) @see precedence_parser.h
 * 
 * @authors Vojtěch Dvořák (xdvora3o), Juraj Dědič (xdedic07)
 *
 */

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
        case '^':
            return POW;
        case '%':
            return MOD;
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


int make_type_str(string_t *dst, char type_c) {
    str_clear(dst); //First clear desetination string

    if(app_char(type_c, dst) != STR_SUCCESS) { //Then add type char
        return INTERNAL_ERROR;
    }

    return EXPRESSION_SUCCESS;
}


sym_dtype_t prim_type(string_t *type_string) {
    return char_to_dtype(to_str(type_string)[0]);
}


void int2num(prog_t *dst) {
    impl_int2num(dst);
}


bool is_compatible_in_arg(prog_t *dst, char par_type, char arg_type) {
    if(char_to_dtype(arg_type) == char_to_dtype(par_type)) {
        return true;
    }
    else if(char_to_dtype(arg_type) == NIL) { //If "rvalue" datatype is nil it is compatible with everything
        return true;
    }
    else if(char_to_dtype(arg_type) == INT && char_to_dtype(par_type) == NUM) {
        int2num(dst);
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


void fcall_sem_error(tok_buffer_t *tok_b, char *f_name, char *msg) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSemantic error:\033[0m ", //Print err msg prolog
            tok_b->scanner->cursor_pos[ROW], tok_b->scanner->cursor_pos[COL]);

    fprintf(stderr, "Bad function call of \033[1;33m%s\033[0m! ", f_name);
    fprintf(stderr, "%s\n", msg);
}


void fcall_syn_error(tok_buffer_t *tok_b, char *f_name, char *msg) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[0;31mSyntax error:\033[0m ", 
            tok_b->scanner->cursor_pos[ROW], tok_b->scanner->cursor_pos[COL]);

    fprintf(stderr, "In function call of \033[1;33m%s\033[0m! ", f_name);
    fprintf(stderr, "%s\n", msg);
}


int parse_arg_expr(size_t *arg_cnt, prog_t *dst_code, 
                   symbol_tables_t *syms, tok_buffer_t *tok_b, 
                   tree_node_t *symbol) {
    
    char *params_s = to_str(&symbol->data.params);
    size_t param_num = strlen(params_s);
    char *f_name = symbol->key;
    bool is_variadic = (params_s[0] == '%') ? true : false; //In our case variadic means - with variable AMOUNT and TYPES of arguments

    string_t ret_type;
    if(str_init(&ret_type) != STR_SUCCESS) {
        return INTERNAL_ERROR;
    }

    bool fcall;
    int expr_retval = parse_expression(tok_b->scanner, syms, &ret_type, &fcall, dst_code);
    if(expr_retval != EXPRESSION_SUCCESS) {
        str_dtor(&ret_type);
        return -expr_retval; /**< Negative return code means "Propagate it, but don't write err msg "*/
    }

    size_t u = 0;
    token_t t = lookahead(tok_b->scanner);
    int ret = EXPRESSION_SUCCESS;
    if(is_error_token(&t, &ret)) { 
        str_dtor(&ret_type);
        return ret;
    }

    if(!is_variadic) {
        while(*arg_cnt < param_num && u < len(&ret_type)) {
            if(!is_compatible_in_arg(dst_code, params_s[*arg_cnt], to_str(&ret_type)[u])) { //Type check of epxression and argument and declared type
                fcall_sem_error(tok_b, f_name, "Bad data types of arguments!");
                str_dtor(&ret_type);
                return SEMANTIC_ERROR_PARAMETERS_EXPR;
            }
            else {
                //Parameter is ok
            }

            u++;
            (*arg_cnt)++;

            if(is_tok_type(SEPARATOR, &t), is_tok_attr(",", &t, tok_b)) { //If there is comma after, pick only one of function return values
                break;
            }
        }

        if(!fcall) {
            *arg_cnt += len(&ret_type) - u; //Add difference to recognize too many arguments (if they are comming from function, they can be disposed)
        }

        if(u < len(&ret_type)) {
            generate_dump_values(dst_code, u, len(&ret_type) - u); //Remove return values that are not used in function call
        }
    }
    

    str_dtor(&ret_type);

    return ret;
}


int argument_parser(prog_t *dst_code, tree_node_t *symbol, 
                    symbol_tables_t *syms, tok_buffer_t *tok_b) {

    int ret = EXPRESSION_SUCCESS;
    char *params_s = to_str(&symbol->data.params); //Getting pointer to string with parameter types
    char *f_name = symbol->key;

    size_t cnt = 0;             
    bool closing_bracket = false;
    bool is_variadic = (params_s[0] == '%') ? true : false; //In our case variadic means - with variable AMOUNT and TYPES of arguments
    while(!closing_bracket) {
        if((ret = token_aging(tok_b)) != EXPRESSION_SUCCESS) {
            return ret;
        }

        token_t t = lookahead(tok_b->scanner);
        if(is_error_token(&t, &ret)) {
            return ret;
        }
        
        if(!is_EOE(tok_b->scanner, &t) && !is_tok_attr(")", &t, tok_b)) {
            
            ret = parse_arg_expr(&cnt, dst_code, syms, tok_b, symbol); //Check expression in argument
            if(ret != EXPRESSION_SUCCESS) {
                return ret;    
            }

        }
        else if(is_tok_type(SEPARATOR, &t) && is_tok_attr(")", &t, tok_b)) { //End of argument list
            closing_bracket = true;
            continue;
        }
        else {
            fcall_syn_error(tok_b, f_name, "Missing ')' after function arguments!\n");
            return SYNTAX_ERROR_IN_EXPR;
        }

        //Check if there will be next argument
        t = lookahead(tok_b->scanner);
        if(is_error_token(&t, &ret)) {
            return ret;
        }

        if(is_tok_type(SEPARATOR, &t) && is_tok_attr(",", &t, tok_b)) {
            if(cnt > strlen(params_s) && !is_variadic) { //Function needs less arguments
                fcall_sem_error(tok_b, f_name, "Too many arguments!");
                return SEMANTIC_ERROR_PARAMETERS_EXPR;
            }
            else {
                //Ok
            }
        }
        else if(is_tok_type(SEPARATOR, &t) && is_tok_attr(")", &t, tok_b)) {
            closing_bracket = true;
            //fprintf(stderr, "%ld %ld\n", cnt, strlen(params_s));
            if((cnt < strlen(params_s)) && !is_variadic) { //Function needs more arguments
                fcall_sem_error(tok_b, f_name, "Missing arguments!");
                return SEMANTIC_ERROR_PARAMETERS_EXPR;
            }
            else if(cnt > strlen(params_s) && !is_variadic) { //Function needs less arguments
                fcall_sem_error(tok_b, f_name, "Too many arguments!");
                return SEMANTIC_ERROR_PARAMETERS_EXPR;
            }
            
        }
        else {
            fcall_syn_error(tok_b, f_name, "Missing ',' or ')' between arguments!\n");
            return SYNTAX_ERROR_IN_EXPR;
        }
    }
    if(cnt == 0 && strlen(params_s) > 0) { //Function needs arguments but there aren't any
        fcall_sem_error(tok_b, f_name, "Missing arguments!");
        return SEMANTIC_ERROR_PARAMETERS_EXPR;
    }

    return EXPRESSION_SUCCESS;
}


int fcall_parser(prog_t *dst_prog,
                 tree_node_t *symbol, 
                 symbol_tables_t *syms, 
                 tok_buffer_t *tok_b) {

    int ret = EXPRESSION_SUCCESS;
    if((ret = token_aging(tok_b)) != EXPRESSION_SUCCESS) {
        return ret;
    }

    token_t t = lookahead(tok_b->scanner); //Finding argument list and '('
    if(is_error_token(&t, &ret)) {
        return ret;
    }

    if(!is_tok_type(SEPARATOR, &t) || !is_tok_attr("(", &t, tok_b)) { //Checking if there is '(' before arguments
        tok_b->current = t;
        fcall_syn_error(tok_b, symbol->key, "Missing '(' after function indentifier!\n");
        return SYNTAX_ERROR_IN_EXPR;
    }
    else {
        ret = argument_parser(dst_prog, symbol, syms, tok_b); //Everything is ok, you can parse arguments
        if(ret != EXPRESSION_SUCCESS) {
            return ret;
        }
    }

    symbol->data.was_used = true; //Update was_used of called function

    return EXPRESSION_SUCCESS;
}


/**
 * @brief Processes identifier got on input
 */
int process_identifier(p_parser_t *pparser, 
                       tok_buffer_t *t_buff, 
                       symbol_tables_t *syms) { 

    char *id_name = get_attr(&(t_buff->current), t_buff->scanner);
    expr_el_t *on_inp = &pparser->on_input;
    tree_node_t *symbol;
    symbol = deep_search(&syms->symtab_st, &syms->symtab, id_name); //Seaerching symbol in symbol tables with variables

    if(symbol == NULL) {
        //Check if it is builtin function
        check_builtin(id_name, &syms->global);

        symbol = search(&syms->global, id_name);
        if(symbol == NULL) { //Symbol was not found in variables nor in global table with functions
            return UNDECLARED_IDENTIFIER;
        }
        else {
            str_cpy((char **)&on_inp->value, id_name, strlen(id_name));
            //Process function call and arguments
            int retval = fcall_parser(pparser->dst_code, symbol, syms, t_buff);
            if(retval != EXPRESSION_SUCCESS) {
                return retval;
            }

            //Current token attribute is restored to function name (for correct behaviour in precedence table)
            t_buff->current.attr = symbol->key;

            //Function was succesfully called
            cpy_strings(&on_inp->dtype, &(symbol->data.ret_types), true);
            on_inp->is_fcall = true;


            return EXPRESSION_SUCCESS;
        }
    }
    else {
        if(symbol->data.status != DEFINED && symbol->data.dtype != NIL) {
            undefined_var_warning(t_buff, id_name);
        }

        pparser->only_f_was_called = false;

        char type_c = dtype_to_char(symbol->data.dtype);
        make_type_str(&on_inp->dtype, type_c);

        return EXPRESSION_SUCCESS;
    }
}


int from_input_token(p_parser_t *pparser, 
                     tok_buffer_t *t_buff, 
                     symbol_tables_t *syms) {

    expr_el_t *on_inp = &(pparser->on_input);
    on_inp->type = tok_to_type(t_buff, &(pparser->only_f_was_called));
    on_inp->value = NULL;
    on_inp->is_zero = false;
    on_inp->is_fcall = false;

    if(str_init(&on_inp->dtype) != STR_SUCCESS) {
        return INTERNAL_ERROR;
    }

    int retval = EXPRESSION_SUCCESS;
    //Resolving data type of symbol on input
    switch (t_buff->current.token_type)
    {
        case INTEGER:
            retval = make_type_str(&on_inp->dtype, 'i');
            break;
        case NUMBER:
            retval = make_type_str(&on_inp->dtype, 'n');
            break;
        case STRING:
            retval = make_type_str(&on_inp->dtype, 's');
            break;
        case IDENTIFIER:
            if(get_precedence(pparser->on_top, operand()) != ' ') { //Simulate state in future (to recognize end of expression)
                retval = process_identifier(pparser, t_buff, syms);
                if(retval != EXPRESSION_SUCCESS) { //Checking if processing identifier was succesfull or not
                    return retval;
                }
            }

            break;
        default:
            if(is_nil(t_buff->scanner, &t_buff->current)) { //Checking if it is nil type
                retval = make_type_str(&on_inp->dtype, 'z');
            }
            else {
                retval = make_type_str(&on_inp->dtype, ' ');
            }

            break;
    }

    if(retval == STR_FAILURE) {
        return INTERNAL_ERROR;
    }

    //Resolving zero flag (to prevent division by zero)
    if(is_zero(t_buff->scanner, &t_buff->current) && PREVENT_ZERO_DIV) {
        on_inp->is_zero = true;
    }

    //Making hard copy of token attribute
    if(on_inp->value == NULL) {
        char * curr_val = get_attr(&(t_buff->current), t_buff->scanner);

        retval = str_cpy((char **)&on_inp->value, curr_val, strlen(curr_val));
        if(retval != STR_SUCCESS) {
            return INTERNAL_ERROR;
        }
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
            precedence_sign.type = '<'; //When there is something else, its always implicitly '<'
            break;
    }

    precedence_sign.value = NULL;
    precedence_sign.is_zero = false;

    return precedence_sign;
}


expr_el_t operand() {
    expr_el_t op;

    op.type = OPERAND;
    op.is_zero = false;
    op.is_fcall = false;
    op.value = NULL;

    return op;
}


char get_precedence(expr_el_t on_stack_top, expr_el_t on_input) {
    static char precedence_table[TERM_NUM][TERM_NUM] = {
    //   #   _   ^   %   *   /   //  +   -  ..   <  <=   >  >=  ==  ~=   (   )   i   $
/*_*/  {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*^*/  {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*#*/  {'<','<','<','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'}, 
/*%*/  {'<','<','<','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/***/  {'<','<','<','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*/*/  {'<','<','<','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*//*/ {'<','<','<','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*+*/  {'<','<','<','<','<','<','<','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*-*/  {'<','<','<','<','<','<','<','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*..*/ {'<','<','<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*<*/  {'<','<','<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*<=*/ {'<','<','<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*>*/  {'<','<','<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*>=*/ {'<','<','<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*==*/ {'<','<','<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*~=*/ {'<','<','<','<','<','<','<','<','<','<','>','>','>','>','>','>','<','>','<','>'},
/*(*/  {'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','=','<',' '},
/*)*/  {'>','>','>','>','>','>','>','>','>','>','>','>','>','>','>','>',' ','>',' ','>'},
/*i*/  {'>','>','>','>','>','>','>','>','>','>','>','>','>','>','>','>',' ','>',' ','>'},
/*$*/  {'<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<','<',' ','<',' '}
    };

    if(on_stack_top.type >= TERM_NUM || on_input.type >= TERM_NUM) { //For safety
        return ' ';
    }

    return precedence_table[on_stack_top.type][on_input.type];
}


char *to_char_sequence(expr_el_t expression_element) {
    static char * cher_seq[] = {
        "#", "_", "^", "\045", "*", "/", "//", "+", "-", "..", "<", 
        "<=", ">", ">=", "==", "~=", "(", ")", "i", "$", "E", NULL
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
        {"(E)", "*", ORIGIN, FIRST ,NULL ,NULL},
        {"i", "*", ORIGIN, FIRST, NULL ,NULL},
        {"E+E", "ni|ni", ORIGIN, ALL, "\"+\" expects numbers/integers as operands", generate_operation_add},
        {"E-E", "ni|ni", ORIGIN, ALL, "\"-\" expects numbers/integers as operands", generate_operation_sub},
        {"E^E", "ni|ni", NUM, NONE, "\"^\" expects numbers/integers as operands", NULL},
        {"E\045E", "ni|!ni", ORIGIN, FIRST, "\"\045\" expects numbers/integers as operands", NULL}, //E%E
        {"E*E", "ni|ni", ORIGIN, ONE, "\"*\" expects numbers/integers as operands", generate_operation_mul},
        {"E/E", "ni|!ni", NUM, FIRST, "\"/\" expects numbers/integers as operands", generate_operation_div},
        {"E//E", "i|!i", INT, FIRST, "\"//\" expects integers as operands", generate_operation_idiv},
        {"_E", "ni", ORIGIN, FIRST, "Unary minus expects numbers/integers as operands", generate_operation_unary_minus},
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


int resolve_res_type(string_t *res, expr_rule_t *rule,
                      expr_el_t cur_op, bool cur_ok) {

    if(prim_type(res) == UNDEFINED) { //If result type was not set yet set it current operand data type
        if(cur_ok) {
            if(cpy_strings(res, &cur_op.dtype, false) != STR_SUCCESS) {
                return INTERNAL_ERROR;
            }
        }
    }
    
    if(rule->return_type != ORIGIN) { //Rule has no influence to result data type
        str_clear(res);
        if(app_char(dtype_to_char(rule->return_type), res) != STR_SUCCESS) {
            return INTERNAL_ERROR;
        }
    }
    else if(prim_type(&(cur_op.dtype)) == NUM && prim_type(res) == INT) { //Implicit recasting of result type to number
        str_clear(res);
        if(app_char(dtype_to_char(NUM), res) != STR_SUCCESS) {
            return INTERNAL_ERROR;
        }
    }

    return EXPRESSION_SUCCESS;
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


int op_type_ch(char c, bool must_be_flag, bool *is_curr_ok, 
                   string_t *must_be, expr_el_t *current) {

    if(must_be_flag) { /**< "Must_be mode" is set on */
        if(prim_type(must_be) == UNDEFINED && 
            prim_type(&(current->dtype)) == char_to_dtype(c)) {

            if(cpy_strings(must_be, &(current->dtype), false) != STR_SUCCESS) {
                return INTERNAL_ERROR;
            }
        }

        if(is_compatible(&(current->dtype), must_be)) {
            *is_curr_ok = true; /**< Type of current operator is compatible with type of earlier operator */
        }
    }
    else { /**< Normal mode */
        if(c == '*' || prim_type(&(current->dtype)) == char_to_dtype(c)) {
            *is_curr_ok = true; //Operand type corresponds with operator specifier
        }
    }

    return EXPRESSION_SUCCESS;
}


//Presumes that macro EXPRESSION_SUCCESS is 0 !!!
int type_check(pp_stack_t op_stack, expr_rule_t *rule, string_t *res_type) {
    bool is_curr_ok = false, must_be_flag = false;
    int ret = EXPRESSION_SUCCESS;
    expr_el_t current = safe_op_pop(&is_curr_ok, &ret, &op_stack); //Getting first operand from operand stack

    //Initialization of strings with safety checks (there can be allocation error)
    string_t tmp_res_type;
    char rule_dtype_c = dtype_to_char(rule->return_type);
    if((str_init(&tmp_res_type) != STR_SUCCESS || 
       app_char(rule_dtype_c, &tmp_res_type) != STR_SUCCESS) && !ret) {
        ret = INTERNAL_ERROR;
    }

    string_t must_be;
    if((str_init(&must_be) != STR_SUCCESS || 
       app_char(' ', &must_be) != STR_SUCCESS) && !ret) {
        ret = INTERNAL_ERROR;
    }

    char c;
    for(int i = 0; (c = rule->operator_types[i]) != '\0' && !ret; i++) {
        if(c == '|') { /**< Next operator symbol */
            if(!is_curr_ok) { /**< Specifier for current operator was not found -> error */
                ret = get_tcheck_ret(&current);
                break;
            }
            else { /**< Specifier was found -> next check operator*/
                current = safe_op_pop(&is_curr_ok, &ret, &op_stack);
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
                ret = DIV_BY_ZERO;
                break;
            }
        }
        else { /**< Current char is type specifier -> check operand type */
            ret = op_type_ch(c, must_be_flag, &is_curr_ok, &must_be, &current);
        }

        if(!ret) {
            ret = resolve_res_type(&tmp_res_type, rule, current, is_curr_ok);
        }
    }

    // Specifier string ended -> if specifier for last operator was not found -> error
    ret = (!is_curr_ok && c == '\0' && !ret) ?  get_tcheck_ret(&current) : ret;

    str_dtor(&must_be);
    if(cpy_strings(res_type, &tmp_res_type, false) != STR_SUCCESS && !ret) {
        ret = INTERNAL_ERROR;
    }

    str_dtor(&tmp_res_type);

    return ret;
}


int get_str_to_reduction(pp_stack_t *s, pp_stack_t *op, string_t *to_be_red) {
    expr_el_t from_top;
    while(!pp_is_empty(s) && (from_top = pp_top(s)).type != STOP_SYM) {
        if(from_top.type == '<') { //If '<' is found, stop 
            pp_pop(s);
            break;
        }

        if(from_top.type == NON_TERM || from_top.type == OPERAND) {
            if(!pp_push(op, from_top)) { //Filling operand stack (to determine result data type)
                return INTERNAL_ERROR;
            }
        }

        char *char_seq = to_char_sequence(from_top);
        if(prep_str(to_be_red, char_seq) != STR_SUCCESS) {
            return INTERNAL_ERROR;
        }

        pp_pop(s);
    }

    return EXPRESSION_SUCCESS;
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


int non_term(expr_el_t *non_terminal, string_t *data_type, bool is_zero) {
    non_terminal->type = NON_TERM;

    if(str_init(&(non_terminal->dtype)) != STR_SUCCESS) {
        return INTERNAL_ERROR;
    }

    //Data type of non-terminal (important for propagating datatype through expression)
    if(cpy_strings(&(non_terminal->dtype), data_type, false) != STR_SUCCESS) {
        return INTERNAL_ERROR;
    }

    non_terminal->value = "NONTERM"; //Be carefull and DONT deallocate this value
    non_terminal->is_zero = is_zero;
    non_terminal->is_fcall = false;

    return EXPRESSION_SUCCESS;
}


int reduce(p_parser_t *pparser, pp_stack_t ops, symbol_tables_t *syms,
           expr_rule_t *rule, string_t *res_type) {

    //Todo fix function calls being generated as variables
    prog_t *dst = pparser->dst_code;
    if(str_cmp(rule->right_side, "i") == 0) {
        expr_el_t element_terminal = pp_top(&ops);

        tree_node_t *res = deep_search(&syms->symtab_st, &syms->symtab, element_terminal.value);
        if(element_terminal.is_fcall) {
            //Only function was called during reduction 
            res = search(&syms->global, element_terminal.value);

            pparser->was_f_call = true;
            pparser->last_call_ret_num = len(&(res->data.ret_types));

            generate_call_function(dst, element_terminal.value);
        }
        else if(res == NULL) {
            //We are pushing a static value

            char prim_dtype_c = to_str(&element_terminal.dtype)[0];
            sym_dtype_t dtype = char_to_dtype(prim_dtype_c);
            
            //We are pushing a static value
            generate_value_push(dst, VAL, dtype, element_terminal.value);
        }
        else{
            //We are pushing variable
            generate_value_push(dst, VAR, res->data.dtype , res->data.name.str);
        }
    }

    //Generate operation code
    if(rule->generator_function != NULL)
        rule->generator_function(dst);

    bool will_be_zero = resolve_res_zero(ops, rule);
    expr_el_t non_terminal;
    if(non_term(&non_terminal, res_type, will_be_zero) != EXPRESSION_SUCCESS) {
        return INTERNAL_ERROR;
    }
    if(!pp_push(&pparser->stack, non_terminal)) { /**< Make non terminal at the top of main stack (with corresponding zero flag)*/
        return INTERNAL_ERROR;
    }
    if(!pp_push(&pparser->garbage, pp_top(&pparser->stack))) { /**< Add nonterminal to garbage collector stack */
        return INTERNAL_ERROR;
    }

    return EXPRESSION_SUCCESS;
}


void only_primary_type(string_t *ret_types, expr_rule_t *rule) {
    if(str_cmp("i", rule->right_side) != 0 && 
       str_cmp("(E)", rule->right_side) != 0) {
        cut_string(ret_types, 1);
    }
}


int reduce_top(p_parser_t *pparser, symbol_tables_t *syms,
               char ** failed_op_msg, string_t *ret_types) {

    string_t to_be_reduced;
    if(str_init(&to_be_reduced) != STR_SUCCESS) {
        return INTERNAL_ERROR;
    }

    pp_stack_t operands; //Initialization of auxiliary stack with operands
    if(!pp_stack_init(&operands)) {
        return INTERNAL_ERROR;
    }

    int ret;
    ret = get_str_to_reduction(&(pparser->stack), &operands, &to_be_reduced);

    fprintf(stderr, "To be reduced: %s\n", to_str(&to_be_reduced));
    expr_rule_t *rule;
    if(ret == EXPRESSION_SUCCESS) {
        ret = EXPRESSION_FAILURE; /**< If rule is not found it is invalid operation -> return EXPR_FAILURE */
        for(int i = 0; (rule = get_rule(i)); i++) {
            if(str_cmp(to_str(&to_be_reduced), rule->right_side) == 0) {
                int t_check_res = type_check(operands, rule, ret_types); /**<Checking type compatibility and getting result data type */
                
                only_primary_type(ret_types, rule);

                if(t_check_res != EXPRESSION_SUCCESS) {
                    *failed_op_msg = rule->error_message;
                    ret = t_check_res;
                }
                else {
                    ret = reduce(pparser, operands, syms, rule, ret_types);
                }

                break;
            }
        }
    }

    pp_stack_dtor(&operands);
    str_dtor(&to_be_reduced);

    return ret;
}


int get_input_symbol(p_parser_t *pparser, tok_buffer_t *t_buff, 
                     symbol_tables_t *symtabs, prog_t *dst) {

    int retval = EXPRESSION_SUCCESS;
    if(pparser->stop_flag || is_EOE(t_buff->scanner, &(t_buff->current))) {
        pparser->on_input = stop_symbol(); //If there is end of expression, put stop symbol to input
    }
    else {
        retval = from_input_token(pparser, t_buff, symtabs);
        if(!pp_push(&(pparser->garbage), pparser->on_input)) {
            return INTERNAL_ERROR;
        }
    }
    //If the last reduced symbol was function call and there is another expression elements we must clear stack
    if(pparser->was_f_call && !pparser->only_f_was_called) {
        //Clear stack to calculate only first return value
        size_t pop_cnt = (pparser->last_call_ret_num > 0) ? pparser->last_call_ret_num - 1 : pparser->last_call_ret_num;
        generate_dump_values(dst, 1, pop_cnt);
    }

    pparser->was_f_call = false; /**< New input symbol -> reset function call flag */

    return retval;
}


int get_top_symbol(p_parser_t *pparser) {
    expr_el_t on_top_tmp = pp_top(&(pparser->stack));
    expr_el_t tmp;

    if(on_top_tmp.type == NON_TERM) { //Ignore nonterminal if there is one
        tmp = pp_pop(&(pparser->stack));
        on_top_tmp = pp_top(&(pparser->stack));
        if(!pp_push(&(pparser->stack), tmp)) {
            return INTERNAL_ERROR;
        }
    }

    pparser->on_top = on_top_tmp;

    return EXPRESSION_SUCCESS;
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

    int ret = EXPRESSION_SUCCESS;
    if(is_error_token(&token_buffer->current, &ret)) {
        return ret;
    }

    return EXPRESSION_SUCCESS;
}


int has_lower_prec(pp_stack_t *stack, expr_el_t on_input) {
    //Just push '<' sign to stack and input symbol after it
    expr_el_t top = pp_top(stack);
    if(top.type == NON_TERM) {
        //If there is nonterminal push '<' after it
        pp_pop(stack);
        if(!pp_push(stack, prec_sign('<'))) {
            return INTERNAL_ERROR;
        }

        if(!pp_push(stack, top)) {
            return INTERNAL_ERROR;
        }
    }
    else {
        if(!pp_push(stack, prec_sign('<'))) {
            return INTERNAL_ERROR;
        }
    }

    if(!pp_push(stack, on_input)) {
        return INTERNAL_ERROR;
    }

    return EXPRESSION_SUCCESS;
}


void print_err_message(int *return_value, 
                       tok_buffer_t *token_buffer, 
                       char **err_m) {

    pos_t r = token_buffer->scanner->cursor_pos[ROW]; //Position of scanner cursor
    pos_t c = token_buffer->scanner->cursor_pos[COL];
    char * attr = get_attr(&(token_buffer->current), token_buffer->scanner); //Attribute of current token

    switch(*return_value)
    {
    case LEXICAL_ERROR:
        break;

    case INTERNAL_ERROR:
        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[1;31mInternal error:\033[0m ", r, c);
        fprintf(stderr, "An error ocurred during precedence parsing!\n");
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
        fprintf(stderr, "Invalid combination of tokens in expression! (nearby \"\033[1;33m%s\033[0m\")\n", attr);
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


void undefined_var_warning(tok_buffer_t *token_buffer, char *variable_name) {
    if(PRINT_EXPR_WARNINGS) {
        pos_t r = token_buffer->scanner->cursor_pos[ROW]; //Position of scanner cursor
        pos_t c = token_buffer->scanner->cursor_pos[COL];

        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| \033[1;33mWarning:\033[0m ", r, c);
        fprintf(stderr, "Uninitialized variable '\033[1;33m%s\033[0m'! It is implicitly nil (but is not nil type)!\n", variable_name);
    }
}


void prepare_buffer(scanner_t *sc, tok_buffer_t *tok_b) {
    tok_b->scanner = sc;
    token_init(&(tok_b->current));
    token_init(&(tok_b->last));
}


int prepare_pp(prog_t *dst, p_parser_t *pp) {
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
    pp->was_f_call = false;

    //Presume that it is only function call (important for stack popping and return codes)
    pp->only_f_was_called = true;

    pp->last_call_ret_num = 0;
    pp->dst_code = dst;

    return EXPRESSION_SUCCESS;
}


int update_structs(scanner_t *sc, symbol_tables_t *s, 
                   tok_buffer_t *tok_buff, p_parser_t *pparser, prog_t *dst) {

    int ret = EXPRESSION_SUCCESS;
    tok_buff->current = lookahead(sc);
    if(is_error_token(&tok_buff->current, &ret)) {
        return ret;
    }

    ret = get_top_symbol(pparser); //Update top symbol (must be before get_input_symbol)
    if(ret != EXPRESSION_SUCCESS) {
        return ret;
    }

    ret = get_input_symbol(pparser, tok_buff, s, dst); //Update input symbol
    if(ret != EXPRESSION_SUCCESS) {
        return ret;
    }

    return ret;
}


bool is_end_of_parsing(p_parser_t *pparser, int *return_value) {
    if(pparser->on_top.type == STOP_SYM && 
       pparser->on_input.type == STOP_SYM) {

        if(pparser->empty_expr) {
            *return_value = MISSING_EXPRESSION; //There cannot be empty expression if it was expected
        }
        else {
            *return_value = EXPRESSION_SUCCESS; //All reductions were OK and there is end of expression
        }
        
        return true;
    }

    return false;
}


int parse_expression(scanner_t *sc, symbol_tables_t *s, string_t *dtypes, 
                     bool *is_only_f_call, prog_t *dst) {

    int ret = EXPRESSION_SUCCESS;
    char *failed_op_msg = NULL;
    
    tok_buffer_t tok_buff;
    prepare_buffer(sc, &tok_buff);

    p_parser_t pparser;
    ret = prepare_pp(dst, &pparser);
    if(ret != EXPRESSION_SUCCESS) {
        return ret;
    }
    
    while(ret == EXPRESSION_SUCCESS) { //Main cycle
        ret = update_structs(sc, s, &tok_buff, &pparser, dst);
        if(ret != EXPRESSION_SUCCESS) {
            break;
        }

        // There is end of the expression on input and stop symbol at the top of the stack
        if(is_end_of_parsing(&pparser, &ret)) {
            break;
        }        

        char precedence = get_precedence(pparser.on_top, pparser.on_input);

        //fprintf(stderr, "%s: %c Top: %d Input: %d", get_attr(&tok_buff.current, sc), precedence, pparser.on_top.type, pparser.on_input.type);
        if(precedence == '=') {
            if(!pp_push(&pparser.stack, pparser.on_input)) {
                ret = INTERNAL_ERROR;
            }

            // Make last token from current token
            ret = (ret == EXPRESSION_SUCCESS) ? token_aging(&tok_buff) : ret;
            pparser.empty_cycle = false;
        }
        else if(precedence == '<') { /**< Put input symbol to the top of the stack*/
            ret = has_lower_prec(&(pparser.stack), pparser.on_input);
            ret = (ret == EXPRESSION_SUCCESS) ? token_aging(&tok_buff) : ret;
            pparser.empty_cycle = false;
        }
        else if(precedence == '>') { /**< Basicaly, reduce while you can't put input symbol to the top of the stack*/
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

    *is_only_f_call = pparser.only_f_was_called;

    //Print error msg to terminal or adjust error code and free resources
    print_err_message(&ret, &tok_buff, &failed_op_msg);
    free_everything(&pparser);

    //token_t next = lookahead(sc); fprintf(stderr, "REST: %s\n", get_attr(&next, sc));
    return ret;
}

/***                     End of precedence_parser.c                        ***/
