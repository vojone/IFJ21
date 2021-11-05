/**
 * @file parser-topdown.c
 * @brief Source file for recursive descent parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */

#define TMP_FILE_NAME "tmp.inp"
#include "parser-topdown.h"

scanner_t scanner;
#define MAXCALLS 200
int current_calls = 0; 
bool reachedEOF = false;

int main(){
    scanner_init(&scanner);    
    
    //run parsing
    bool res = global_statement_list();
    // while(!reachedEOF){
    //     //token_t t = lookahead(&scanner);
    //     token_t t = get_next_token(&scanner);
        
    //     if(t.token_type == EOF_TYPE)
    //         reachedEOF = true;
    // }
    fprintf(stderr,"Finished! success: %i, at: (%lu,%lu)\n",res, scanner.cursor_pos[0], scanner.cursor_pos[1]);
}
bool global_statement_list(){
    bool ret = true;
    ret = global_statement();
    if(!ret)
        return false;
    if(reachedEOF)
        return true;

    return global_statement_list();
}
//<statement-list>        -> <statement> <statement-list>
bool statement_list(){
    bool ret = true;
    ret = statement();
    //fprintf(stderr,"statement success %i\n",ret);
    if(ret == false)
        return false;
    token_t t = lookahead(&scanner);
    if( t.token_type == KEYWORD ){
        if(0 == strcmp(t.attr, "end")){
            fprintf(stderr,"got end\n");
            return true;
        }
    }
    return statement_list();
}
bool global_statement(){
    if(current_calls >= MAXCALLS){
        fprintf(stderr,"Max calls exceeded, terminating \n");
        return false;
    }
    current_calls++;

    token_t t = get_next_token(&scanner);
    //fprintf(stderr,"PARSING INSIDE FUNCTION DEFINITION Finished! success: %i, at: (%lu,%lu), token type: %i\n", scanner.cursor_pos[0], scanner.cursor_pos[1], t); 
    // fprintf(stderr,"Token type is: %i | (%lu:%lu) state %i\n",t.token_type,scanner.cursor_pos[0],scanner.cursor_pos[1],scanner.state);
    if(t.token_type == KEYWORD){
        if(0 == strcmp(t.attr, "require")){
            //<statement>             -> require <str>
            return parse_str();
        }else if(0 == strcmp(t.attr, "function")){
            //<statement>             -> function [id](<param-list>) : <type-list> <statement-list> end
            fprintf(stderr,"function ...\n");
            return parse_function_def();
        }
    }else if(t.token_type == EOF_TYPE){
        reachedEOF = true;
        return true;
    }else{
        incorrectToken("This is global scope so keyword such as require or function definition expected",t.token_type,&scanner);
        return false;
    }
    return false;
}
bool statement(){
    //just to be sure for development. remove later
    if(current_calls >= MAXCALLS){
        fprintf(stderr,"Max calls exceeded, terminating \n");
        return false;
    }
    current_calls++;


    token_t t = lookahead(&scanner);
    //fprintf(stderr,"PARSING INSIDE FUNCTION DEFINITION Finished! success: %i, at: (%lu,%lu), token type: %i\n", scanner.cursor_pos[0], scanner.cursor_pos[1], t); 
    // fprintf(stderr,"Token type is: %i | (%lu:%lu) state %i\n",t.token_type,scanner.cursor_pos[0],scanner.cursor_pos[1],scanner.state);
    switch (t.token_type)
    {
    case KEYWORD:
        if(0 == strcmp(t.attr, "local")){
            //<statement>             -> require <str>
            t = get_next_token(&scanner);
            bool res = parse_variable_def();
            return res;
        }else if(0 == strcmp(t.attr, "global")){
            //<statement>             -> function [id](<param-list>) : <type-list> <statement-list> end
            t = get_next_token(&scanner);
            return parse_variable_def();
        }else if(0 == strcmp(t.attr, "end")){
            return true;
        }
        
        break;
    case EOF_TYPE:
        fprintf(stderr,"ERROR! Reached EOF inside a function. Did you forget 'end'?\n");
        reachedEOF = true;
        return false;
        break;
    default:
        fprintf(stderr,"Token type is: %i\n",t.token_type);
        return false;
        break;
    }
    return false;
}
bool parse_variable_def(){
    bool id = checkNextToken(&scanner, IDENTIFIER);
    bool comma = checkNextTokenAttr(&scanner,SEPARATOR,":");
    bool type = checkNextToken(&scanner, KEYWORD);   //later check for datatypes only
    token_t t = lookahead(&scanner);

    //there can be a value assigment
    if(t.token_type == OPERATOR){ 
        bool assigment = value_assignment();
        return id && comma && type && assigment;
    }
    return id && comma && type;
}
bool value_assignment(){
    bool eq = checkNextTokenAttr(&scanner, OPERATOR,"=");
    bool expression = parseExpression(&scanner);
    return eq && expression;
}
bool parse_str(){
    token_t t = get_next_token(&scanner);
    if(t.token_type == STRING)
        return true;
    else
        incorrectToken("STRING",t.token_type,&scanner);
    return false;
}
bool parse_function_def(){
    //parsing function definition signature
    bool id = checkNextToken(&scanner,IDENTIFIER);
    bool left_bracket = checkNextTokenAttr(&scanner,SEPARATOR,"(");
    bool right_bracket= checkNextTokenAttr(&scanner,SEPARATOR,")");
    if(!(id && left_bracket && right_bracket)){
        fprintf(stderr,"ERROR INVALID FUNCTION SINATURE!\n");
        return false;
    }

    //parsing inside function
    fprintf(stderr,"parsing inside function...\n");
    bool res = statement_list();
    fprintf(stderr,"PARSING INSIDE FUNCTION DEFINITION Finished! success: %i, at: (%lu,%lu)\n",res, scanner.cursor_pos[0], scanner.cursor_pos[1]); 

    token_t t = get_next_token(&scanner);
    if(t.token_type == KEYWORD){
        if( strcmp( (char *) t.attr , "end") == 0 ){
            fprintf(stderr,"Successfully ended function definition!\n");
        }
    }else{
        incorrectToken("Better check the implementation! expected END!",t.token_type,&scanner);
    }
    return res;
}

/**
 * *Temporary solution!
 * Expressions should temporarily be replaced by INTEGER or STRING or NUMBER
 */
bool parseExpression(){
    fprintf(stderr,"parsing expression..\n");
    token_t t = get_next_token(&scanner);
    if(t.token_type == NUMBER || t.token_type == INTEGER || t.token_type == STRING)
        return true;
    return false;
}
void incorrectToken(char * expected, int got, scanner_t * scanner){
    fprintf(stderr, "Error, Wrong token! %s expected, but token type is %i!\n",expected, got);
    fprintf(stderr,"Error occured at row: %lu col: %lu\n",(scanner->cursor_pos[0]), (scanner->cursor_pos[1]));
}
bool lookaheadkToken(scanner_t * scanner,token_type_t expecting){
    token_t t = lookahead(scanner);
    if(t.token_type == expecting)
        return true;
    return false;
}
bool lookaheadTokenAttr(scanner_t * scanner,token_type_t expecting, char * expecting_attr){
    token_t t = lookahead(scanner);
    if( t.token_type == expecting ){
        if(0 == strcmp(expecting_attr,(char*) t.attr)){
            return true;
        }
    }
    return false;
}
bool checkNextToken(scanner_t * scanner,token_type_t expecting){
    token_t t = get_next_token(scanner);
    if(t.token_type == expecting){
        return true;
    }
    char exp_str[3];
    sprintf(exp_str, "%d", expecting);
    incorrectToken(exp_str, t.token_type, scanner);
    return false;
}
bool checkNextTokenAttr(scanner_t * scanner,token_type_t expecting, char * expecting_attr){
    token_t t = get_next_token(scanner);
    if( t.token_type == expecting ){
        if(0 == strcmp(expecting_attr,(char*) t.attr)){
            return true;
        }
    }
    char exp_str[3];
    sprintf(exp_str, "%d", expecting);
    incorrectToken(exp_str, t.token_type, scanner);
    return false;
}