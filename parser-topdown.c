/**
 * @file parser-topdown.c
 * @brief Source file for recursive descent parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */

#define TMP_FILE_NAME "tmp.inp"
#include "parser-topdown.h"

scanner_t scanner;

int main(){
    scanner_init(&scanner);    
    
    //run parsing
    statement_list();
}

void statement_list(){
    token_t t = get_next_token(&scanner);
    fprintf(stderr,"Token type is: %i\n",t.token_type);
    
    if(t.token_type == KEYWORD){
        fprintf(stderr,"The token is: %s",(char *) t.attr);
    }
}

void statement(){
    
}