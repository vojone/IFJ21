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

static rule_t ruleset_global[] = {
    {parse_require,             {KEYWORD, UNSET, "require"},  true },
    {parse_function_dec,        {KEYWORD, UNSET, "global"},   true },
    {parse_function_def,        {KEYWORD, UNSET, "function"}, true },
    {parse_global_identifier,   {IDENTIFIER, UNSET, NULL},    false},
    {EOF_global_rule,           {EOF_TYPE, UNSET, NULL},      false},
};
#define RULESET_GLOBAL_LENGTH 5

static rule_t ruleset_inside[] = {
    {parse_local_var,   {KEYWORD, UNSET, "local"},  true  },
    {parse_if,          {KEYWORD, UNSET, "if"},     true  },
    {parse_else,        {KEYWORD, UNSET, "else"},   true  },
    {parse_while,       {KEYWORD, UNSET, "while"},  true  },
    {parse_return,      {KEYWORD, UNSET, "return"}, true  },
    {parse_end,         {KEYWORD, UNSET, "end"},    true  },
    {parse_identifier,  {IDENTIFIER, UNSET, NULL},  false },
    {EOF_fun_rule,      {EOF_TYPE, UNSET, NULL},    false },
};
#define RULESET_INSIDE_LENGTH 8

#define DEBUG true

tree_node_t * search_in_tables(symtab_t *symbol_table, char *key) {
    symtab_t *curr_tab = symbol_table;
    
    while(curr_tab != NULL) {
        tree_node_t * result_of_searching = search(curr_tab, key);
        if(result_of_searching) {
            return result_of_searching;
        }
        else {
            curr_tab = symtabs_get_ptr(&parser->symtabs, curr_tab->parent_ind);
        }
    }

    return NULL;
}

/**
 * @brief Sets symtab to elder symbol table of old outer context 
 */ 
void to_outer_ctx(parser_t *p) {
    destroy_tab(&symtab);

    symtab = symtabs_pop(&p->symtabs);
}

/**
 * @brief Creates new symbol table (new context) and sets symtable to it
 */ 
void to_inner_ctx(parser_t *p) {
    symtabs_push(&p->symtabs, symtab); //Save copy of old symtab to the stack of symtabs

    symtab_t new_ctx; //Create and init new symtab
    init_tab(&new_ctx);
    new_ctx.parent_ind = symtabs_get_top_ind(&p->symtabs); //Save reference to the parent symtab

    symtab = new_ctx;
}

void parser_setup(parser_t *p, scanner_t *s) {
    parser = p;
    p->reached_EOF = false;
    p->return_code = 0;
    scanner = s;

    symtabs_stack_init(&p->symtabs);

    symtab_t global;
    init_tab(&global);
    symtab = global;
}

//<program>               -> <global-statement-list>
int parse_program() {
    //scanner_init(scanner);    
    
    //run parsing
    int res = global_statement_list();

    fprintf(stderr, "Finished! return code: %i, at: (%lu,%lu)\n",res, scanner->cursor_pos[0], scanner->cursor_pos[1]);

    destroy_tab(&symtab);
    while(!symtabs_is_empty(&(parser->symtabs))) {
        symtab_t current = symtabs_pop(&(parser->symtabs));
        destroy_tab(&current);
    }

    symtabs_stack_dtor(&(parser->symtabs));

    return res;

}

//<global-statement-list> -> <global-statement> <global-statement-list>
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
    if(t.token_type == KEYWORD) {
        if(compare_token_attr(t, KEYWORD, "end")) {
            debug_print("got end\n");
            return PARSE_SUCCESS;
        }
        if(compare_token_attr(t, KEYWORD, "else")) {
            debug_print("got else\n");
            return PARSE_SUCCESS;
        }
    }

    return statement_list();
}

int global_statement() {
    debug_print("parsing next global statement...\n");
    token_t t = lookahead(scanner);

    //get the apropriate rule
    rule_t rule_to_use = determine_rule(t,ruleset_global);
    
    //call the right function
    int res = rule_to_use.rule_function();

    //if there is (null) instead the first token of the rule, it means that the rule is using only token type and not attribute 
    debug_print("global statement %s returned code %i\n",rule_to_use.rule_first.attr,res);
    if(res == SYNTAX_ERROR)
        error_unexpected_token("This is global scope so keyword such as require or function definition or call expected",t);
    
    return res;
}


int statement() {
    debug_print("parsing next statement...\n");
    token_t t = lookahead(scanner);

    //get the apropriate rule
    rule_t rule_to_use = determine_rule(t,ruleset_inside);
    
    //call the right function
    int res = rule_to_use.rule_function();

    //if there is (null) instead the first token of the rule, it means that the rule is using only token type and not attribute 
    debug_print("statement %s returned code %i\n",rule_to_use.rule_first.attr,res);

    return res;
}

int assignment(token_t t) {
    //this function expects that the current token is identifier
    //so next token should be = or ,
    /*
    token_t current_token;
    token_init(&current_token);
    */    

    //char* var_id = last_token.attr;
    //debug_print("\n\nvar_id: %s\n\n"); //current token);
    token_t var_id = t;
    debug_print("var id: %s\n",get_attr(&var_id,scanner));

    int token_count = 0;
    bool foundAssignmentOp = false;

    //this should be either = or ,
    // t = lookahead(scanner);
    // debug_print("the token after identifer is %s\n",get_attr(&t,scanner));
    //first loop checks and counts left side of the assignment
    while(!foundAssignmentOp) {
        //first token is already read at the beginning, so we check wheter we are at the beginning
        if(token_count != 0) {
            t = get_next_token(scanner);
        }
        //the next token should be identifier
        if(t.token_type == IDENTIFIER) {
            token_count++;
            if(!search_in_tables(&symtab, get_attr(&t, scanner))) {
                error_semantic("%d Undeclared: %s", symtab.parent_ind, get_attr(&t, scanner));
                return SEMANTIC_ERROR_ASSIGNMENT;
            }
        }
        else {
            error_unexpected_token("IDENTIFIER", t);
            return SYNTAX_ERROR;
        }
        //comma should follow, or assignment operator
        t = get_next_token(scanner);
        if(compare_token_attr(t, SEPARATOR, ",")) {
            //ok
        }
        else if(compare_token_attr(t, OPERATOR, "=")) {
            foundAssignmentOp = true;

            // Semantics for value assignment to variable
            tree_node_t* symtab_var = search_in_tables(&symtab, get_attr(&var_id, scanner));
            if(!symtab_var) {
                return SEMANTIC_ERROR_DEFINITION;
            }

            sym_dtype_t d_ret_type;
            int dtype = parse_expression(scanner, &symtab, &d_ret_type);
            if (dtype == 0){
                debug_print("\nDTYPE of EXP: '%d'\n\n", d_ret_type);
                if (symtab_var->data.dtype != d_ret_type){
                    return SEMANTIC_ERROR_ASSIGNMENT;
                }    
            }
            else{
                return INTERNAL_ERROR_;
            }
        }
        else {
            error_unexpected_token("SEPARATOR ',' or OPERATOR '='", t);
            return SYNTAX_ERROR;
        }
    }

    debug_print("found %i assignment...\n", token_count);

    //checking the RHS part
    for(int i = 0; i < token_count; i++) {
        //check for valid expression
        debug_print("Calling precedence parser...\n");
        sym_dtype_t ret_type;
        int expr_retval = parse_expression(scanner, &symtab, &ret_type);
        if(expr_retval == EXPRESSION_SUCCESS) {
            //ok
        }
        else {
            debug_print("Error while parsing expression for multiple assignment\n");
            return expr_retval;
        }
        //if we are not at the end check for comma
        if(i + 1 != token_count) {
            token_t t = get_next_token(scanner);
            if(compare_token_attr(t, SEPARATOR, ",")) {
                //ok
            }
            else {
                error_unexpected_token("Missing SEPARATOR ',' in the assignment, which was", t);
                return SYNTAX_ERROR;
            }
        }
    }

    return PARSE_SUCCESS;
}


int parse_local_var() {
    //go one token forward
    get_next_token(scanner);

    //should be identifier
    token_t t = get_next_token(scanner);
    token_t var_id = t;
    //debug_print("Var_ID is: %s <---------------\n\n", var_id);
    if(!compare_token(t, IDENTIFIER)) {
        error_unexpected_token("identifier", t);
        return t.token_type == EOF_TYPE ? LEXICAL_ERROR : SYNTAX_ERROR;
    }

    sym_data_t new_data = {(char *)"inserted", VAR};
    insert_sym(&symtab, get_attr(&t, scanner), new_data);

    //should be a comma
    bool comma = check_next_token_attr(SEPARATOR, ":");
    if(!comma) {
        return SYNTAX_ERROR;
    }

    //should be a data type
    t = get_next_token(scanner);

    // Determining data type of declared variable
    int var_type;
    if (str_cmp(get_attr(&t, scanner), "string") == 0) {
        var_type = 2;
    }
    else if(str_cmp(get_attr(&t, scanner), "integer") == 0) {
        var_type = 0;
    }
    else if(str_cmp(get_attr(&t, scanner), "number") == 0) {
        var_type = 1;
    }

    bool type = is_datatype(t);
    if(!type) {
        error_unexpected_token("datatype", t);
        return SYNTAX_ERROR;
    }
    //there can be a value assignment
    if(lookahead_token_attr(OPERATOR,"=")) { 
        get_next_token(scanner);
        //if there is = we check the value assignment
        debug_print("Calling precedence parser...\n");
        sym_dtype_t ret_type;
        int assignment = parse_expression(scanner, &symtab, &ret_type);
        if(assignment != PARSE_SUCCESS)
            error_unexpected_token("Valid expression", t);
        return assignment;
    }
    // Adding variable and its datatype into symtable
    sym_data_t symdata_var = {get_attr(&var_id, scanner), VAR, .dtype = var_type};
    insert_sym(&symtab, get_attr(&var_id, scanner), symdata_var);
    
    return PARSE_SUCCESS;
}


int parse_global_var() {
    //go one token forward
    get_next_token(scanner);

    //should be identifier
    token_t t = get_next_token(scanner);

    // Save identifier
    token_t var_id = t;

    if(!compare_token(t, IDENTIFIER)) {
        error_unexpected_token("identifier", t);
        return SYNTAX_ERROR;
    }

    sym_data_t new_data = {(char *)"inserted", VAR};
    insert_sym(&symtab, get_attr(&t, scanner), new_data);

    //should be a comma
    bool comma = check_next_token_attr(SEPARATOR, ":");
    if(!comma) {
        return SYNTAX_ERROR;
    }

    //should be a data type
    t = get_next_token(scanner);

    // Save data type
    int var_type;
    if (str_cmp(get_attr(&t, scanner),"string") == 0) {
        var_type = 2;
        //debug_print("\n\n VAR: %s\tString\n\n",var_id);
    }else if(str_cmp(get_attr(&t, scanner),"integer") == 0) {
        var_type = 0;
        //debug_print("\n\n VAR: %s\tInteger or num\n\n",var_id);
    }else if(str_cmp(get_attr(&t, scanner), "number") == 0) {
        var_type = 1;
        //debug_print("\n\n VAR: %s\tNumber or int\n\n",var_id);
    }

    bool type = is_datatype(t);
    if(!type) {
        error_unexpected_token("datatype", t);
        return SYNTAX_ERROR;
    }

    //there can be a value assignment
    if(lookahead_token_attr(OPERATOR, "=")) { 
        get_next_token(scanner);
        //if there is = we check the value assignment
        debug_print("Calling precedence parser...\n");
        sym_dtype_t ret_type;
        int assignment = parse_expression(scanner, &symtab, &ret_type);
        if(assignment != PARSE_SUCCESS)
            error_unexpected_token("Valid expression", t);
        return assignment;
    }

    // semantics
    sym_data_t symdata_var = {get_attr(&var_id, scanner), VAR, .dtype = var_type};
    insert_sym(&symtab, get_attr(&var_id, scanner), symdata_var);

    return PARSE_SUCCESS;
}

int parse_require() {
    //go one token forward
    get_next_token(scanner);

    token_t t = get_next_token(scanner);
    if(t.token_type == STRING) {
        return PARSE_SUCCESS;
    }
    else {
        error_unexpected_token("STRING", t);
    }

    return SYNTAX_ERROR;
}


int parse_function_dec() {
    //this token is just 'global' we skip it
    get_next_token(scanner);

    token_t id = get_next_token(scanner);
    if(!compare_token(id,IDENTIFIER)){
        error_unexpected_token("IDENTIFIER",id);
        return SYNTAX_ERROR;
    }
    
    if(!check_next_token_attr(SEPARATOR,":"))
        return SYNTAX_ERROR;
    
    if(!check_next_token_attr(KEYWORD,"function"))
        return SYNTAX_ERROR;
    
    if(!check_next_token_attr(SEPARATOR,"("))
        return SYNTAX_ERROR;

    //parsing param types if there is param type
    if(is_datatype(lookahead(scanner))){
        debug_print("parsing function param types...\n");

        bool finished = false;
        while(!finished) {
            //should be datatype
            token_t t = get_next_token(scanner);
            if(!is_datatype(t)){
                error_unexpected_token("data type", t);
                finished = true;
                return SYNTAX_ERROR;
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(SEPARATOR, ",");
            if(!comma) {
                finished = true;
            }
            else {
                //we go one token forward
                get_next_token(scanner);
            }
        }
    }

    if(!check_next_token_attr(SEPARATOR,")"))
        return SYNTAX_ERROR;

    //parsing return types if there is colon
    if(lookahead_token_attr(SEPARATOR, ":")){
        //will just get the ':'
        debug_print("parsing function return types...\n");
        token_t t = get_next_token(scanner);

        //app_char( písmeno, datatype);
        bool finished = false;
        while(!finished) {
            //should be datatype
            t = get_next_token(scanner);
            if(!is_datatype(t)){
                error_unexpected_token("data type", t);
                finished = true;
                return SYNTAX_ERROR;
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(SEPARATOR, ",");
            if(!comma) {
                finished = true;
            }
            else {
                //we go one token forward
                get_next_token(scanner);
            }
        }
    }
    return PARSE_SUCCESS;
}


int parse_function_def() {
    //this token is just 'function' so we skip it
    get_next_token(scanner);

    token_t id_fc = get_next_token(scanner);

    //parsing function definition signature
    bool id = (id_fc.token_type == (IDENTIFIER));
    bool left_bracket = check_next_token_attr(SEPARATOR, "(");
    int params = 0;
    int returns = 0;

    // check if there are parameters
    token_t t = lookahead(scanner);
    if(t.token_type == IDENTIFIER) {
        //will just get the ':'
        debug_print("parsing function parameters...\n");
        bool finished = false;
        while(!finished) {
            //should be IDENTIFIER
            t = get_next_token(scanner);
            if(t.token_type != IDENTIFIER) {
                error_unexpected_token("IDENTIFIER", t);
                finished = true;
                return SYNTAX_ERROR;
            }
            
            //should be COLON
            t = get_next_token(scanner);
            if(!compare_token_attr(t, SEPARATOR, ":")) {
                error_unexpected_token("':'", t);
                finished = true;
                return SYNTAX_ERROR;
            }

            //should be DATATYPE
            t = get_next_token(scanner);
            if(!is_datatype(t)) {
                error_unexpected_token("DATA TYPE", t);
                finished = true;
                return SYNTAX_ERROR;
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(SEPARATOR, ",");
            if(!comma) {
                finished = true;
            }
            else {
                //we go one token forward
                get_next_token(scanner);
            }

            params++;
        }
    }


    bool right_bracket = check_next_token_attr(SEPARATOR, ")");
    if(!(id && left_bracket && right_bracket)) {
        debug_print("ERROR INVALID FUNCTION SINATURE!\n");
        return SYNTAX_ERROR;
    }

    //parsing types if there is colon
    if(lookahead_token_attr(SEPARATOR, ":")){
        //will just get the ':'
        debug_print("parsing function types...\n");
        token_t t = get_next_token(scanner);

        //app_char( písmeno, datatype);
        bool finished = false;
        while(!finished) {
            //should be datatype
            t = get_next_token(scanner);
            if(!is_datatype(t)){
                error_unexpected_token("data type", t);
                finished = true;
                return SYNTAX_ERROR;
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(SEPARATOR, ",");
            if(!comma) {
                finished = true;
            }
            else {
                //we go one token forward
                get_next_token(scanner);
            }
            returns++;
        }
    }

    sym_data_t symtab_data = {get_attr(&id_fc, scanner), FUNC};
    insert_sym(&symtab, get_attr(&id_fc, scanner), symtab_data);

    to_inner_ctx(parser);

    //parsing inside function
    debug_print("parsing inside function...\n");

    int res = statement_list();

    to_outer_ctx(parser);

    debug_print("parsing function finished! return code: %i, at: (%lu,%lu)\n", res, scanner->cursor_pos[0], scanner->cursor_pos[1]); 

    t = get_next_token(scanner);
    if(t.token_type == KEYWORD) {

        if(compare_token_attr(t, KEYWORD, "end")) {
            debug_print("Successfully ended function definition!\n");
            return res;
        }
        else {
            error_unexpected_token("Wrong keyword! Only END", t);
            return (res != PARSE_SUCCESS) ? res : SYNTAX_ERROR;
        }

        error_unexpected_token("[]", t);
        return (res != PARSE_SUCCESS) ? res : SYNTAX_ERROR;
    }
    else {
        error_unexpected_token("Better check the implementation! END", t);
    }

    return (res != PARSE_SUCCESS) ? res : SYNTAX_ERROR;
}


int parse_function_call() {
    debug_print("parsing function call...\n");
    bool opening_bracket = check_next_token_attr(SEPARATOR, "(");

    if(opening_bracket) {
        int args = parse_function_arguments();
        debug_print("function args success %i\n", (int)args);
        return args;
    }
    else {
        return SYNTAX_ERROR;
    }

    return PARSE_SUCCESS;
}
int parse_function_arguments() {
    bool closing_bracket = false;
    while(!closing_bracket){
        token_t t = get_next_token(scanner);
        if(is_expression(t)){
            //ok
        }
        else if(compare_token_attr(t, SEPARATOR, ")")) {
            closing_bracket = true;
            continue;
        }
        else{
            error_unexpected_token("EXPRESSION", t);
        }
        t = get_next_token(scanner);
        if(compare_token_attr(t, SEPARATOR, ",")) {
            //ok
        }
        else if(compare_token_attr(t, SEPARATOR, ")")) {
            closing_bracket = true;
        }
        else {
            error_unexpected_token("',' as a separator between args or ')' as an end of argument list",t);
            return SYNTAX_ERROR;
        }
    }

    return closing_bracket ? PARSE_SUCCESS : SYNTAX_ERROR;
}

int parse_if(){
    //go one token forward
    get_next_token(scanner);

    debug_print("Calling precedence parser...\n");
    sym_dtype_t ret_type;
    int condition = parse_expression(scanner, &symtab, &ret_type);
    
    if(condition != EXPRESSION_SUCCESS)
        return SYNTAX_ERROR;

    bool then = check_next_token_attr(KEYWORD, "then");
    if(!then)
        return SYNTAX_ERROR;

    to_inner_ctx(parser);

    int inside_statement_list = statement_list();
    
    to_outer_ctx(parser);

    token_t t = get_next_token(scanner);
    if(t.token_type == KEYWORD) {
        if(compare_token_attr(t, KEYWORD, "end")) {
            debug_print("Ended if\n");
            return inside_statement_list;
        }
        else if(compare_token_attr(t, KEYWORD, "else")) {

            to_inner_ctx(parser);

            //<else-branch>           -> else <statement-list>
            debug_print("parsing else branch...\n");
            int else_branch_statement_list = statement_list();

            to_outer_ctx(parser);

            t = get_next_token(scanner);
    
            //ended else branch
            if(inside_statement_list == PARSE_SUCCESS && else_branch_statement_list == PARSE_SUCCESS)
                return PARSE_SUCCESS;
            if(inside_statement_list != PARSE_SUCCESS)
                return inside_statement_list;
            if(else_branch_statement_list != PARSE_SUCCESS)
                return else_branch_statement_list;
        }

        error_unexpected_token("[2]", t);
        return (inside_statement_list != PARSE_SUCCESS) ? inside_statement_list : SYNTAX_ERROR;
    }
    else {
        error_unexpected_token("Better check the implementation! END", t);
        return (inside_statement_list != PARSE_SUCCESS) ? inside_statement_list : SYNTAX_ERROR;
    }
    return (inside_statement_list != PARSE_SUCCESS) ? inside_statement_list : SYNTAX_ERROR;
}

int parse_else() {
    return PARSE_SUCCESS;
}

int parse_return(){
    //go one token forward
    get_next_token(scanner);
    
    debug_print("Calling precedence parser...\n");
    sym_dtype_t ret_type;
    return parse_expression(scanner, &symtab, &ret_type);

    //TODO CHECK THE SYMTABLE TO KNOW HOW MANY EXPRESSIONS SHOULD BE RETURNED
    //! The ACTUAL return parsing is handled in statement() currently, it checks for exactly ONE expression
    //* SOMETHING LIKE THIS WILL BE HERE LATER
    // bool finished = false;
    // while(/* check for symtable */finished){
    //     //should be expression
    //     token_t t = get_next_token(scanner);
    //     if(!is_expression(t)){
    //         error_unexpected_token("EXPRESSION",t);
    //         finished = true;
    //         return SEMANTIC_ERROR_ASSIGNMENT;
    //     }
    //     //if there is no comma we should be at the end of the list
    //     bool comma = lookahead_token_attr(SEPARATOR,",");
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

int parse_while() {
    //go one token forward
    get_next_token(scanner);
    
    debug_print("Calling precedence parser...\n");
    sym_dtype_t ret_type;
    int condition = parse_expression(scanner, &symtab, &ret_type);
    if(condition != EXPRESSION_SUCCESS) {
        return condition;
    }
    bool then = check_next_token_attr( KEYWORD, "do");
    if(!then) {
        return false;
    }

    to_inner_ctx(parser);

    int inside_statement_list = statement_list();

    to_outer_ctx(parser);

    token_t t = get_next_token(scanner);
    if(t.token_type == KEYWORD) {
        if(compare_token_attr(t, KEYWORD, "end")) {

            debug_print("Ended while\n");
            return inside_statement_list;
        }
        
        error_unexpected_token("[]", t);
        return (inside_statement_list != PARSE_SUCCESS) ? inside_statement_list : SYNTAX_ERROR;
    }
    else {
        error_unexpected_token("Better check the implementation! END", t);
        return (inside_statement_list != PARSE_SUCCESS) ? inside_statement_list : SYNTAX_ERROR;
    }

    return (inside_statement_list != PARSE_SUCCESS) ? inside_statement_list : SYNTAX_ERROR;
}

/**
 * *MORE GENERAL RULE PARSING ATTEMPT
 * Determines next rule based on first lexeme
 */
rule_t determine_rule(token_t t, rule_t ruleset[]) {
    
    // size_t rules_n = sizeof(ruleset) / sizeof(rule_t);
    for (size_t i = 0; i < RULESET_INSIDE_LENGTH; i++)
    {
        token_t to_be_checked = ruleset[i].rule_first;
        if(ruleset[i].attrib_relevant){
            //the atribute is relevant (rules with same token types)
            /**< Here can be .attr because the value of token is certainly referenced by pointer */
            if(compare_token_attr(t, to_be_checked.token_type, to_be_checked.attr)) {
                return ruleset[i];
            }
        }else{
            //the atribute is irrelevant we check only for matching token type
            if(compare_token(t, to_be_checked.token_type)) {
                return ruleset[i];
            }
        }
    }

    error_unexpected_token("NO RULE can be used to parse this token! Other", t);
    return (rule_t){error_rule, {-1, UNSET, NULL}};
}
int parse_global_identifier(){
    token_t id_token = get_next_token(scanner);
    debug_print("got identifier\n");
    char *func = get_attr(&id_token, scanner);

    debug_print("Call function %s\n", func);
    tree_node_t *func_valid = search_in_tables(&symtab, func);

    // Function is not declared
    if (func_valid == NULL) { //----------------------------------------------------------------------------
        error_semantic("Function name '%s' not defined!", func);
        return SEMANTIC_ERROR_DEFINITION;
    }
    return parse_function_call();
}

int parse_identifier(){
    //should be identifier
    token_t id_token = get_next_token(scanner);
    debug_print("got identifier\n");
    char *func = get_attr(&id_token, scanner);
    // it is neccessary to look at the next lexeme also
    // token_t id_token = get_next_token(scanner);
    token_t t = lookahead(scanner);
    bool is_multiple_assignment = compare_token_attr(t, SEPARATOR, ",");
    bool is_single_assignment = compare_token_attr(t, OPERATOR, "=");
    //check if it is a function call
    if(compare_token_attr(t, SEPARATOR, "(")) {
        debug_print("Call function %s\n", func);
        tree_node_t *func_valid = search_in_tables(&symtab, func);

        // Function is not declared
        if (func_valid == NULL) { //----------------------------------------------------------------------------
            error_semantic("Function name '%s' not defined!", func);
            return SEMANTIC_ERROR_DEFINITION;
        }

        return parse_function_call();
    }
    else if(is_multiple_assignment || is_single_assignment) {
        debug_print("parsing assignment...\n");
        return assignment(id_token);
    }
    else {
        error_unexpected_token("After IDENTIFIER a function call or an assignment", t);
    }
    return SYNTAX_ERROR;
}

int EOF_fun_rule(){
    //will not actually get the token upon which has been decided to use this rule
    //but will get the next token
    //otherwise i would have to use params for each one of these rule functions
    token_t t = lookahead(scanner);
    error_unexpected_token("Reached EOF inside a function. Did you forget 'end'? END",t);
    parser->reached_EOF = true;
    return SYNTAX_ERROR;
}

int EOF_global_rule(){
    //will not actually get the token upon which has been decided to use this rule
    //but will get the next token
    //otherwise i would have to use params for each one of these rule functions
    parser->reached_EOF = true;
    return PARSE_SUCCESS;
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
//         error_unexpected_token("Current implementation of expression has INTEGER, STRING, NUMBER, ID",t);
//     return SYNTAX_ERROR;
// }
bool is_datatype(token_t t) {
    return (compare_token_attr(t, KEYWORD, "string") || 
           compare_token_attr(t, KEYWORD, "number") || 
           compare_token_attr(t, KEYWORD, "integer") || 
           compare_token_attr(t, KEYWORD, "bool"));
}


int parse_datatype(){
    token_t t = get_next_token(scanner);
    if( is_datatype(t)) {
        return PARSE_SUCCESS;
    }

    error_unexpected_token("DATATYPE keyword", t);
    return SYNTAX_ERROR;
}

void error_unexpected_token(char * expected, token_t t) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t|\033[0;31m Syntax error:\033[0m Wrong token! %s expected, but token is: \033[1;33m%s\033[0m type: \033[0;33m%i\033[0m!\n",(scanner->cursor_pos[0]), (scanner->cursor_pos[1]),expected, get_attr(&t, scanner), t.token_type);
}

void error_semantic(const char * _Format, ...) {
    va_list args;
    va_start(args,_Format);
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t|\033[0;31m Semantic error: \033[0m",(scanner->cursor_pos[0]), (scanner->cursor_pos[1]));
    vfprintf(stderr, _Format, args);
    fprintf(stderr,"\n");
}

bool lookahead_token(token_type_t expecting) {
    token_t t = lookahead(scanner);
    return compare_token(t, expecting);
}

bool lookahead_token_attr(token_type_t expecting_type, char * expecting_attr) {
    token_t t = lookahead(scanner);
    return compare_token_attr(t, expecting_type, expecting_attr);
}


bool check_next_token(token_type_t expecting) {
    token_t t = get_next_token(scanner);
    
    if(compare_token(t, expecting))
        return true;

    parser->return_code = (t.token_type == ERROR_TYPE) ? LEXICAL_ERROR : SYNTAX_ERROR;
    char exp_str[3];
    sprintf(exp_str, "%d", expecting);
    error_unexpected_token(exp_str, t);
    return false;
}


bool check_next_token_attr(token_type_t expecting_type, char * expecting_attr) {
    token_t t = get_next_token(scanner);

    if( compare_token_attr(t, expecting_type, expecting_attr))
        return true;

    parser->return_code = (t.token_type == ERROR_TYPE) ? LEXICAL_ERROR : SYNTAX_ERROR;
    error_unexpected_token(expecting_attr, t);
    return false;
}


bool compare_token(token_t t, token_type_t expecting) {
    if(t.token_type == expecting){
        return true;
    }
    return false;
}


bool compare_token_attr(token_t t, token_type_t expecting_type, char * expecting_attr){
    if(t.token_type == expecting_type){
        if(str_cmp(get_attr(&t, scanner), expecting_attr) == 0) {
            return true;
        }
    }

    return false;
}

void debug_print(const char *const _Format, ...) {
    if(DEBUG) {
        //get the arguments
        va_list args;
        va_start(args, _Format);
        //use variable argument printf
        vfprintf(stderr, _Format, args);
    }
}
