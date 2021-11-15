/**
 * @file parser-topdown.c
 * @brief Source file for recursive descent parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */

#define TMP_FILE_NAME "tmp.inp"
#include "parser-topdown.h"

scanner_t scanner;
#define MAXCALLS 100
int current_calls = 0; 
bool reachedEOF = false;

int main(){
    scanner_init(&scanner);    
    
    //run parsing
    bool res = global_statement_list();

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
    bool ret = statement();
    //fprintf(stderr,"statement success %i\n",ret);
    if(ret == false)
        return false;
    token_t t = lookahead(&scanner);
    if( t.token_type == KEYWORD ){
        if(0 == strcmp(t.attr, "end")){
            fprintf(stderr,"got end\n");
            return true;
        }
        if(0 == strcmp(t.attr, "else")){
            fprintf(stderr,"got else\n");
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
    if(t.token_type == KEYWORD){
        if(0 == strcmp(t.attr, "require")){
            //<statement>             -> require <str>
            return parse_str();
        }else if(0 == strcmp(t.attr, "function")){
            //<statement>             -> function [id](<param-list>) : <type-list> <statement-list> end
            fprintf(stderr,"function ...\n");
            return parse_function_def();
        }
            
        incorrect_token("This is global scope so keyword such as require or function definition or call expected", t, &scanner);
        return false;
    }else if(t.token_type == IDENTIFIER){
        if(lookahead_token_attr(&scanner,SEPARATOR,"(")){
            return parse_function_call();
        }
        incorrect_token("function call",t,&scanner);
        return false;
    }else if(t.token_type == EOF_TYPE){
        reachedEOF = true;
        return true;
    }

    incorrect_token("This is global scope so keyword such as require or function definition or call expected",t,&scanner);
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
    switch (t.token_type)
    {
    case IDENTIFIER:
        // it is neccessary to look at the next lexeme also
        t = get_next_token(&scanner);
        t = lookahead(&scanner);
        bool is_multiple_assignment = (t.token_type == SEPARATOR && 0 == strcmp(",",t.attr));
        bool is_single_assignment = (t.token_type == OPERATOR && 0 == strcmp("=",t.attr));
        //check if it is a function call
        if(t.token_type == SEPARATOR && 0 == strcmp("(",t.attr)){
            return parse_function_call();
        }else if(is_multiple_assignment || is_single_assignment){
            return multiple_assignment();
        }else{
            incorrect_token("After IDENTIFIER a function call or an assignment",t,&scanner);
        }
        break;
    case KEYWORD:
        if(0 == strcmp(t.attr, "local")){
            //<statement>             -> local  [id] : [type] <value-assignment>
            t = get_next_token(&scanner);
            bool res = parse_variable_def();
            return res;
        }else if(0 == strcmp(t.attr, "global")){
            //<statement>             -> global  [id] : [type] <value-assignment>
            t = get_next_token(&scanner);
            return parse_variable_def();
        }else if(0 == strcmp(t.attr, "if")){
            //<statement>             -> return <exp>
            t = get_next_token(&scanner);
            return parse_if();
        }else if(0 == strcmp(t.attr, "else")){
            return true;
        }else if(0 == strcmp(t.attr, "while")){
            t = get_next_token(&scanner);
            return parse_while();
        }else if(0 == strcmp(t.attr, "return")){
            //<statement>             -> return <exp>
            t = get_next_token(&scanner);
            return parse_expression();
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
bool multiple_assignment(){
    int token_count = 0;
    bool foundAssignmentOp = false;
    //first loop checks and counts left side of the assigment
    while(!foundAssignmentOp){
        //identifier should be first
        token_t t;
        //first token is already read at the beginning, so we check wheter we are at the beginning
        if(token_count != 0)
            t = get_next_token(&scanner);
        if(t.token_type == IDENTIFIER || token_count == 0)
            token_count++;
        else{
            incorrect_token("IDENTIFIER",t,&scanner);
            return false;
        }
        //comma should follow, or assignment operator
        t = get_next_token(&scanner);
        if(t.token_type == SEPARATOR && 0 == strcmp((char *)t.attr, ",")){
            //ok
        }else if(t.token_type == OPERATOR && 0 == strcmp((char *)t.attr, "=")){
            foundAssignmentOp = true;
        }else{
            incorrect_token("SEPARATOR ',' or OPERATOR '='",t,&scanner);
            return false;
        }
    }
    fprintf(stderr,"found %i assigment...\n", token_count);
    //checking the RHS part
    for(int i = 0; i < token_count;i++){
        //check for valid expression
        if(parse_expression()){
            //ok
        }else{
            fprintf(stderr, "Error while parsing expression for multiple assignemt\n");
            return false;
        }
        //if we are not at the end check for comma
        if(i+1 != token_count){
            token_t t = get_next_token(&scanner);
            if( t.token_type == SEPARATOR && 0 == strcmp(t.attr, ",") ){
                //ok
            }else{
                incorrect_token("Missing SEPARATOR ',' in the assigment, which was",t,&scanner);
                return false;
            }
        }
    }
    return true;
}
bool parse_variable_def(){
    bool id = check_next_token(&scanner, IDENTIFIER);
    bool comma = check_next_token_attr(&scanner,SEPARATOR,":");
    bool type = parse_datatype();
    token_t t = lookahead(&scanner);

    //there can be a value assigment
    if(t.token_type == OPERATOR){ 
        bool assigment = value_assignment();
        return id && comma && type && assigment;
    }
    return id && comma && type;
}
bool value_assignment(){
    bool eq = check_next_token_attr(&scanner, OPERATOR,"=");
    bool expression = parse_expression(&scanner);
    return eq && expression;
}
bool parse_str(){
    token_t t = get_next_token(&scanner);
    if(t.token_type == STRING)
        return true;
    else
        incorrect_token("STRING",t,&scanner);
    return false;
}
bool parse_function_def(){
    //parsing function definition signature
    bool id = check_next_token(&scanner,IDENTIFIER);
    bool left_bracket = check_next_token_attr(&scanner,SEPARATOR,"(");
    //add attributes
    bool right_bracket= check_next_token_attr(&scanner,SEPARATOR,")");
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
            return res;
        }else{
            incorrect_token("Wrong keyword! Only END",t, &scanner);
            return false;
        }
        incorrect_token("[]",t,&scanner);
        return false;
    }else{
        incorrect_token("Better check the implementation! END",t,&scanner);
    }
    return false;
}
bool parse_function_call(){
    fprintf(stderr,"parsing function call...\n");
    bool opening_bracket = check_next_token_attr(&scanner, SEPARATOR, "(");
    if(opening_bracket){
        bool args = parse_function_arguments();
        fprintf(stderr,"function args success %i\n",(int) args);
        return args;
    }else{
        return false;
    }
    return true;
}
bool parse_function_arguments(){
    bool closing_bracket = false;
    while(!closing_bracket){
        token_t t = get_next_token(&scanner);
        if(is_expression(t)){
            //ok
        }else if(t.token_type == SEPARATOR && 0 == strcmp(t.attr , ")")){
            closing_bracket = true;
            continue;
        }else{
            incorrect_token("EXPRESSION",t,&scanner);
        }
        t = get_next_token(&scanner);
        if(t.token_type == SEPARATOR && 0 == strcmp(t.attr , ",")){
            //ok
        }else if(t.token_type == SEPARATOR && 0 == strcmp(t.attr , ")")){
            closing_bracket = true;
        }
        else{
            incorrect_token("',' as a separator between args or ')' as an end of argument list",t,&scanner);
            return false;
        }
    }
    return closing_bracket;
}

bool parse_if(){
    bool condition = parse_expression();
    if(!condition)
        return false;
    bool then = check_next_token_attr(&scanner, KEYWORD, "then");
    if(!then)
        return false;
    bool inside_statement_list = statement_list();
    
    token_t t = get_next_token(&scanner);
    if(t.token_type == KEYWORD){
        if( strcmp( (char *) t.attr , "end") == 0 ){
            fprintf(stderr,"Ended if\n");
            return inside_statement_list;
        }
        else if( strcmp ((char *) t.attr, "else") == 0){
            //<else-branch>           -> else <statement-list>
            fprintf(stderr,"parsing else branch...\n");
            bool else_branch_statement_list = statement_list();
            t = get_next_token(&scanner);
            if(t.token_type == KEYWORD && strcmp( (char *) t.attr , "end") == 0){
                //ended else branch
                return inside_statement_list && else_branch_statement_list;
            }
            incorrect_token("[]",t,&scanner);
            return false;
        }
        incorrect_token("[]",t,&scanner);
        return false;
    }else{
        incorrect_token("Better check the implementation! END",t,&scanner);
        return false;
    }
    return inside_statement_list;
}

bool parse_while(){
    bool condition = parse_expression();
    if(!condition)
        return false;
    bool then = check_next_token_attr(&scanner, KEYWORD, "do");
    if(!then)
        return false;
    bool inside_statement_list = statement_list();
    
    token_t t = get_next_token(&scanner);
    if(t.token_type == KEYWORD){
        if( strcmp( (char *) t.attr , "end") == 0 ){
            fprintf(stderr,"Ended while\n");
            return inside_statement_list;
        }
        incorrect_token("[]",t,&scanner);
        return false;
    }else{
        incorrect_token("Better check the implementation! END",t,&scanner);
        return false;
    }
    return inside_statement_list;
}
/**
 * *Temporary solution!
 * Expressions should temporarily be replaced by INTEGER or STRING or NUMBER or ID
 */
bool is_expression(token_t t){
    return (t.token_type == NUMBER || t.token_type == INTEGER || t.token_type == STRING || t.token_type == IDENTIFIER);
}
bool parse_expression(){
    fprintf(stderr,"parsing expression..\n");
    token_t t = get_next_token(&scanner);
    if(is_expression(t))
        return true;
    else
        incorrect_token("Current implementation of expression has INTEGER, STRING, NUMBER, ID",t,&scanner);
    return false;
}
bool parse_datatype(){
    token_t t = get_next_token(&scanner);
    if(t.token_type != KEYWORD)
        return false;
    if(strcmp("string",t.attr) == 0 || strcmp("number",t.attr) == 0 || strcmp("integer",t.attr) == 0)
        return true;
    incorrect_token("DATATYPE keyword",t,&scanner);
    return false;
}
void incorrect_token(char * expected, token_t t, scanner_t * scanner){
    fprintf(stderr, "stdin:\033[1;37m%lu:%lu\033[0m: \033[0;31mError, Wrong token!\033[0m %s expected, but token is: \033[1;33m%s\033[0m type: \033[0;33m%i\033[0m!\n",(scanner->cursor_pos[0]), (scanner->cursor_pos[1]),expected, (char *) t.attr, t.token_type);
}
bool lookahead_token(scanner_t * scanner,token_type_t expecting){
    token_t t = lookahead(scanner);
    if(t.token_type == expecting)
        return true;
    return false;
}
bool lookahead_token_attr(scanner_t * scanner,token_type_t expecting, char * expecting_attr){
    token_t t = lookahead(scanner);
    if( t.token_type == expecting ){
        if(0 == strcmp(expecting_attr,(char*) t.attr)){
            return true;
        }
    }
    return false;
}
bool check_next_token(scanner_t * scanner,token_type_t expecting){
    token_t t = get_next_token(scanner);
    if(t.token_type == expecting){
        return true;
    }
    char exp_str[3];
    sprintf(exp_str, "%d", expecting);
    incorrect_token(exp_str, t, scanner);
    return false;
}
bool check_next_token_attr(scanner_t * scanner,token_type_t expecting, char * expecting_attr){
    token_t t = get_next_token(scanner);
    if( t.token_type == expecting ){
        if(0 == strcmp(expecting_attr,(char*) t.attr)){
            return true;
        }
    }
    // char exp_str[3];
    // sprintf(exp_str, "%d", expecting);
    incorrect_token(expecting_attr, t, scanner);
    return false;
}