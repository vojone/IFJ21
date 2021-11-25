/**
 * @file parser-topdown.c
 * @brief Source file for recursive descent parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */

#include "parser_topdown.h"
#include "dstack.h"

static scanner_t * scanner;
static parser_t * parser;
static symbol_tables_t sym;

#define RULESET_GLOBAL_LENGTH 5

static rule_t ruleset_global[RULESET_GLOBAL_LENGTH] = {
    {parse_require,             {KEYWORD, UNSET, "require"},  true },
    {parse_function_dec,        {KEYWORD, UNSET, "global"},   true },
    {parse_function_def,        {KEYWORD, UNSET, "function"}, true },
    {parse_global_identifier,   {IDENTIFIER, UNSET, NULL},    false},
    {EOF_global_rule,           {EOF_TYPE, UNSET, NULL},      false},
};

#define RULESET_INSIDE_LENGTH 8

static rule_t ruleset_inside[RULESET_INSIDE_LENGTH] = {
    {parse_local_var,   {KEYWORD, UNSET, "local"},  true  },
    {parse_if,          {KEYWORD, UNSET, "if"},     true  },
    {parse_else,        {KEYWORD, UNSET, "else"},   true  },
    {parse_while,       {KEYWORD, UNSET, "while"},  true  },
    {parse_return,      {KEYWORD, UNSET, "return"}, true  },
    {parse_end,         {KEYWORD, UNSET, "end"},    true  },
    {parse_identifier,  {IDENTIFIER, UNSET, NULL},  false },
    {EOF_fun_rule,      {EOF_TYPE, UNSET, NULL},    false },
};

#define DEBUG true


/**
 * @brief Sets symtab to elder symbol table of old outer context 
 */ 
void to_outer_ctx(parser_t *p) {
    if(!symtabs_is_empty(&sym.symtab_st)) {
        destroy_tab(&sym.symtab);
        sym.symtab = symtabs_pop(&sym.symtab_st);
    }
}


/**
 * @brief Creates new symbol table (new context) and sets symtable to it
 */ 
void to_inner_ctx(parser_t *p) {
    symtabs_push(&sym.symtab_st, sym.symtab); //Save copy of old symtab to the stack of symtabs

    symtab_t new_ctx; //Create and init new symtab
    init_tab(&new_ctx);
    new_ctx.parent_ind = symtabs_get_top_ind(&sym.symtab_st); //Save reference to the parent symtab

    sym.symtab = new_ctx;
}

/**-----------------------------------------------------
void str_app_str(string_t* dst, const char *src, size_t length){
    for (size_t i = 0; i < length; i++)
    {
        app_char(src[i], dst);
    }
}

void param_app(string_t* dst, const char *src){
    str_app_str(dst, src, sizeof(src));
    app_char('&',dst);
}
*/

// void semantic_init() {
//     sym_data_t symtab_data;
//     init_data(&symtab_data);

//     string_t params;
//     str_init(&params);

//     app_char('%', &params);
//     symtab_data.params = params;
//     insert_sym(&symtab, "write", symtab_data);
// }

void parser_setup(parser_t *p, scanner_t *s) {
    parser = p;
    parser->reached_EOF = false;
    parser->found_return = false;
    parser->return_code = 0;
    scanner = s;

    symtabs_stack_init(&sym.symtab_st);
    tok_stack_init(&parser->decl_func);

    symtab_t global_tab;
    init_tab(&global_tab);
    sym.global = global_tab;

    //This symtab will not be used (unless global variables are supported), 
    //but it keeps switching context consistent for all cases
    symtab_t symbol_tab;
    init_tab(&symbol_tab);
    sym.symtab = symbol_tab;

    //load_builtin_f(&symtab);
    //semantic_init();
    parser->decl_cnt = 0;
}


/**
 * @brief Frees all resources hold by parser and its components
 */ 
void parser_dtor() {
    destroy_tab(&sym.symtab);
    destroy_tab(&sym.global);

    while(!symtabs_is_empty(&(sym.symtab_st))) {
        symtab_t current = symtabs_pop(&(sym.symtab_st));
        destroy_tab(&current);
    }

    symtabs_stack_dtor(&(sym.symtab_st));
    tok_stack_dtor(&(parser->decl_func));
}


void check_builtin(token_t *id_token) {
    sym_data_t *bfunc_data_ptr = search_builtin(get_attr(id_token, scanner));
    if(bfunc_data_ptr) {
        insert_sym(&sym.global, to_str(&bfunc_data_ptr->name), *bfunc_data_ptr);
    }
}


/**
 * @brief Checks if all declared functions were defined
 */ 
int check_if_defined() {
    while(!tok_is_empty(&parser->decl_func)) {
        token_t func_id = tok_pop(&parser->decl_func);
        tree_node_t * symbol = search(&sym.global, get_attr(&func_id, scanner));

        if(symbol) { //Just for safety
            if(symbol->data.status == DECLARED) {
                error_semantic("Function \033[1;33m%s\033[0m was declared but not defined!");
                return SEMANTIC_ERROR_OTHER;
            }
        }
    }

    return PARSE_SUCCESS;
}


//<program>               -> <global-statement-list>
int parse_program() {
    //scanner_init(scanner); 
    generate_init();   
    
    //run parsing
    int res = global_statement_list();

    debug_print("Finished! return code: %i, at: (%lu,%lu)\n", res, scanner->cursor_pos[0], scanner->cursor_pos[1]);

    res = (res == PARSE_SUCCESS) ? check_if_defined() : res;

    parser_dtor();

    if(parser->return_code != 0) {
        res = parser->return_code;
    }
    
    return res;

}


//<global-statement-list> -> <global-statement> <global-statement-list>
int global_statement_list() {
    int ret = global_statement();
    
    if(ret != PARSE_SUCCESS)
        return ret;

    if(parser->reached_EOF)
        return PARSE_SUCCESS;

    return global_statement_list();
}


//<statement-list>        -> <statement> <statement-list>
int statement_list() {
    int ret = statement();

    if(ret != PARSE_SUCCESS) {
        return ret;
    }

    token_t t = lookahead(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(compare_token(t, KEYWORD)) {
        if(compare_token_attr(t, KEYWORD, "end")) {
            debug_print("got end\n");
            return PARSE_SUCCESS;
        }
        else if(compare_token_attr(t, KEYWORD, "else")) {
            debug_print("got else\n");
            return PARSE_SUCCESS;
        }
    }

    return statement_list();
}


int global_statement() {
    debug_print("parsing next global statement...\n");
    token_t t = lookahead(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    //get the apropriate rule
    rule_t rule_to_use = determine_rule(t, ruleset_global);
    
    //call the right function
    int res = rule_to_use.rule_function();

    //if there is (null) instead the first token of the rule, it means that the rule is using only token type and not attribute 
    debug_print("global statement %s returned code %i\n", rule_to_use.rule_first.attr, res);
    // if(res == SYNTAX_ERROR)
    //     error_unexpected_token("This is global scope so keyword such as require or function definition or call expected",t);
    
    return res;
}


int statement() {
    debug_print("parsing next statement...\n");
    token_t t = lookahead(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    //get the apropriate rule
    rule_t rule_to_use = determine_rule(t, ruleset_inside);
    
    //call the right function
    int res = rule_to_use.rule_function();

    //if there is (null) instead the first token of the rule, it means that the rule is using only token type and not attribute 
    debug_print("statement %s returned code %i\n", rule_to_use.rule_first.attr, res);

    return res;
}



void int2num_conv() {

}


void num2int_conv() {

}


/**
 * @brief Resolves compatibility of data types in assignment
 */ 
bool is_valid_assign(sym_dtype_t var_type, sym_dtype_t r_side_type) {
    if(var_type == r_side_type || r_side_type == NIL) {
        return true;
    }
    else if(var_type == NUM && r_side_type == INT) {
        int2num_conv();
        return true;
    }
    else if(var_type == INT && r_side_type == NUM) {
        num2int_conv();
        return true;
    }
    else {
        return false;
    }
}


/**
 * @brief Parses lside of assignment (supports also multiple assignment)
 * @param start_id Pointer to token with first identifier
 * @param id_types Pointer to initalized string where will be written data types of all identifiers
 * @param id_number Pointer to integer where will be written amount of identifiers on left side of assignment
 */ 
int assignment_lside(token_t* start_id, string_t *id_types, size_t *id_number, tok_stack_t *var_names) {
    *id_number = 0;
    token_t t = *start_id;
    bool foundAssignmentOp = false;
    while(!foundAssignmentOp) {
        
        //First token is already read at the beginning, so we check wheter we are at the beginning
        if(*id_number != 0) {
            t = get_next_token(scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
        }

        //The next token should be identifier
        if(t.token_type == IDENTIFIER) {
            (*id_number)++;

            //Try to find identifier in symbol table (or in parent symbol table)
            char * id_str = get_attr(&t, scanner);
            tree_node_t * symbol = search_in_tables(&sym.symtab_st, &sym.symtab, id_str);
    
            if(!symbol || symbol->data.type != VAR) {
                error_semantic("Undeclared variable \033[1;33m%s\033[0m!", id_str);
                return SEMANTIC_ERROR_DEFINITION;
            }
            else {
                //push it to the var name stack for code generation
                tok_push(var_names,t);

                app_char(dtype_to_char(symbol->data.dtype), id_types);
            }

        }
        else {
            error_unexpected_token("IDENTIFIER", t);
            return SYNTAX_ERROR;
        }

        //Comma should follow, or assignment operator
        t = get_next_token(scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }
        else if(compare_token_attr(t, SEPARATOR, ",")) {
            //Ok, next token should be mext identifier
        }
        else if(compare_token_attr(t, OPERATOR, "=")) {
            foundAssignmentOp = true;
        }
        else {
            error_unexpected_token("SEPARATOR ',' or OPERATOR '='", t);
            return SYNTAX_ERROR;
        }

    }

    return PARSE_SUCCESS;
}


/**
 * @brief Parses right side of assignment
 */ 
int assignment_rside(token_t* start_id, string_t *id_types, size_t *id_number) {
    for(size_t i = 0; i < *id_number; i++) {
        //check for valid expression
        debug_print("Calling precedence parser...\n");

        sym_dtype_t ret_type;
        int expr_retval = parse_expression(scanner, &sym.symtab_st, &sym.symtab, &ret_type);
        if(expr_retval == EXPRESSION_SUCCESS) {
            if(!is_valid_assign(char_to_dtype(id_types->str[i]), ret_type)) {
                error_semantic("Type of variable is not compatible with rvalue in assignment!");
                return SEMANTIC_ERROR_DEFINITION;
            }
            else {
                //Assignment is ok
            }
        }
        else {
            debug_print("Error while parsing expression for multiple assignment\n");
            return expr_retval;
        }

        //If we are not at the end check for comma
        if(i + 1 != *id_number) {
            token_t t = get_next_token(scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
            if(compare_token_attr(t, SEPARATOR, ",")) {
                //Ok
            }
            else {
                error_unexpected_token("Missing SEPARATOR ',' in the assignment, which was", t);
                return SYNTAX_ERROR;
            }
        }
    }

    return PARSE_SUCCESS;
}


//<assignment>            -> <id-list> = <expression-list>
/**
 * @note This function expects that the current token is identifier
 */ 
int assignment(token_t t) {
    //Next token should be = or ,
    /*
    token_t current_token;
    token_init(&current_token);
    */    

    token_t var_id = t;

    debug_print("var id: %s\n", get_attr(&var_id, scanner));

    size_t id_number = 0;
    string_t id_types;
    str_init(&id_types);

    //todo push names to the stack in lside
    tok_stack_t var_names;
    tok_stack_init(&var_names);

    int retval = assignment_lside(&var_id, &id_types, &id_number, &var_names);
    debug_print("found %i assignment with types %s ...\n", id_number, to_str(&id_types));
    
    retval = (retval == PARSE_SUCCESS) ? assignment_rside(&var_id, &id_types, &id_number) : retval;        
    
    //todo call generate_params
    generate_multiple_assignment(&sym.symtab_st, &sym.symtab, &var_names,scanner);

    str_dtor(&id_types);
    tok_stack_dtor(&var_names);

    return retval;
}


/**
 * @brief Converts keyword (must be data type) to enum type
 * @note If keyword is not recognized as valid data type specifier returns INT
 */ 
sym_dtype_t keyword_to_dtype(token_t * t, scanner_t *sc) {
    if (str_cmp(get_attr(t, scanner), "string") == 0) {
        return STR;
    }
    else if(str_cmp(get_attr(t, scanner), "integer") == 0) {
        return INT;
    }
    else if(str_cmp(get_attr(t, scanner), "number") == 0) {
        return NUM;
    }
    else if(str_cmp(get_attr(t, scanner), "nil") == 0) {
        return NIL;
    }

    return INT;
}


//<value-assignment>
/**
 * @brief Parses assignment after declaration of variable in function
 * @warning This function expects that the current token was get by lookahead
 * @note If everything went well, changes variable status to defined
 */ 
int local_var_assignment(token_t *current_token, sym_status_t *status, token_t *var_id) {
    if(lookahead_token_attr(OPERATOR, "=")) { 
        get_next_token(scanner);
        //if there is = we check the value assignment
        debug_print("Calling precedence parser...\n");


        sym_dtype_t ret_type;
        int return_val = parse_expression(scanner, &sym.symtab_st, &sym.symtab, &ret_type);
        if(return_val != EXPRESSION_SUCCESS) {
            return return_val;
        }
        else {
            *status = DEFINED; //Ok
        }
        //generate assigment code
        char * unique_name = get_unique_name(&sym.symtab_st, &sym.symtab, var_id, scanner).str;
        generate_assign_value(unique_name);
    }

    return PARSE_SUCCESS;
}


//: [type]
/**
 * @brief Parses data type part variable declaration (e.g ": integer")
 * @return PARSE SUCCESS if everthing was OK
 */ 
int local_var_datatype(token_t *current_token, sym_dtype_t *var_type) {
    //Should be a comma
    bool comma = check_next_token_attr(SEPARATOR, ":");
    if(!comma) {
        return SYNTAX_ERROR;
    }
    
    //Should be a data type
    *current_token = get_next_token(scanner);

    //Determining data type of declared variable
    if(compare_token(*current_token, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(!is_datatype(*current_token)) {
        error_unexpected_token("datatype", *current_token);
        return SYNTAX_ERROR;
    }
    else {
        *var_type = keyword_to_dtype(current_token, scanner);
    }

    return PARSE_SUCCESS;
}


/**
 * @brief Inserts variable into current symbol table
 */ 
void ins_var(token_t *id_token, sym_status_t status, sym_dtype_t dtype) {

    //size_t id_len = strlen(get_attr(id_token, scanner));
    size_t cur_f_len = strlen(get_attr(parser->curr_func_id, scanner));
    string_t var_name;
    str_init(&var_name);

    str_cpy_tostring(&var_name, get_attr(parser->curr_func_id, scanner), cur_f_len); //'name_of_current_function'
    app_char('$', &var_name); //'name_of_current_function'$
    app_str(&var_name, get_attr(id_token, scanner)); //'name_of_current_function'$'name_of_variable_in_ifj21'
    app_char('$', &var_name); //'name_of_current_function'$'name_of_variable_in_ifj21'$

    char conv_buff[DECLARATION_COUNTER_MAX_LEN];
    snprintf(conv_buff, DECLARATION_COUNTER_MAX_LEN, "%ld", parser->decl_cnt);

    app_str(&var_name, conv_buff);
    
    sym_data_t symdata_var = {.name = var_name, .type = VAR, 
                              .dtype = dtype, .status = status};

    debug_print("Putting %s into symbol table... its name is %s and status is %d\n", get_attr(id_token, scanner), to_str(&var_name), status);

    insert_sym(&sym.symtab, get_attr(id_token, scanner), symdata_var);
}


//local  [id] : [type] <value-assignment>
/**
 * @brief Parses declaration of local variable in function
 */ 
int parse_local_var() {
    //go one token forward
    get_next_token(scanner);

    //should be identifier
    token_t t = get_next_token(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    
    token_t var_id = t;
    sym_dtype_t var_type;
    sym_status_t status = DECLARED;


    //debug_print("Var_ID is: %s <---------------\n\n", var_id);
    if(!compare_token(t, IDENTIFIER)) {
        error_unexpected_token("identifier", t);
        return t.token_type == EOF_TYPE ? LEXICAL_ERROR : SYNTAX_ERROR;
    }

    if(search(&sym.symtab, get_attr(&var_id, scanner))) { //Variable is declared in current scope
        error_semantic("Redeclaration of variable \033[1;33m%s\033[0m!", get_attr(&var_id, scanner));
        return SEMANTIC_ERROR_DEFINITION;
    }

    if(search(&sym.global, get_attr(&var_id, scanner))) { //Variable is declared in current scope
        error_semantic("Name of variable \033[1;33m%s\033[0m is colliding with function name!", get_attr(&var_id, scanner));
        return SEMANTIC_ERROR_DEFINITION;
    }


    //There must be data type
    int retval;
    retval = local_var_datatype(&t, &var_type);

    //insert to symtab and generate code
    //? we have to insert the function to symtable before calling local_var_assignment
    ins_var(&var_id, status, var_type);
    parser->decl_cnt++;
    generate_declare_variable(&sym.symtab_st, &sym.symtab, &var_id,scanner);

    //There can be a value assignment
    retval = (retval == PARSE_SUCCESS) ? local_var_assignment(&t, &status, &var_id) : retval;
    
    
    // Adding variable and its datatype into symtable
    // if(retval == PARSE_SUCCESS) {
        
    // }
    
    return retval;
}


//require [string]
/**
 * @brief Parses prolog at the start of the program
 */ 
int parse_require() {
    //go one token forward
    get_next_token(scanner);

    token_t t = get_next_token(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(compare_token(t, STRING)) {
        return PARSE_SUCCESS;
    }
    else {
        error_unexpected_token("STRING", t);
    }

    return SYNTAX_ERROR;
}


/**
 * @brief Inserts function into symbol table or updates existing element in current symbol table with given data
 */ 
void ins_func(token_t *id_token, sym_data_t *data) {

    size_t id_len = strlen(get_attr(id_token, scanner));
    //There is no need to creation original name of function (it always must be original in whole program)
    str_cpy_tostring(&data->name, get_attr(id_token, scanner), id_len);
    debug_print("Putting %s into symbol table... its name is %s and (status: %d, ret_types: %s, params: %s)\n", get_attr(id_token, scanner), to_str(&data->name), data->status, to_str(&data->ret_types), to_str(&data->params));
    insert_sym(&sym.global, get_attr(id_token, scanner), *data);
}


//<type-list>             -> : [type] <type-list-1>
/**
 * @brief Parses function return data types (in declaration of function)
 */ 
int func_dec_returns(string_t *returns) {
    if(lookahead_token_attr(SEPARATOR, ":")) {
        //Will just get the ':'
        debug_print("parsing function return types...\n");
        token_t t = get_next_token(scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }

        bool finished = false;
        while(!finished) {
            //should be datatype
            t = get_next_token(scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
            else if(!is_datatype(t)) {
                error_unexpected_token("data type", t);
                finished = true;
                return SYNTAX_ERROR;
            }
            else {
                char dtype_c = dtype_to_char(keyword_to_dtype(&t, scanner));
                app_char(dtype_c, returns);
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


//(<param-list>)
/**
 * @brief Parses function parameter list in declaration of function
 */ 
int func_dec_params(string_t *params) {
    if(!check_next_token_attr(SEPARATOR, "("))
        return SYNTAX_ERROR;

    if(is_datatype(lookahead(scanner))) {
        debug_print("parsing function param types...\n");

        bool finished = false;
        while(!finished) {
            //should be datatype
            token_t t = get_next_token(scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
            else if(!is_datatype(t)) {
                error_unexpected_token("data type", t);
                finished = true;
                return SYNTAX_ERROR;
            }
            else {
                char type_c = dtype_to_char(keyword_to_dtype(&t, scanner));
                app_char(type_c, params);
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

    if(!check_next_token_attr(SEPARATOR, ")")) {
        return SYNTAX_ERROR;
    }

    return PARSE_SUCCESS;
}


//<global-statement>      -> global [id] : function(<param-list>) <type-list>
/**
 * @brief Parses declaration of function in global scope (in other scopes it is not valid)
 */ 
int parse_function_dec() {
    //This token is just 'global' we skip it
    get_next_token(scanner);

    //this is function ID
    token_t id_fc = get_next_token(scanner);
    if(compare_token(id_fc, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    debug_print("SHOULD BE ID_FC: %s\n\n\n", get_attr(&id_fc, scanner));

    //parsing function definition signature
    bool id = (id_fc.token_type == (IDENTIFIER));
    if(!id){
        error_unexpected_token("FUNCTION IDENTIFIER", id_fc);
    }
    //should be ':'
    bool colon = check_next_token_attr(SEPARATOR, ":");
    //shound be 'function'
    bool function_keyword = check_next_token_attr(KEYWORD, "function");
    if(!id || !colon || !function_keyword) {
        return SYNTAX_ERROR;
    }
    
    //!probably not because we ignore declarations
    //generate code for start,
    //generate_start_function(get_attr(&id_fc, scanner));
    
    tok_push(&parser->decl_func, id_fc); //For checking if function is defined

    if(!compare_token(id_fc, IDENTIFIER)) {
        error_unexpected_token("IDENTIFIER", id_fc);
        return SYNTAX_ERROR;
    }

    if(search(&sym.global, get_attr(&id_fc, scanner))) { //Function is declared in current scope (global scope)
        error_semantic("Redeclaration of function \033[1;33m%s\033[0m!", get_attr(&id_fc, scanner));
        return SEMANTIC_ERROR_DEFINITION;
    }
    
    // if(!check_next_token_attr(SEPARATOR, ":"))
    //     return SYNTAX_ERROR;
    
    // if(!check_next_token_attr(KEYWORD, "function"))
    //     return SYNTAX_ERROR;

    
    sym_data_t f_data; //Preparation for saving param types and return types into strings
    init_data(&f_data);

    f_data.status = DECLARED;

    //Parsing param types if there is param type
    int retval;
    retval = func_dec_params(&f_data.params);

    retval = (retval == PARSE_SUCCESS) ? func_dec_returns(&f_data.ret_types) : retval;

    if(retval == PARSE_SUCCESS) {
        ins_func(&id_fc, &f_data);
    }
    else {
        data_dtor(&f_data);
    }

    return retval;
}


/**
 * @brief Searches for function in current symbol table and initializes symbol data due to result
 * @note If symbol was declared initializes structure passed as argument to values from symbol table
 * @return SEMANTIC_ERROR_DEFINITION if function was declared and defined (and data structure is not initialized), 
 *         otherwise returns PARSE_SUCCESS
 */ 
int check_if_declared(bool *was_decl, token_t *id_tok, sym_data_t *sym_data) {
    char *f_name = get_attr(id_tok, scanner);
    tree_node_t * symbol = search(&sym.global, f_name);
    if(symbol) {
        *was_decl = true;

        if(symbol->data.status == DEFINED) { //Function were defined
            error_semantic("Redefinition of function %s!", f_name);
            return SEMANTIC_ERROR_DEFINITION;
        }
        else { //Function was declared but not defined
            init_data(sym_data);
            cpy_strings(&sym_data->name, &symbol->data.name);
            cpy_strings(&sym_data->params, &symbol->data.params);
            cpy_strings(&sym_data->ret_types, &symbol->data.ret_types);
        }
    }
    else {
        init_data(sym_data);
        str_cpy_tostring(&sym_data->name, f_name, strlen(f_name));
        *was_decl = false;
    }

    return PARSE_SUCCESS;
}


//[id] :
/**
 * @brief Parses prolog of function parameters (parameter name and ':')
 */
int func_def_params_prolog(token_t *param_id) {
    //Should be IDENTIFIER
    *param_id = get_next_token(scanner);
    if(compare_token(*param_id, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(param_id->token_type != IDENTIFIER) {
        error_unexpected_token("IDENTIFIER", *param_id);
        return SYNTAX_ERROR;
    }
    
    //Should be COLON
    token_t t = get_next_token(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(!compare_token_attr(t, SEPARATOR, ":")) {
        error_unexpected_token("':'", t);
        return SYNTAX_ERROR;
    }

    return PARSE_SUCCESS;
}


//<param-list>            -> [id] : [type] <param-list-1>
/**
 * @brief Parses parameters in function definition (and makes semantics checks)
 */ 
//TODO stack for param name saving
int func_def_params(token_t *id_token, bool was_decl, sym_data_t *f_data) {
    size_t params_cnt = 0;
    //Check if there are parameters
    token_t t = lookahead(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    tok_stack_t param_names;
    tok_stack_init(&param_names);

    if(t.token_type == IDENTIFIER) {
        debug_print("parsing function parameters...\n");

        bool finished = false;
        while(!finished) {
            token_t param_id;
            int prolog_ret = func_def_params_prolog(&param_id);
            if(prolog_ret != PARSE_SUCCESS) {
                tok_stack_dtor(&param_names);
                return prolog_ret;
            }

            //push it to param_names for code generation
            tok_push(&param_names,param_id);

            //Should be DATATYPE
            t = get_next_token(scanner);
            if(compare_token(t, ERROR_TYPE)) {
                tok_stack_dtor(&param_names);
                return LEXICAL_ERROR;
            }
            else if(!is_datatype(t)) {
                error_unexpected_token("DATA TYPE", t);
                tok_stack_dtor(&param_names);
                return SYNTAX_ERROR;
            }
            else {
                sym_dtype_t dtype;
                if(was_decl) {
                    char * params_s = to_str(&f_data->params);
                    dtype = char_to_dtype(params_s[params_cnt]);

                    if(params_s[params_cnt] == '\0') { //There is bigger amount of parameteres than should be
                        error_semantic("Parameter AMOUNT mismatch in definition of \033[1;33m%s\033[0m (there are to many of them)!", get_attr(id_token, scanner));
                        tok_stack_dtor(&param_names);
                        return SEMANTIC_ERROR_OTHER;
                    }
                    else if(dtype != keyword_to_dtype(&t, scanner)) {
                        error_semantic("Parameter DATA TYPE mismatch in definition of \033[1;33m%s\033[0m!", get_attr(id_token, scanner));
                        tok_stack_dtor(&param_names);
                        return SEMANTIC_ERROR_OTHER;
                    }
                }
                else {
                    dtype = keyword_to_dtype(&t, scanner);
                    char dtype_c = dtype_to_char(keyword_to_dtype(&t, scanner));
                    app_char(dtype_c, &f_data->params);
                }


                ins_var(&param_id, DECLARED, dtype);
                parser->decl_cnt++;
                
                //OK
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(SEPARATOR, ",");
            if(!comma) {
                finished = true;
                if(params_cnt != len(&f_data->params) - 1) {
                    error_semantic("Parameter AMOUNT mismatch in definition of \033[1;33m%s\033[0m (missing parameters)!", get_attr(id_token, scanner));
                    tok_stack_dtor(&param_names);
                    return SEMANTIC_ERROR_OTHER;
                }

            }
            else {
                //we go one token forward
                get_next_token(scanner);
            }

            params_cnt++;
        }
    }
    else if(was_decl && len(&f_data->ret_types) > 0) {
        error_semantic("Return values AMOUNT mismatch in definition of function \033[1;33m%s\033[0m (missing parameters)!", get_attr(id_token, scanner));
        tok_stack_dtor(&param_names);
        return SEMANTIC_ERROR_OTHER;
    }

    //generate code for parameters
    generate_parameters(&(sym.symtab_st), &sym.symtab, &param_names, scanner);

    tok_stack_dtor(&param_names);
    return PARSE_SUCCESS;
}


/**
 * @brief Checks validity of function signature
 * @param id if true, function identifier was found
 * @param left_bracket if true, left bracket was found
 */ 
int check_function_signature(bool id, bool left_bracket) {
    bool right_bracket = check_next_token_attr(SEPARATOR, ")");
    if(!(id && left_bracket && right_bracket)) {
        debug_print("ERROR INVALID FUNCTION SINATURE!\n");

        return SYNTAX_ERROR;
    }

    return PARSE_SUCCESS;
}


//<type-list>             -> : [type] <type-list-1>
/**
 * @brief Parses function return values when function is defined
 */ 
int func_def_returns(token_t *id_token, bool was_decl, sym_data_t *f_data) {
    size_t ret_cnt = 0;
    //Parsing types if there is colon
    
    if(lookahead_token_attr(SEPARATOR, ":")) {
        //Will just get the ':'
        debug_print("parsing function types...\n");
        token_t t = get_next_token(scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }

        bool finished = false;
        while(!finished) {
            //Should be datatype
            t = get_next_token(scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
            else if(!is_datatype(t)) {
                error_unexpected_token("data type", t);
                finished = true;

                return SYNTAX_ERROR;
            }
            else {
                if(was_decl) {
                    char * returns_str = to_str(&f_data->ret_types);
                    sym_dtype_t dtype = char_to_dtype(returns_str[ret_cnt]);
                    if(returns_str[ret_cnt] == '\0') {
                        error_semantic("Return values AMOUNT mismatch in definition of \033[1;33m%s\033[0m (there are too many of them)!", get_attr(id_token, scanner));
                        return SEMANTIC_ERROR_OTHER;
                    }
                    else if(dtype != keyword_to_dtype(&t, scanner)) {
                        error_semantic("Return value DATA TYPE mismatch in definition of \033[1;33m%s\033[0m!", get_attr(id_token, scanner));
                        return SEMANTIC_ERROR_OTHER;
                    }
                }
                else {
                    char dtype_c = dtype_to_char(keyword_to_dtype(&t, scanner));
                    app_char(dtype_c, &f_data->ret_types);
                }

                //OK
            }

            //If there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(SEPARATOR, ",");
            if(!comma) {
                finished = true;
                if(ret_cnt != len(&f_data->ret_types) - 1) {
                    error_semantic("Return values amount mismatch in definition of \033[1;33m%s\033[0m (something is missing)!", get_attr(id_token, scanner));
                    return SEMANTIC_ERROR_OTHER;
                }
            }
            else {
                //We go one token forward
                get_next_token(scanner);
            }

            ret_cnt++;
        }
    }
    else if(!lookahead_token_attr(SEPARATOR, ":") && was_decl) {
        if(len(&f_data->ret_types) > 0) {
            error_semantic("Return values AMOUNT mismatch in definition of function \033[1;33m%s\033[0m(something is missing)!", get_attr(id_token, scanner));
            return SEMANTIC_ERROR_OTHER;
        }
    }

    return PARSE_SUCCESS;
}

//end
/**
 * @brief Parses function tail (end keyword)
 */ 
int func_def_epilogue() {
    token_t t = get_next_token(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    //generate function end
    generate_end_function(get_attr(parser->curr_func_id, scanner));

    debug_print("parsing function finished! at: (%lu,%lu)\n", scanner->cursor_pos[0], scanner->cursor_pos[1]); 

    if(t.token_type == KEYWORD) {
        if(compare_token_attr(t, KEYWORD, "end")) {
            debug_print("Successfully ended function definition!\n");
            return PARSE_SUCCESS;
        }
        else {
            error_unexpected_token("Wrong keyword! Function must end only with 'end'", t);
            return SYNTAX_ERROR;
        }

        // error_unexpected_token("[]", t);
        // return (retval != PARSE_SUCCESS) ? retval : SYNTAX_ERROR;
    }
    else {
        error_unexpected_token("Better check the implementation, function must end with keyword 'end'!", t);
        return SYNTAX_ERROR;
    }
}


/**
 * @brief Checks if function has return statement inside (if function returns something)
 */ 
int check_return(token_t *id_fc) {
    tree_node_t *symbol = search(&sym.global, get_attr(id_fc, scanner));
    if(symbol && len(&symbol->data.ret_types) > 0 && !parser->found_return) {
        error_semantic("Function \033[1;33m%s\033[0m can reach the end without return!", get_attr(id_fc, scanner));
        return SEMANTIC_ERROR_OTHER;
    }

    return PARSE_SUCCESS;
}


/**
 * @brief Parses function definition 
 */ 
int parse_function_def() {
    //this token is just 'function' so we skip it
    get_next_token(scanner);

    token_t id_fc = get_next_token(scanner);
    if(compare_token(id_fc, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    check_builtin(&id_fc); //Adds builtin function into symtable so semantic checks can detects its redefinition

    //generate function start
    generate_start_function(get_attr(&id_fc, scanner));

    parser->curr_func_id = &id_fc;
    parser->found_return = false;

    //parsing function definition signature
    bool id = (id_fc.token_type == (IDENTIFIER));
    bool left_bracket = check_next_token_attr(SEPARATOR, "(");

    sym_data_t func_d;
    bool was_declared = false;
    int retval = check_if_declared(&was_declared, &id_fc, &func_d);
    func_d.status = DEFINED;

    to_inner_ctx(parser); //Switch context (params must be in this context)
    
    retval = (retval == PARSE_SUCCESS) ? func_def_params(&id_fc, was_declared, &func_d) : retval;

    retval = (retval == PARSE_SUCCESS) ? check_function_signature(id, left_bracket) : retval;

    retval = (retval == PARSE_SUCCESS) ? func_def_returns(&id_fc, was_declared, &func_d) : retval;
    
    if(retval != PARSE_SUCCESS) { //Stop if an error ocurred
        if(!was_declared) {
            data_dtor(&func_d);
        }

        return retval;
    }
    
    ins_func(&id_fc, &func_d);

    //Parsing inside function
    debug_print("parsing inside function...\n");
    retval = statement_list();
    debug_print("parsing function finished! return code: %i, at: (%lu,%lu)\n", retval, scanner->cursor_pos[0], scanner->cursor_pos[1]);

    to_outer_ctx(parser); //Go back to higher context level

    retval = (retval == PARSE_SUCCESS) ? func_def_epilogue() : retval;

    retval = (retval == PARSE_SUCCESS) ? check_return(&id_fc) : retval;

    parser->curr_func_id = NULL;

    return retval;
}


//<statement>             -> [function-id](<argument-list>)
int parse_function_call(token_t *id_func) {
    debug_print("parsing function call...\n");
    bool opening_bracket = check_next_token_attr(SEPARATOR, "(");
    
    if(opening_bracket) {
        int retval = parse_function_arguments(id_func);
        debug_print("function args success %i\n", (int)retval);

        return retval;
    }
    else {
        return SYNTAX_ERROR;
    }
}


/**
 * @brief Parses function arguments when function is called
 * @note Functon argument can be also expression as well as variable or immediate value
 */ 
int parse_function_arguments(token_t *id_func) {
    bool closing_bracket = false;

    tree_node_t *symbol = search(&sym.global, get_attr(id_func, scanner));
    char * params_str = to_str(&symbol->data.params);

    size_t argument_cnt = 0;
    bool is_variadic = (params_str[0] == '%') ? true : false; //In our case variadic means - with variable AMOUNT and TYPES of arguments
    while(!closing_bracket) {
        token_t t = lookahead(scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }
        else if(is_expression(t)) {
            sym_dtype_t ret_type;
            int expr_retval = parse_expression(scanner, &sym.symtab_st, &sym.symtab, &ret_type);
            if(expr_retval != EXPRESSION_SUCCESS) {
                return expr_retval;
            }

            if(!is_variadic) {
                sym_dtype_t d_type = char_to_dtype(params_str[argument_cnt]);
                if(!is_valid_assign(d_type, ret_type)) {
                    error_semantic("Bad function call of \033[1;33m%s\033[0m! Bad data types of arguments!", get_attr(id_func, scanner));
                    return SEMANTIC_ERROR_PARAMETERS;
                }
                else {
                    //Ok
                }
            }
        }
        else if(compare_token_attr(t, SEPARATOR, ")")) {
            closing_bracket = true;
            continue;
        }
        else {
            error_unexpected_token("EXPRESSION", t);
        }

        //Check if there will be next argument
        t = get_next_token(scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }
        else if(compare_token_attr(t, SEPARATOR, ",")) {
            if(argument_cnt + 1 > strlen(params_str) - 1 && !is_variadic) { //Function needs less arguments
                error_semantic("Bad function call of \033[1;33m%s\033[0m! Too many arguments!", get_attr(id_func, scanner));
                return SEMANTIC_ERROR_PARAMETERS;
            }
            else {
                //Ok
            }
        }
        else if(compare_token_attr(t, SEPARATOR, ")")) {
            closing_bracket = true;
            if(argument_cnt != strlen(params_str) - 1 && !is_variadic) { //Function needs more arguments
                error_semantic("Bad function call of \033[1;33m%s\033[0m! Missing arguments!", get_attr(id_func, scanner));
                return SEMANTIC_ERROR_PARAMETERS;
            }
        }
        else {
            error_unexpected_token("',' as a separator between args or ')' as an end of argument list", t);
            return SYNTAX_ERROR;
        }

        argument_cnt++;
    }
    if(argument_cnt == 0 && strlen(params_str) > 0) { //Function needs arguments but there aren't any
        error_semantic("Bad function call of \033[1;33m%s\033[0m! Missing arguments!", get_attr(id_func, scanner));
        return SEMANTIC_ERROR_PARAMETERS;
    }

    return closing_bracket ? PARSE_SUCCESS : SYNTAX_ERROR;
}


//<statement>             -> if <expression> then <statement-list> <else-branch> end
/**
 * @brief Parses if statement
 */ 
int parse_if() {
    //Go one token forward
    get_next_token(scanner);

    debug_print("Calling precedence parser...\n");

    sym_dtype_t ret_type;
    int expr_retval = parse_expression(scanner, &sym.symtab_st, &sym.symtab, &ret_type);
    if(expr_retval != EXPRESSION_SUCCESS) {
        return expr_retval;
    }

    bool then = check_next_token_attr(KEYWORD, "then");
    if(!then) {
        return SYNTAX_ERROR;
    }

    to_inner_ctx(parser); //Switch the context

    int retval = statement_list();
    if(retval != PARSE_SUCCESS) {
        return retval;
    }

    bool ret_in_if_branch = parser->found_return;
    parser->found_return = false;
    
    to_outer_ctx(parser); //Back to higher leve context

    token_t t = get_next_token(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(compare_token(t, KEYWORD)) {
        if(compare_token_attr(t, KEYWORD, "end")) {
            debug_print("Ended if\n");

            return PARSE_SUCCESS;
        }
        else if(compare_token_attr(t, KEYWORD, "else")) {

            to_inner_ctx(parser); //Switch the context

            //<else-branch>           -> else <statement-list>
            debug_print("parsing else branch...\n");

            retval = statement_list();
            if(retval != PARSE_SUCCESS) {
                return retval;
            }

            bool ret_in_else_branch = parser->found_return;
            parser->found_return = ret_in_if_branch && ret_in_else_branch; //Function must ends in both branches

            to_outer_ctx(parser);

            t = get_next_token(scanner);
    
            return PARSE_SUCCESS; //Back to higher level context
        }

        error_unexpected_token("[2]", t);
        return SYNTAX_ERROR;
    }
    else {
        error_unexpected_token("Better check the implementation! END", t);
        return SYNTAX_ERROR;
    }
}


int parse_else() {
    return PARSE_SUCCESS;
}


//return <expression-list>
/**
 * @brief Parses return with following expression(s)
 */ 
int parse_return() {
    //go one token forward
    get_next_token(scanner);
    token_t* id_fc = parser->curr_func_id;

    debug_print("Calling precedence parser to parse return in %s...\n", get_attr(id_fc, scanner));

    tree_node_t *symbol = search(&sym.global, get_attr(id_fc, scanner));
    
    size_t returns_cnt = 0;
    bool finished = false;
    char * returns_str = to_str(&symbol->data.ret_types);
    while(!finished) {
        token_t t = lookahead(scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }
        else if(!is_expression(t)) {
            finished = true;

            if(len(&symbol->data.ret_types) > 0) {
                error_semantic("Missing return values after return in function \033[1;33m%s\033[0m!", get_attr(id_fc, scanner));
                return SEMANTIC_ERROR_ASSIGNMENT;
            }
        }
        else {
            sym_dtype_t ret_type;
            int retval = parse_expression(scanner, &sym.symtab_st, &sym.symtab, &ret_type);
            if(retval != EXPRESSION_SUCCESS) {
                return retval;
            }
            else {
                sym_dtype_t dec_type = char_to_dtype(returns_str[returns_cnt]);
                if(!is_valid_assign(dec_type, ret_type)) {
                    error_semantic("Bad data type of return in function \033[1;33m%s\033[0m!", get_attr(id_fc, scanner));
                    return SEMANTIC_ERROR_OTHER;
                }
                else {
                    //Ok
                }
            }
        }

        //If there is no comma we should be at the end of the list
        bool comma = lookahead_token_attr(SEPARATOR, ",");
        if(!comma) {
            finished = true;
            if(len(&symbol->data.ret_types) - 1 > returns_cnt) {
                error_semantic("Function \033[1;33m%s\033[0m returns %d values but only %d were found!", get_attr(id_fc, scanner), len(&symbol->data.ret_types), returns_cnt + 1);
                return SEMANTIC_ERROR_ASSIGNMENT;
            }
        }
        else {
            //we go one token forward
            get_next_token(scanner);
            if(len(&symbol->data.ret_types) - 1 < returns_cnt + 1) {
                error_semantic("Function \033[1;33m%s\033[0m returns %d values but more return values were found!", get_attr(id_fc, scanner), returns_cnt + 1, len(&symbol->data.ret_types));
                return SEMANTIC_ERROR_ASSIGNMENT;
            }
        }

        returns_cnt++;
    }

    parser->found_return = true;
    
    return PARSE_SUCCESS;
}


int parse_end() {
    return PARSE_SUCCESS;
}


/**
 * @brief Parses while statement
 */ 
int parse_while() {
    //go one token forward
    get_next_token(scanner);
    
    debug_print("Calling precedence parser...\n");
    sym_dtype_t ret_type;
    int expr_retval = parse_expression(scanner, &sym.symtab_st, &sym.symtab, &ret_type);
    if(expr_retval != EXPRESSION_SUCCESS) {
        return expr_retval;
    }

    bool then = check_next_token_attr( KEYWORD, "do");
    if(!then) {
        return false;
    }

    to_inner_ctx(parser); //Switch context

    int retval = statement_list();
    if(retval != PARSE_SUCCESS) {
        return retval;
    }

    to_outer_ctx(parser); //Switch context to higher level

    token_t t = get_next_token(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(compare_token(t, KEYWORD)) {
        if(compare_token_attr(t, KEYWORD, "end")) {

            debug_print("Ended while\n");
            return PARSE_SUCCESS;
        }
        
        error_unexpected_token("[]", t);
        return SYNTAX_ERROR;
    }
    else {
        error_unexpected_token("Better check the implementation! END", t);
        return SYNTAX_ERROR;
    }

    return SYNTAX_ERROR;
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
        if(ruleset[i].attrib_relevant) {
            //the atribute is relevant (rules with same token types)
            /**< Here can be .attr because the value of token is certainly referenced by pointer */
            if(compare_token_attr(t, to_be_checked.token_type, to_be_checked.attr)) {
                return ruleset[i];
            }
        }
        else {
            //the atribute is irrelevant we check only for matching token type
            if(compare_token(t, to_be_checked.token_type)) {
                return ruleset[i];
            }
        }
    }

    error_unexpected_token("NO RULE can be used to parse this token! Other", t);
    return (rule_t){error_rule, {-1, UNSET, NULL}};
}


int parse_global_identifier() {
    token_t id_token = get_next_token(scanner);
    if(compare_token(id_token, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    debug_print("got identifier\n");

    check_builtin(&id_token); //Puts builtin fction into symtable if (identifier is symtable)

    char *func = get_attr(&id_token, scanner);
    tree_node_t *func_valid = search(&sym.global, func);
    if (func_valid == NULL) { // Function is not declared
        error_semantic("Function with name '\033[1;33m%s\033[0m' not defined!", func);
        return SEMANTIC_ERROR_DEFINITION;
    }
    //this should leave the arguments at the top of the stack
    int ret = parse_function_call(&id_token);
    //generate function call
    generate_call_function(get_attr(&id_token, scanner));
    return ret;
}


int parse_identifier() {
    //Should be identifier
    token_t id_token = get_next_token(scanner);
    if(compare_token(id_token, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    debug_print("got identifier\n");
    
    // It is neccessary to look at the next lexeme also
    // token_t id_token = get_next_token(scanner);
    token_t t = lookahead(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    bool is_multiple_assignment = compare_token_attr(t, SEPARATOR, ",");
    bool is_single_assignment = compare_token_attr(t, OPERATOR, "=");

    //Check if it is a function call
    if(compare_token_attr(t, SEPARATOR, "(")) {

        check_builtin(&id_token); //Adds builtin functions into symtable

        char *func = get_attr(&id_token, scanner);
        tree_node_t *func_valid = search(&sym.global, func);
        if (func_valid == NULL) { // Function is not declared
            error_semantic("Function with name '\033[1;33m%s\033[0m' not defined!", func);
            return SEMANTIC_ERROR_DEFINITION;
        }

        debug_print("Call function %s\n", func);
        int res = parse_function_call(&id_token);
        generate_call_function(get_attr(&id_token,scanner));
        return res;
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


int EOF_fun_rule() {
    //will not actually get the token upon which has been decided to use this rule
    //but will get the next token
    //otherwise i would have to use params for each one of these rule functions
    token_t t = lookahead(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    error_unexpected_token("Reached EOF inside a function. Did you forget 'end'? END",t);
    parser->reached_EOF = true;
    return SYNTAX_ERROR;
}


int EOF_global_rule() {
    //will not actually get the token upon which has been decided to use this rule
    //but will get the next token
    //otherwise i would have to use params for each one of these rule functions
    parser->reached_EOF = true;
    return PARSE_SUCCESS;
}

int error_rule() {
    return SYNTAX_ERROR;
}

/**
 * //TODO
 * @brief Recognizes start of expression
 */
bool is_expression(token_t t) {
    return (t.token_type == NUMBER || 
            t.token_type == INTEGER || 
            t.token_type == STRING || 
            t.token_type == IDENTIFIER ||
            (t.token_type == KEYWORD && str_cmp(get_attr(&t, scanner), "nil") == 0));
}

/**
 * @brief Recognizes data type tokens
 */ 
bool is_datatype(token_t t) {
    return (compare_token_attr(t, KEYWORD, "string") || 
           compare_token_attr(t, KEYWORD, "number") || 
           compare_token_attr(t, KEYWORD, "integer") || 
           compare_token_attr(t, KEYWORD, "nil"));
}


int parse_datatype(){ 
    token_t t = get_next_token(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if( is_datatype(t)) {
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
    if(compare_token(t, ERROR_TYPE)) {
        parser->return_code = LEXICAL_ERROR;
        return false;
    }

    return compare_token(t, expecting);
}


bool lookahead_token_attr(token_type_t expecting_type, char * expecting_attr) {
    token_t t = lookahead(scanner);
    if(compare_token(t, ERROR_TYPE)) {
        parser->return_code = LEXICAL_ERROR;
        return false;
    }

    return compare_token_attr(t, expecting_type, expecting_attr);
}


bool check_next_token(token_type_t expecting) {
    token_t t = get_next_token(scanner);
    
    if(compare_token(t, expecting)) {
        return true;
    }

    parser->return_code = (t.token_type == ERROR_TYPE) ? LEXICAL_ERROR : SYNTAX_ERROR;
    error_unexpected_token(tok_type_to_str(t.token_type), t);
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


bool compare_token_attr(token_t t, token_type_t expecting_type, char * expecting_attr) {
    if(t.token_type == expecting_type){
        if(str_cmp(get_attr(&t, scanner), expecting_attr) == 0) {
            return true;
        }
    }

    return false;
}


/**
 * @brief Prints messages to stderr. To (de)activate debug prints (un)define macro DEBUG 
 */ 
void debug_print(const char *const _Format, ...) {
    if(DEBUG) {
        //get the arguments
        va_list args;
        va_start(args, _Format);
        //use variable argument printf
        vfprintf(stderr, _Format, args);
    }
}
