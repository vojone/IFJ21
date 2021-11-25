/******************************************************************************
 *                                  IFJ21
 *                            precedence_parser.c
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *              Purpose:  Implementation of precedence parsing (PP)
 * 
 *                    Last change:
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
 * @brief Creates expression element from current and last token
 */ 
int from_input_token(expr_el_t *result, 
                     tok_buffer_t *tok_b,
                     symtabs_stack_t *symstack,
                     symtab_t *symtab) {

    result->type = tok_to_type(tok_b);
    result->value = NULL;
    result->is_zero = false;
    tree_node_t * symbol;
    switch (tok_b->current.token_type)
    {
    case INTEGER:
        result->dtype = INT;
        break;
    case NUMBER:
        result->dtype = NUM;
        break;
    case STRING:
        result->dtype = STR;
        break;
    case IDENTIFIER:
        symbol = search_in_tables(symstack, symtab, 
                                  get_attr(&(tok_b->current), 
                                  tok_b->scanner));

        if(symbol == NULL) {
            return UNDECLARED_IDENTIFIER;
        }

        result->dtype = symbol->data.dtype;
        break;
    default:
        if(is_nil(tok_b->scanner, &tok_b->current)) {
            result->dtype = NIL;
        }
        else {
            result->dtype = UNDEFINED;
        }

        break;
    }

    if(is_zero(tok_b->scanner, &tok_b->current)) {
        result->is_zero = true;
    }

    char * curr_val = get_attr(&(tok_b->current), tok_b->scanner);
    str_cpy((char **)&result->value, curr_val, strlen(curr_val));

    return EXPRESSION_SUCCESS;
}

/**
 * @brief Creates stop symbol and initializes it
 */ 
expr_el_t stop_symbol() {
    expr_el_t stop_symbol;

    stop_symbol.type = STOP_SYM;
    stop_symbol.dtype = UNDEFINED;
    stop_symbol.value = NULL;
    stop_symbol.is_zero = false;

    return stop_symbol;
}

/**
 * @brief Creates precedence sign due to given parameter and initializes it
 */ 
expr_el_t prec_sign(char sign) {
    expr_el_t precedence_sign;
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
    precedence_sign.dtype = UNDEFINED;
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
        {"_E", "ni", ORIGIN, FIRST, "Unary minus expects number/integer as operands", generate_operation_unary_minus},
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
bool is_compatible(sym_dtype_t dtype1, sym_dtype_t dtype2) {
    return (dtype1 == dtype2) ||
           (dtype1 == NUM && dtype2 == INT) || 
           (dtype2 == NUM && dtype1 == INT);
}

/**
 * @brief Resolves which data type attribut should have newly created nonterminal on stack
 */ 
void resolve_res_type(sym_dtype_t *res, expr_rule_t *rule, 
                      expr_el_t cur_op, bool cur_ok) {

    if(*res == UNDEFINED) {
        if(cur_ok) {
            *res = cur_op.dtype;
        }
    }
    
    if(rule->return_type != UNDEFINED) {
        *res = rule->return_type;
    }
    else if(cur_op.dtype == NUM && *res == INT) {
        *res = NUM;
    }
}

int get_tcheck_ret(expr_el_t *current_operand) {
    int result;
    if(current_operand->dtype == NIL) {
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
int type_check(pp_stack_t op_stack, expr_rule_t *rule, sym_dtype_t* res_t) {

    expr_el_t current = pp_pop(&op_stack);
    bool is_curr_ok = false, must_be_flag = false;
    int result = EXPRESSION_SUCCESS;
    sym_dtype_t tmp_res_type = rule->return_type, must_be = UNDEFINED;

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
                if(must_be == UNDEFINED && current.dtype == char_to_dtype(c)) {
                    must_be = current.dtype;
                }

                if(is_compatible(current.dtype, must_be)) { /**< Type of current operator is compatible with type of earlier operator */
                    is_curr_ok = true;
                }
            }
            else { /**< Normal mode */
                if(c == '*' || current.dtype == char_to_dtype(c)) {
                    is_curr_ok = true; //Operand type corresponds with operator specifier
                }
            }
        }

        resolve_res_type(&tmp_res_type, rule, current, is_curr_ok);
    }

    // Specifier string ended -> if specifier for last operator was not found -> error
    result = (!is_curr_ok && c == '\0') ?  get_tcheck_ret(&current) : result;
    *res_t = tmp_res_type;
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

    fprintf(stderr, "To be reduced: %s\n", to_be_red->str);
}


void print_operands(pp_stack_t *ops, symtabs_stack_t *sym_stack, 
                    symtab_t *symtab) {

    while(!pp_is_empty(ops)) {
        expr_el_t cur = pp_pop(ops);
        fprintf(stderr, "Operand: %s (%d)\n", (char *)cur.value, cur.is_zero);

        tree_node_t *symbol = search_in_tables(sym_stack, symtab, (char *)cur.value);
        if(symbol) {
            fprintf(stderr, "Operand name: %s\n", to_str(&symbol->data.name));
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
expr_el_t non_term(sym_dtype_t data_type, bool is_zero) {
    expr_el_t non_terminal;
    non_terminal.type = NON_TERM;
    non_terminal.dtype = data_type;
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
int reduce(pp_stack_t *st, pp_stack_t *ops, symtabs_stack_t *sym_stack,
           symtab_t *symtab,
           expr_rule_t *rule,
           sym_dtype_t *result_type,
           bool will_be_zero) {
    
    if(strcmp(rule->right_side, "i") == 0){
        expr_el_t element_terminal = pp_top(ops);
        tree_node_t *res = search_in_tables(sym_stack, symtab, element_terminal.value);
        if(res == NULL){
            //we are pushing a static value
            generate_value_push(VAL,element_terminal.dtype,element_terminal.value);
        }else{
            //we are pushing variable
            fprintf(stderr,"Pushing variable %s to stack\n",res->data.name.str);
            generate_value_push(VAR, res->data.dtype , res->data.name.str);
        }
    }
    //generate operation code
    if(rule->generator_function != NULL)
        rule->generator_function();
    print_operands(ops, sym_stack, symtab); /**< It will be probably substituted for code generating */

    if(!pp_push(st, non_term(*result_type, will_be_zero))) {
        return INTERNAL_ERROR;
    }

    return EXPRESSION_SUCCESS;
}

/**
 * @brief Tries to reduce top of stack to non_terminal due to rules in get_rule()
 */ 
int reduce_top(pp_stack_t *s, symtabs_stack_t *sym_stack, symtab_t *symtab,
               char ** failed_op_msg, sym_dtype_t *ret_type) {

    string_t to_be_reduced;
    str_init(&to_be_reduced);

    pp_stack_t operands;
    if(!pp_stack_init(&operands)) {
        return INTERNAL_ERROR;
    }

    get_str_to_reduction(s, &operands, &to_be_reduced); /**< Takes top of the stack and creates substitutable string  from it*/
    int retval = EXPRESSION_FAILURE; /**< If rule is not found it is invalid operation -> return EXPR_FAILURE */
    expr_rule_t *rule;
    for(int i = 0; (rule = get_rule(i)); i++) {
        if(str_cmp(to_str(&to_be_reduced), rule->right_side) == 0) {
            int t_check_res = type_check(operands, rule, ret_type);
            if(t_check_res != EXPRESSION_SUCCESS) { /**< Type check was not succesfull */
                *failed_op_msg = rule->error_message;
                retval = t_check_res;
            }
            else {
                bool will_be_zero = resolve_res_zero(operands, rule);
                retval = reduce(s, &operands, sym_stack,
                                symtab, rule, ret_type, will_be_zero);
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
int get_input_symbol(bool stop_flag,
                     expr_el_t *on_input, 
                     tok_buffer_t *t_buff, 
                     symtabs_stack_t *symstack,
                     symtab_t *symtab,
                     pp_stack_t *garbage_stack) {

    int retval = EXPRESSION_SUCCESS;
    if(stop_flag || is_EOE(t_buff->scanner, &(t_buff->current))) {
        *on_input = stop_symbol();
    }
    else {
        retval = from_input_token(on_input, t_buff, symstack, symtab);
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
        free(current_el.value);
    }
    
    pp_stack_dtor(garbage);
    pp_stack_dtor(stack);
}

/**
 * @brief Makes last token from current token and destructs old token
 */ 
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
        fprintf(stderr, "(%ld:%ld)\t| \033[0;31mLexical error:\033[0m ", r, c);
        fprintf(stderr, "Not recognized token! (\"\033[1;33m%s\033[0m\")\n", attr);
        break;

    case SEM_ERROR_IN_EXPR:
        fprintf(stderr, "(%ld:%ld)\t| \033[0;31mSemantic error:\033[0m ", r, c);
        fprintf(stderr, "Bad data types in expression!\n");
        if(*err_m) {
            fprintf(stderr, "\t| \033[0;33m%s\033[0m\n", *err_m);
        }

        break;

    case UNDECLARED_IDENTIFIER:
        fprintf(stderr, "(%ld:%ld)\t| \033[0;31mSemantic error:\033[0m ", r, c);
        fprintf(stderr, "Undeclared identifier \"\033[1;33m%s\033[0m\"!\n", attr);
        break;

    case NIL_ERROR:
        fprintf(stderr, "(%ld:%ld)\t| \033[0;31mSemantic (nil) error:\033[0m ", r, c);
        fprintf(stderr, "Cannot use nil in expression like this!\n");
        break;
    
    case DIV_BY_ZERO:
        fprintf(stderr, "(%ld:%ld)\t| \033[0;31mDivision by zero:\033[0m ", r, c);
        fprintf(stderr, "Cannot divide by zero!\n");
        break;

    case EXPRESSION_FAILURE:
        fprintf(stderr, "(%ld:%ld)\t| \033[0;31mSyntax error:\033[0m ", r, c);
        fprintf(stderr, "Invalid combination of tokens in expression! (\"\033[1;33m%s%s\033[0m\")\n", attr, lattr);
        *return_value = SYNTAX_ERROR_IN_EXPR;
        break;

    case MISSING_EXPRESSION:
        fprintf(stderr, "(%ld:%ld)\t| \033[0;31mSyntax error:\033[0m ", r, c);
        fprintf(stderr, "Expected expression, but it was not found! (found \"\033[1;33m%s\033[0m\" instead)\n", attr);
        *return_value = SYNTAX_ERROR_IN_EXPR;
        break;

    default:
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


int parse_expression(scanner_t *sc, void *symstack, 
                     symtab_t *symtab, sym_dtype_t *ret_type) {

    int retval = EXPRESSION_SUCCESS;
    char *failed_op_msg = NULL;
    
    tok_buffer_t tok_buffer;
    pp_stack_t stack;
    pp_stack_t garbage;
    
    prepare_buffer(sc, &tok_buffer);
    retval = prepare_stacks(&stack, &garbage);
    if(retval != EXPRESSION_SUCCESS) {
        return retval;
    }
    
    bool stop_flag = false, empty_expr = true, empty_cycle = false;
    while(retval == EXPRESSION_SUCCESS) { //Main cycle
        tok_buffer.current = lookahead(sc);

        if(tok_buffer.current.token_type == ERROR_TYPE) {
            retval = LEXICAL_ERROR;
            break;
        }

        expr_el_t on_input, on_top;
        retval = get_input_symbol(stop_flag, &on_input, &tok_buffer, 
                                  (symtabs_stack_t *)symstack, 
                                  symtab, &garbage);

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
        //fprintf(stderr, "%s: %c %d(%d) %d(%d) Stop flag: %d\n", get_attr(&tok_buffer.current, sc), precedence, on_top.type, on_top.is_zero, on_input.type, on_input.is_zero, stop_flag);
        if(precedence == '=') {
            if(!pp_push(&stack, on_input)) {
                retval = INTERNAL_ERROR;
            }

            token_aging(&tok_buffer); /**< Make last token from current token */
            empty_cycle = false;
        }
        else if(precedence == '<') {  /**< Put input symbol to the top of the stack*/
            has_lower_prec(&stack, on_input);
            token_aging(&tok_buffer);
            empty_cycle = false;
        }
        else if(precedence == '>') { /**< Basicaly, reduct while you can't put input symbol to the top of the stack*/
            retval = reduce_top(&stack, (symtabs_stack_t *)symstack, 
                                symtab, &failed_op_msg, ret_type);
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

    print_err_message(&retval, &tok_buffer, &failed_op_msg);
    free_everything(&stack, &garbage);

    //token_t next = lookahead(sc); fprintf(stderr, "%s\n", get_attr(&next, sc));
    return retval;
}

/***                     End of precedence_parser.c                        ***/
