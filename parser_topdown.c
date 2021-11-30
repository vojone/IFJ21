/**
 * @file parser-topdown.c
 * @brief Source file for recursive descent parser
 * 
 * @authors Radek Marek (xmarek77), Vojtěch Dvořák (xdvora3o), 
 *          Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 */

#include "parser_topdown.h"


#define DEBUG false

#define PRINT_WARNINGS true



#define RULESET_GLOBAL_LENGTH 5 /**< Number of rules that can be used for parsing rules in global scopes */

rule_t * get_global_rule(size_t index) {
    if(index >= RULESET_GLOBAL_LENGTH) {
        return NULL;
    }

    static rule_t ruleset_global[RULESET_GLOBAL_LENGTH] = {
        {parse_require,             {KEYWORD, UNSET, "require"},  true },
        {parse_function_dec,        {KEYWORD, UNSET, "global"},   true },
        {parse_function_def,        {KEYWORD, UNSET, "function"}, true },
        {parse_global_identifier,   {IDENTIFIER, UNSET, NULL},    false},
        {EOF_global_rule,           {EOF_TYPE, UNSET, NULL},      false},
    };

    return &ruleset_global[index];
}



#define RULESET_INSIDE_LENGTH 8 /**< Number of rules that can be used for parsing rules in local scopes */

rule_t * get_inside_rule(size_t index) {
    if(index >= RULESET_INSIDE_LENGTH) {
        return NULL;
    }

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

    return &ruleset_inside[index];
}


void to_outer_ctx(parser_t *parser) {
    if(!symtabs_is_empty(&parser->sym.symtab_st)) {
        destroy_tab(&parser->sym.symtab);
        parser->sym.symtab = symtabs_pop(&parser->sym.symtab_st);
    }
}


void to_inner_ctx(parser_t *parser) {
    symtabs_push(&parser->sym.symtab_st, parser->sym.symtab); //Save copy of old symtab to the stack of symtabs

    symtab_t new_ctx; //Create and init new symtab
    init_tab(&new_ctx);
    new_ctx.parent_ind = symtabs_get_top_ind(&parser->sym.symtab_st); //Save reference to the parent symtab

    parser->sym.symtab = new_ctx;
}


void parser_setup(parser_t *parser, scanner_t *scanner) {
    parser->scanner = scanner;

    symtabs_stack_init(&parser->sym.symtab_st);
    tok_stack_init(&parser->decl_func);

    symtab_t global_tab;
    init_tab(&global_tab);
    parser->sym.global = global_tab; //Initialization of global symbol table

    symtab_t symbol_tab;
    init_tab(&symbol_tab);
    parser->sym.symtab = symbol_tab; 

    parser->decl_cnt = 0;
    parser->cond_cnt = 0;
    parser->loop_cnt = 0;

    parser->reached_EOF = false;
    parser->found_return = false;

    parser->return_code = 0;
}


void parser_dtor(parser_t *parser) {
    destroy_tab(&parser->sym.symtab);
    destroy_tab(&parser->sym.global);

    while(!symtabs_is_empty(&(parser->sym.symtab_st))) { //Free all symbol tables inside stack
        symtab_t current = symtabs_pop(&(parser->sym.symtab_st));
        destroy_tab(&current);
    }

    symtabs_stack_dtor(&(parser->sym.symtab_st));
    tok_stack_dtor(&(parser->decl_func));
}


int check_if_defined(parser_t *parser) {
    while(!tok_is_empty(&parser->decl_func)) {
        token_t func_id = tok_pop(&parser->decl_func);
        char *func_name = get_attr(&func_id, parser->scanner);
        tree_node_t * symbol = search(&parser->sym.global, func_name);

        if(symbol) { //Just for safety
            if(symbol->data.status == DECLARED) {
                error_semantic(parser, "Function \033[1;33m%s\033[0m was declared but not defined!");
                return SEMANTIC_ERROR_OTHER;
            }
        }
    }

    return PARSE_SUCCESS;
}


//<program>               -> <global-statement-list>
int parse_program(parser_t *parser) {
    //scanner_init(scanner); 
    generate_init();   
    
    //run parsing
    int res = global_statement_list(parser);

    debug_print("Finished! return code: %i, at: (%lu,%lu)\n", res, parser->scanner->cursor_pos[ROW], parser->scanner->cursor_pos[COL]);

    res = (res == PARSE_SUCCESS) ? check_if_defined(parser) : res;

    parser_dtor(parser);

    if(parser->return_code != 0) {
        res = parser->return_code;
    }
    
    return res;

}


//<global-statement-list> -> <global-statement> <global-statement-list>
int global_statement_list(parser_t *parser) {
    int ret = global_statement(parser);
    
    if(ret != PARSE_SUCCESS)
        return ret;

    if(parser->reached_EOF)
        return PARSE_SUCCESS;

    return global_statement_list(parser);
}


//<statement-list>        -> <statement> <statement-list>
int statement_list(parser_t *parser) {
    int ret = statement(parser);

    if(ret != PARSE_SUCCESS) {
        return ret;
    }

    token_t t = lookahead(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(compare_token(t, KEYWORD)) {
        if(compare_token_attr(parser, t, KEYWORD, "end")) {
            debug_print("got end\n");
            return PARSE_SUCCESS;
        }
        else if(compare_token_attr(parser, t, KEYWORD, "else")) {
            debug_print("got else\n");
            return PARSE_SUCCESS;
        }
    }

    return statement_list(parser);
}


int global_statement(parser_t *parser) {
    debug_print("parsing next global statement...\n");
    token_t t = lookahead(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    //get the apropriate rule
    rule_t* rule_to_use = determine_rule(parser, t, get_global_rule);
    if(rule_to_use == NULL) {
        return SYNTAX_ERROR;
    }
    
    //call the right function
    int res = rule_to_use->rule_function(parser);

    //if there is (null) instead the first token of the rule, it means that the rule is using only token type and not attribute 
    debug_print("global statement %s returned code %i\n", rule_to_use->rule_first.attr, res);
    // if(res == SYNTAX_ERROR)
    //     error_unexpected_token("This is global scope so keyword such as require or function definition or call expected",t);
    
    return res;
}


int statement(parser_t *parser) {
    debug_print("parsing next statement...\n");
    token_t t = lookahead(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    //get the apropriate rule
    rule_t* rule_to_use = determine_rule(parser, t, get_inside_rule);
    if(rule_to_use == NULL) {
        return SYNTAX_ERROR;
    }
    
    //call the right function
    int res = rule_to_use->rule_function(parser);

    //if there is (null) instead the first token of the rule, it means that the rule is using only token type and not attribute 
    debug_print("statement %s returned code %i\n", rule_to_use->rule_first.attr, res);

    return res;
}


void int2num_conv() {
    code_print("INT2FLOATS");
}


void num2int_conv() {
    code_print("FLOAT2INTS");
}


bool is_valid_assign(sym_dtype_t var_type, sym_dtype_t r_side_type) {
    if(var_type == r_side_type || r_side_type == NIL) {
        return true;
    }
    else if(var_type == NUM && r_side_type == INT) {
        int2num_conv();
        return true;
    }
    else {
        return false;
    }
}


int assignment_lside(parser_t *parser, token_t* start_id, 
                     string_t *id_types, size_t *id_number, 
                     tok_stack_t *var_names) {
    *id_number = 0;
    token_t t = *start_id;
    bool foundAssignmentOp = false;
    while(!foundAssignmentOp) {
        
        //First token is already read at the beginning, so we check wheter we are at the beginning
        if(*id_number != 0) {
            t = get_next_token(parser->scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
        }

        //The next token should be identifier
        if(t.token_type == IDENTIFIER) {
            (*id_number)++;

            //Try to find identifier in symbol table (or in parent symbol table)
            char * id_str = get_attr(&t, parser->scanner);
            tree_node_t * symbol = search_in_tables(&parser->sym.symtab_st, &parser->sym.symtab, id_str);
    
            if(!symbol || symbol->data.type != VAR) {
                error_semantic(parser, "Undeclared variable \033[1;33m%s\033[0m!", id_str);
                return SEMANTIC_ERROR_DEFINITION;
            }
            else {
                //push it to the var name stack for code generation
                tok_push(var_names,t);

                app_char(dtype_to_char(symbol->data.dtype), id_types);
            }

        }
        else {
            error_unexpected_token(parser, "IDENTIFIER", t);
            return SYNTAX_ERROR;
        }

        //Comma should follow, or assignment operator
        t = get_next_token(parser->scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }
        else if(compare_token_attr(parser, t, SEPARATOR, ",")) {
            //Ok, next token should be mext identifier
        }
        else if(compare_token_attr(parser, t, OPERATOR, "=")) {
            foundAssignmentOp = true;
        }
        else {
            error_unexpected_token(parser, "SEPARATOR ',' or OPERATOR '='", t);
            return SYNTAX_ERROR;
        }

    }

    return PARSE_SUCCESS;
}


int assignment_rside(parser_t *parser, token_t* start_id, 
                     string_t *id_types, size_t *id_number) {
    size_t i = 0;
    bool was_f_called = false;

    string_t ret_types;
    str_init(&ret_types);
    for(; i < *id_number; i++) {
        token_t t = lookahead(parser->scanner);
        if(!is_expression(parser, t)) {
            str_dtor(&ret_types);
            error_semantic(parser, "Missing rvalue in assignment (expected expression)!");
            return SEMANTIC_ERROR_ASSIGNMENT;
        }

        //check for valid expression
        debug_print("Calling precedence parser...\n");
    
        int expr_retval = parse_expression(parser->scanner, &parser->sym, &ret_types, &was_f_called);
        if(expr_retval == EXPRESSION_SUCCESS) {
            size_t u = 0;
            for(; to_str(&ret_types)[u] != '\0'; u++) {
                sym_dtype_t cur_dtype = char_to_dtype(to_str(&ret_types)[u]);

                if(!is_valid_assign(char_to_dtype(id_types->str[i + u]), cur_dtype)) {
                    str_dtor(&ret_types);
                    if(!was_f_called) {
                        error_semantic(parser, "Type of variable is not compatible with rvalue of assignment!");
                        return SEMANTIC_ERROR_ASSIGNMENT;
                    }
                    else {
                        error_semantic(parser, "Return type of function in assignment is not compatible with variable!");
                        return SEMANTIC_ERROR_PARAMETERS;
                    }
                }
                else {
                    //Assignment is ok
                }
            }

            i += (u > 0) ? (u - 1) : u; //If there was one return value, don't icrement, because i increments
        }
        else {
            debug_print("Error while parsing expression for multiple assignment\n");
            str_dtor(&ret_types);
            return expr_retval;
        }

        //If we are not at the end check for comma
        if(i + 1 != *id_number) {
            token_t t = lookahead(parser->scanner);
            if(compare_token(t, ERROR_TYPE)) {
                str_dtor(&ret_types);
                return LEXICAL_ERROR;
            }
            else if(compare_token_attr(parser, t, SEPARATOR, ",")) {
                //Ok
                get_next_token(parser->scanner);
            }
            else {
                str_dtor(&ret_types);
                error_unexpected_token(parser, ",", t);
                return SYNTAX_ERROR;
            }
        }
    }

    str_dtor(&ret_types);

    return PARSE_SUCCESS;
}


//<assignment>            -> <id-list> = <expression-list>
int assignment(parser_t *parser, token_t t) {
    //Next token should be = or ,
    /*
    token_t current_token;
    token_init(&current_token);
    */    

    token_t var_id = t;

    debug_print("var id: %s\n", get_attr(&var_id, parser->scanner));

    size_t id_number = 0;
    string_t id_types;
    str_init(&id_types);

    // push names to the stack in lside
    tok_stack_t var_names;
    tok_stack_init(&var_names);

    int retval = assignment_lside(parser, &var_id, &id_types, &id_number, &var_names);
    debug_print("found %i assignment with types %s ...\n", id_number, to_str(&id_types));
    
    retval = (retval == PARSE_SUCCESS) ? assignment_rside(parser, &var_id, &id_types, &id_number) : retval;        
    
    //generate assignment of the values on stack
    generate_multiple_assignment(&parser->sym.symtab_st, &parser->sym.symtab, &var_names, parser->scanner);

    str_dtor(&id_types);
    tok_stack_dtor(&var_names);

    return retval;
}


sym_dtype_t keyword_to_dtype(parser_t *parser, token_t * t, scanner_t *sc) {
    if (str_cmp(get_attr(t, parser->scanner), "string") == 0) {
        return STR;
    }
    else if(str_cmp(get_attr(t, parser->scanner), "integer") == 0) {
        return INT;
    }
    else if(str_cmp(get_attr(t, parser->scanner), "number") == 0) {
        return NUM;
    }
    else if(str_cmp(get_attr(t, parser->scanner), "nil") == 0) {
        return NIL;
    }

    return INT;
}


/**
 * @brief Inserts variable into current symbol table
 */ 
void ins_var(parser_t *parser, token_t *id_token, 
             sym_status_t status, sym_dtype_t dtype) {

    //size_t id_len = strlen(get_attr(id_token, scanner));
    size_t cur_f_len = strlen(get_attr(parser->curr_func_id, parser->scanner));
    string_t var_name;
    str_init(&var_name);

    str_cpy_tostring(&var_name, get_attr(parser->curr_func_id, parser->scanner), cur_f_len); //'name_of_current_function'
    app_char('$', &var_name); //'name_of_current_function'$
    app_str(&var_name, get_attr(id_token, parser->scanner)); //'name_of_current_function'$'name_of_variable_in_ifj21'
    app_char('$', &var_name); //'name_of_current_function'$'name_of_variable_in_ifj21'$

    //TODO
    char conv_buff[DECLARATION_COUNTER_MAX_LEN];
    snprintf(conv_buff, DECLARATION_COUNTER_MAX_LEN, "%ld", parser->decl_cnt);

    app_str(&var_name, conv_buff);
    
    sym_data_t symdata_var = {.name = var_name, .type = VAR, 
                              .dtype = dtype, .status = status};

    debug_print("Putting %s into symbol table... its name is %s and status is %d\n", get_attr(id_token, parser->scanner), to_str(&var_name), status);

    insert_sym(&parser->sym.symtab, get_attr(id_token, parser->scanner), symdata_var);
}


//<value-assignment>
/**
 * @brief Parses assignment after declaration of variable in function
 * @warning This function expects that the current token was get by lookahead
 * @note If everything went well, changes variable status to defined
 */
int local_var_assignment(parser_t *parser, sym_status_t *status, 
                         sym_dtype_t dtype, token_t *var_id) {

    if(lookahead_token_attr(parser, OPERATOR, "=")) {
        //Delete it temporarly, because it is hiding same name variables in outer scope (they can be used in initialization)
        delete_sym(&parser->sym.symtab, get_attr(var_id, parser->scanner));

        get_next_token(parser->scanner);
        //if there is = we check the value assignment
        debug_print("Calling precedence parser...\n");

        char *id_char = get_attr(var_id, parser->scanner);
        
        string_t ret_types;
        str_init(&ret_types);
        bool was_f_called;
        int return_val = parse_expression(parser->scanner, &parser->sym, &ret_types, &was_f_called);
    
        if(return_val != EXPRESSION_SUCCESS) {
            str_dtor(&ret_types);
            return return_val;
        }
        else {
            *status = DEFINED; //Ok
        }

        if(!is_valid_assign(dtype, char_to_dtype(to_str(&ret_types)[0]))) {
            error_semantic(parser, "Incomatible data types in initialization of variable '\033[1;33m%s\033[0m'\n", id_char);
            str_dtor(&ret_types);
            return SEMANTIC_ERROR_ASSIGNMENT;
        }

        str_dtor(&ret_types);

        ins_var(parser, var_id, *status, dtype); //After initialization, it must be inserted to the symtable again

        //generate assigment code
        char * unique_name = get_unique_name(&parser->sym.symtab_st, &parser->sym.symtab, var_id, parser->scanner).str;
        generate_assign_value(unique_name);
    }

    return PARSE_SUCCESS;
}


//: [type]
int local_var_datatype(parser_t *p, token_t *curr_tok, sym_dtype_t *var_type) {
    //Should be a comma
    bool comma = check_next_token_attr(p, SEPARATOR, ":");
    if(!comma) {
        return SYNTAX_ERROR;
    }
    
    //Should be a data type
    *curr_tok = get_next_token(p->scanner);

    //Determining data type of declared variable
    if(compare_token(*curr_tok, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(!is_datatype(p, *curr_tok)) {
        error_unexpected_token(p, "datatype", *curr_tok);
        return SYNTAX_ERROR;
    }
    else {
        *var_type = keyword_to_dtype(p, curr_tok, p->scanner);
    }

    return PARSE_SUCCESS;
}


//local  [id] : [type] <value-assignment>
int parse_local_var(parser_t *parser) {
    //Go one token forward
    get_next_token(parser->scanner);

    //Should be identifier
    token_t t = get_next_token(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    
    token_t var_id = t;
    sym_dtype_t var_type;
    sym_status_t status = DECLARED;


    //debug_print("Var_ID is: %s <---------------\n\n", var_id);
    if(!compare_token(t, IDENTIFIER)) {
        error_unexpected_token(parser, "identifier", t);
        return t.token_type == EOF_TYPE ? LEXICAL_ERROR : SYNTAX_ERROR;
    }

    if(search(&parser->sym.symtab, get_attr(&var_id, parser->scanner))) { //Variable is declared in current scope
        error_semantic(parser, "Redeclaration of variable \033[1;33m%s\033[0m!", get_attr(&var_id, parser->scanner));
        return SEMANTIC_ERROR_DEFINITION;
    }

    if(search(&parser->sym.global, get_attr(&var_id, parser->scanner))) { //Variable is declared in current scope
        error_semantic(parser, "Name of variable \033[1;33m%s\033[0m is colliding with function name!", get_attr(&var_id, parser->scanner));
        return SEMANTIC_ERROR_DEFINITION;
    }


    //There must be data type
    int retval;
    retval = local_var_datatype(parser, &t, &var_type);

    //insert to symtab and generate code
    ins_var(parser, &var_id, status, var_type);
    generate_declare_variable(&parser->sym.symtab_st, &parser->sym.symtab, &var_id, parser->scanner);

    //There can be a value assignment
    retval = (retval == PARSE_SUCCESS) ? local_var_assignment(parser, &status, var_type, &var_id) : retval;
    
    parser->decl_cnt++;

    return retval;
}


//require [string]
int parse_require(parser_t *parser) {
    //Go one token forward
    get_next_token(parser->scanner);

    token_t t = get_next_token(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(compare_token(t, STRING)) {
        return PARSE_SUCCESS;
    }
    else {
        error_unexpected_token(parser, "STRING", t);
    }

    return SYNTAX_ERROR;
}


void ins_func(parser_t *parser, token_t *id_token, sym_data_t *data) {

    size_t id_len = strlen(get_attr(id_token, parser->scanner));
    //There is no need to creation original name of function (it always must be original in whole program)
    str_cpy_tostring(&data->name, get_attr(id_token, parser->scanner), id_len);
    debug_print("Putting %s into symbol table... its name is %s and (status: %d, ret_types: %s, params: %s)\n", get_attr(id_token, parser->scanner), to_str(&data->name), data->status, to_str(&data->ret_types), to_str(&data->params));
    insert_sym(&parser->sym.global, get_attr(id_token, parser->scanner), *data);
}


//<type-list>             -> : [type] <type-list-1>
int func_dec_returns(parser_t *parser, string_t *returns) {
    if(lookahead_token_attr(parser, SEPARATOR, ":")) {
        //Will just get the ':'
        debug_print("parsing function return types...\n");
        token_t t = get_next_token(parser->scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }

        bool finished = false;
        while(!finished) {
            //should be datatype
            t = get_next_token(parser->scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
            else if(!is_datatype(parser, t)) {
                error_unexpected_token(parser, "data type", t);
                finished = true;
                return SYNTAX_ERROR;
            }
            else {
                char dtype_c = dtype_to_char(keyword_to_dtype(parser, &t, parser->scanner));
                app_char(dtype_c, returns);
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(parser, SEPARATOR, ",");
            if(!comma) {
                finished = true;
            }
            else {
                //we go one token forward
                get_next_token(parser->scanner);
            }
        }
    }

    return PARSE_SUCCESS;
}


//(<param-list>) 
int func_dec_params(parser_t *parser, string_t *params) {
    if(!check_next_token_attr(parser, SEPARATOR, "("))
        return SYNTAX_ERROR;

    if(is_datatype(parser, lookahead(parser->scanner))) {
        debug_print("parsing function param types...\n");

        bool finished = false;
        while(!finished) {
            //should be datatype
            token_t t = get_next_token(parser->scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
            else if(!is_datatype(parser, t)) {
                error_unexpected_token(parser, "data type", t);
                finished = true;
                return SYNTAX_ERROR;
            }
            else {
                char type_c = dtype_to_char(keyword_to_dtype(parser, &t, parser->scanner));
                app_char(type_c, params);
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(parser, SEPARATOR, ",");
            if(!comma) {
                finished = true;
            }
            else {
                //we go one token forward
                get_next_token(parser->scanner);
            }
        }
    }

    if(!check_next_token_attr(parser, SEPARATOR, ")")) {
        return SYNTAX_ERROR;
    }

    return PARSE_SUCCESS;
}


//<global-statement>      -> global [id] : function(<param-list>) <type-list>
int parse_function_dec(parser_t *parser) {
    //This token is just 'global' we skip it
    get_next_token(parser->scanner);

    //this is function ID
    token_t id_fc = get_next_token(parser->scanner);
    if(compare_token(id_fc, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    //Puts builtin function into symtable 
    bool is_builtin = check_builtin(get_attr(&id_fc, parser->scanner), &parser->sym.global);
    if(is_builtin) {
        error_semantic(parser, "There is builtin function with the same name as '\033[1;33m%s\033[0m' (change the name of your function)!", get_attr(&id_fc, parser->scanner));
        return SEMANTIC_ERROR_DEFINITION;
    }

    debug_print("SHOULD BE ID_FC: %s\n\n\n", get_attr(&id_fc, parser->scanner));

    //parsing function definition signature
    bool id = compare_token(id_fc, IDENTIFIER);
    if(!id){
        error_unexpected_token(parser, "FUNCTION IDENTIFIER", id_fc);
        return SYNTAX_ERROR;
    }
    //should be ':'
    bool colon = check_next_token_attr(parser, SEPARATOR, ":");
    if(!colon) {
        return SYNTAX_ERROR;
    }

    //shound be 'function'
    bool function_keyword = check_next_token_attr(parser, KEYWORD, "function");
    if(!function_keyword) {
        return SYNTAX_ERROR;
    }
    
    //!probably not because we ignore declarations
    //generate code for start,
    //generate_start_function(get_attr(&id_fc, scanner));
    
    tok_push(&parser->decl_func, id_fc); //For checking if function is defined

    if(!compare_token(id_fc, IDENTIFIER)) {
        error_unexpected_token(parser, "IDENTIFIER", id_fc);
        return SYNTAX_ERROR;
    }

    if(search(&parser->sym.global, get_attr(&id_fc, parser->scanner))) { //Function is declared in current scope (global scope)
        error_semantic(parser, "Redeclaration of function \033[1;33m%s\033[0m!", get_attr(&id_fc, parser->scanner));
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
    retval = func_dec_params(parser, &f_data.params);

    retval = (retval == PARSE_SUCCESS) ? func_dec_returns(parser, &f_data.ret_types) : retval;

    if(retval == PARSE_SUCCESS) {
        ins_func(parser, &id_fc, &f_data);
    }
    else {
        data_dtor(&f_data);
    }

    return retval;
}


int check_if_declared(parser_t *parser, bool *was_decl, 
                      token_t *id_tok, sym_data_t *sym_data) {

    char *f_name = get_attr(id_tok, parser->scanner);
    tree_node_t * symbol = search(&parser->sym.global, f_name);
    if(symbol) {
        *was_decl = true;

        if(symbol->data.status == DEFINED) { //Function were defined
            error_semantic(parser, "Redefinition of function %s!", f_name);
            return SEMANTIC_ERROR_DEFINITION;
        }
        else { //Function was declared but not defined
            init_data(sym_data);
            cpy_strings(&sym_data->name, &symbol->data.name, false);
            cpy_strings(&sym_data->params, &symbol->data.params, false);
            cpy_strings(&sym_data->ret_types, &symbol->data.ret_types, false);
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
int func_def_params_prolog(parser_t *parser, token_t *param_id) {
    //Should be IDENTIFIER
    *param_id = get_next_token(parser->scanner);
    if(compare_token(*param_id, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(param_id->token_type != IDENTIFIER) {
        error_unexpected_token(parser, "IDENTIFIER", *param_id);
        return SYNTAX_ERROR;
    }
    
    //Should be COLON
    token_t t = get_next_token(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(!compare_token_attr(parser, t, SEPARATOR, ":")) {
        error_unexpected_token(parser, "':'", t);
        return SYNTAX_ERROR;
    }

    return PARSE_SUCCESS;
}


//<param-list>            -> [id] : [type] <param-list-1>
int func_def_params(parser_t *parser, token_t *id_token, 
                    bool was_decl, sym_data_t *f_data) {
    size_t params_cnt = 0;
    //Check if there are parameters
    token_t t = lookahead(parser->scanner);
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
            int prolog_ret = func_def_params_prolog(parser, &param_id);
            if(prolog_ret != PARSE_SUCCESS) {
                tok_stack_dtor(&param_names);
                return prolog_ret;
            }

            //push it to param_names for code generation
            tok_push(&param_names,param_id);

            //Should be DATATYPE
            t = get_next_token(parser->scanner);
            if(compare_token(t, ERROR_TYPE)) {
                tok_stack_dtor(&param_names);
                return LEXICAL_ERROR;
            }
            else if(!is_datatype(parser, t)) {
                error_unexpected_token(parser, "DATA TYPE", t);
                tok_stack_dtor(&param_names);
                return SYNTAX_ERROR;
            }
            else {
                sym_dtype_t dtype;
                if(was_decl) {
                    char * params_s = to_str(&f_data->params);
                    dtype = char_to_dtype(params_s[params_cnt]);

                    if(params_s[params_cnt] == '\0') { //There is bigger amount of parameteres than should be
                        error_semantic(parser, "Parameter AMOUNT mismatch in definition of \033[1;33m%s\033[0m (there are to many of them)!", get_attr(id_token, parser->scanner));
                        tok_stack_dtor(&param_names);
                        return SEMANTIC_ERROR_DEFINITION;
                    }
                    else if(dtype != keyword_to_dtype(parser, &t, parser->scanner)) {
                        error_semantic(parser, "Parameter DATA TYPE mismatch in definition of \033[1;33m%s\033[0m!", get_attr(id_token, parser->scanner));
                        tok_stack_dtor(&param_names);
                        return SEMANTIC_ERROR_DEFINITION;
                    }
                }
                else {
                    dtype = keyword_to_dtype(parser, &t, parser->scanner);
                    char dtype_c = dtype_to_char(keyword_to_dtype(parser, &t, parser->scanner));
                    app_char(dtype_c, &f_data->params);
                }


                ins_var(parser, &param_id, DECLARED, dtype);
                parser->decl_cnt++;
                params_cnt++;
                
                //OK
            }

            //if there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(parser, SEPARATOR, ",");
            if(!comma) {
                finished = true;
                if(params_cnt < len(&f_data->params)) {
                    error_semantic(parser, "Parameter AMOUNT mismatch in definition of \033[1;33m%s\033[0m (missing parameters)!", get_attr(id_token, parser->scanner));
                    tok_stack_dtor(&param_names);
                    return SEMANTIC_ERROR_DEFINITION;
                }

            }
            else {
                //we go one token forward
                get_next_token(parser->scanner);
            }
        }
    }
    else if(was_decl && len(&f_data->params) > 0) {
        error_semantic(parser, "Return values AMOUNT mismatch in definition of function \033[1;33m%s\033[0m (missing parameters)!", get_attr(id_token, parser->scanner));
        tok_stack_dtor(&param_names);
        return SEMANTIC_ERROR_DEFINITION;
    }

    //generate code for parameters
    generate_parameters(&(parser->sym.symtab_st), &parser->sym.symtab, &param_names, parser->scanner);

    tok_stack_dtor(&param_names);
    return PARSE_SUCCESS;
}


int check_function_signature(parser_t *parser, bool id, bool left_bracket) {
    bool right_bracket = check_next_token_attr(parser, SEPARATOR, ")");
    if(!(id && left_bracket && right_bracket)) {
        debug_print("ERROR INVALID FUNCTION SINATURE!\n");

        return SYNTAX_ERROR;
    }

    return PARSE_SUCCESS;
}


//<type-list>             -> : [type] <type-list-1>
int func_def_returns(parser_t *parser, token_t *id_token, 
                     bool was_decl, sym_data_t *f_data) {

    size_t ret_cnt = 0;
    //Parsing types if there is colon
    
    if(lookahead_token_attr(parser, SEPARATOR, ":")) {
        //Will just get the ':'
        debug_print("parsing function types...\n");
        token_t t = get_next_token(parser->scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }

        bool finished = false;
        while(!finished) {
            //Should be datatype
            t = get_next_token(parser->scanner);
            if(compare_token(t, ERROR_TYPE)) {
                return LEXICAL_ERROR;
            }
            else if(!is_datatype(parser, t)) {
                error_unexpected_token(parser, "data type", t);
                finished = true;

                return SYNTAX_ERROR;
            }
            else {
                if(was_decl) {
                    char * returns_str = to_str(&f_data->ret_types);
                    sym_dtype_t dtype = char_to_dtype(returns_str[ret_cnt]);
                    if(returns_str[ret_cnt] == '\0') {
                        error_semantic(parser, "Return values AMOUNT mismatch in definition of \033[1;33m%s\033[0m (there are too many of them)!", get_attr(id_token, parser->scanner));
                        return SEMANTIC_ERROR_DEFINITION;
                    }
                    else if(dtype != keyword_to_dtype(parser, &t, parser->scanner)) {
                        error_semantic(parser, "Return value DATA TYPE mismatch in definition of \033[1;33m%s\033[0m!", get_attr(id_token, parser->scanner));
                        return SEMANTIC_ERROR_DEFINITION;
                    }
                }
                else {
                    char dtype_c = dtype_to_char(keyword_to_dtype(parser, &t, parser->scanner));
                    app_char(dtype_c, &f_data->ret_types);
                }

                //OK
            }

            //If there is no comma we should be at the end of the list
            bool comma = lookahead_token_attr(parser, SEPARATOR, ",");
            if(!comma) {
                finished = true;
                if(ret_cnt != len(&f_data->ret_types) - 1) {
                    error_semantic(parser, "Return values amount mismatch in definition of \033[1;33m%s\033[0m (something is missing)!", get_attr(id_token, parser->scanner));
                    return SEMANTIC_ERROR_DEFINITION;
                }
            }
            else {
                //We go one token forward
                get_next_token(parser->scanner);
            }

            ret_cnt++;
        }
    }
    else if(!lookahead_token_attr(parser, SEPARATOR, ":") && was_decl) {
        if(len(&f_data->ret_types) > 0) {
            error_semantic(parser, "Return values AMOUNT mismatch in definition of function \033[1;33m%s\033[0m(something is missing)!", get_attr(id_token, parser->scanner));
            return SEMANTIC_ERROR_DEFINITION;
        }
    }

    return PARSE_SUCCESS;
}

//end
int func_def_epilogue(parser_t *parser) {
    token_t t = get_next_token(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    //generate function end
    generate_end_function(get_attr(parser->curr_func_id, parser->scanner));

    debug_print("parsing function finished! at: (%lu,%lu)\n", parser->scanner->cursor_pos[ROW], parser->scanner->cursor_pos[COL]); 

    if(t.token_type == KEYWORD) {
        if(compare_token_attr(parser, t, KEYWORD, "end")) {
            debug_print("Successfully ended function definition!\n");
            return PARSE_SUCCESS;
        }
        else {
            error_unexpected_token(parser, "Wrong keyword! Function must end only with 'end'", t);
            return SYNTAX_ERROR;
        }

        // error_unexpected_token("[]", t);
        // return (retval != PARSE_SUCCESS) ? retval : SYNTAX_ERROR;
    }
    else {
        error_unexpected_token(parser, "Better check the implementation, function must end with keyword 'end'!", t);
        return SYNTAX_ERROR;
    }
}


int check_return(parser_t *parser, token_t *id_fc) {
    tree_node_t *symbol = search(&parser->sym.global, get_attr(id_fc, parser->scanner));
    if(symbol && len(&symbol->data.ret_types) > 0 && !parser->found_return) {
        warn(parser, "Function \033[1;33m%s\033[0m can reach the end without return!", get_attr(id_fc, parser->scanner));
    }

    return PARSE_SUCCESS;
}

 
int parse_function_def(parser_t *parser) {
    //this token is just 'function' so we skip it
    get_next_token(parser->scanner);

    token_t id_fc = get_next_token(parser->scanner);
    if(compare_token(id_fc, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    //Adds builtin function into symtable, so semantic checks can detects its redefinition
    bool is_builtin = check_builtin(get_attr(&id_fc, parser->scanner), &parser->sym.global);
    if(is_builtin) {
        error_semantic(parser, "There is builtin function with the same name as '\033[1;33m%s\033[0m' (change the name of your function)!", get_attr(&id_fc, parser->scanner));
        return SEMANTIC_ERROR_DEFINITION;
    }

    //generate function start
    generate_start_function(get_attr(&id_fc, parser->scanner));

    parser->curr_func_id = &id_fc;
    parser->found_return = false;

    //parsing function definition signature
    bool id = (id_fc.token_type == (IDENTIFIER));
    bool left_bracket = check_next_token_attr(parser, SEPARATOR, "(");

    sym_data_t func_d;
    bool was_declared = false;
    int retval = check_if_declared(parser, &was_declared, &id_fc, &func_d);
    func_d.status = DEFINED;

    to_inner_ctx(parser); //Switch context (params must be in this context)
    
    retval = (retval == PARSE_SUCCESS) ? func_def_params(parser, &id_fc, was_declared, &func_d) : retval;

    retval = (retval == PARSE_SUCCESS) ? check_function_signature(parser, id, left_bracket) : retval;

    retval = (retval == PARSE_SUCCESS) ? func_def_returns(parser, &id_fc, was_declared, &func_d) : retval;
    
    if(retval != PARSE_SUCCESS) { //Stop if an error ocurred
        if(!was_declared) {
            data_dtor(&func_d);
        }

        return retval;
    }
    
    ins_func(parser, &id_fc, &func_d);

    //Parsing inside function
    debug_print("parsing inside function...\n");
    retval = statement_list(parser);
    debug_print("parsing function finished! return code: %i, at: (%lu,%lu)\n", retval, parser->scanner->cursor_pos[ROW], parser->scanner->cursor_pos[COL]);

    to_outer_ctx(parser); //Go back to higher context level

    retval = (retval == PARSE_SUCCESS) ? func_def_epilogue(parser) : retval;

    retval = (retval == PARSE_SUCCESS) ? check_return(parser, &id_fc) : retval;

    parser->curr_func_id = NULL;

    return retval;
}


//<statement>             -> [function-id](<argument-list>)
int parse_function_call(parser_t *parser, token_t *id_func) {
    debug_print("parsing function call...\n");
    bool opening_bracket = check_next_token_attr(parser, SEPARATOR, "(");
    
    if(opening_bracket) {

        //parsing arguments should leave the arguments at the top of the stack 
        int retval = parse_function_arguments(parser, id_func);
        if(retval != PARSE_SUCCESS) {
            return retval;
        }

        debug_print("function args success %i\n", (int)retval);

        tree_node_t *symbol = search(&parser->sym.global, get_attr(id_func, parser->scanner));
        char * params_str = to_str(&symbol->data.params);
        bool is_variadic = (params_str[0] == '%') ? true : false;
        //generate function call unless variadic
        if(!is_variadic) {
            debug_print("function %s is not variadic", id_func);
            generate_call_function(get_attr(id_func, parser->scanner));
        }

        return retval;
    }
    else {
        return SYNTAX_ERROR;
    }
}


//!Check the implementation for strlen: "#E"
int parse_function_arguments(parser_t *parser, token_t *id_func) {
    bool closing_bracket = false;

    tree_node_t *symbol = search(&parser->sym.global, get_attr(id_func, parser->scanner));
    char * params_str = to_str(&symbol->data.params);

    size_t argument_cnt = 0;
    bool is_variadic = (params_str[0] == '%') ? true : false; //In our case variadic means - with variable AMOUNT and TYPES of arguments
    while(!closing_bracket) {
        token_t t = lookahead(parser->scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }
        else if(is_expression(parser, t)) {
            string_t ret_types;
            str_init(&ret_types);
            bool was_f_called;
            int expr_retval = parse_expression(parser->scanner, &parser->sym, &ret_types, &was_f_called);
            if(expr_retval != EXPRESSION_SUCCESS) {
                str_dtor(&ret_types);
                return expr_retval;
            }

            if(!is_variadic) {
                sym_dtype_t d_type = char_to_dtype(params_str[argument_cnt]);
                sym_dtype_t prim_dtype = char_to_dtype(to_str(&ret_types)[0]);
                if(!is_valid_assign(d_type, prim_dtype)) {
                    error_semantic(parser, "Bad function call of \033[1;33m%s\033[0m! Bad data types of arguments!", get_attr(id_func, parser->scanner));
                    str_dtor(&ret_types);
                    return SEMANTIC_ERROR_PARAMETERS;
                }
                else {
                    //Ok
                }
            }else{
                //if variadic call for each argument
                generate_call_function(get_attr(id_func, parser->scanner));
            }

            str_dtor(&ret_types);
        }
        else if(compare_token_attr(parser, t, SEPARATOR, ")")) {
            get_next_token(parser->scanner);
            closing_bracket = true;
            continue;
        }
        else {
            error_unexpected_token(parser, "EXPRESSION", t);
        }

        //Check if there will be next argument
        t = get_next_token(parser->scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }
        else if(compare_token_attr(parser, t, SEPARATOR, ",")) {
            if(argument_cnt + 1 > strlen(params_str) - 1 && !is_variadic) { //Function needs less arguments
                error_semantic(parser, "Bad function call of \033[1;33m%s\033[0m! Too many arguments!", get_attr(id_func, parser->scanner));
                return SEMANTIC_ERROR_PARAMETERS;
            }
            else {
                //Ok
            }
        }
        else if(compare_token_attr(parser, t, SEPARATOR, ")")) {
            closing_bracket = true;
            if(argument_cnt != strlen(params_str) - 1 && !is_variadic) { //Function needs more arguments
                error_semantic(parser, "Bad function call of \033[1;33m%s\033[0m! Missing arguments!", get_attr(id_func, parser->scanner));
                return SEMANTIC_ERROR_PARAMETERS;
            }
        }
        else {
            error_unexpected_token(parser, "',' as a separator between args or ')' as an end of argument list", t);
            return SYNTAX_ERROR;
        }

        argument_cnt++;
    }
    if(argument_cnt == 0 && strlen(params_str) > 0) { //Function needs arguments but there aren't any
        error_semantic(parser, "Bad function call of \033[1;33m%s\033[0m! Missing arguments!", get_attr(id_func, parser->scanner));
        return SEMANTIC_ERROR_PARAMETERS;
    }

    return closing_bracket ? PARSE_SUCCESS : SYNTAX_ERROR;
}


//<statement>             -> if <expression> then <statement-list> <else-branch> end
int parse_if(parser_t *parser) {
    //we must copy the value, because there can be nested ifs
    size_t current_cond_cnt = parser->cond_cnt;
    parser->cond_cnt++;
    //Go one token forward
    get_next_token(parser->scanner);

    debug_print("Calling precedence parser...\n");

    string_t ret_types;
    str_init(&ret_types);
    bool was_f_called;
    int expr_retval = parse_expression(parser->scanner, &parser->sym, &ret_types, &was_f_called);
    str_dtor(&ret_types);
    if(expr_retval != EXPRESSION_SUCCESS) {
        return expr_retval;
    }

    bool then = check_next_token_attr(parser, KEYWORD, "then");
    if(!then) {
        return SYNTAX_ERROR;
    }
    //generate conditions and jumps 
    generate_if_condition(current_cond_cnt);

    to_inner_ctx(parser); //Switch the context

    int retval = statement_list(parser);
    if(retval != PARSE_SUCCESS) {
        return retval;
    }

    bool ret_in_if_branch = parser->found_return;
    parser->found_return = false;
    
    to_outer_ctx(parser); //Back to higher leve context

    token_t t = get_next_token(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(compare_token(t, KEYWORD)) {
        //generates end of if part statement
        generate_if_end(current_cond_cnt);
        if(compare_token_attr(parser, t, KEYWORD, "end")) {
            debug_print("Ended if\n");
            // generate end of the whole if - (else) statement
            generate_else_end(current_cond_cnt);
            return PARSE_SUCCESS;
        }
        else if(compare_token_attr(parser, t, KEYWORD, "else")) {

            to_inner_ctx(parser); //Switch the context

            //<else-branch>           -> else <statement-list>
            debug_print("parsing else branch...\n");

            retval = statement_list(parser);
            if(retval != PARSE_SUCCESS) {
                return retval;
            }

            bool ret_in_else_branch = parser->found_return;
            parser->found_return = ret_in_if_branch && ret_in_else_branch; //Function must ends in both branches

            to_outer_ctx(parser);

            t = get_next_token(parser->scanner);
            //generate end of the whole if - (else) statement
            generate_else_end(current_cond_cnt);
            return PARSE_SUCCESS; //Back to higher level context
        }

        error_unexpected_token(parser, "[2]", t);
        return SYNTAX_ERROR;
    }
    else {
        error_unexpected_token(parser, "Better check the implementation! END", t);
        return SYNTAX_ERROR;
    }
}


int parse_else(parser_t *parser) {
    return PARSE_SUCCESS;
}


//return <expression-list>
int parse_return(parser_t *parser) {
    //go one token forward
    get_next_token(parser->scanner);
    token_t* id_fc = parser->curr_func_id;

    debug_print("Calling precedence parser to parse return in %s...\n", get_attr(id_fc, parser->scanner));

    tree_node_t *symbol = search(&parser->sym.global, get_attr(id_fc, parser->scanner));
    
    size_t returns_cnt = 0;
    bool finished = false;
    char * returns_str = to_str(&symbol->data.ret_types);
    while(!finished) {
        token_t t = lookahead(parser->scanner);
        if(compare_token(t, ERROR_TYPE)) {
            return LEXICAL_ERROR;
        }
        else if(!is_expression(parser, t)) {
            finished = true;
            if(len(&symbol->data.ret_types) > 0) {
                //Implicit nil return
                size_t difference = len(&symbol->data.ret_types)  - returns_cnt;
                generate_additional_returns(difference);
            }

            break;
        }
        else {
            string_t ret_types;
            str_init(&ret_types);
            bool was_f_called;
            int retval = parse_expression(parser->scanner, &parser->sym, &ret_types, &was_f_called);
            if(retval != EXPRESSION_SUCCESS) {
                str_dtor(&ret_types);
                return retval;
            }
            else {
                sym_dtype_t dec_type = char_to_dtype(returns_str[returns_cnt]);
                sym_dtype_t prim_dtype = char_to_dtype(to_str(&ret_types)[0]);
                if(!is_valid_assign(dec_type, prim_dtype)) {
                    str_dtor(&ret_types);

                    if(was_f_called) {
                        error_semantic(parser, "Function call in return statement doesn't return value, which function \033[1;33m%s\033[0m returns!", get_attr(id_fc, parser->scanner));
                    }
                    else {
                        error_semantic(parser, "Bad data type of return in function \033[1;33m%s\033[0m!", get_attr(id_fc, parser->scanner));
                    }

                    return SEMANTIC_ERROR_PARAMETERS;
                }
                else {
                    //Ok
                }
            }

            str_dtor(&ret_types);
        }

        //If there is no comma we should be at the end of the list
        bool comma = lookahead_token_attr(parser, SEPARATOR, ",");
        if(!comma) {
            finished = true;
            if(len(&symbol->data.ret_types) - 1 > returns_cnt) {
                //Implicit nil return
                size_t difference = (len(&symbol->data.ret_types) -1) - returns_cnt;
                generate_additional_returns(difference);
            }
        }
        else {
            //we go one token forward
            get_next_token(parser->scanner);
            if(len(&symbol->data.ret_types) - 1 < returns_cnt + 1) {
                error_semantic(parser, "Function \033[1;33m%s\033[0m returns %d values but more return values were found!", get_attr(id_fc, parser->scanner), returns_cnt + 1, len(&symbol->data.ret_types));
                return SEMANTIC_ERROR_PARAMETERS;
            }
        }

        returns_cnt++;
    }

    parser->found_return = true;

    generate_return();
    
    return PARSE_SUCCESS;
}


int parse_end(parser_t *parser) {
    return PARSE_SUCCESS;
}


int parse_while(parser_t *parser) {
    //go one token forward
    get_next_token(parser->scanner);

    //save the counter to prevent overwriting in nested loops
    size_t current_cnt = parser->loop_cnt;
    parser->loop_cnt++;

    //generate while beginning
    generate_while_condition_beginning(current_cnt);
    
    debug_print("Calling precedence parser...\n");
    string_t ret_types;
    str_init(&ret_types);
    bool was_f_called;
    int expr_retval = parse_expression(parser->scanner, &parser->sym, &ret_types, &was_f_called);
    str_dtor(&ret_types);
    if(expr_retval != EXPRESSION_SUCCESS) {
        return expr_retval;
    }

    bool then = check_next_token_attr(parser, KEYWORD, "do");
    if(!then) {
        return false;
    }

    //generate additional condition check
    generate_while_condition_evaluate(current_cnt);

    to_inner_ctx(parser); //Switch context

    int retval = statement_list(parser);
    if(retval != PARSE_SUCCESS) {
        return retval;
    }

    to_outer_ctx(parser); //Switch context to higher level

    token_t t = get_next_token(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if(compare_token(t, KEYWORD)) {
        if(compare_token_attr(parser, t, KEYWORD, "end")) {

            //generate end of while
            generate_while_end(current_cnt);

            debug_print("Ended while\n");
            return PARSE_SUCCESS;
        }
        
        error_unexpected_token(parser, "[]", t);
        return SYNTAX_ERROR;
    }
    else {
        error_unexpected_token(parser, "Better check the implementation! END", t);
        return SYNTAX_ERROR;
    }

    return SYNTAX_ERROR;
}


rule_t *determine_rule(parser_t *p, token_t t, rule_t *(*ruleset_f)(size_t)) {
    
    // size_t rules_n = sizeof(ruleset) / sizeof(rule_t);
    rule_t *rule = NULL;
    for (size_t i = 0; (rule = ruleset_f(i)) != NULL; i++)
    {
        token_t to_be_checked = rule->rule_first;
        if(rule->attrib_relevant) {
            //the atribute is relevant (rules with same token types)
            /**< Here can be .attr because the value of token is certainly referenced by pointer */
            if(compare_token_attr(p, t, to_be_checked.token_type, to_be_checked.attr)) {
                return rule;
            }
        }
        else {
            //the atribute is irrelevant we check only for matching token type
            if(compare_token(t, to_be_checked.token_type)) {
                return rule;
            }
        }
    }

    error_unexpected_token(p, "NO RULE can be used to parse this token! Other", t);
    return NULL;
}


int parse_global_identifier(parser_t *parser) {
    token_t id_token = get_next_token(parser->scanner);
    if(compare_token(id_token, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    debug_print("got identifier\n");

    token_t next = lookahead(parser->scanner);
    bool opening_bracket = compare_token_attr(parser, next, SEPARATOR, "(");
    if(!opening_bracket) {
        error_unexpected_token(parser, "'(' after identifier (to be function call)", next);
        return SYNTAX_ERROR;
    }

    //Puts builtin function into symtable (if it exists)
    check_builtin(get_attr(&id_token, parser->scanner), &parser->sym.global);

    char *func = get_attr(&id_token, parser->scanner);
    tree_node_t *func_valid = search(&parser->sym.global, func);
    if (func_valid == NULL) { // Function is not declared
        error_semantic(parser, "Function with name '\033[1;33m%s\033[0m' not defined!", func);
        return SEMANTIC_ERROR_DEFINITION;
    }

    return parse_function_call(parser, &id_token);
}


int parse_identifier(parser_t *p) {
    //Should be identifier
    token_t id_token = get_next_token(p->scanner);
    if(compare_token(id_token, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    debug_print("got identifier\n");
    
    // It is neccessary to look at the next lexeme also
    // token_t id_token = get_next_token(scanner);
    token_t t = lookahead(p->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    bool is_multiple_assignment = compare_token_attr(p, t, SEPARATOR, ",");
    bool is_single_assignment = compare_token_attr(p, t, OPERATOR, "=");

    //Check if it is a function call
    if(compare_token_attr(p, t, SEPARATOR, "(")) {

        char *func = get_attr(&id_token, p->scanner);
        check_builtin(func, &p->sym.global); //Adds builtin correspondent function into symtable

        tree_node_t *func_valid = search(&p->sym.global, func);
        if (func_valid == NULL) { // Function is not declared
            error_semantic(p, "Function with name '\033[1;33m%s\033[0m' not defined!", func);
            return SEMANTIC_ERROR_DEFINITION;
        }

        debug_print("Call function %s\n", func);
        return parse_function_call(p, &id_token);
    }
    else if(is_multiple_assignment || is_single_assignment) {
        debug_print("parsing assignment...\n");
        return assignment(p, id_token);
    }
    else {
        error_unexpected_token(p, "After IDENTIFIER a function call or an assignment", t);
    }

    return SYNTAX_ERROR;
}


int EOF_fun_rule(parser_t *parser) {
    //will not actually get the token upon which has been decided to use this rule
    //but will get the next token
    //otherwise i would have to use params for each one of these rule functions
    token_t t = lookahead(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }

    error_unexpected_token(parser, "Reached EOF inside a function. Did you forget 'end'? END",t);
    parser->reached_EOF = true;
    return SYNTAX_ERROR;
}


int EOF_global_rule(parser_t *parser) {
    //will not actually get the token upon which has been decided to use this rule
    //but will get the next token
    //otherwise i would have to use params for each one of these rule functions
    parser->reached_EOF = true;
    return PARSE_SUCCESS;
}

int error_rule() {
    return SYNTAX_ERROR;
}


bool is_expression(parser_t *parser, token_t t) {
    return (compare_token(t , NUMBER) ||
            compare_token(t , INTEGER) ||
            compare_token(t , STRING) ||
            compare_token(t , IDENTIFIER) ||
            compare_token(t , OPERATOR) ||
            compare_token_attr(parser, t, KEYWORD, "nil") ||
            compare_token_attr(parser, t, SEPARATOR, "("));
}


bool is_datatype(parser_t *parser, token_t t) {
    return (compare_token_attr(parser, t, KEYWORD, "string") || 
           compare_token_attr(parser, t, KEYWORD, "number") || 
           compare_token_attr(parser, t, KEYWORD, "integer") || 
           compare_token_attr(parser, t, KEYWORD, "nil"));
}


int parse_datatype(parser_t *parser) { 
    token_t t = get_next_token(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        return LEXICAL_ERROR;
    }
    else if( is_datatype(parser, t)) {
        return PARSE_SUCCESS;
    }

    error_unexpected_token(parser, "DATATYPE keyword", t);
    return SYNTAX_ERROR;
}


void error_unexpected_token(parser_t *parser, char * expected, token_t t) {
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t|\033[0;31m Syntax error:\033[0m ", (parser->scanner->cursor_pos[ROW]), (parser->scanner->cursor_pos[COL]));
    fprintf(stderr, "Wrong token! '%s' expected, but token is: \033[1;33m%s\033[0m type: \033[0;33m%i\033[0m!\n", expected, get_attr(&t, parser->scanner), t.token_type);
}


void error_semantic(parser_t *parser, const char * _Format, ...) {
    va_list args;
    va_start(args,_Format);
    fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t|\033[0;31m Semantic error: \033[0m", (parser->scanner->cursor_pos[ROW]), (parser->scanner->cursor_pos[COL]));
    vfprintf(stderr, _Format, args);
    fprintf(stderr,"\n");
}

void warn(parser_t *parser, const char * _Format, ...) {
    if(PRINT_WARNINGS) {
        va_list args;
        va_start(args,_Format);
        fprintf(stderr, "(\033[1;37m%lu:%lu\033[0m)\t|\033[1;33m Warning: \033[0m", (parser->scanner->cursor_pos[ROW]), (parser->scanner->cursor_pos[COL]));
        vfprintf(stderr, _Format, args);
        fprintf(stderr,"\n");
    }
}


bool lookahead_token(parser_t *parser, token_type_t expecting) {
    token_t t = lookahead(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        parser->return_code = LEXICAL_ERROR;
        return false;
    }

    return compare_token(t, expecting);
}


bool lookahead_token_attr(parser_t *parser, token_type_t expecting_type, char * expecting_attr) {
    token_t t = lookahead(parser->scanner);
    if(compare_token(t, ERROR_TYPE)) {
        parser->return_code = LEXICAL_ERROR;
        return false;
    }

    return compare_token_attr(parser, t, expecting_type, expecting_attr);
}


bool check_next_token(parser_t *parser, token_type_t expecting) {
    token_t t = get_next_token(parser->scanner);
    
    if(compare_token(t, expecting)) {
        return true;
    }

    parser->return_code = (t.token_type == ERROR_TYPE) ? LEXICAL_ERROR : SYNTAX_ERROR;
    error_unexpected_token(parser, tok_type_to_str(t.token_type), t);
    return false;
}


bool check_next_token_attr(parser_t *parser, token_type_t expecting_type, char * expecting_attr) {
    token_t t = get_next_token(parser->scanner);

    if(compare_token_attr(parser, t, expecting_type, expecting_attr))
        return true;

    parser->return_code = (t.token_type == ERROR_TYPE) ? LEXICAL_ERROR : SYNTAX_ERROR;
    error_unexpected_token(parser, expecting_attr, t);
    return false;
}


bool compare_token(token_t t, token_type_t expecting) {
    if(t.token_type == expecting){
        return true;
    }
    return false;
}


bool compare_token_attr(parser_t *parser, token_t t, token_type_t expecting_type, char * expecting_attr) {
    if(t.token_type == expecting_type){
        if(str_cmp(get_attr(&t, parser->scanner), expecting_attr) == 0) {
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
