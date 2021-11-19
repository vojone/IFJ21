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

bool is_allowed_separator(scanner_t *sc, token_t *token) {
    return token->token_type == SEPARATOR && 
           (str_cmp(get_attr(token, sc), ")") == 0 || 
           str_cmp(get_attr(token, sc), "(") == 0);
}

/**
 * @brief Resolves if token can be part of expression
 */ 
bool is_EOE(scanner_t *sc, token_t *token) {
    token_type_t type = token->token_type;

    if(type == IDENTIFIER || type == OPERATOR ||
       is_allowed_separator(sc, token) || type == STRING ||
       type == NUMBER || type == INTEGER) {
        return false;
    }
    else {
        return true;
    }
}

/**
 * @brief Tries to determine if current token is unary or binary minus operator (due to last_token)
 */ 
bool is_unary_minus(scanner_t *sc, token_t *last_token) {
    return  last_token == UNKNOWN || 
            str_cmp(get_attr(last_token, sc), "(") == 0 ||
            last_token->token_type == OPERATOR;
}

/**
 * @brief Transforms token to symbol used in precedence parser (see expr_el_t in .h)
 */ 
int tok_to_type(scanner_t *sc, token_t *last_token, token_t * token) {
    if(token->token_type == IDENTIFIER || token->token_type == NUMBER ||
       token->token_type == STRING || token->token_type == INTEGER) {
           return OPERAND;
    }
    else if(token->token_type == OPERATOR || 
            token->token_type == SEPARATOR) {
        int char_num = 0;
        char first_ch = (get_attr(token, sc))[char_num++],
        next_ch = (get_attr(token, sc))[char_num];

        switch (first_ch)
        {
        case '#':
            return HASH;
        case '*':
            return MULT;
        case '-':
            if(is_unary_minus(sc, last_token)) {
                return MINUS;
            }
            return SUB;
        case '/':
            if(next_ch == '/') {
                return INT_DIV;
            }
            return DIV;
        case '+':
            return ADD;
        case '<':
            if(next_ch == '=') {
                return LTE;
            }
            return LT;
        case '>':
            if(next_ch == '=') {
                return GTE;
            }
            return GT;
        case '=':
            if(next_ch == '=') {
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
        }
    }//switch(first_ch)

    return UNDEFINED;
}

/**
 * @brief Creates expression element from current and last token
 */ 
int from_input_token(expr_el_t *result, scanner_t *sc, symtab_t *symtab,
                     token_t *last, token_t *current) {

    result->type = tok_to_type(sc, last, current);
    result->value = NULL;
    tree_node_t * symbol;
    switch (current->token_type)
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
        symbol = search(symtab, get_attr(current, sc));
        if(symbol == NULL) {
            return UNDECLARED_IDENTIFIER;
        }

        result->dtype = symbol->data.dtype;
        break;
    default:
        result->dtype = UNDEFINED;
        break;
    }

    char * curr_val = get_attr(current, sc);
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

    return precedence_sign;
}

/**
 * @brief Contains precedence table of operators in IFJ21
 */ 
char get_precedence(expr_el_t on_stack_top, expr_el_t on_input) {
    static char precedence_table[TERM_NUM][TERM_NUM] = {
    //   #    _   *   /   //  +   -  ..   <  <=  >  >=  ==  ~=   (   )   i   $
/*#*/  {'>','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'}, 
/*_*/  {'<','>','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/***/  {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*/*/  {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*//*/ {'<','<','>','>','>','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*+*/  {'<','<','<','<','<','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*-*/  {'<','<','<','<','<','>','>','>','>','>','>','>','>','>','<','>','<','>'},
/*..*/ {'<','<','<','<','<','>','>','>','>','>','>','>','>','>','<','>','<','>'},
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
 *       ( types symbols after it defines type for resting operators (integer and number are compatible)
 *       Example: (nis|nis = First and sec operand can be number/integer/string and if it's e. g. string second must be string too
 */ 
expr_rule_t *get_rule(unsigned int index) {
    if(index >= REDUCTION_RULES_NUM) { //Safety check
        return NULL;
    }

    static expr_rule_t rules[REDUCTION_RULES_NUM] = {
        {"(E)", "*", ORIGIN, NULL},
        {"i", "*", ORIGIN, NULL},
        {"E+E", "ni|ni", ORIGIN, "\"+\" expects number/integer as operands"},
        {"E-E", "ni|ni", ORIGIN, "\"-\" expects number/integer as operands"},
        {"E*E", "ni|ni", ORIGIN, "\"*\" expects number/integer as operands"},
        {"E/E", "ni|ni", ORIGIN, "\"/\" expects number/integer as operands"},
        {"E//E", "ni|ni", ORIGIN, "\"//\" expects number/integer as operands"},
        {"_E", "ni", ORIGIN, "Unary minus expects number/integer as operands"},
        {"#E", "s", INT, "Only string can be operand of \"#\""},
        {"E<E", "(nis|nis", INT, "Incompatible operands of \"<\""},
        {"E>E", "(nis|nis", INT, "Incompatible operands of \">\""},
        {"E<=E", "(nis|nis", INT, "Incompatible operands of \"<=\""},
        {"E>=E", "(nis|nis", INT, "Incompatible operands of \">=\""},
        {"E==E", "(nis|nis", INT, "Incompatible operands of \"==\""},
        {"E~=E", "(nis|nis", INT, "Incompatible operands of \"~=\""},
        {"E..E", "s|s", STR, "Operation \"..\" needs strings as operands"},
    };

    return &(rules[index]);
}

/**
 * @brief Pops operand stack if it is not empty
 */ 
expr_el_t safe_op_pop(bool *cur_ok, bool* check_res, pp_stack_t *op_stack) {
    if(!pp_is_empty(op_stack)) {
        *cur_ok = false;
        return pp_pop(op_stack);
    }
    else {
        *cur_ok = false;
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

bool type_check(pp_stack_t op_stack, expr_rule_t *rule, sym_dtype_t* res_t) {

    expr_el_t current = pp_pop(&op_stack);
    bool is_curr_op_ok = false, must_be_flag = false, result = true;
    sym_dtype_t tmp_res_type = rule->return_type, must_be = UNDEFINED;

    char c;
    for(int i = 0; (c = rule->operator_types[i]) != '\0'; i++) {
        if(c == '|' && !is_curr_op_ok) { //Cannot found corresponding symbol for current datatype in rule
            result = false;
            break;
        }
        else if(c == '|' || c == '(') {
            if(c == '|')
                current = safe_op_pop(&is_curr_op_ok, &result, &op_stack);
            else
                must_be_flag = true;

            continue;
        }

        //Operand before current op. defined type of resting operands
        if(must_be_flag && 
          (is_compatible(current.dtype, must_be) || must_be == UNDEFINED)) {
            is_curr_op_ok = true;
            if(must_be == UNDEFINED && current.dtype == char_to_dtype(c)) {
                must_be = current.dtype;
            }
        }
        else if(!must_be_flag) { 
            if(c == '*' || current.dtype == char_to_dtype(c)) {
                is_curr_op_ok = true; //Operand type corresponds with current possiblity
            }
        }

        resolve_res_type(&tmp_res_type, rule, current, is_curr_op_ok);
    }

    result = (!is_curr_op_ok && c == '\0') ?  false : result;

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


void print_operands(pp_stack_t *ops) {
    while(!pp_is_empty(ops)) {
        fprintf(stderr, "Operand: %s\n", (char *)pp_pop(ops).value);
    }
}

/**
 * @brief Creates non-terminal (E) expression symbol (there is only one non-terminal in rules)
 */ 
expr_el_t non_term(sym_dtype_t data_type) {
    expr_el_t non_terminal;
    non_terminal.type = NON_TERM;
    non_terminal.dtype = data_type;
    non_terminal.value = "NONTERM";

    return non_terminal;
}

/**
 * @brief Makes reduction of top of the main stack including type check and resolving errors
 * @param st Main stack where reductions are performed
 * @param ops Stack with operands
 * @param rule Rule containing information about result type and operand data types
 * @param err_m Pointer will be set to the string that explains semantic error in expression (if occured)
 */ 
int reduce(pp_stack_t *st, pp_stack_t *ops, expr_rule_t *rule, char **err_m) {

    sym_dtype_t result_type;
    *err_m = NULL;

    bool t_check_res = type_check(*ops, rule, &result_type);
    if(!t_check_res) { /**< Type check was not succesfull */
        *err_m = rule->error_message;
        return SEM_ERROR_IN_EXPR;
    }

    print_operands(ops); /**< It will be probably substituted for code generating */

    if(!pp_push(st, non_term(result_type))) {
        return INTERNAL_ERROR;
    }

    return EXPRESSION_SUCCESS;
}

/**
 * @brief Tries to reduce top of stack to non_terminal due to rules in get_rule()
 */ 
int reduce_top(pp_stack_t *s, char ** failed_operation) {
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
            retval = reduce(s, &operands, rule, failed_operation);
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
int get_input_symbol(expr_el_t *on_input, scanner_t *sc, token_t *last,
                           token_t *current, symtab_t * symtab,
                           pp_stack_t *garbage_stack) {

    int retval = EXPRESSION_SUCCESS;
    if(is_EOE(sc, current)) {
        *on_input = stop_symbol();
    }
    else {
        retval = from_input_token(on_input, sc, symtab, last, current);
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
        on_top_tmp = pp_pop(stack);
        pp_push(stack, on_top_tmp);
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
void token_aging(scanner_t *sc, token_t *last, token_t *cur) {
    *last = *cur;
    *cur = get_next_token(sc);
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
 */ 
void print_err_message(int return_value, scanner_t *sc, 
                       token_t *last_tok, token_t *cur_tok, 
                       char **err_m) {

    switch(return_value)
    {
    case LEXICAL_ERROR:
        fprintf(stderr, 
                "(%ld:%ld)\t| \033[0;31mLexical error:\033[0m Not recognized token! (\"%s\")\n",
                sc->cursor_pos[ROW], 
                sc->cursor_pos[COL], 
                get_attr(cur_tok, sc));
        break;
    case SEM_ERROR_IN_EXPR:
        fprintf(stderr, 
                "(%ld:%ld)\t| \033[0;31mSemantic error:\033[0m Bad data types in expression!\n",
                sc->cursor_pos[ROW], 
                sc->cursor_pos[COL]);
        if(*err_m) {
            fprintf(stderr, "\t| %s\n", *err_m);
        }

        break;
    case UNDECLARED_IDENTIFIER:
        fprintf(stderr, 
                "(%ld:%ld)\t| \033[0;31mSemantic error:\033[0m Undeclared identifier \"%s\"!\n",
                sc->cursor_pos[ROW], 
                sc->cursor_pos[COL],
                get_attr(cur_tok, sc));

        break;
    case EXPRESSION_FAILURE:
        fprintf(stderr, 
                "(%ld:%ld)\t| \033[0;31mSyntax error:\033[0m Invalid combination of tokens in expression! (\"%s%s\")\n", 
                sc->cursor_pos[ROW], 
                sc->cursor_pos[COL],
                get_attr(last_tok, sc),
                get_attr(cur_tok, sc));

        break;
    default:
        break;
    }
}

int parse_expression(scanner_t *sc, symtab_t *symtab) {
    int retval = EXPRESSION_SUCCESS;
    char *failed_op = NULL;
    token_t current_token, last_token;
    token_init(&last_token);
    token_init(&current_token);

    pp_stack_t stack;
    pp_stack_t garbage;
    if(!pp_stack_init(&stack) || !pp_stack_init(&garbage)) {
        retval = INTERNAL_ERROR;
    }

    if(!pp_push(&stack, stop_symbol())) {
        retval = INTERNAL_ERROR;
    }
    
    while(retval == EXPRESSION_SUCCESS) {
        current_token = lookahead(sc);

        if(current_token.token_type == ERROR_TYPE) {
            retval = LEXICAL_ERROR;
            break;
        }

        expr_el_t on_input, on_top;
        retval = get_input_symbol(&on_input, sc, &last_token, &current_token, symtab, &garbage);
        if(retval != EXPRESSION_SUCCESS) {
            break;
        }

        get_top_symbol(&on_top, &stack);
        /*There is end of the expression on input and stop symbol at the top of the stack*/
        if(on_top.type == STOP_SYM && on_input.type == STOP_SYM) {
            retval = EXPRESSION_SUCCESS;
            break;
        }
        char precedence = get_precedence(on_top, on_input);
        //fprintf(stderr, "%s: %c %d %d\n", get_attr(&current_token, sc), precedence, on_top.type, on_input.type);
        if(precedence == '=') {
            if(!pp_push(&stack, on_input)) {
                retval = INTERNAL_ERROR;
            }

            token_aging(sc, &last_token, &current_token); //TODO
        }
        else if(precedence == '<') {  /**< Put input symbol to the top of the stack*/
            has_lower_prec(&stack, on_input);
            token_aging(sc, &last_token, &current_token); //TODO
        }
        else if(precedence == '>') { /**< Basicaly, reduct while you can't put input symbol to the top of the stack*/
            retval = reduce_top(&stack, &failed_op);
        }
        else {
            retval = EXPRESSION_SUCCESS;
            break;
        }
    }

    print_err_message(retval, sc, &last_token, &current_token, &failed_op);
    free_everything(&stack, &garbage);
    //token_t next = lookahead(sc); fprintf(stderr, "%s\n", get_attr(&next, sc));
    return retval;
}

/***                     End of precedence_parser.c                        ***/
