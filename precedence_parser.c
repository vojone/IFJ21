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

/**
 * @brief Resolves if token can be part of expression
 */ 
bool is_EOE(token_t *token) {
    token_type_t type = token->token_type;

    if(type == IDENTIFIER || type == OPERATOR ||
       type == SEPARATOR || type == STRING ||
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
bool is_unary_minus(token_t *last_token) {
    return  last_token == UNKNOWN || 
            strcmp(last_token->attr, "(") == 0 ||
            last_token->token_type == OPERATOR;
}

/**
 * @brief Transforms token to symbol used in precedence parser (see expr_el_t in .h)
 */ 
int tok_to_type(token_t *last_token, token_t * token) {
    if(token->token_type == IDENTIFIER || token->token_type == NUMBER ||
       token->token_type == STRING || token->token_type == INTEGER) {
           return OPERAND;
    }
    else if(token->token_type == OPERATOR || 
            token->token_type == SEPARATOR) {
        int char_num = 0;
        char first_ch = ((char *)token->attr)[char_num++],
        next_ch = ((char *)token->attr)[char_num];

        switch (first_ch)
        {
        case '#':
            return HASH;
        case '*':
            return MULT;
        case '-':
            if(is_unary_minus(last_token)) {
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
expr_el_t from_input_token(token_t *last, token_t *current) {
    expr_el_t result;
    result.type = tok_to_type(last, current);
    
    switch (current->token_type)
    {
    case INTEGER:
        result.dtype = INT;
        break;
    case NUMBER:
        result.dtype = NUM;
        break;
    case STRING:
        result.dtype = STR;
        break;
    case IDENTIFIER:
        result.dtype = NUM;
        break;
    default:
        result.dtype = UNDEFINED;
        break;
    }

    result.value = current->attr;

    return result;
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

    return precedence_table[on_stack_top.type][on_input.type];
}

/**
 * @brief Additional function to get real expression element from enum type
 */ 
char *to_char_seqence(expr_el_t expression_element) {
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
    if(index > REDUCTION_RULES_NUM) { //Safety check
        return NULL;
    }

    static expr_rule_t rules[REDUCTION_RULES_NUM] = {
        {"(E)", "*", ORIGIN},
        {"i", "*", ORIGIN},
        {"E+E", "ni|ni", ORIGIN},
        {"E-E", "ni|ni", ORIGIN},
        {"E*E", "ni|ni", ORIGIN},
        {"E/E", "ni|ni", ORIGIN},
        {"E//E", "ni|ni", ORIGIN},
        {"_E", "ni", ORIGIN},
        {"#E", "s", INT},
        {"E<E", "(nis|nis", INT},
        {"E>E", "(nis|nis", INT},
        {"E<=E", "(nis|nis", INT},
        {"E>=E", "(nis|nis", INT},
        {"E==E", "(nis|nis", INT},
        {"E~=E", "(nis|nis", INT},
        {"E..E", "s|s", STR},
    };

    return &(rules[index]);
}

/**
 * @brief Converts character used in operand type grammar in get_rule() to sym_dtype enum
 */ 
sym_dtype_t char_to_dtype(char type_c) {
    sym_dtype_t type;
    switch (type_c)
    {
    case 'n':
        type = NUM;
        break;
    case 'i':
        type = INT;
        break;
    case 's':
        type = STR;
        break;
    default:
        type = UNDEFINED;
    }

    return type;
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

bool type_check(pp_stack_t *op_stack, expr_rule_t *rule, sym_dtype_t* res_t) {

    expr_el_t current = pp_pop(op_stack);
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
                current = safe_op_pop(&is_curr_op_ok, &result, op_stack);
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

        char *char_seq = to_char_seqence(from_top);
        prep_str(to_be_red, char_seq);
        pp_pop(s);
    }
}

/**
 * @brief Tries to reduce top of stack to non_terminal due to rules in get_rule()
 */ 
bool reduce_top(pp_stack_t *s) {
    string_t to_be_reduced;
    str_init(&to_be_reduced);

    pp_stack_t operands;
    pp_stack_init(&operands);

    get_str_to_reduction(s, &operands, &to_be_reduced);

    for(int i = 0; get_rule(i)->right_side; i++) {
        if(strcmp(to_str(&to_be_reduced), get_rule(i)->right_side) == 0) {
            sym_dtype_t result_type;
            bool type_check_result = type_check(&operands, get_rule(i), &result_type);

            pp_stack_dtor(&operands);
            str_dtor(&to_be_reduced);
            if(!type_check_result) {
                return false;
            }

            expr_el_t non_terminal;
            non_terminal.type = NON_TERM;
            non_terminal.dtype = result_type;
            pp_push(s, non_terminal);
            return true;
        }
    }

    pp_stack_dtor(&operands);
    str_dtor(&to_be_reduced);
    return false;
}


/**
 * @brief Gets symbol from input (if it is valid as expression element otherwise is set to STOP_SYM)
 */ 
expr_el_t get_input_symbol(token_t *last, token_t *current) {
    expr_el_t on_input;
    if(is_EOE(current)) {
        on_input = stop_symbol();
    }
    else {
        on_input = from_input_token(last, current);
    }

    return on_input;
}

/**
 * @brief Gets first nonterminal from top of the stack (There can't be sequence of them) 
 */ 
expr_el_t get_top_symbol(pp_stack_t *stack) {
    expr_el_t on_top = pp_top(stack);
    expr_el_t tmp;
    if(on_top.type == NON_TERM) {
        tmp = pp_pop(stack);
        on_top = pp_pop(stack);
        pp_push(stack, on_top);
        pp_push(stack, tmp);
    }

    return on_top;
}




/**
 * @brief Frees all resources (expecially dynamic allocated memory) of PP
 */ 
void free_everything(pp_stack_t *stack, token_t *last, token_t *cur) {
    pp_stack_dtor(stack);
    token_dtor(last);
    token_dtor(cur);
}

/**
 * @brief Makes last token from current token and destructs old token
 */ 
void token_aging(scanner_t *sc, token_t *last, token_t *cur) {
    token_dtor(last);
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
 * @brief Does necessary actions when stack top has higher precedence than input (reductions on stack top)
 * @return True if everything (reduction) goes well
 */
bool has_higher_prec(pp_stack_t *stack) {
    bool is_reduced = reduce_top(stack);
        
    if(!is_reduced) {
        fprintf(stderr, "Precedence parser: Cannot reduce the top of the stack!\n");
        return false;
    }
    else {
        fprintf(stderr, "Reducted! On top:%d\n", pp_top(stack).type);
        pp_show(stack);
    }

    return true;
}

int parse_expression(scanner_t *sc) {
    token_t current_token, last_token;
    token_init(&last_token);
    token_init(&current_token);

    pp_stack_t stack;
    pp_stack_init(&stack);
    pp_push(&stack, stop_symbol());
    while(true) {
        current_token = lookahead(sc);
        if(current_token.token_type == ERROR_TYPE) {

            fprintf(stderr, 
                    "(%ld,%ld)\t| \033[0;31mLexical error:\033[0m Not recognized token ! (\"%s\")\n", 
                    sc->cursor_pos[ROW], 
                    sc->cursor_pos[COL], 
                    (char *)current_token.attr);

            free_everything(&stack, &last_token, &current_token);
            return LEXICAL_ERROR;
        }

        expr_el_t on_input = get_input_symbol(&last_token, &current_token);
        expr_el_t on_top = get_top_symbol(&stack);
        /*There is end of the expression on input and stop symbol at the top of the stack*/
        if(on_top.type == STOP_SYM && on_input.type == STOP_SYM) {
            free_everything(&stack, &last_token, &current_token);
            return EXPRESSION_SUCCESS;
        }
        char precedence = get_precedence(on_top, on_input);

        //fprintf(stderr, "%s: %c %d %d\n", (char *)current_token.attr, precedence, on_top, on_input);
        if(precedence == '=') {
            pp_push(&stack, on_input);

            token_aging(sc, &last_token, &current_token); //TODO
        }
        else if(precedence == '<') {  /**< Put input symbol to the top of the stack*/
            has_lower_prec(&stack, on_input);
            token_aging(sc, &last_token, &current_token); //TODO
        }
        else if(precedence == '>') { /**< Basicaly, reduct while you can't put input symbol to the top of the stack*/
            if(!has_higher_prec(&stack)) {
                fprintf(stderr, 
                    "(%ld,%ld)\t| \033[0;31mSyntax error:\033[0m Invalid combination of tokens ! (\"%s%s\")\n", 
                    sc->cursor_pos[ROW], 
                    sc->cursor_pos[COL],
                    (char *)last_token.attr,
                    (char *)current_token.attr);

                free_everything(&stack, &last_token, &current_token);

                return EXPRESSION_FAILURE;
            }
        }
        else {
            fprintf(stderr, 
                    "(%ld,%ld)\t| \033[0;31mSyntax error:\033[0m Invalid combination of tokens ! (\"%s%s\")\n", 
                    sc->cursor_pos[ROW], 
                    sc->cursor_pos[COL],
                    (char *)last_token.attr,
                    (char *)current_token.attr);

            free_everything(&stack, &last_token, &current_token);

            return EXPRESSION_FAILURE;
        }
    }

    free_everything(&stack, &last_token, &current_token);
    return EXPRESSION_SUCCESS;
}

/***                     End of precedence_parser.c                        ***/
