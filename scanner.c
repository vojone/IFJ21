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

#define INIT_SPACE 1

int add_char(char c, token_t *token) {
    if(token->attr == NULL) {
        token->attr = malloc(sizeof(char)*INIT_SPACE);
        if(token->attr == NULL) {
            fprintf(stderr, "Cannot allocate memory!");
            return EXIT_FAILURE;
        }

        token->allocated = INIT_SPACE;
        ((char *)token->attr)[token->attr_size++] = c;
    }
    else {
        if(token->attr_size + 1 > token->allocated) {
            token->attr = realloc(token->attr, sizeof(char)*token->allocated*2);
            if(token->attr == NULL) {
                fprintf(stderr, "Cannot allocate memory!");
                return EXIT_FAILURE;
            }

            token->allocated *= 2;
        }

        ((char *)token->attr)[token->attr_size++] = c;
    }

    return EXIT_SUCCESS;
}

/**
 * Prepares scanner structure and sets its attributes to initial values
 */
void scanner_init(scanner_t *scanner) {
    scanner->state = INIT;
    scanner->input_buffer = '\0';
}


bool is_keyword(token_t *token) {
    static char * keyword_table[] = {"do", "else", "end", "function", "global", 
                                     "if", "local", "nil", "require", "return", 
                                     "then", "while", "integer", "number", 
                                     "string", NULL};
    
    for(int i = 0; keyword_table[i]; i++) {
        if(strcmp(keyword_table[i], (char *)token->attr) == 0) {
            return true;
        }
    }

    return false;
}

void ungetchar(char c, scanner_t *scanner) {
    scanner->input_buffer = c;
}

char next_char(scanner_t *scanner) {
    char next;
    if(scanner->input_buffer != '\0') {
        next = scanner->input_buffer;
        scanner->input_buffer = '\0';
    }
    else {
        next = getchar();    
    }

    return next;
}

void got_token(token_type_t type, char c, token_t *token, scanner_t *scanner) {
    if(scanner->state != INIT) {
        ungetchar(c, scanner);
    }
    
    scanner->state = INIT;
    token->token_type = type;
}

void from_init_state(char c, token_t * token, scanner_t *scanner) {
    if(isalpha(c) || c == '_') {
        scanner->state = ID_F;
        add_char(c, token);
    }
    else if(isdigit(c)) {
        scanner->state = INT_F;
    }
    else if(c == '-') {
        scanner->state = OP_F3;
    }
    else if(isspace(c)) {
        scanner->state = INIT; 
    }
    else if(c == '"') {
        scanner->state = STR_1; 
    }
    else if(strchr(",:()", c)) {
        scanner->state = SEP_F; 
    }
    else if(strchr("<>~", c)) {
        scanner->state = OP_1; 
    }
    else if(strchr("+*#", c)) {
        scanner->state = OP_F1;  
    }
    else if(c == '/') {
        scanner->state = OP_F2;
    }
    else if(c == '=') {
        scanner->state = OP_F4;
    }
    else if(c == '.') {
        scanner->state = OP_2;
    }
    else if(c == EOF) {
        scanner->state = EOF_F;
    }
    else {
        got_token(ERROR_TYPE, c, token, scanner);
    }
}

token_t get_next_token(scanner_t *scanner) {
    token_t result;
    result.attr = NULL;
    result.attr_size = 0;
    result.allocated = 0;
    result.token_type = UNKNOWN;
    
    char c;
    while(result.token_type == UNKNOWN) {
        c = next_char(scanner);
       
        switch (scanner->state)
        {
        case INIT:
            from_init_state(c, &result, scanner);
            break;
            
        case ID_F:
            if(isalpha(c) || c == '_' || isdigit(c)) {
                scanner->state = ID_F;
                add_char(c, &result);
            }
            else {
                got_token(IDENTIFIER, c, &result, scanner);
                add_char('\0', &result);
                if(is_keyword(&result)) {
                    got_token(KEYWORD, c, &result, scanner);
                }   
            }
            break;
        case INT_F:
            if(isdigit(c)) {
                scanner->state = INT_F;
            }
            else if(c == '.') {
                scanner->state = NUM_1;
            }
            else if(strchr("eE", c)) {
                scanner->state = NUM_2;
            }
            else {
                got_token(INTEGER, c, &result, scanner);
            }
            break;
        case OP_F3:
            if(isdigit(c)) {
                scanner->state = INT_F;
            }
            else if(c == '-') {
                scanner->state = COM_1;
            }
            else {
                got_token(OPERATOR, c, &result, scanner);
            }
            break;
        case NUM_1:
            if(isdigit(c)) {
                scanner->state = NUM_F;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case NUM_2:
            if(isdigit(c)) {
                scanner->state = NUM_F;
            }
            else if(strchr("+-", c)) {
                scanner->state = NUM_3;
            }   
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case NUM_3:
            if(isdigit(c)) {
                scanner->state = NUM_F;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case NUM_F:
            if(isdigit(c)) {
                scanner->state = NUM_F;
            }
            else {
                got_token(NUMBER, c, &result, scanner);
            }
            break;
        case COM_1:
            if(c == '[') {
                scanner->state = COM_2;
            }
            else if(c == '\n') {
                scanner->state = COM_F;
            }
            else {
                scanner->state = COM_1;
            }
            break;
        case COM_2:
            if(c == '[') {
                scanner->state = COM_3;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case COM_3:
            if(c != ']') {
                scanner->state = COM_3;
            }
            else {
                scanner->state = COM_4;
            }
            break;
        case COM_4:
            if(c == ']') {
                scanner->state = COM_F;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case COM_F:
            scanner->state = INIT; //Just ignore comments
            break;
        case STR_1:
            if(!iscntrl(c) && c != '\\' && c != '"' && c != EOF) {
                scanner->state = STR_1;
            }
            else if(c == '\\') {
                scanner->state = STR_2;
            }
            else if(c == '"') {
                scanner->state = STR_F;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case STR_2:
            if(strchr("\",n,t,\\", c)) {
                scanner->state = STR_1;
            }
            else if(isdigit(c)) {
                scanner->state = STR_3;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case STR_3:
            if(isdigit(c)) {
                scanner->state = STR_4;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case STR_4:
            if(isdigit(c)) {
                scanner->state = STR_1;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case STR_F:
            got_token(STRING, c, &result, scanner);
            break;
        case SEP_F:
            got_token(SEPARATOR, c, &result, scanner);
            break;
        case EOF_F:
            got_token(EOF_TYPE, c, &result, scanner);
            break;
        case OP_1:
            if(c == '=') {
                scanner->state = OP_F1;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case OP_2:
            if(c == '.') {
                scanner->state = OP_F1;
            }
            else {
                got_token(ERROR_TYPE, c, &result, scanner);
            }
            break;
        case OP_F1:
            got_token(OPERATOR, c, &result, scanner);
            break;
        case OP_F2:
            if(c == '/') {
                scanner->state = OP_F1;
            }
            else {
                got_token(OPERATOR, c, &result, scanner);
            }
            break;
        case OP_F4:
            if(c == '=') {
                scanner->state = OP_F1;
            }
            else {
                got_token(OPERATOR, c, &result, scanner);
            }
            break;
        }
    }

    return result;
}




/***                             End of scanner.c                        ***/
