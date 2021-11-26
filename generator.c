/******************************************************************************
 *                                  IFJ21
 *                               generator.c
 * 
 *          Authors: Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 *              Purpose: Implementation of code generating functions
 * 
 *                    Last change: 25. 11. 2021
 *****************************************************************************/

/**
 * @file generator.c
 * @brief Source file with implementation of code generator
 * 
 * @authors Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 */ 

//?done:TODO work on function calling with arguments
//TODO write tests (probably not in Google Tests)

#include "generator.h"



void generate_init(){
    code_print(".IFJcode21");
    code_print("CREATEFRAME");
    generate_write_function();
    generate_reads_function();
    generate_checkzero_function();
    generate_unaryminus_function();
    generate_same_types();
    generate_force_floats();
}

/**
 * *---------FUNCTIONS---------
 */ 

void generate_start_function(const char * name){
    code_print("\n");
    code_print("JUMP $END_FUN$%s",name);
    code_print("#function %s ()",name);
    code_print("LABEL $FUN$%s", name);
    code_print("CREATEFRAME");
}


void generate_parameters( void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner){

    tok_stack_t *params = param_names;
    
    while (!tok_is_empty(params))
    {
        //get it from the token name
        token_t name_token = tok_pop(params);
        string_t name_unique = get_unique_name(sym_stack, symtab,&name_token,scanner);

        generate_parameter(name_unique.str);

    }

}


void generate_parameter(const char * name){
    code_print("#define param %s",name);
    code_print("DEFVAR TF@&VAR&%s",name); //creates temporary variable
    code_print("POPS TF@&VAR&%s",name);   //assigns one argument from stack to temporary variable
}


void generate_end_function(const char * name){
    code_print("\n");
    code_print("RETURN");
    code_print("LABEL $END_FUN$%s",name);
    code_print("");
}


void generate_call_function(const char *name){
    code_print("# %s()",name);
    code_print("PUSHFRAME");
    code_print("CALL $FUN$%s",name);
    code_print("POPFRAME");
}


/**
 * *---------VARIABLES---------
 */

void generate_declare_variable( void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner){
    string_t name = get_unique_name(sym_stack,symtab,var_id,scanner);
    code_print("DEFVAR TF@&VAR&%s",name.str);
}


void generate_multiple_assignment( void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner){
    tok_stack_t *params = param_names;
    while (!tok_is_empty(params))
    {
        //get it from the token name
        token_t name_token = tok_pop(params);
        string_t name_unique = get_unique_name(sym_stack,symtab,&name_token,scanner);
        
        generate_assign_value(name_unique.str);
    }
}


void generate_assign_value(const char * name){
    code_print("#assign value to %s",name);
    code_print("POPS TF@&VAR&%s",name);   //assigns one argument from stack to temporary variable
}


void generate_value_push( sym_type_t type, sym_dtype_t dtype, const char * token ){
    
    if(type == VAR){
        code_print("PUSHS TF@&VAR&%s",token);
    }else if(type == VAL){
        if(dtype == STR){
            string_t token_s;
            str_init(&token_s);
            to_ascii(token, &token_s);

            code_print("PUSHS %s@%s",convert_type(dtype), token_s.str);
            
            str_dtor(&token_s);
        }else if(dtype == NUM){
            double num = atof(token);
            code_print("PUSHS %s@%a",convert_type(dtype), num);
        }else{
            code_print("PUSHS %s@%s",convert_type(dtype), token);
        }
    }else{
        fprintf(stderr,"Code generation: Error not supported yet\n");
    }

}

/**
 * *--------CONDIONS--------
 */ 
void generate_if_condition(size_t n){
    code_print("#if %i",n);
    code_print("PUSHS bool@true");
    code_print("JUMPIFNEQS $ELSE$START$%i",n);
}

void generate_if_end(size_t n){
    code_print("#end of if %i",n);
    code_print("JUMP $ELSE$END$%i",n);
    code_print("LABEL $ELSE$START$%i",n);
}

void generate_else_end(size_t n){
    code_print("#end of else and the whole if %i statement",n);
    code_print("LABEL $ELSE$END$%i",n);
}

/**
 * *LOOPS
 */ 

void generate_while_condition_beginning(size_t n){
    code_print("#while %i",n);
    code_print("LABEL $WHILE$COND$%i",n);
}

void generate_while_condition_evaluate(size_t n){
    code_print("PUSHS bool@true");
    code_print("JUMPIFNEQS $WHILE$END$%i",n);
}

void generate_while_end(size_t n){
    code_print("#end of while %i loop",n);
    code_print("JUMP $WHILE$COND$%i",n);
    code_print("LABEL $WHILE$END$%i",n);
}


/**
 * *---------OPERATIONS---------
 */ 
void generate_operation_add(){
    generate_call_function("$BUILTIN$sametypes");
    code_print("ADDS");
}

void generate_operation_sub(){
    generate_call_function("$BUILTIN$sametypes");
    code_print("SUBS");
}

void generate_operation_mul(){
    generate_call_function("$BUILTIN$sametypes");
    code_print("MULS");
}

void generate_operation_div(){
    // generate_call_function("$OP$checkzero");
    generate_call_function("$BUILTIN$forcefloats");
    code_print("DIVS");
}

void generate_operation_idiv(){
    code_print("IDIVS");
}

void generate_operation_unary_minus(){
    generate_call_function("$OP$unaryminus");
}

void generate_operation_eq(){
    generate_call_function("$BUILTIN$sametypes");
    code_print("# start operator A==B");
    code_print("EQS");
    code_print("# end operator A==B");
}

void generate_operation_gt(){
    generate_call_function("$BUILTIN$sametypes");
    code_print("# start operator A>B");
    code_print("GTS");
    code_print("# end operator A>B");
}

void generate_operation_lt(){
    generate_call_function("$BUILTIN$sametypes");
    code_print("# start operator A<B");
    code_print("LTS");
    code_print("# end operator A<B");
}

void generate_operation_gte(){
    generate_call_function("$BUILTIN$sametypes");
    code_print("# start operator A>=B");
    code_print("PUSHFRAME");
    code_print("CREATEFRAME");

    //define temp operands A & B
    code_print("DEFVAR TF@!TMP!A");
    code_print("DEFVAR TF@!TMP!B");

    code_print("POPS TF@!TMP!B");
    code_print("POPS TF@!TMP!A");

    //make the stack to this form ->[B,A,B,A]
    code_print("PUSHS TF@!TMP!A");
    code_print("PUSHS TF@!TMP!B");
    code_print("PUSHS TF@!TMP!A");
    code_print("PUSHS TF@!TMP!B");

    //evaluate
    code_print("DEFVAR TF@!TMP!LTRES");
    code_print("GTS");
    code_print("POPS TF@!TMP!LTRES");
    code_print("EQS");
    code_print("PUSHS TF@!TMP!LTRES");
    code_print("ORS");

    //the result will be on top of the stack

    code_print("POPFRAME");
    code_print("# end operator A>=B");
}

void generate_operation_lte(){
    generate_call_function("$BUILTIN$sametypes");
    code_print("# start operator A<=B");
    code_print("PUSHFRAME");
    code_print("CREATEFRAME");

    //define temp operands A & B
    code_print("DEFVAR TF@!TMP!A");
    code_print("DEFVAR TF@!TMP!B");

    code_print("POPS TF@!TMP!B");
    code_print("POPS TF@!TMP!A");
    
    code_print("DEFVAR TF@!TYPE!A");
    code_print("TYPE TF@!TYPE!A TF@!TMP!A");

    //make the stack to this form ->[B,A,B,A]
    code_print("PUSHS TF@!TMP!A");
    code_print("PUSHS TF@!TMP!B");
    code_print("PUSHS TF@!TMP!A");
    code_print("PUSHS TF@!TMP!B");

    //evaluate
    code_print("DEFVAR TF@!TMP!LTRES");
    code_print("LTS");
    code_print("POPS TF@!TMP!LTRES");
    code_print("EQS");
    code_print("PUSHS TF@!TMP!LTRES");
    code_print("ORS");

    //the result will be on top of the stack

    code_print("POPFRAME");
    code_print("# end operator A<=B");
}

void generate_operation_strlen(){
    code_print("# start operator #A");
    code_print("PUSHFRAME");
    code_print("CREATEFRAME");

    //define temp operands A
    code_print("DEFVAR TF@!TMP!A");
    code_print("DEFVAR TF@!TMP!RESULT");

    code_print("POPS TF@!TMP!A");

    code_print("STRLEN TF@!TMP!RESULT TF@!TMP!A");

    code_print("PUSHS TF@!TMP!RESULT");

    code_print("POPFRAME");
    code_print("# end operator #A");
}

void generate_operation_concat(){
    code_print("# start operator A..B");
    code_print("PUSHFRAME");
    code_print("CREATEFRAME");

    //define temp operands A & B
    code_print("DEFVAR TF@!TMP!A");
    code_print("DEFVAR TF@!TMP!B");
    code_print("DEFVAR TF@!TMP!RESULT");

    code_print("POPS TF@!TMP!B");
    code_print("POPS TF@!TMP!A");

    code_print("CONCAT TF@!TMP!RESULT TF@!TMP!A TF@!TMP!B");

    code_print("PUSHS TF@!TMP!RESULT");

    code_print("POPFRAME");
    code_print("# end operator A..B");
}

/**
 * *---------BUILTIN---------
 */ 

void generate_write_function(){
    generate_start_function("write");   //function write()
    generate_parameter("str");
    code_print("WRITE TF@&VAR&%s","str");  //writes it
    generate_end_function("write");         //end
}

void generate_reads_function(){
    generate_start_function("reads");
    code_print("DEFVAR TF@$TEMP$");
    code_print("READ TF@$TEMP$ string");
    generate_end_function("reads");
}

void generate_checkzero_function(){
    //->[b,a]
    generate_start_function("$OP$checkzero");   //function write()
    generate_parameter("$TEMP_CHECKZERO$");

    code_print("PUSHS float@%a",0.0f);
    code_print("PUSHS TF@&VAR&$TEMP_CHECKZERO$");
    code_print("JUMPIFNEQS $CHECKZERO$");
    code_print("EXIT int@9");
    code_print("LABEL $CHECKZERO$");
    code_print("PUSHS TF@&VAR&$TEMP_CHECKZERO$");

    generate_end_function("$OP$checkzero");         //end

}

void generate_unaryminus_function(){
    //start
    generate_start_function("$OP$unaryminus");
    generate_parameter("$TEMP$");
    //define string for type
    code_print("DEFVAR TF@$TEMP$type");
    code_print("TYPE TF@$TEMP$type TF@&VAR&$TEMP$");
    
    //generate 
    code_print("PUSHS TF@$TEMP$type");
    code_print("PUSHS string@int");
    code_print("JUMPIFNEQS $UNARY$FLOAT$");
        code_print("PUSHS int@-1");
        code_print("JUMP $UNARY$END$");
    code_print("LABEL $UNARY$FLOAT$");
        code_print("PUSHS float@%a",-1.0f);
    code_print("LABEL $UNARY$END$");
    code_print("PUSHS TF@&VAR&$TEMP$");
    code_print("MULS");
    generate_end_function("$OP$unaryminus");
}

void generate_same_types(){
    generate_start_function("$BUILTIN$sametypes");

    //get param B
    generate_parameter("$TEMP$B");
    code_print("DEFVAR TF@$TEMP$typeB");
    code_print("TYPE TF@$TEMP$typeB TF@&VAR&$TEMP$B");
    
    //get param A
    generate_parameter("$TEMP$A");
    code_print("DEFVAR TF@$TEMP$typeA");
    code_print("TYPE TF@$TEMP$typeA TF@&VAR&$TEMP$A");
    

    code_print("PUSHS TF@$TEMP$typeA");
    code_print("PUSHS TF@$TEMP$typeB");
    code_print("JUMPIFEQS $BUILTIN$sametypes$END");
    //if they are not equal convert one of them to float
    code_print("PUSHS string@int");
    code_print("PUSHS TF@$TEMP$typeA");
    code_print("JUMPIFNEQS $BTOFLOAT$");
    //CONVERT A to FLOAT
    code_print("PUSHS TF@&VAR&$TEMP$A");
    code_print("INT2FLOATS");
    code_print("POPS TF@&VAR&$TEMP$A");
    
    code_print("JUMP $BUILTIN$sametypes$END");
    code_print("LABEL $BTOFLOAT$");
    //CONVERT B to FLOAT
    code_print("PUSHS TF@&VAR&$TEMP$B");
    code_print("INT2FLOATS");
    code_print("POPS TF@&VAR&$TEMP$B");

    //end
    code_print("LABEL $BUILTIN$sametypes$END");
    code_print("PUSHS TF@&VAR&$TEMP$A");
    code_print("PUSHS TF@&VAR&$TEMP$B");
    generate_end_function("$BUILTIN$sametypes");
}


void generate_force_floats(){
    generate_start_function("$BUILTIN$forcefloats");

    //get param B
    generate_parameter("$TEMP$B");
    code_print("DEFVAR TF@$TEMP$typeB");
    code_print("TYPE TF@$TEMP$typeB TF@&VAR&$TEMP$B");
    
    //get param A
    generate_parameter("$TEMP$A");
    code_print("DEFVAR TF@$TEMP$typeA");
    code_print("TYPE TF@$TEMP$typeA TF@&VAR&$TEMP$A");
    
    //check if A is INT
    code_print("PUSHS string@int");
    code_print("PUSHS TF@$TEMP$typeA");
    code_print("JUMPIFNEQS $SKIPCONVA$");
    
    //CONVERT A to INT
    code_print("PUSHS TF@&VAR&$TEMP$A");
    code_print("INT2FLOATS");
    code_print("POPS TF@&VAR&$TEMP$A");
    code_print("LABEL $SKIPCONVA$");

    //check if B is INT
    code_print("PUSHS string@int");
    code_print("PUSHS TF@$TEMP$typeB");
    code_print("JUMPIFNEQS $SKIPCONVB$");
    
    //CONVERT B to INT
    code_print("PUSHS TF@&VAR&$TEMP$B");
    code_print("INT2FLOATS");
    code_print("POPS TF@&VAR&$TEMP$B");
    code_print("LABEL $SKIPCONVB$");

    //end
    code_print("PUSHS TF@&VAR&$TEMP$A");
    code_print("PUSHS TF@&VAR&$TEMP$B");
    generate_end_function("$BUILTIN$forcefloats");
}



const char *convert_type(sym_dtype_t dtype){
    switch (dtype)
    {
    case INT:
        return "int";
    case BOOL:
        return "bool";
    case STR:
        return "string";
    case NUM:
        return "float";
    default:
        return "nil";
        break;
    }
}


/**
 * *---------UTILITY---------
 */ 

void code_print(const char *const _Format, ...) {
    //get the arguments
    va_list args;
    va_start(args, _Format);
    //use variable argument printf
    vfprintf(stdout, _Format, args);
    fprintf(stdout,"\n");
}

string_t get_unique_name( void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner ){
    
    //get it from the token name;
    char *name = get_attr(var_id,scanner);

    //search it in the table
    tree_node_t *name_element = search_in_tables(sym_stack,symtab,name);
    if(name_element == NULL)
        fprintf(stderr,"!CODE GENERATION ERROR! ID %s not in SYMTAB!\n",name);
    string_t name_unique = name_element->data.name;
    return name_unique;

}

bool is_prefix_of(char * prefix, char * str){
    if(strlen(prefix) > strlen(str))
        return false;
    for (int i = 0; i < strlen(prefix); i++)
    {
        if(prefix[i] != str[i])
            return false;
    }
    return true;
}

char_mapping_t get_mapping(char * buffer){
    char_mapping_t mappings[] = {
        {"\n","\\010"},
        {"\n","\\010"},
    };
    int mappings_length = sizeof(mappings)/sizeof(char_mapping_t);
    for (int i = 0; i < mappings_length; i++)
    {
        char_mapping_t map = mappings[i];
        if(is_prefix_of(map.input,buffer))
            return map;
    }
    char_mapping_t null_map = {"",""};
    return null_map;
}

// void to_ascii(const char * str, string_t * out){
//     for (size_t i = 1; i < strlen(str)-1; i++)
//     {
//         char c = str[i];
//         if( c == ' '){
//             app_str(out, "\\032");
//         }else{
//             app_char(c,out);
//         }
//     }
// }

void to_ascii(const char * str, string_t * out){
    char c ='\0';
    bool escape_sequence = c;
    for (size_t i = 1; i < strlen(str)-1; i++)
    {
        bool escape_sequence_previous = false;
        c = str[i];
        if(c == ' '){
            app_str(out,"\\032");
        }else if(escape_sequence){
                escape_sequence_previous = true;
                escape_sequence = false;
            switch (c)
            {
            case 'n':
                app_str(out,"010");
                break;
            case '"':
                app_str(out,"034");
                break;
            case '\\':
                app_str(out,"092");
                break;
            case 't':
                app_str(out,"009");
                break;
            default:
                app_char(c,out);    
                break;
            }
        }else{
            app_char(c,out);
        }
        if(!escape_sequence_previous && c == 92)
            escape_sequence = true;
    }
}


/***                            End of generator.c                         ***/
