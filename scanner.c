/******************************************************************************
 *                                  IFJ21
 *                                scanner.c
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *           Purpose: Source file with implementation of lexer (scanner)
 * 
 *                  Last change: 
 *****************************************************************************/ 

/**
 * @file scanner.c
 * @brief Source file with implementation of lexer (scanner)
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#include "scanner.h"


/**
 * Prepares scanner structure and sets its attributes to initial values
 */
void scanner_init(scanner_t *sc) {
    sc->state = INIT;
    sc->is_input_buffer_full = false;

    sc->is_tok_buffer_full = false;

    sc->cursor_pos[ROW] = 1;
    sc->cursor_pos[COL] = 1;
    
    int ret = str_init(&sc->str_buffer);
    if(ret == STR_FAILURE) {
        return;
    }
}

void scanner_dtor(scanner_t *sc) {
    str_dtor(&sc->str_buffer);
}

/**
 * @brief Fills input buffer with character from argument
 * @note Returns scanner cursor position one column back
 */
void ungetchar(char c, scanner_t *sc) {
    sc->input_buffer = c;
    sc->is_input_buffer_full = true;

    sc->cursor_pos[COL]--;
}

/**
 * @brief Updates position of cursor due to given character
 */
void update_cursor_pos(char c, scanner_t *sc) {
    if(c == '\n') {
        sc->cursor_pos[ROW]++;
        sc->cursor_pos[COL] = 1;
    }
    else {
        sc->cursor_pos[COL]++;
    }
}

/**
 * @brief Returns next char from stdin or from input buffer
 */
char next_char(scanner_t *sc) {
    char next;
    if(sc->is_input_buffer_full == true) {
        next = sc->input_buffer;
        sc->is_input_buffer_full = false;
    }
    else {
        next = getchar();
    }

    update_cursor_pos(next, sc);

    return next;
}

void got_token(token_type_t type, char c, token_t *token, scanner_t *sc) {
    if(sc->state != INIT) {
        ungetchar(c, sc);
    }

    if(sc->str_buffer.length != 0) {
        str_cpy((char **)&token->attr, 
                to_str(&sc->str_buffer), 
                sc->str_buffer.length);

        str_clear(&sc->str_buffer);
    }
    
    token->token_type = type;

    sc->state = INIT; //Reset automata state
}

/**
 * Sets initial values to token
 */ 
void init_token(token_t *token) {
    token->token_type = UNKNOWN;
    token->attr = NULL;
}

/**
 * Transitions from intial state of FSM
 */ 
void from_init_state(char c, token_t * token, scanner_t *sc) {
    if(get_chtype(c) == ALPHA || c == '_') {
        sc->state = ID_F;
    }
    else if(get_chtype(c) == DIGIT) {
        sc->state = INT_F;
    }
    else if(c == '-') {
        sc->state = OP_F3;
    }
    else if(get_chtype(c) == WHITESPACE) {
        sc->state = INIT; 
    }
    else if(c == '"') {
        sc->state = STR_1; 
    }
    else if(str_search(c, ",:()")) {
        sc->state = SEP_F; 
    }
    else if(c == '~') {
        sc->state = OP_1;
    }
    else if(str_search(c, "+*#")) {
        sc->state = OP_F1; 
    }
    else if(c == '/') {
        sc->state = OP_F2;
    }
    else if(str_search(c, "<>=")) {
        sc->state = OP_F4;
    }
    else if(c == '.') {
        sc->state = OP_2;
    }
    else if(c == EOF) {
        sc->state = EOF_F;
    }
    else {
        app_char(c, &sc->str_buffer);
        got_token(ERROR_TYPE, c, token, sc);
    }
}

token_t get_next_token(scanner_t *sc) {
    token_t result;
    init_token(&result);

    if(sc->is_tok_buffer_full) {
       result = sc->tok_buffer;
       sc->is_tok_buffer_full = false; 
    }

    char c;
    char * table_token = NULL;
    while(result.token_type == UNKNOWN) {
        c = next_char(sc);
        //fprintf(stderr, "%c", c);
       
        switch (sc->state)
        {
        case INIT:
            from_init_state(c, &result, sc);
            break;
            
        case ID_F:
            if(get_chtype(c) == ALPHA || get_chtype(c) == DIGIT || c == '_') {
                sc->state = ID_F;
            }
            else {
                table_token = match(to_str(&sc->str_buffer), get_keyword);
                if(table_token) {
                    str_clear(&sc->str_buffer);
                    result.attr = table_token; 
                    got_token(KEYWORD, c, &result, sc);
                }
                else {
                    got_token(IDENTIFIER, c, &result, sc);
                }
            }
            break;
        case INT_F:
            if(get_chtype(c) == DIGIT) {
                sc->state = INT_F;
            }
            else if(c == '.') {
                sc->state = NUM_1;
            }
            else if(str_search(c, "eE")) {
                sc->state = NUM_2;
            }
            else {
                got_token(INTEGER, c, &result, sc);
            }
            break;
        case OP_F3:
            if(get_chtype(c) == DIGIT) {
                sc->state = INT_F;
            }
            else if(c == '-') {
                sc->state = COM_1;
            }
            else {
                table_token = match(to_str(&sc->str_buffer), get_operator);
                if(table_token) {
                    str_clear(&sc->str_buffer);
                    result.attr = table_token; 
                    got_token(OPERATOR, c, &result, sc);
                }
                else {
                    got_token(ERROR_TYPE, c, &result, sc);
                }
            }
            break;
        case NUM_1:
            if(get_chtype(c) == DIGIT) {
                sc->state = NUM_F;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case NUM_2:
            if(get_chtype(c) == DIGIT) {
                sc->state = NUM_F;
            }
            else if(str_search(c, "+-")) {
                sc->state = NUM_3;
            }   
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case NUM_3:
            if(get_chtype(c) == DIGIT) {
                sc->state = NUM_F;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case NUM_F:
            if(get_chtype(c) == DIGIT) {
                sc->state = NUM_F;
            }
            else {
                got_token(NUMBER, c, &result, sc);
            }
            break;
        case COM_1:
            if(c == '[') {
                sc->state = COM_2;
            }
            else if(c == '\n') {
                sc->state = COM_F;
            }
            else {
                sc->state = COM_1;
            }
            break;
        case COM_2:
            if(c == '[') {
                sc->state = COM_3;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case COM_3:
            if(c != ']') {
                sc->state = COM_3;
            }
            else {
                sc->state = COM_4;
            }
            break;
        case COM_4:
            if(c == ']') {
                sc->state = COM_F;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case COM_F:
            ungetchar(c, sc);
            str_clear(&sc->str_buffer);
            sc->state = INIT; //Just ignore comments
            break;
        case STR_1:
            if(get_chtype(c) != CONTROL && c != '\\' && c != '"' && c != EOF) {
                sc->state = STR_1;
            }
            else if(c == '\\') {
                sc->state = STR_2;
            }
            else if(c == '"') {
                sc->state = STR_F;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case STR_2:
            if(str_search(c, "\",n,t,\\")) {
                sc->state = STR_1;
            }
            else if(get_chtype(c) == DIGIT) {
                sc->state = STR_3;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case STR_3:
            if(get_chtype(c) == DIGIT) {
                sc->state = STR_4;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case STR_4:
            if(get_chtype(c) == DIGIT) {
                sc->state = STR_1;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case STR_F:
            got_token(STRING, c, &result, sc);
            break;
        case SEP_F:
            table_token = match(to_str(&sc->str_buffer), get_separator);
            if(table_token) {
                str_clear(&sc->str_buffer);
                result.attr = table_token; 
                got_token(SEPARATOR, c, &result, sc);
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case EOF_F:
            got_token(EOF_TYPE, c, &result, sc);
            break;
        case OP_1:
            if(c == '=') {
                sc->state = OP_F1;
            }
            else {
                got_token(OPERATOR, c, &result, sc);
            }
            break;
        case OP_2:
            if(c == '.') {
                sc->state = OP_F1;
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case OP_F1:
            table_token = match(to_str(&sc->str_buffer), get_operator);
            if(table_token) {
                str_clear(&sc->str_buffer);
                result.attr = table_token; 
                got_token(OPERATOR, c, &result, sc);
            }
            else {
                got_token(ERROR_TYPE, c, &result, sc);
            }
            break;
        case OP_F2:
            if(c == '/') {
                sc->state = OP_F1;
            }
            else {
                table_token = match(to_str(&sc->str_buffer), get_operator);
                if(table_token) {
                    str_clear(&sc->str_buffer);
                    result.attr = table_token; 
                    got_token(OPERATOR, c, &result, sc);
                }
                else {
                    got_token(ERROR_TYPE, c, &result, sc);
                }
            }
            break;
        case OP_F4:
            if(c == '=') {
                sc->state = OP_F1;
            }
            else {
                table_token = match(to_str(&sc->str_buffer), get_operator);
                if(table_token) {
                    str_clear(&sc->str_buffer);
                    result.attr = table_token; 
                    got_token(OPERATOR, c, &result, sc);
                }
                else {
                    got_token(ERROR_TYPE, c, &result, sc);
                }
            }
            break;

        } //switch (sc->state)

        if(sc->state != INIT) {
            app_char(c, &sc->str_buffer);
        }

    } //while(result.token_type == UNKNOWN)
    fprintf(stderr,"Got token at: (%lu:%lu), token type: %i, attr: %s\n", sc->cursor_pos[0], sc->cursor_pos[1], result.token_type,(char *) result.attr);
    return result;
}


token_t lookahead(scanner_t *sc) {
    token_t result;
    init_token(&result);

    if(sc->is_tok_buffer_full) {
        result = sc->tok_buffer;
    }
    else {
        result = get_next_token(sc);
        sc->tok_buffer = result;
        sc->is_tok_buffer_full = true;
    }
    fprintf(stderr,"^ Lookahead ^\n");
    return result;
}




/***                             End of scanner.c                        ***/
