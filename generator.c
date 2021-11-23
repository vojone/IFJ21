/******************************************************************************
 *                                  IFJ21
 *                               generator.c
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *              Purpose: Implementation of code generating functions
 * 
 *                    Last change:
 *****************************************************************************/

/**
 * @file generator.c
 * @brief Source file with implementation of code generator
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

//TODO work on function calling with arguments
//TODO write tests (probably not in Google Tests)

#include "generator.h"

void generate_init(){
    code_print(".IFJcode21");

    generate_write_function();
}

void generate_ending(){

}


void generate_start_function(const char * name){
    code_print("\n");
    code_print("JUMP $END_FUN$%s",name);
    code_print("#function $FUN$%s ()",name);
    code_print("label $FUN$%s", name);
    code_print("CREATEFRAME");
    code_print("\n");
}

/*
void generate_parameters(string_t params, int param_count){
    for (int i = param_count-1; i >= 0; i--)
    {
        generate_parameter(params[i]);
    }
}
*/

void generate_parameter(const char * name){
    code_print("DEFVAR TF@&VAR&%s",name); //creates temporary variable
    code_print("POPS TF@&VAR&%s",name);   //assigns one argument from stack to temporary variable
}

void generate_end_function(const char * name){
    code_print("\n");
    code_print("CREATEFRAME");  
    code_print("RETURN");
    code_print("label $END_FUN$%s",name);
}

void generate_write_function(){
    generate_start_function("write");   //function write()
    // code_print("DEFVAR TF@!WRITE_VAR"); //creates temporary variable
    // code_print("POPS TF@!WRITE_VAR");   //assigns one argument from stack to temporary variable
    generate_parameter("str");
    code_print("WRITE TF@&VAR&%s","str");  //writes it
    generate_end_function("write");         //end
}

void generate_declare_variable(const char * name){
    code_print("DEFVAR TF@&VAR&%s",name );
}

//a = 1+2+3
void generate_value_push( sym_type_t type, sym_dtype_t dtype, const char * name ){
    if(type == VAR){
        code_print("PUSHS TF@&VAR&%s",name);
    }else if(type == VAL){
        code_print("PUSHS %s@%s",convert_type(dtype), name);
    }else{
        fprintf(stderr,"Code generation: Error not supported yet\n");
    }
}

void generate_operation(grm_sym_type_t type){
    switch (type)
    {
    case ADD:
        code_print("ADDS");
        break;
    case SUB:
        code_print("SUBS");
        break;
    case MULT:
        code_print("MULS");
        break;
    case DIV:
        code_print("DIV");
        break;
    case INT_DIV:
        code_print("IDIV");
        break;
    case LTE:
        //stack: [a,b,..]
        //temporary variables
        //stack: [a,b] (LT) => bool
        //stack: [a,b] (EQ) => bool
        //RES?: LT OR EQ
        fprintf(stderr,"Code generator: Error operation not supported yet!\n");
    default:
        fprintf(stderr,"Code generator: Error operation not supported yet!\n");
        break;
    }
}

//assumes that the result of the expression is at the top of the stack
void generate_assign_variable( const char *name ){
    //get the type from symtab
    code_print("#%s = <top of the stack>",name);
    code_print("POPS TF@&VAR&%s",name);
    code_print("CLEARS");
}


// void generate_main(){
//     code_print("\n#internal main function\n");
//     code_print("LABEL $main");
// }


void generate_call_function(const char *name){
    code_print("# %s()",name);
    code_print("CALL $FUN$%s",name);
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

void code_print(const char *const _Format, ...) {
    //get the arguments
    va_list args;
    va_start(args, _Format);
    //use variable argument printf
    vfprintf(stdout, _Format, args);
    fprintf(stdout,"\n");
}

void to_ascii(const char * str, string_t * out){
    int i = 0;
    char c = str[i];
    str_init(out);
    while (c != '\0')
    {
        if(c == ' '){
            
        }
        i++;
        c = str[i];
    }
}



/***                            End of generator.c                         ***/
