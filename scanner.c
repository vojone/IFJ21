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
 * @brief Sets initial values to token
 */ 
void init_token(token_t *token) {
    token->token_type = UNKNOWN;
    token->attr = NULL;
}


/**
 * @brief Tries to find token in table
 * @param tab_func Function with static table in which will be searching executed
 * @param token Current processed token
 * @param sc Scanner structure
 * @return True if string was found in given table
 */ 
bool from_tab(char *(*tab_func)(unsigned int), token_t *token, scanner_t *sc) {

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

    char * table_token = NULL;
    table_token = match(to_str(&sc->str_buffer), tab_func, table_size);
    if(table_token != NULL) {
        str_clear(&sc->str_buffer);
        token->attr = table_token;

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


void COM_1_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '[') {
        sc->state = COM_2;
    }
    else if(c == '\n' || c == EOF) {
        sc->state = COM_F;
    }
    else {
        sc->state = COM_1;
    }
}


void COM_2_trans(char c, token_t * token, scanner_t *sc) {
    if(c == '[') {
        sc->state = COM_3;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void COM_3_trans(char c, token_t * token, scanner_t *sc) {
    if(c != ']') {
        sc->state = COM_3;
    }
    else {
        sc->state = COM_4;
    }
}


void COM_4_trans(char c, token_t * token, scanner_t *sc) {
    if(c == ']') {
        sc->state = COM_F;
    }
    else {
        got_token(ERROR_TYPE, c, token, sc);
    }
}


void COM_F_trans(char c, token_t * token, scanner_t *sc) {
    ungetchar(c, sc);
    str_clear(&sc->str_buffer);
    sc->state = INIT; //Just ignore comments
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
        got_token(OPERATOR, c, token, sc);
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
    if(get_chtype(c) == DIGIT) {
        sc->state = INT_F;
    }
    else if(c == '-') {
        sc->state = COM_1;
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
    transition_functions[COM_1] = COM_1_trans;
    transition_functions[COM_2] = COM_2_trans;
    transition_functions[COM_3] = COM_3_trans;
    transition_functions[COM_4] = COM_4_trans;
    transition_functions[COM_F] = COM_F_trans;
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


token_t get_next_token(scanner_t *sc) {
    token_t result;
    init_token(&result);

    if(sc->is_tok_buffer_full) {
       result = sc->tok_buffer;
       sc->is_tok_buffer_full = false; 
    }

    while(result.token_type == UNKNOWN) {
        char c = next_char(sc);
        update_cursor_pos(c, sc);

        //Get transitions from current state
        trans_func_t do_transition = get_trans(sc->state);
        do_transition(c, &result, sc);

        if(sc->state != INIT) {
            app_char(c, &sc->str_buffer);
        }

    } //while(result.token_type == UNKNOWN)
    
    fprintf(stderr,"Got token at: (%lu:%lu), token type: %i, attr: %s\n", sc->cursor_pos[0], sc->cursor_pos[1], result.token_type,(char *) result.attr);
    return result;
} //get_next_token()



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
