/******************************************************************************
 *                                  IFJ21
 *                                scanner.c
 * 
 *                  Authors: Vojtěch Dvořák (xdvora3o)
 *           Purpose: Source file with implementation of lexer (scanner)
 * 
 *                     Last change: 25. 11. 2021
 *****************************************************************************/ 

/**
 * @file scanner.c
 * @brief Source file with implementation of lexer (scanner)
 * 
 * @authors Vojtěch Dvořák (xdvora3o)
 */ 

#include "scanner.h"

/**
 * @brief Sets initial values to token
 */ 
void token_init(token_t *token) {
    token->token_type = UNKNOWN;
    token->attr = NULL;
    token->first_ch_index = UNSET;
}

/**
 * @brief Correctly frees resources holds by token
 */ 
void token_dtor(token_t *token) {

    switch (token->token_type)
    {
    case KEYWORD:
    case SEPARATOR:
    case OPERATOR:
        break;
    default:
        if(token->attr) {
            free(token->attr);
        }
        
        break;
    }

    token->attr = NULL;
    token->token_type = UNKNOWN;
}

char * get_attr(token_t *token, scanner_t *sc) {
    if(token->attr) {
        return token->attr;
    }
    else {
        return &(to_str(&sc->str_buffer)[token->first_ch_index]);
    }
}


/**
 * @brief Prepares scanner structure and sets its attributes to initial values
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

    sc->first_ch_index = UNSET;
}

void scanner_dtor(scanner_t *sc) {
    sc->first_ch_index = UNSET;
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
 * @brief Reads character from stdin (or from input buffer)
 */
char next_char(scanner_t *sc) {
    char next;
    if(sc->is_input_buffer_full == true) {
        next = sc->input_buffer;
        sc->is_input_buffer_full = false;
    }
    else {
        next = getchar();
        update_cursor_pos(next, sc);
    }

    return next;
}


/**
 * @brief Assigns type to token and perform other necessary actions to
 */ 
void got_token(token_type_t type, char c, token_t *token, scanner_t *sc) {
    if(sc->state != INIT) {
        ungetchar(c, sc);
    }

    if(type == EOF_TYPE) {
        cut_string(&sc->str_buffer, sc->first_ch_index);
        sc->first_ch_index = UNSET;

        token->attr = "EOF";
    }
    else if(sc->first_ch_index != UNSET) {
        token->first_ch_index = sc->first_ch_index;

        app_char('\0', &(sc->str_buffer));
        sc->first_ch_index = UNSET;
    }
    
    token->token_type = type;

    sc->state = INIT; //Reset automata state
}


/**
 * @brief Auxiliary function to ignore comments
 */ 
void got_comment(char c, token_t *token, scanner_t *sc) {
    ungetchar(c, sc);

    cut_string(&(sc->str_buffer), sc->first_ch_index);
    sc->first_ch_index = UNSET;

    sc->state = INIT;
}

/**
 * @brief Tries to find token in table
 * @param tab_func Function with static table in which will be searching executed
 * @param token Current processed token
 * @param sc Scanner structure
 * @return True if string was found in given table
 */ 
bool from_tab(char *(*tab_func)(unsigned int), token_t *token, scanner_t *sc) {
    if(sc->first_ch_index == UNSET) {
        return false;
    }

    size_t table_size;
    if(tab_func == get_keyword) {
        table_size = KEYWORD_TABLE_SIZE;
    }
    else if(tab_func == get_operator) {
        table_size = OPERATOR_TABLE_SIZE;
    }
    else {
        table_size = SEPARATOR_TABLE_SIZE;
    }

    char * tab_token = NULL;
    char * first_ch = &((to_str(&sc->str_buffer))[sc->first_ch_index]);
    tab_token = match(first_ch, tab_func, table_size);
    if(tab_token != NULL) {
        token->attr = tab_token;
        cut_string(&sc->str_buffer, sc->first_ch_index);
        sc->first_ch_index = UNSET;

        return true;
    }
    else {
        return false;
    }
}

/*****************************Transition functions****************************/

/**
 * @brief Transitions from intial state of FSM
 */ 
void INIT_trans(char c, token_t * token, scanner_t *sc) {
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
        if(sc->first_ch_index == UNSET) {
            sc->first_ch_index = sc->str_buffer.length;
        }

        app_char(c, &sc->str_buffer); //Saving error token to show it to user
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void ID_F_trans(char c, token_t * token, scanner_t *sc) {
    if(get_chtype(c) == ALPHA || get_chtype(c) == DIGIT || c == '_') {
        sc->state = ID_F;
    }
    else {
        if(from_tab(get_keyword, token, sc)) {
            got_token(KEYWORD, c, token, sc);
        }
        else {
            got_token(IDENTIFIER, c, token, sc);
        }
    }
}


void INT_F_trans(char c, token_t * token, scanner_t *sc) {
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
        got_token(INTEGER, c, token, sc);
    }
}


void NUM_1_trans(char c, token_t * token, scanner_t *sc) {
    if(get_chtype(c) == DIGIT) {
        sc->state = NUM_F;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void NUM_2_trans(char c, token_t * token, scanner_t *sc) {
    if(get_chtype(c) == DIGIT) {
        sc->state = NUM_F;
    }
    else if(str_search(c, "+-")) {
        sc->state = NUM_3;
    }   
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void NUM_3_trans(char c, token_t * token, scanner_t *sc) {
    if(get_chtype(c) == DIGIT) {
        sc->state = NUM_F;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void NUM_F_trans(char c, token_t * token, scanner_t *sc) {
    if(get_chtype(c) == DIGIT) {
        sc->state = NUM_F;
    }
    else {
        got_token(NUMBER, c, token, sc);
    }
}


void COM_F1_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '[') {
        sc->state = COM_F2;
    }
    else if(c == '\n' || c == EOF) {
       got_comment(c, token, sc);
    }
    else {
        sc->state = COM_F1;
    }
}


void COM_F2_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '[') {
        sc->state = COM_F3;
    }
    else if(c == '\n' || c == EOF) {
       got_comment(c, token, sc);
    }
    else {
        sc->state = COM_F1; //There is character between two [ -> still one line comment
    }
}


void COM_F3_trans(char c, token_t * token, scanner_t *sc) {
    if(c == ']') {
        sc->state = COM_F4;
    }
    else if(c == EOF) {
       got_comment(c, token, sc);
    }
    else {
        sc->state = COM_F3;
    }
}


void COM_F4_trans(char c, token_t * token, scanner_t *sc) {
    if(c == ']') {
        sc->state = COM_F5;
    }
    else if(c == EOF) {
       got_comment(c, token, sc);
    }
    else {
        sc->state = COM_F3; //There is character between two ] -> still comment
    }
}


void COM_F5_trans(char c, token_t * token, scanner_t *sc) {
    got_comment(c, token, sc);
}


void STR_1_trans(char c, token_t * token, scanner_t *sc) {
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
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void STR_2_trans(char c, token_t * token, scanner_t *sc) {
    if(str_search(c, "\",n,t,\\")) {
        sc->state = STR_1;
    }
    else if(get_chtype(c) == DIGIT) {
        sc->state = STR_3;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void STR_3_trans(char c, token_t * token, scanner_t *sc) {
    if(get_chtype(c) == DIGIT) {
        sc->state = STR_4;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void STR_4_trans(char c, token_t * token, scanner_t *sc) {
    if(get_chtype(c) == DIGIT) {
        sc->state = STR_1;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void STR_F_trans(char c, token_t * token, scanner_t *sc) {
    got_token(STRING, c, token, sc);
}


void SEP_F_trans(char c, token_t * token, scanner_t *sc) {
    if(from_tab(get_separator, token, sc)) {
        got_token(SEPARATOR, c, token, sc);
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void OP_1_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '=') {
        sc->state = OP_F1;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void OP_2_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '.') {
        sc->state = OP_F1;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void OP_F1_trans(char c, token_t * token, scanner_t *sc) {
    if(from_tab(get_operator, token, sc)) {
        got_token(OPERATOR, c, token, sc);
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void OP_F2_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '/') {
        sc->state = OP_F1;
    }
    else {
        if(from_tab(get_operator, token, sc)) {
            got_token(OPERATOR, c, token, sc);
        }
        else {
            got_token(ERROR_TYPE, c, token, sc);
        }
    }
}


void OP_F3_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '-') {
        sc->state = COM_F1;
    }
    else {
        if(from_tab(get_operator, token, sc)) {
            got_token(OPERATOR, c, token, sc);
        }
        else {
            got_token(ERROR_TYPE, c, token, sc);
        }
    }
}


void OP_F4_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '=') {
        sc->state = OP_F1;
    }
    else {
        if(from_tab(get_operator, token, sc)) {
            got_token(OPERATOR, c, token, sc);
        }
        else {
            got_token(ERROR_TYPE, c, token, sc);
        }
    }
}


void EOF_F_trans(char c, token_t * token, scanner_t *sc) {
    got_token(EOF_TYPE, c, token, sc);
}


//^^^^^^^^^^^^^^^^^^^^^^^^End of transition functions^^^^^^^^^^^^^^^^^^^^^^^^^/


/**
 * @brief Returns correspoding transition function to given state
 * @param state state of FSM
 */ 
trans_func_t get_trans(fsm_state_t state) {

    static trans_func_t transition_functions[STATE_NUM];
    transition_functions[INIT] = INIT_trans;
    transition_functions[ID_F] = ID_F_trans;
    transition_functions[INT_F] = INT_F_trans;
    transition_functions[NUM_1] = NUM_1_trans;
    transition_functions[NUM_2] = NUM_2_trans;
    transition_functions[NUM_3] = NUM_3_trans;
    transition_functions[NUM_F] = NUM_F_trans;
    transition_functions[COM_F1] = COM_F1_trans;
    transition_functions[COM_F2] = COM_F2_trans;
    transition_functions[COM_F3] = COM_F3_trans;
    transition_functions[COM_F4] = COM_F4_trans;
    transition_functions[COM_F5] = COM_F5_trans;
    transition_functions[STR_1] = STR_1_trans;
    transition_functions[STR_2] = STR_2_trans;
    transition_functions[STR_3] = STR_3_trans;
    transition_functions[STR_4] = STR_4_trans;
    transition_functions[STR_F] = STR_F_trans;
    transition_functions[SEP_F] = SEP_F_trans;
    transition_functions[OP_1] = OP_1_trans;
    transition_functions[OP_2] = OP_2_trans;
    transition_functions[OP_F1] = OP_F1_trans;
    transition_functions[OP_F2] = OP_F2_trans;
    transition_functions[OP_F3] = OP_F3_trans;
    transition_functions[OP_F4] = OP_F4_trans;
    transition_functions[EOF_F] = EOF_F_trans;

    return transition_functions[state];
}


char *tok_type_to_str(token_type_t tok_type) {
    if(tok_type >= TOK_TYPE_NUM || tok_type < 0) {
        return NULL;
    }

    static char* tok_type_meanings[TOK_TYPE_NUM] = {
        "unknown", "identifier", "keyword", "integer", 
        "number", "string", "operator", "separator",
        "end of file", "invalid token"
    };

    return tok_type_meanings[tok_type];
}

void lex_err(scanner_t *sc, token_t *bad_token) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t| ", (sc->cursor_pos[ROW]), (sc->cursor_pos[COL]));
    fprintf(stderr, "\033[0;31mLexical error:\033[0m ");
    fprintf(stderr, "Unrecognized token '\033[1;33m%s\033[0m'!\n", get_attr(bad_token, sc));

}

token_t get_next_token(scanner_t *sc) {
    token_t result;
    token_init(&result);

    if(sc->is_tok_buffer_full) {
       result = sc->tok_buffer;
       sc->is_tok_buffer_full = false; 
    }

    while(result.token_type == UNKNOWN) {
        char c = next_char(sc);

        //Get transitions from current state
        trans_func_t do_transition = get_trans(sc->state);
        do_transition(c, &result, sc);

        if(sc->state != INIT) {
            if(sc->first_ch_index == UNSET) {
                sc->first_ch_index = sc->str_buffer.length;
            }

            app_char(c, &sc->str_buffer);
        }

    } //while(result.token_type == UNKNOWN)

    if(result.token_type == ERROR_TYPE) {
        lex_err(sc, &result);
    }
    // fprintf(stderr,"Got token at: (%lu:%lu), token type: %i, attr: %s\n", sc->cursor_pos[ROW], sc->cursor_pos[COL], result.token_type, get_attr(&result, sc));
    return result;
} //get_next_token()



token_t lookahead(scanner_t *sc) {
    token_t result;
    token_init(&result);

    if(sc->is_tok_buffer_full) {
        result = sc->tok_buffer;
    }
    else {
        //fprintf(stderr, "Lookahead: ");
        result = get_next_token(sc);
        sc->tok_buffer = result;
        sc->is_tok_buffer_full = true;
    }
    return result;
}


/***                             End of scanner.c                        ***/
