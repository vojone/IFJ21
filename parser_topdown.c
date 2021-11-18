/**
 * @file parser-topdown.c
 * @brief Source file for recursive descent parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */

#include "parser_topdown.h"


static scanner_t * scanner;
static parser_t * parser;
static symtab_t symtab;
#define MAXCALLS 100
static int current_calls = 0;
#define DEBUG true

void parser_setup(parser_t * p,scanner_t *s){
    parser = p;
    p->reached_EOF=false;
    p->return_code=0;
    scanner = s;
    init_tab(&(symtab));
}
int parse_program(){
    //scanner_init(scanner);    
    
    //run parsing
    int res = global_statement_list();

    fprintf(stderr,"Finished! return code: %i, at: (%lu,%lu)\n",res, scanner->cursor_pos[0], scanner->cursor_pos[1]);

    return res;

}
int global_statement_list(){
    int ret = global_statement();
    
    if(ret != PARSE_SUCCESS)
        return ret;

    if(parser->reached_EOF)
        return PARSE_SUCCESS;

    return global_statement_list();
}
//<statement-list>        -> <statement> <statement-list>
int statement_list(){
    int ret = statement();

    if(ret != PARSE_SUCCESS)
        return ret;

    token_t t = lookahead(scanner);
    if( t.token_type == KEYWORD ){
        if( compare_token_attr(t, KEYWORD, "end") ){
            debug_print("got end\n");
            return PARSE_SUCCESS;
        }
        if( compare_token_attr(t, KEYWORD, "else") ){
            debug_print("got else\n");
            return PARSE_SUCCESS;
        }
    }
    return statement_list();
}
int global_statement(){
    //* remove later
    if(current_calls >= MAXCALLS){
        debug_print("Max calls exceeded, terminating \n");
        return INTERNAL_ERROR;
    }
    current_calls++;

    token_t t = get_next_token(scanner);
    if(compare_token(t, KEYWORD)){
        if(compare_token_attr(t,KEYWORD,"require")){
            //<statement>             -> require <str>
            return parse_str();
        }else if(compare_token_attr(t,KEYWORD,"function")){
            //<statement>             -> function [id](<param-list>) : <type-list> <statement-list> end
            debug_print("function ...\n");
            return parse_function_def();
        }
            
        incorrect_token("This is global scope so keyword such as require or function definition or call expected", t, scanner);
        return false;
    }else if(compare_token(t,IDENTIFIER)){
        char *func = t.attr;
        tree_node_t *func_valid = search(&(symtab), func);
        if (func_valid == NULL){ //----------------------------------------------------------------------------
            return SEMANTIC_ERROR_DEFINITION;
        }
        if(lookahead_token_attr(scanner,SEPARATOR,"(")){
            return parse_function_call();
        }
        incorrect_token("function call",t,scanner);
        return SYNTAX_ERROR;
    }else if(compare_token(t,EOF_TYPE)){
        parser->reached_EOF = true;
        return PARSE_SUCCESS;
    }

    incorrect_token("This is global scope so keyword such as require or function definition or call expected",t,scanner);
    return SYNTAX_ERROR;
}
int statement () {
    //just to be sure for development. remove later
    if(current_calls >= MAXCALLS){
        debug_print("Max calls exceeded, terminating \n");
        return INTERNAL_ERROR;
    }
    current_calls++;
    token_t t = lookahead(scanner);
    
    switch (t.token_type)
    {
    case IDENTIFIER:
        true;
        char *func = t.attr;
        // it is neccessary to look at the next lexeme also
        t = get_next_token(scanner);
        t = lookahead(scanner);
        bool is_multiple_assignment = compare_token_attr(t,SEPARATOR,",");
        bool is_single_assignment = compare_token_attr(t,OPERATOR,"=");
        //check if it is a function call
        debug_print("Calling function ------------");
        if(compare_token_attr(t,SEPARATOR, "(")){
            debug_print("Call function %s\n", func);
            tree_node_t *func_valid = search(&(symtab), func);
            if (func_valid == NULL){ //----------------------------------------------------------------------------
                return SEMANTIC_ERROR_DEFINITION;
            }
            return parse_function_call();
        }else if(is_multiple_assignment || is_single_assignment){
            return assignment();
        }else{
            incorrect_token("After IDENTIFIER a function call or an assignment",t,scanner);
        }
        break;
    case KEYWORD:
        // true;
        // rule_t rule_to_use = determine_rule(t);
        // t = get_next_token(scanner);
        // int res = rule_to_use.rule_function();
        // debug_print("Using rule %s, which returned %i\n",(char *)rule_to_use.rule_first.attr,res);
        // return res;
        if(compare_token_attr(t,KEYWORD,"local")){
            //<statement>             -> local  [id] : [type] <value-assignment>
            t = get_next_token(scanner);
            return parse_local_var();
        }else if(compare_token_attr(t,KEYWORD,"global")){
            //<statement>             -> global  [id] : [type] <value-assignment>
            t = get_next_token(scanner);
            debug_print("parsing global");
            return parse_global_var();
        }else if(compare_token_attr(t,KEYWORD,"if")){
            //<statement>             -> return <exp>
            t = get_next_token(scanner);
            return parse_if();
        }else if(compare_token_attr(t,KEYWORD,"else")){
            return PARSE_SUCCESS;
        }else if(compare_token_attr(t,KEYWORD,"while")){
            t = get_next_token(scanner);
            return parse_while();
        }else if(compare_token_attr(t,KEYWORD,"return")){
            //<statement>             -> return <exp>
            t = get_next_token(scanner);
            debug_print("Calling precedence parser...\n");
            sym_dtype_t ret_type;
            return parse_expression(scanner, &ret_type);
        }else if(compare_token_attr(t,KEYWORD,"end")){
            return PARSE_SUCCESS;
        }
        break;
    case EOF_TYPE:
        incorrect_token("Reached EOF inside a function. Did you forget 'end'? END",t,scanner);
        parser->reached_EOF = true;
        return SYNTAX_ERROR;
        break;
    default:
        debug_print("NO RULE TO USE: Token type is: %i\n",t.token_type);
        return SYNTAX_ERROR;
        break;
    }
    return SYNTAX_ERROR;
}
int assignment(){
    int token_count = 0;
    bool foundAssignmentOp = false;
    //first loop checks and counts left side of the assignment
    while(!foundAssignmentOp){
        //identifier should be first
        token_t t;
        //first token is already read at the beginning, so we check wheter we are at the beginning
        if(token_count != 0)
            t = get_next_token(scanner);
        if(t.token_type == IDENTIFIER || token_count == 0)
            token_count++;
        else{
            incorrect_token("IDENTIFIER",t,scanner);
            return SYNTAX_ERROR;
        }
        //comma should follow, or assignment operator
        t = get_next_token(scanner);
        if(compare_token_attr(t,SEPARATOR,",")){
            //ok
        }else if(compare_token_attr(t,OPERATOR,"=")){
            foundAssignmentOp = true;
        }else{
            incorrect_token("SEPARATOR ',' or OPERATOR '='",t,scanner);
            return SYNTAX_ERROR;
        }
    }
    debug_print("found %i assignment...\n", token_count);
    //checking the RHS part
    for(int i = 0; i < token_count;i++){
        //check for valid expression
        debug_print("Calling precedence parser...\n");
        sym_dtype_t ret_type;
        if(parse_expression(scanner, &ret_type) == PARSE_SUCCESS){
            //ok
        }else{
            debug_print("Error while parsing expression for multiple assignemt\n");
            return SYNTAX_ERROR;
        }
        //if we are not at the end check for comma
        if(i+1 != token_count){
            token_t t = get_next_token(scanner);
            if( compare_token_attr(t,SEPARATOR,",")){
                //ok
            }else{
                incorrect_token("Missing SEPARATOR ',' in the assignment, which was",t,scanner);
                return SYNTAX_ERROR;
            }
        }
    }
    return PARSE_SUCCESS;
}
int parse_local_var(){
    //should be identifier
    token_t t = get_next_token(scanner);
    char* var_id = t.attr;
    debug_print("Var_ID is: %s <---------------\n\n", var_id);
    if(!compare_token(t,IDENTIFIER)){
        incorrect_token("identifier",t,scanner);
        return t.token_type == EOF_TYPE ? LEXICAL_ERROR : SYNTAX_ERROR;
    }
    //should be a comma
    bool comma = check_next_token_attr(scanner,SEPARATOR,":");
    if(!comma)
        return SYNTAX_ERROR;

    //should be a data type
    t = get_next_token(scanner);
    char* var_type = t.attr;
    debug_print("Var type is: %s <---------------\n\n", var_type);
    bool type = is_datatype(t);
    if(!type){
        incorrect_token("datatype",t,scanner);
        return SYNTAX_ERROR;
    }

    //there can be a value assignment
    if(lookahead_token_attr(scanner,OPERATOR,"=")){ 
        get_next_token(scanner);
        //if there is = we check the value assignment
        debug_print("Calling precedence parser...\n");
        sym_dtype_t ret_type;
        int assignment = parse_expression(scanner, &ret_type);
        if(assignment != PARSE_SUCCESS)
            incorrect_token("Valid expression",t,scanner);
        return assignment;
    }
    //  //  //
    // TODO //
    //  //  //
    char* expr_type = "s";
    debug_print("Data type of identifier %s vs %s <===================\n\n", var_type, expr_type);
    
    if (strcmp(var_type,expr_type) == 0){
        debug_print("Var: %s Type: %s => TS\n\n", var_id, var_type);
        //sym_data_t var_data = {var_id, var_type};
        //insert_sym(&(symtab), var_id, var_data);
    }else{
        return SEMANTIC_ERROR_ASSIGNMENT;
    }
     
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    return PARSE_SUCCESS;
}
int parse_global_var(){
    //should be identifier
    token_t t = get_next_token(scanner);
    char* var_id = t.attr;
    if(!compare_token(t,IDENTIFIER)){
        incorrect_token("identifier",t,scanner);
        return SYNTAX_ERROR;
    }
    //should be a comma
    bool comma = check_next_token_attr(scanner,SEPARATOR,":");
    if(!comma)
        return SYNTAX_ERROR;

    //should be a data type
    t = get_next_token(scanner);
    char* var_type = t.attr;
    bool type = is_datatype(t);
    if(!type){
        incorrect_token("datatype",t,scanner);
        return SYNTAX_ERROR;
    }

    //there can be a value assignment
    if(lookahead_token_attr(scanner,OPERATOR,"=")){ 
        get_next_token(scanner);
        //if there is = we check the value assignment
        debug_print("Calling precedence parser...\n");
        sym_dtype_t ret_type;
        int assignment = parse_expression(scanner, &ret_type);
        if(assignment != PARSE_SUCCESS)
            incorrect_token("Valid expression",t,scanner);
        return assignment;
    }
    //  //  //
    // TODO //
    //  //  //
    char* expr_type = "s";
    debug_print("Data type of identifier %s vs %s <===================\n\n", var_type, expr_type);
    
    if (strcmp(var_type,expr_type) == 0){
        debug_print("Var: %s Type: %s => TS\n\n", var_id, var_type);
        //sym_data_t var_data = {var_id, var_type};
        //insert_sym(&(symtab), var_id, var_data);
    }else{
        return SEMANTIC_ERROR_ASSIGNMENT;
    }
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    return PARSE_SUCCESS;
}

int parse_str(){
    token_t t = get_next_token(scanner);
    if(t.token_type == STRING)
        return PARSE_SUCCESS;
    else
        incorrect_token("STRING",t,scanner);
    return SYNTAX_ERROR;
}
int parse_function_def(){

    token_t id_fc = get_next_token(scanner);
    string_t datatype;
    str_init(&datatype);
    //parsing function definition signature
    bool id = (id_fc.token_type == (IDENTIFIER));
    bool left_bracket = check_next_token_attr(scanner,SEPARATOR,"(");
    int params = 0;
    int returns = 0;

    // check if there are parameters
    token_t t = lookahead(scanner);
    if(t.token_type == IDENTIFIER){
        //will just get the ':'
        debug_print("parsing function parameters...\n");
        bool finished = false;
        while(!finished){
            //should be IDENTIFIER
            t = get_next_token(scanner);
            if( t.token_type != IDENTIFIER ){
                incorrect_token("IDENTIFIER",t,scanner);
                finished = true;
                return SYNTAX_ERROR;
            }
            
            //should be COLON
            t = get_next_token(scanner);
            if( !compare_token_attr(t,SEPARATOR,":") ){
                incorrect_token("':'",t,scanner);
                finished = true;
                return SYNTAX_ERROR;
            }

            //should be DATATYPE
            t = get_next_token(scanner);
            if( !is_datatype(t) ){
                incorrect_token("DATA TYPE",t,scanner);
                finished = true;
                return SYNTAX_ERROR;
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(scanner, SEPARATOR,",");
            if(!comma)
                finished = true;
            else{
                //we go one token forward
                get_next_token(scanner);
            }
            params++;
        }
    }


    bool right_bracket= check_next_token_attr(scanner,SEPARATOR,")");
    if(!(id && left_bracket && right_bracket)){
        debug_print("ERROR INVALID FUNCTION SINATURE!\n");
        return SYNTAX_ERROR;
    }

    //parsing types if there is colon
    if(lookahead_token_attr(scanner, SEPARATOR, ":")){
        //will just get the ':'
        debug_print("parsing function types...\n");
        token_t t = get_next_token(scanner);

        /*
        if (strcmp(char_to_dtype((datatype.str)),"INT") == 1){

        }else if (strcmp(char_to_dtype((datatype.str)),"STR") == 1){

        }else if (strcmp(char_to_dtype((datatype.str)),"NUM") == 1){

        }else{

        }
        */
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------

        //app_char( písmeno, datatype);
        bool finished = false;
        while(!finished){
            //should be datatype
            t = get_next_token(scanner);
            if(!is_datatype(t)){
                incorrect_token("data type",t,scanner);
                finished = true;
                return SYNTAX_ERROR;
            }
            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(scanner, SEPARATOR,",");
            if(!comma)
                finished = true;
            else{
                //we go one token forward
                get_next_token(scanner);
            }
            returns++;
        }
    }

    sym_data_t symtab_data = {id_fc.attr, FUNC, datatype};
    insert_sym(&(symtab), id_fc.attr, symtab_data);
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------

    //parsing inside function
    debug_print("parsing inside function...\n");
    int res = statement_list();
    debug_print("parsing function finished! return code: %i, at: (%lu,%lu)\n",res, scanner->cursor_pos[0], scanner->cursor_pos[1]); 

    t = get_next_token(scanner);
    if(t.token_type == KEYWORD){
        if( compare_token_attr(t,KEYWORD,"end") ){
            debug_print("Successfully ended function definition!\n");
            return res;
        }else{
            incorrect_token("Wrong keyword! Only END",t, scanner);
            return SYNTAX_ERROR;
        }
        incorrect_token("[]",t,scanner);
        return SYNTAX_ERROR;
    }else{
        incorrect_token("Better check the implementation! END",t,scanner);
    }
    return SYNTAX_ERROR;
}
int parse_function_call(){
    debug_print("parsing function call...\n");
    bool opening_bracket = check_next_token_attr(scanner, SEPARATOR, "(");
    if(opening_bracket){
        int args = parse_function_arguments();
        debug_print("function args success %i\n",(int) args);
        return args;
    }else{
        return SYNTAX_ERROR;
    }
    return PARSE_SUCCESS;
}
int parse_function_arguments(){
    bool closing_bracket = false;
    while(!closing_bracket){
        token_t t = get_next_token(scanner);
        if(is_expression(t)){
            //ok
        }else if(compare_token_attr(t,SEPARATOR,")")){
            closing_bracket = true;
            continue;
        }else{
            incorrect_token("EXPRESSION",t,scanner);
        }
        t = get_next_token(scanner);
        if(compare_token_attr(t,SEPARATOR,",")){
            //ok
        }else if(compare_token_attr(t,SEPARATOR,")")){
            closing_bracket = true;
        }
        else{
            incorrect_token("',' as a separator between args or ')' as an end of argument list",t,scanner);
            return SYNTAX_ERROR;
        }
    }
    return closing_bracket ? PARSE_SUCCESS : SYNTAX_ERROR;
}

int parse_if(){
    debug_print("Calling precedence parser...\n");
    sym_dtype_t ret_type;
    int condition = parse_expression(scanner, &ret_type);
    
    if(condition != PARSE_SUCCESS)
        return SYNTAX_ERROR;

    bool then = check_next_token_attr(scanner, KEYWORD, "then");
    if(!then)
        return SYNTAX_ERROR;

    int inside_statement_list = statement_list();
    
    token_t t = get_next_token(scanner);
    if(t.token_type == KEYWORD){
        if( compare_token_attr(t,KEYWORD,"end") ){
            debug_print("Ended if\n");
            return inside_statement_list;
        }
        else if( compare_token_attr(t,KEYWORD,"else") ){
            //<else-branch>           -> else <statement-list>
            debug_print("parsing else branch...\n");
            int else_branch_statement_list = statement_list();
            t = get_next_token(scanner);
    
            //ended else branch
            if(inside_statement_list == PARSE_SUCCESS && else_branch_statement_list == PARSE_SUCCESS)
                return PARSE_SUCCESS;
            if(inside_statement_list != PARSE_SUCCESS)
                return inside_statement_list;
            if(else_branch_statement_list != PARSE_SUCCESS)
                return else_branch_statement_list;
        }
        incorrect_token("[2]",t,scanner);
        return SYNTAX_ERROR;
    }else{
        incorrect_token("Better check the implementation! END",t,scanner);
        return SYNTAX_ERROR;
    }
    return inside_statement_list;
}

int parse_else(){
    return PARSE_SUCCESS;
}

int parse_return(){
    //TODO CHECK THE SYMTABLE TO KNOW HOW MANY EXPRESSIONS SHOULD BE RETURNED
    //! The ACTUAL return parsing is handled in statement() currently, it checks for exactly ONE expression
    //* SOMETHING LIKE THIS WILL BE HERE LATER
    // bool finished = false;
    // while(/* check for symtable */finished){
    //     //should be expression
    //     token_t t = get_next_token(scanner);
    //     if(!is_expression(t)){
    //         incorrect_token("EXPRESSION",t,scanner);
    //         finished = true;
    //         return SEMANTIC_ERROR_ASSIGNMENT;
    //     }
    //     //if there is no comma we should be at the end of the list
    //     bool comma = lookahead_token_attr(scanner, SEPARATOR,",");
    //     if(!comma)
    //         finished = true;
    //     else{
    //         //we go one token forward
    //         get_next_token(scanner);
    //     }
    // }
    
    return PARSE_SUCCESS;
}

int parse_end(){
    return PARSE_SUCCESS;
}

int parse_while(){
    debug_print("Calling precedence parser...\n");
    sym_dtype_t ret_type;
    int condition = parse_expression(scanner, &ret_type);
    if(condition != PARSE_SUCCESS)
        return condition;
    bool then = check_next_token_attr(scanner, KEYWORD, "do");
    if(!then)
        return false;
    int inside_statement_list = statement_list();
    
    token_t t = get_next_token(scanner);
    if(t.token_type == KEYWORD){
        if( compare_token_attr(t,KEYWORD,"end") ){
            debug_print("Ended while\n");
            return inside_statement_list;
        }
        incorrect_token("[]",t,scanner);
        return SYNTAX_ERROR;
    }else{
        incorrect_token("Better check the implementation! END",t,scanner);
        return SYNTAX_ERROR;
    }
    return inside_statement_list;
}

/**
 * *MORE GENERAL RULE PARSING ATTEMPT
 */
rule_t determine_rule(token_t t){
    rule_t statement_rules[] = {
        {parse_local_var,   {KEYWORD,"local"}   },
        {parse_global_var,  {KEYWORD,"global"}  },
        {parse_if,          {KEYWORD,"if"}      },
        {parse_else,        {KEYWORD,"else"}    },
        {parse_while,       {KEYWORD,"while"}   },
        {parse_return,      {KEYWORD,"return"}  },
        {parse_end,         {KEYWORD,"end"}     }
    };
    size_t rules_n = sizeof(statement_rules) / sizeof(rule_t);
    for (size_t i = 0; i < rules_n; i++)
    {
        token_t expected = statement_rules[i].rule_first;
        if(compare_token_attr(t,expected.token_type,expected.attr)){
            statement_rules[i].rule_function();
            return statement_rules[i];
        }
    }
    incorrect_token("NO RULE can be used to parse this token! Other",t,scanner);
    return (rule_t) {error_rule, {-1,NULL}};
}
int error_rule(){
    return SYNTAX_ERROR;
}

/**
 * *Temporary solution!
 * Expressions should temporarily be replaced by INTEGER or STRING or NUMBER or ID
 */
bool is_expression(token_t t){
    return (t.token_type == NUMBER || t.token_type == INTEGER || t.token_type == STRING || t.token_type == IDENTIFIER);
}
// int parse_expression(){
//     debug_print("parsing expression..\n");
//     token_t t = get_next_token(scanner);
//     if(is_expression(t))
//         return PARSE_SUCCESS;
//     else
//         incorrect_token("Current implementation of expression has INTEGER, STRING, NUMBER, ID",t,scanner);
//     return SYNTAX_ERROR;
// }
bool is_datatype(token_t t){
    return (compare_token_attr(t, KEYWORD, "string") || compare_token_attr(t, KEYWORD, "number") || compare_token_attr(t, KEYWORD, "integer") || compare_token_attr(t, KEYWORD, "bool"));
}
int parse_datatype(){
    token_t t = get_next_token(scanner);
    if( is_datatype(t) )
        return PARSE_SUCCESS;
    incorrect_token("DATATYPE keyword",t,scanner);
    return SYNTAX_ERROR;
}
void incorrect_token(char * expected, token_t t, scanner_t * scanner){
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t|\033[0;31m Syntax error:\033[0m Wrong token! %s expected, but token is: \033[1;33m%s\033[0m type: \033[0;33m%i\033[0m!\n",(scanner->cursor_pos[0]), (scanner->cursor_pos[1]),expected, (char *) t.attr, t.token_type);
}
bool lookahead_token(scanner_t * scanner,token_type_t expecting){
    token_t t = lookahead(scanner);
    return compare_token(t, expecting);
}
bool lookahead_token_attr(scanner_t * scanner,token_type_t expecting_type, char * expecting_attr){
    token_t t = lookahead(scanner);
    return compare_token_attr(t, expecting_type, expecting_attr);
}
bool check_next_token(scanner_t * scanner,token_type_t expecting){
    token_t t = get_next_token(scanner);
    
    if(compare_token(t, expecting))
        return true;

    parser->return_code = (t.token_type == ERROR_TYPE) ? LEXICAL_ERROR : SYNTAX_ERROR;
    char exp_str[3];
    sprintf(exp_str, "%d", expecting);
    incorrect_token(exp_str, t, scanner);
    return false;
}
bool check_next_token_attr(scanner_t * scanner, token_type_t expecting_type, char * expecting_attr){
    token_t t = get_next_token(scanner);

    if( compare_token_attr(t, expecting_type, expecting_attr) )
        return true;

    parser->return_code = (t.token_type == ERROR_TYPE) ? LEXICAL_ERROR : SYNTAX_ERROR;
    incorrect_token(expecting_attr, t, scanner);
    return false;
}
bool compare_token(token_t t, token_type_t expecting){
    if(t.token_type == expecting){
        return true;
    }
    return false;
}
bool compare_token_attr(token_t t, token_type_t expecting_type, char * expecting_attr){
    if( t.token_type == expecting_type ){
        if(strcmp(t.attr, expecting_attr) == 0){
            return true;
        }
    }
    return false;
}

void debug_print(const char *const _Format, ...){
    if(DEBUG){
        //get the arguments
        va_list args;
        va_start(args,_Format);
        //use variable argument printf
        vfprintf(stderr,_Format,args);
    }
}
