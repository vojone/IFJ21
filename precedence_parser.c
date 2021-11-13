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
 * @brief Transforms token to symbol used in precedence parser (see grm_sym_type_t in .h)
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

            return PP_ERROR;
        case '~':
            return NOTEQ;
        case '(':
            return L_PAR;
        case ')':
            return R_PAR;
        case '.':
            return CONCAT;
        default:
            return PP_ERROR;
        }
    }//switch(first_ch)

    return PP_ERROR;
}

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
        result.dtype = 123;
        break;
    }

    result.value = current->attr;

    return result;
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
    static char * right_sides[] = {
        "#",
        "_",
        "*",
        "/",
        "//",
        "+",
        "-",
        "..",
        "<",
        "<=",
        ">",
        ">=",
        "==",
        "~=",
        "(",
        ")",
        "i",
        "$",
        "E",
        NULL
    };

    return right_sides[expression_element.type];
}

/**
 * @brief Contains rules of reduction on top of the pp_stack
 */ 
expr_rule_t *get_rule(unsigned int index) {
    static expr_rule_t rules[] = {
        {"(E)", "*", ORIGIN},
        {"i", "*", ORIGIN},
        {"E+E", "ni|ni", ORIGIN},
        {"E-E", "ni|ni", ORIGIN},
        {"E*E", "ni|ni", ORIGIN},
        {"E/E", "ni|ni", ORIGIN},
        {"E//E", "ni|ni", ORIGIN},
        {"_E", "ni", ORIGIN},
        {"#E", "s", INT},
        {"E<E", "ni(s|ni", INT},
        {"E>E", "ni(s|ni", INT},
        {"E<=E", "ni(s|ni", INT},
        {"E>=E", "ni(s|ni", INT},
        {"E==E", "ni(s|ni", INT},
        {"E~=E", "ni(s|ni", INT},
        {"E..E", "s|s", STR},
        {NULL, NULL, NUM}
    };

    return &(rules[index]);
}

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

bool type_check(pp_op_stack_t *op_stack, expr_rule_t *rule, sym_dtype_t* res) {
    expr_el_t current = pp_op_pop(op_stack);
    fprintf(stderr, "%d\n", current.dtype);

    bool is_ok = false;
    sym_dtype_t res_type = rule->return_type, must_be = ORIGIN;
    char c;
    for(int i = 0; (c = rule->operator_types[i]) != '\0' ; i++) {
        if(c == '|' && !is_ok) { //Cannot found corresponding symbol for current datatype in rule
            *res = res_type;
            return false;
        }
        else if(c == '|') {
            if(!pp_op_is_empty(op_stack)) {
                current = pp_op_pop(op_stack);
                is_ok = false;
                fprintf(stderr, "%d\n", current.dtype);
            }
            else {
                *res = res_type;
                return false;
            }
        }
        else if(c == '(') {
            char next_char = rule->operator_types[i + 1];
            must_be = char_to_dtype(next_char);
        }
        
       if(must_be != ORIGIN) {
            if(current.dtype == must_be) {
                is_ok = true;
            }
        }
        else {
            if(c == '*' || current.dtype == char_to_dtype(c)) {
                is_ok = true;
            }
        }

        if(is_ok) {
            if(res_type == ORIGIN || //Result of operation will have same type as his operators
               (res_type == INT && current.dtype == NUM)) { //If at least on operan is NUM result type is num 
                res_type = current.dtype;
            }
        }
    }

    *res = res_type;
    return true;
}

/**
 * @brief Tries to reduce top of stack to non_terminal due to rules in get_rule()
 */ 
bool reduce_top(pp_stack_t *s) {
    string_t to_be_reduced;
    str_init(&to_be_reduced);

    pp_op_stack_t operands;
    pp_op_stack_init(&operands);

    expr_el_t from_top;
    while((from_top = pp_top(s)).type != STOP_SYM) {
        if(from_top.type == '<') {
            pp_pop(s);
            break;
        }

        if(from_top.type == NON_TERM || from_top.type == OPERAND) {
            pp_op_push(&operands, from_top);
        }

        char *char_seq = to_char_seqence(from_top);
        prep_str(&to_be_reduced, char_seq);
        pp_pop(s);
    }


    for(int i = 0; get_rule(i)->right_side; i++) {
        if(strcmp(to_str(&to_be_reduced), get_rule(i)->right_side) == 0) {
            fprintf(stderr, "REDUCED: %s\n", get_rule(i)->right_side);

            sym_dtype_t result_type;
            bool ok = type_check(&operands, get_rule(i), &result_type);
            fprintf(stderr, "=%d %d\n", ok, result_type);
            pp_op_stack_dtor(&operands);
            str_dtor(&to_be_reduced);

            expr_el_t non_terminal;
            non_terminal.type = NON_TERM;
            non_terminal.dtype = result_type;
            pp_push(s, non_terminal);
            return true;
        }
    }

    pp_op_stack_dtor(&operands);
    str_dtor(&to_be_reduced);
    return false;
}

expr_el_t stop_symbol() {
    expr_el_t stop_symbol;

    stop_symbol.type = STOP_SYM;
    stop_symbol.dtype = NUM;
    stop_symbol.value = NULL;

    return stop_symbol;
}

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
    precedence_sign.dtype = NUM;

    return precedence_sign;
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
