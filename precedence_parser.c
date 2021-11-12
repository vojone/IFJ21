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
 * @brief Transforms token to symbol used in precedence parser (see grm_sym_type_t in .h)
 */ 
int tok_to_term_type(token_t *last_token, token_t * token) {
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
            if(last_token == UNKNOWN ||
               strcmp(last_token->attr, "(") == 0 ||
               last_token->token_type == OPERATOR) {
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

/**
 * @brief Contains precedence table of operators in IFJ21
 */ 
char get_precedence(int on_stack_top, int on_input) {
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

    return precedence_table[on_stack_top][on_input];
}

/**
 * 
 */ 
char *to_char_seqence(int integer_symbol) {
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

    return right_sides[integer_symbol];
}

/**
 * @brief Contains rules of reduction on top of the pp_stack
 */ 
char *get_rule(unsigned int index) {
    static char * right_sides[] = {
        "(E)",
        "i",
        "E+E",
        "E-E",
        "E*E",
        "E/E",
        "E//E",
        "#E",
        "_E",
        "<E",
        ">E",
        "E<=E",
        "E>=E",
        "E==E",
        "E~=E",
        "E..E",
        NULL
    };

    return right_sides[index];
}

/**
 * @brief Tries to reduce top of stack to non_terminal due to rules in get_rule()
 */ 
bool reduce_top(scanner_t *sc, pp_stack_t *s) {
    string_t to_be_reduced;
    str_init(&to_be_reduced);

    char from_top;
    while((from_top = pp_top(s)) != STOP_SYM) {
        if(from_top == '<') {
            pp_pop(s);
            break;
        }

        char *char_seq = to_char_seqence(from_top);
        prep_str(&to_be_reduced, char_seq);
        pp_pop(s);
    }

    for(int i = 0; get_rule(i); i++) {
        if(strcmp(to_str(&to_be_reduced), get_rule(i)) == 0) {
            fprintf(stderr, "REDUCED: %s\n", get_rule(i));
            str_dtor(&to_be_reduced);
            pp_push(s, NON_TERM);
            return true;
        }
    }

    str_dtor(&to_be_reduced);
    return false;
}

bool parse_expression(scanner_t *sc) {
    token_t current_token, last_token;

    pp_stack_t stack;
    pp_stack_init(&stack);
    pp_push(&stack, STOP_SYM);

    last_token.token_type = UNKNOWN;
    while(true) {
        current_token = lookahead(sc);
        if(current_token.token_type == ERROR_TYPE) {
            fprintf(stderr, "Lexical error: Not recognized token ! (\"%s\")\n", (char *)current_token.attr);
            return false;
        }

        int on_input;
        if(is_EOE(&current_token)) {
            on_input = STOP_SYM;
        }
        else {
            on_input = tok_to_term_type(&last_token, &current_token);
        }
        
        int on_top = pp_top(&stack);
        int tmp = -1;
        if(on_top == NON_TERM) {
            tmp = pp_pop(&stack);
            on_top = pp_pop(&stack);
            pp_push(&stack, on_top);
            pp_push(&stack, tmp);
        }

        if(on_top == STOP_SYM && on_input == STOP_SYM) {
            return true;
        }

        int precedence = get_precedence(on_top, on_input);

        //fprintf(stderr, "%s: %c %d %d\n", (char *)current_token.attr, precedence, on_top, on_input);
        if(precedence == '=') {
            pp_push(&stack, on_input);
            last_token = current_token;
            current_token = get_next_token(sc);
        }
        else if(precedence == '<') {
            if(pp_top(&stack) == NON_TERM) {
                pp_pop(&stack);
                pp_push(&stack, '<');
                pp_push(&stack, NON_TERM);
            }
            else {
                pp_push(&stack, '<');
            }
    
            pp_push(&stack, on_input);
            last_token = current_token;
            current_token = get_next_token(sc);
        }
        else if(precedence == '>') {
            bool is_reduced = reduce_top(sc, &stack);
        
            if(!is_reduced) {
                fprintf(stderr, "Precedence parser: Cannot reduce the top of the stack!\n");
                return false;
            }
            else {
                fprintf(stderr, "Reducted! On top:%d\n", pp_top(&stack));
                pp_show(&stack);
            }
        }
        else {
            fprintf(stderr, "Precedence parser: Unallowed combination of symbols!\n");
            return false;
        }
    }

    return true;
}

/***                     End of precedence_parser.c                        ***/
