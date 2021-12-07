/******************************************************************************
 *                                  IFJ21
 *                               generator.c
 * 
 *          Authors: Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 *              Purpose: Implementation of code generating functions
 *               For function description see correspoding .h file
 * 
 *                       Last change: 25. 11. 2021
 *****************************************************************************/

/**
 * @file generator.c
 * @brief Source file with implementation of code generator
 * @note For function description see correspoding .h file
 * 
 * @authors Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 */ 

//?done:TODO work on function calling with arguments
//TODO write tests (probably not in Google Tests)
//?done:TODO return null implicitly
//?done:todo variables assigned nil implicitly
//?done:todo checkzero for operations
//?done:todo checknil for operation
//todo builtin functions
//?done//todo cycle declaration
//?done//todo write "nil" when input nil

#include "generator.h"
#define VAR_FORMAT "TF@&VAR&"


/***                 Functions for internal represenation                  ***/

DSTACK(instr_t *, instr,)

DSTACK(prog_t, prog,)


instr_t *new_instruction() {
    instr_t *instr = NULL;

    instr = (instr_t *)malloc(sizeof(instr_t));
    if(!instr) {
        return NULL;
    }

    instr->next = NULL;
    instr->prev = NULL;

    if(str_init(&instr->content) != STR_SUCCESS) {
        free(instr);
        return NULL;
    }

    return instr;
}


int set_instruction(instr_t *instr, const char *const _Format, va_list args) {
    size_t written = 0;
    string_t *cont = &(instr->content);

    va_list args_tmp;
    va_copy(args_tmp, args); //Saving args for multiple use

    written = vsnprintf(cont->str, cont->alloc_size, _Format, args); //First attemt to store 

    while(written >= cont->alloc_size) { //Capacity of dynamic string is too small
        str_clear(cont);
        
        if(extend_string(cont) != STR_SUCCESS) {
            return INTERNAL_ERROR;
        }
    
        va_copy(args, args_tmp); //Restore args (its undefined)
        written = vsnprintf(cont->str, cont->alloc_size, _Format, args); //Next attempt
    }

    return EXIT_SUCCESS;
}


void instr_dtor(instr_t *instr) {
    instr->next = NULL;
    instr->prev = NULL;

    str_dtor(&instr->content);
    free(instr);
}


void init_new_prog(prog_t *program) {
    program->first_instr = NULL;
    program->last_instr = NULL;

    program->cycle_nest_lvl = 0;
}


void program_dtor(prog_t *program) {
    instr_t *current = program->last_instr;
    instr_t *next_deletion;

    while(current) { //Deletion of every instruction in given program from its end
        next_deletion = current->prev; 
 
        instr_dtor(current);

        current = next_deletion;
    }

    program->first_instr = NULL;
    program->last_instr = NULL;
}


instr_t *get_last(prog_t *program) {
    return program->last_instr;
}


instr_t *get_first(prog_t *program) {
    return program->last_instr;
}


instr_t *get_next(instr_t *instr) {
    return instr->next;
}


instr_t *get_prev(instr_t *instr) {
    return instr->prev;
}


int app_instr(prog_t *dst, const char *const _Format, ...) {
    //Creation of new instruction
    instr_t *new_instr = new_instruction();
    if(new_instr == NULL) {
        return INTERNAL_ERROR;
    }

    va_list args;
    va_start(args, _Format);

    if(set_instruction(new_instr, _Format, args) != EXIT_SUCCESS) {
        return INTERNAL_ERROR;
    }

    va_end(args);
    
    //Putting instruction to the end of program
    if(dst->first_instr == NULL) {
        dst->first_instr = new_instr;
    }
    else {
        new_instr->prev = dst->last_instr;
        dst->last_instr->next = new_instr;
    }

    dst->last_instr = new_instr;

    return EXIT_SUCCESS;
}


void app_prog(prog_t *dst, prog_t *prog) {
    if(get_first(dst) != NULL) { //Destination program can be empty
        
        if(get_first(prog) != NULL) { //Apended program can be empty
            prog->first_instr->prev = dst->last_instr;
        }

        dst->last_instr->next = prog->first_instr;     
    }
    else {
        dst->first_instr = prog->first_instr;
    }

    dst->last_instr = prog->last_instr;
}


int ins_after(prog_t *dst, instr_t *instr, const char *const _Format, ...) {
    if(instr == NULL) {
        return EXIT_SUCCESS;
    }

    //Creation of new instruction
    instr_t *new_instr  = new_instruction();
    if(new_instr == NULL) {
        return INTERNAL_ERROR;
    }

    va_list args;
    va_start(args, _Format);

    if(set_instruction(new_instr, _Format, args) != EXIT_SUCCESS) {
        return INTERNAL_ERROR;
    }

    va_end(args);

    instr_t * instr_after = instr->next;

    //Putting instruction inside list of instructions (program)
    instr->next = new_instr;
    new_instr->prev = instr;
    if(instr_after != NULL) {
        instr_after->prev = new_instr;
        new_instr->next = instr_after;
    }
    else {
        dst->last_instr = new_instr;
        new_instr->next = NULL;
    }

    return EXIT_SUCCESS;
}


int ins_before(prog_t *dst, instr_t *instr, const char *const _Format, ...) {
    if(instr == NULL) {
        return EXIT_SUCCESS;
    }

    instr_t *new_instr  = new_instruction();
    if(new_instr == NULL) {
        return INTERNAL_ERROR;
    }

    va_list args;
    va_start(args, _Format);

    if(set_instruction(new_instr, _Format, args) != EXIT_SUCCESS) {
        return INTERNAL_ERROR;
    }

    va_end(args);

    instr_t * instr_before = instr->prev;

    //Putting instruction inside list of instructions (program)
    instr->prev = new_instr;
    new_instr->next = instr;
    if(instr_before != NULL) {
        instr_before->next = new_instr;
        new_instr->prev = instr_before;
    }
    else {
        dst->first_instr = new_instr;
        new_instr->prev = NULL;
    }

    return EXIT_SUCCESS;
}


instr_t * fill_stack(instr_stack_t *to_be_filled, prog_t *dst, 
                     instr_t *from, instr_t *to) {

    instr_t * current = from;
    while(current) {
        if(!instr_push(to_be_filled, current)) {
            return NULL;
        }

        if(current == to) {
            break;
        }

        current = current->next;
    }

    return current;
}


void revert(prog_t *dst, instr_t *from, instr_t *to) {
    if(from == NULL || to == NULL) {
        return;
    }

    instr_stack_t stack;
    if(!instr_stack_init(&stack)) {
        return;
    }

    instr_t * instr_before = from->prev;
    instr_t * instr_after = to->next;
    
    if(!fill_stack(&stack, dst, from, to)) { //Fill stack with instructions that will be reverted
        instr_stack_dtor(&stack);
        return;
    }

    instr_t *last = NULL;
    instr_t *current = NULL;
    if(!instr_is_empty(&stack)) { //Handling instruction before
        current = instr_top(&stack);

        if(instr_before) {
            instr_before->next = instr_pop(&stack);
            current->prev = instr_before;
        }
        else {
            dst->first_instr = instr_pop(&stack);
            current->prev = NULL;
        }
    }
    while(!instr_is_empty(&stack)) //Inserts instructions from stack (in reversed order)
    {
        current->next = instr_top(&stack);
        last = current;
        current = instr_pop(&stack);
        current->prev = last;
    }
    if(current) { //Handling instruction after
        if(instr_after) {
            current->next = instr_after;
            instr_after->prev = current;
        }
        else {
            dst->last_instr = current;
            current->next = NULL;
        }
    }

    instr_stack_dtor(&stack);
}


void print_program(prog_t *source) {
    instr_t *current_instr = source->first_instr;

    while(current_instr) {
        fprintf(stdout, "%s\n", to_str(&current_instr->content));

        current_instr = current_instr->next;
    }
}

/***                 End of functions for internal representation          ***/


void generate_init(prog_t *dst){
    
    app_instr(dst,".IFJcode21");
    app_instr(dst,"CREATEFRAME");

    //builtin
    generate_write_function(dst);
    generate_reads_function(dst);
    generate_readi_function(dst);
    generate_readn_function(dst);
    generate_tointeger_function(dst);
    generate_chr_function(dst);
    generate_ord_function(dst);
    generate_substr_function(dst);
    
    //operation functions
    generate_unaryminus_function(dst);

    //custom builtin
    generate_checkzero_function_float(dst);
    generate_checkzero_function_int(dst);
    generate_checknil_function_single(dst);
    generate_checknil_function_double(dst);
    generate_same_types(dst);
    generate_force_floats(dst);
    generate_force_ints(dst);
    generate_tobool(dst);
    generate_int2num(dst);
}

/**
 * *---------FUNCTIONS---------
 */ 

void generate_start_function(prog_t *dst, const char * name){
    app_instr(dst,"\n");
    app_instr(dst,"JUMP $END_FUN$%s",name);
    app_instr(dst,"#function %s ()",name);
    app_instr(dst,"LABEL $FUN$%s", name);
    app_instr(dst,"CREATEFRAME");
}


void generate_parameters( prog_t *dst, void *sym_stack,symtab_t *symtab , void *param_names, scanner_t * scanner){

    tok_stack_t *params = param_names;
    
    while (!tok_is_empty(params))
    {
        //get it from the token name
        token_t name_token = tok_pop(params);
        string_t name_unique = get_unique_name(sym_stack, symtab,&name_token,scanner);

        generate_parameter(dst,name_unique.str);

    }

}


void generate_parameter(prog_t *dst, const char * name){
    app_instr(dst,"#define param %s",name);
    app_instr(dst,"DEFVAR %s%s",VAR_FORMAT,name); //creates temporary variable
    app_instr(dst,"POPS %s%s",VAR_FORMAT,name);   //assigns one argument from stack to temporary variable
}


void generate_end_function(prog_t *dst, const char * name){
    app_instr(dst,"\n");
    app_instr(dst,"RETURN");
    app_instr(dst,"LABEL $END_FUN$%s",name);
    app_instr(dst,"");
}


void generate_call_function(prog_t *dst, const char *name){
    app_instr(dst,"# %s()",name);
    app_instr(dst,"PUSHFRAME");
    app_instr(dst,"CALL $FUN$%s",name);
    app_instr(dst,"POPFRAME");
}

void generate_additional_returns(prog_t *dst, size_t n){
    for (size_t i = 0; i < n; i++)
    {
        app_instr(dst,"PUSHS nil@nil");
    }
}

void generate_return(prog_t *dst){
    app_instr(dst,"RETURN");
}

void generate_dump_values(prog_t *dst, size_t save_n, size_t delete_n){
    app_instr(dst, "PUSHFRAME");
    app_instr(dst, "CREATEFRAME");

    for (size_t i = 0; i < save_n; i++) //Storing save_n values from top
    {
        app_instr(dst,"DEFVAR TF@TMP_STORAGE$%ld", i);
        app_instr(dst, "POPS TF@TMP_STORAGE$%ld", i);
    }

    app_instr(dst, "DEFVAR TF@TMP_DUMP");
    for (size_t i = 0; i < delete_n; i++) //Deleting delete_n values
    {
        app_instr(dst, "POPS TF@TMP_DUMP");
    }

    for (long int i = save_n - 1; i >= 0; i--) //Pushing values back in to achieve original order
    {
        app_instr(dst, "PUSHS TF@TMP_STORAGE$%ld", i);
    }

    app_instr(dst,"POPFRAME");
}


void generate_reverse_stack(prog_t *dst, size_t n_values) {
    app_instr(dst, "PUSHFRAME");
    app_instr(dst, "CREATEFRAME");

    for (size_t i = 0; i < n_values; i++) //Storing save_n values from top
    {
        app_instr(dst,"DEFVAR TF@TMP_STORAGE$%ld", i);
        app_instr(dst, "POPS TF@TMP_STORAGE$%ld", i);
    }

    for (long int i = 0;  i < n_values; i++) //Pushing values back in reversed order
    {
        app_instr(dst, "PUSHS TF@TMP_STORAGE$%ld", i);
    }

    app_instr(dst,"POPFRAME");
}

/**
 * *---------VARIABLES---------
 */

void generate_declare_variable(prog_t *dst,  void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner){
    string_t name = get_unique_name(sym_stack,symtab,var_id,scanner);
    if(dst->cycle_nest_lvl == 0) {
        //we are not in a while loop, we append it
        app_instr(dst,"DEFVAR %s%s",VAR_FORMAT,name.str);
        app_instr(dst,"MOVE %s%s nil@nil",VAR_FORMAT,name.str);
    }
    else{ 
        //if we are in a while loop we insert it before the while loops start
        fprintf(stderr,"declaring var nest level: %li\n",dst->cycle_nest_lvl);
        ins_before(dst,instr_top(&dst->cycle_stack),"DEFVAR %s%s",VAR_FORMAT,name.str);
        ins_before(dst,instr_top(&dst->cycle_stack),"MOVE %s%s nil@nil",VAR_FORMAT,name.str);
        //will actually print them in reverse order because we are using ins_before
        //BUT it will keep MOVE after DEFVAR
    }
}


void generate_multiple_assignment(prog_t *dst,  void *sym_stack, symtab_t *symtab , void *param_names, scanner_t * scanner){
    tok_stack_t *params = param_names;
    while (!tok_is_empty(params))
    {
        //get it from the token name
        token_t name_token = tok_pop(params);
        string_t name_unique = get_unique_name(sym_stack,symtab,&name_token,scanner);
        
        generate_assign_value(dst,name_unique.str);
    }
}


void generate_assign_value(prog_t *dst, const char * name){
    app_instr(dst,"#assign value to %s",name);
    app_instr(dst,"POPS %s%s",VAR_FORMAT,name);   //assigns one argument from stack to temporary variable
}


void generate_value_push(prog_t *dst,  sym_type_t type, sym_dtype_t dtype, const char * token){
    
    if(type == VAR){
        app_instr(dst,"PUSHS %s%s",VAR_FORMAT,token);
    }else if(type == VAL){
        if(dtype == STR){
            string_t token_s;
            str_init(&token_s);
            to_ascii(token, &token_s);
            app_instr(dst,"#here");
            app_instr(dst,"PUSHS %s@%s",convert_type(dtype), token_s.str);
            
            str_dtor(&token_s);
        }else if(dtype == NUM){
            char *rest = NULL;
            double num = strtod(token, &rest);

            if(errno == ERANGE) { //Prevent inf
                if(GEN_WARNING) {
                    fprintf(stderr, "\t|\033[1;33m Warning: \033[0m");
                    fprintf(stderr, "Numeric literal '\033[1;33m%s\033[0m' is out of compilers range. It will be truncated to '%e'\n", token, DBL_MAX);
                }
                
                num = DBL_MAX;
            }

            app_instr(dst,"PUSHS %s@%a",convert_type(dtype), num);
        }else{
            app_instr(dst,"PUSHS %s@%s",convert_type(dtype), token);
        }
    }else{
        fprintf(stderr,"Code generation: Error not supported yet\n");
    }

}

/**
 * *--------CONDIONS--------
 */ 
void generate_if_condition(prog_t *dst, size_t n){
    app_instr(dst,"#if %i",n);
    //convert other types to bool
    generate_call_function(dst,"$BUILTIN$tobool");

    app_instr(dst,"PUSHS bool@true");
    app_instr(dst,"JUMPIFNEQS $ELSE$START$%i",n);
}

void generate_if_end(prog_t *dst, size_t n){
    app_instr(dst,"#end of if %i",n);
    app_instr(dst,"JUMP $ELSE$END$%i",n);
    app_instr(dst,"LABEL $ELSE$START$%i",n);
}

void generate_else_end(prog_t *dst, size_t n){
    app_instr(dst,"#end of else and the whole if %i statement",n);
    app_instr(dst,"LABEL $ELSE$END$%i",n);
}

/**
 * *LOOPS
 */ 

void generate_while_condition_beginning(prog_t *dst, size_t n){
    app_instr(dst,"#while %i, nest level %i",n,dst->cycle_nest_lvl);
    app_instr(dst,"LABEL $WHILE$COND$%i",n);
    //todo not generating other while structures
    if(dst->cycle_nest_lvl == 0){
        instr_push(&dst->cycle_stack, get_last(dst));
    }
    dst->cycle_nest_lvl++;
}

void generate_while_condition_evaluate(prog_t *dst, size_t n){
    //convert other types to bool
    generate_call_function(dst,"$BUILTIN$tobool");
    
    app_instr(dst,"PUSHS bool@true");
    app_instr(dst,"JUMPIFNEQS $WHILE$END$%i",n);
}

void generate_while_end(prog_t *dst, size_t n){
    app_instr(dst,"#end of while %i loop",n);
    app_instr(dst,"JUMP $WHILE$COND$%i",n);
    app_instr(dst,"LABEL $WHILE$END$%i",n);
    dst->cycle_nest_lvl--;

    if(!instr_is_empty(&dst->cycle_stack)) {
        instr_pop(&dst->cycle_stack);
    }
}


/**
 * *---------OPERATIONS---------
 */ 
void generate_operation_add(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"ADDS");
}

void generate_operation_sub(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"SUBS");
}

void generate_operation_mul(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"MULS");
}

void generate_operation_div(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$BUILTIN$forcefloats");
    generate_call_function(dst,"$OP$checkzero_float");
    app_instr(dst,"DIVS");
}

void generate_operation_idiv(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$OP$checkzero_int");
    app_instr(dst,"IDIVS");
}

void generate_operation_unary_minus(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_single");
    generate_call_function(dst,"$OP$unaryminus");
}

void generate_operation_eq(prog_t *dst){
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"# start operator A==B");
    app_instr(dst,"EQS");
    app_instr(dst,"# end operator A==B");
}

void generate_operation_neq(prog_t *dst){
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"# start operator A~=B");
    app_instr(dst,"EQS");
    app_instr(dst,"NOTS");
    app_instr(dst,"# end operator A~=B");
}

void generate_operation_gt(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"# start operator A>B");
    app_instr(dst,"GTS");
    app_instr(dst,"# end operator A>B");
}

void generate_operation_lt(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"# start operator A<B");
    app_instr(dst,"LTS");
    app_instr(dst,"# end operator A<B");
}

void generate_operation_gte(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"# start operator A>=B");
    app_instr(dst,"PUSHFRAME");
    app_instr(dst,"CREATEFRAME");

    //define temp operands A & B
    app_instr(dst,"DEFVAR TF@!TMP!A");
    app_instr(dst,"DEFVAR TF@!TMP!B");

    app_instr(dst,"POPS TF@!TMP!B");
    app_instr(dst,"POPS TF@!TMP!A");

    //make the stack to this form ->[B,A,B,A]
    app_instr(dst,"PUSHS TF@!TMP!A");
    app_instr(dst,"PUSHS TF@!TMP!B");
    app_instr(dst,"PUSHS TF@!TMP!A");
    app_instr(dst,"PUSHS TF@!TMP!B");

    //evaluate
    app_instr(dst,"DEFVAR TF@!TMP!LTRES");
    app_instr(dst,"GTS");
    app_instr(dst,"POPS TF@!TMP!LTRES");
    app_instr(dst,"EQS");
    app_instr(dst,"PUSHS TF@!TMP!LTRES");
    app_instr(dst,"ORS");

    //the result will be on top of the stack

    app_instr(dst,"POPFRAME");
    app_instr(dst,"# end operator A>=B");
}

void generate_operation_lte(prog_t *dst){
    generate_call_function(dst,"$OP$checknil_double");
    generate_call_function(dst,"$BUILTIN$sametypes");
    app_instr(dst,"# start operator A<=B");
    app_instr(dst,"PUSHFRAME");
    app_instr(dst,"CREATEFRAME");

    //define temp operands A & B
    app_instr(dst,"DEFVAR TF@!TMP!A");
    app_instr(dst,"DEFVAR TF@!TMP!B");

    app_instr(dst,"POPS TF@!TMP!B");
    app_instr(dst,"POPS TF@!TMP!A");
    
    app_instr(dst,"DEFVAR TF@!TYPE!A");
    app_instr(dst,"TYPE TF@!TYPE!A TF@!TMP!A");

    //make the stack to this form ->[B,A,B,A]
    app_instr(dst,"PUSHS TF@!TMP!A");
    app_instr(dst,"PUSHS TF@!TMP!B");
    app_instr(dst,"PUSHS TF@!TMP!A");
    app_instr(dst,"PUSHS TF@!TMP!B");

    //evaluate
    app_instr(dst,"DEFVAR TF@!TMP!LTRES");
    app_instr(dst,"LTS");
    app_instr(dst,"POPS TF@!TMP!LTRES");
    app_instr(dst,"EQS");
    app_instr(dst,"PUSHS TF@!TMP!LTRES");
    app_instr(dst,"ORS");

    //the result will be on top of the stack

    app_instr(dst,"POPFRAME");
    app_instr(dst,"# end operator A<=B");
}

void generate_operation_strlen(prog_t *dst){
    app_instr(dst,"# start operator #A");
    app_instr(dst,"PUSHFRAME");
    app_instr(dst,"CREATEFRAME");

    generate_call_function(dst,"$OP$checknil_single");

    //define temp operands A
    app_instr(dst,"DEFVAR TF@!TMP!A");
    app_instr(dst,"DEFVAR TF@!TMP!RESULT");

    app_instr(dst,"POPS TF@!TMP!A");

    app_instr(dst,"STRLEN TF@!TMP!RESULT TF@!TMP!A");

    app_instr(dst,"PUSHS TF@!TMP!RESULT");

    app_instr(dst,"POPFRAME");
    app_instr(dst,"# end operator #A");
}

void generate_operation_concat(prog_t *dst){
    app_instr(dst,"# start operator A..B");
    app_instr(dst,"PUSHFRAME");
    app_instr(dst,"CREATEFRAME");

    generate_call_function(dst,"$OP$checknil_double");

    //define temp operands A & B
    app_instr(dst,"DEFVAR TF@!TMP!A");
    app_instr(dst,"DEFVAR TF@!TMP!B");
    app_instr(dst,"DEFVAR TF@!TMP!RESULT");

    app_instr(dst,"POPS TF@!TMP!B");
    app_instr(dst,"POPS TF@!TMP!A");

    app_instr(dst,"CONCAT TF@!TMP!RESULT TF@!TMP!A TF@!TMP!B");

    app_instr(dst,"PUSHS TF@!TMP!RESULT");

    app_instr(dst,"POPFRAME");
    app_instr(dst,"# end operator A..B");
}

/**
 * *---------BUILTIN---------
 */ 

void generate_write_function(prog_t *dst){
    generate_start_function(dst,"write");   //function write()
    generate_parameter(dst,"str");

    app_instr(dst,"PUSHS %s%s",VAR_FORMAT,"str");
    app_instr(dst,"PUSHS nil@nil");
    app_instr(dst,"JUMPIFNEQS $CHECKNIL_WRITE$");
    app_instr(dst,"WRITE string@nil");

    app_instr(dst,"JUMP $WRITESKIP$");
    app_instr(dst,"LABEL $CHECKNIL_WRITE$");
    app_instr(dst,"WRITE %s%s",VAR_FORMAT,"str");  //writes it
    
    app_instr(dst,"LABEL $WRITESKIP$");
    generate_end_function(dst,"write");         //end
}

void generate_reads_function(prog_t *dst){
    generate_start_function(dst,"reads");
    app_instr(dst,"DEFVAR TF@$TEMP$");
    app_instr(dst,"READ TF@$TEMP$ string");
    app_instr(dst,"PUSHS TF@$TEMP$");
    generate_end_function(dst,"reads");
}

void generate_readi_function(prog_t *dst){
    generate_start_function(dst,"readi");
    app_instr(dst,"DEFVAR TF@$TEMP$");
    app_instr(dst,"READ TF@$TEMP$ int");
    app_instr(dst,"PUSHS TF@$TEMP$");
    generate_end_function(dst,"readi");
}

void generate_readn_function(prog_t *dst){
    generate_start_function(dst,"readn");
    app_instr(dst,"DEFVAR TF@$TEMP$");
    app_instr(dst,"READ TF@$TEMP$ float");
    app_instr(dst,"PUSHS TF@$TEMP$");
    generate_end_function(dst,"readn");
}

void generate_tointeger_function(prog_t *dst){
    generate_start_function(dst,"tointeger");
    //local a = a
    app_instr(dst,"DEFVAR %s$TEMP_CHECKNIL$",VAR_FORMAT);
    app_instr(dst,"POPS %s$TEMP_CHECKNIL$",VAR_FORMAT);
    //if(a != nil) 
    app_instr(dst,"PUSHS %s$TEMP_CHECKNIL$",VAR_FORMAT);
    app_instr(dst,"PUSHS nil@nil");
    app_instr(dst,"PUSHS %s$TEMP_CHECKNIL$",VAR_FORMAT);
    
    app_instr(dst,"JUMPIFNEQS $TOINTCONV$");
    app_instr(dst,"JUMP $TOINTSKIP$");
    
    app_instr(dst,"LABEL $TOINTCONV$");
    //convert
    app_instr(dst,"FLOAT2INTS");

    app_instr(dst,"LABEL $TOINTSKIP$");
    generate_end_function(dst,"tointeger");
}

void generate_chr_function(prog_t *dst){
    generate_start_function(dst,"chr");
    
    generate_call_function(dst,"$OP$checknil_single");

    //local a = a
    app_instr(dst,"DEFVAR %s$TEMP_chr$",VAR_FORMAT);
    app_instr(dst,"POPS %s$TEMP_chr$",VAR_FORMAT);
    //if(a != nil) 
    app_instr(dst,"PUSHS %s$TEMP_chr$",VAR_FORMAT);
    app_instr(dst,"PUSHS int@0");
    generate_operation_gte(dst);

    app_instr(dst,"PUSHS %s$TEMP_chr$",VAR_FORMAT);
    app_instr(dst,"PUSHS int@255",VAR_FORMAT);
    generate_operation_lte(dst);

    app_instr(dst,"JUMPIFNEQS $SKIPCONVER$");
    app_instr(dst,"PUSHS %s$TEMP_chr$",VAR_FORMAT);
    app_instr(dst,"INT2CHARS");
    app_instr(dst,"JUMP $CHARSEND$");
    app_instr(dst,"LABEL $SKIPCONVER$");
    
    app_instr(dst,"PUSHS nil@nil");

    app_instr(dst,"LABEL $CHARSEND$");
    generate_end_function(dst,"chr");
}

void generate_ord_function(prog_t *dst){
    generate_start_function(dst,"ord");
    
    generate_call_function(dst,"$OP$checknil_double");

    generate_parameter(dst, "POS");
    generate_parameter(dst, "STR");    



//     var cond1 = 1 <= pos;
//     var cond2 = pos <= str.length;

    app_instr(dst, "PUSHS int@1");
    app_instr(dst, "PUSHS %s%s",VAR_FORMAT,"POS");
    //1 <= pos
    generate_operation_lte(dst);

    app_instr(dst, "PUSHS %s%s",VAR_FORMAT,"POS");
    app_instr(dst, "PUSHS %s%s",VAR_FORMAT,"STR");
    generate_operation_strlen(dst);
    //pos <= strlen(str)
    generate_operation_lte(dst);

    app_instr(dst, "JUMPIFNEQS $ORDNIL$");
    app_instr(dst, "PUSHS %s%s",VAR_FORMAT,"STR");
    app_instr(dst, "PUSHS %s%s",VAR_FORMAT,"POS");
    app_instr(dst, "PUSHS int@1");
    app_instr(dst, "SUBS");
    app_instr(dst, "STRI2INTS");

    app_instr(dst, "JUMP $ORDEND$");
    app_instr(dst, "LABEL $ORDNIL$");
    app_instr(dst, "PUSHS nil@nil");

    app_instr(dst, "LABEL $ORDEND$");
    
    generate_end_function(dst,"ord");
}

void generate_substr_function(prog_t *dst){
    generate_start_function(dst,"substr");   //function write()
    //generate params and check for nil
    generate_call_function(dst,"$OP$checknil_double");
    generate_parameter(dst,"finish");
    generate_parameter(dst,"start");
    generate_call_function(dst,"$OP$checknil_single");
    generate_parameter(dst,"string");

    app_instr(dst,"DEFVAR TF@result");
    app_instr(dst,"MOVE TF@result string@");

    app_instr(dst,"DEFVAR TF@char");

    //save strlen
    app_instr(dst,"DEFVAR TF@strlen");
    app_instr(dst,"PUSHS %sstring",VAR_FORMAT);
    generate_operation_strlen(dst);
    app_instr(dst,"POPS TF@strlen");

    app_instr(dst,"PUSHS %sstart",VAR_FORMAT);
    app_instr(dst,"PUSHS %sfinish",VAR_FORMAT);
    generate_call_function(dst,"$BUILTIN$forceints");
    app_instr(dst,"POPS %sfinish",VAR_FORMAT);
    app_instr(dst,"POPS %sstart",VAR_FORMAT);
    
    // var cond1 = (1 <= start) && (1 <= finish) //jumpif !cond1
    // if(str.length != 0)
    //     var cond2 = (start <= str.length) && (finish <= str.length); //jumpif !cond2
    // else{
    //          var cond2 = (start == str.length + 1) && (finish == str.length + 1); //jumpif !cond2
    //          return ""
    //     }

    //start of cond1
    app_instr(dst, "PUSHS int@1");
    app_instr(dst, "PUSHS %sstart",VAR_FORMAT);
    generate_operation_lte(dst);
    
    app_instr(dst, "PUSHS int@1");
    app_instr(dst, "PUSHS %sfinish",VAR_FORMAT);
    generate_operation_lte(dst);
    app_instr(dst, "ANDS");
    app_instr(dst, "PUSHS bool@true");
    //var cond1 = (1 <= start) && (1 <= finish)
    app_instr(dst,"JUMPIFNEQS $SUBSTRNIL$");

    //cond 2 start
    app_instr(dst,"PUSHS TF@strlen");
    app_instr(dst,"PUSHS int@0");
    // if(str.length != 0) then 
    app_instr(dst,"JUMPIFEQS $SUBSTRCOND_FALSE$");
    //true branch (strlen != 0)
    
    app_instr(dst, "PUSHS %sstart",VAR_FORMAT);
    app_instr(dst, "PUSHS TF@strlen");
    generate_operation_lte(dst);
    
    app_instr(dst, "PUSHS %sfinish",VAR_FORMAT);
    app_instr(dst, "PUSHS TF@strlen");
    generate_operation_lte(dst);
    
    app_instr(dst, "ANDS");
    app_instr(dst, "PUSHS bool@true");
    app_instr(dst,"JUMPIFNEQS $SUBSTRNIL$");
    
    
    //false branch (strlen == 0)
    app_instr(dst,"JUMP $SUBSTRCOND_FALSE_END$");
    app_instr(dst,"LABEL $SUBSTRCOND_FALSE$");

    app_instr(dst, "PUSHS %sstart",VAR_FORMAT);
    app_instr(dst, "PUSHS int@1");
    generate_operation_eq(dst);
    
    app_instr(dst, "PUSHS %sfinish",VAR_FORMAT);
    app_instr(dst, "PUSHS int@1");
    generate_operation_eq(dst);
    
    app_instr(dst, "ANDS");
    app_instr(dst, "PUSHS bool@true");
    app_instr(dst,"JUMPIFNEQS $SUBSTRNIL$");

    app_instr(dst,"PUSHS string@");
    app_instr(dst,"JUMP $SUBSTREND$");
    
    app_instr(dst,"LABEL $SUBSTRCOND_FALSE_END$");

    
    //start = start - 1;
    app_instr(dst, "SUB %sstart %sstart int@1",VAR_FORMAT,VAR_FORMAT);

    //cycle start
    app_instr(dst, "LABEL $SUBSTR_CYCLE$");
    app_instr(dst, "JUMPIFEQ $SUBSTR_CYCLE_END$ %sstart %sfinish",VAR_FORMAT,VAR_FORMAT);

    //cycle body
    app_instr(dst, "GETCHAR TF@char %sstring %sstart",VAR_FORMAT,VAR_FORMAT);
    app_instr(dst, "CONCAT TF@result TF@result TF@char");

    //cycle end
    app_instr(dst, "ADD %sstart %sstart int@1",VAR_FORMAT,VAR_FORMAT);
    app_instr(dst, "JUMP $SUBSTR_CYCLE$");
    app_instr(dst, "LABEL $SUBSTR_CYCLE_END$");

    app_instr(dst, "PUSHS TF@result");

    app_instr(dst,"JUMP $SUBSTREND$");
    app_instr(dst,"LABEL $SUBSTRNIL$");
    
    app_instr(dst, "PUSHS nil@nil");

    app_instr(dst,"LABEL $SUBSTREND$");
    app_instr(dst, "BREAK");
    generate_end_function(dst,"substr");
}

void generate_checknil_function_single(prog_t *dst){
    //->[b,a]
    generate_start_function(dst,"$OP$checknil_single");   //function write()
    generate_parameter(dst,"$TEMP_CHECKNIL$");

    app_instr(dst,"PUSHS %s$TEMP_CHECKNIL$",VAR_FORMAT);
    app_instr(dst,"PUSHS %s$TEMP_CHECKNIL$",VAR_FORMAT);
    app_instr(dst,"PUSHS nil@nil");
    app_instr(dst,"JUMPIFNEQS $CHECKNIL_single$");
    app_instr(dst,"EXIT int@8");
    app_instr(dst,"LABEL $CHECKNIL_single$");

    generate_end_function(dst,"$OP$checknil_single");         //end
}

void generate_checknil_function_double(prog_t *dst){
    //->[b,a]
    generate_start_function(dst,"$OP$checknil_double");   //function write()

    //get param B
    generate_parameter(dst,"$TEMP$B");
    
    //get param A
    generate_parameter(dst,"$TEMP$A");
    
    //check if A is NIL
    app_instr(dst,"PUSHS nil@nil");
    app_instr(dst,"PUSHS %s$TEMP$A",VAR_FORMAT);

    //check if B
    app_instr(dst,"PUSHS nil@nil");
    app_instr(dst,"PUSHS %s$TEMP$B",VAR_FORMAT);
    
    //CHECK B
    app_instr(dst,"JUMPIFNEQS $SKIPEXIT1$");
    app_instr(dst,"EXIT int@8");
    app_instr(dst,"LABEL $SKIPEXIT1$");

    //CHECK A
    app_instr(dst,"JUMPIFNEQS $SKIPEXIT2$");
    app_instr(dst,"EXIT int@8");
    app_instr(dst,"LABEL $SKIPEXIT2$");
    
    //end
    app_instr(dst,"LABEL $ENDCHECKNILDOUBLE$");
    app_instr(dst,"PUSHS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"PUSHS %s$TEMP$B",VAR_FORMAT);


    generate_end_function(dst,"$OP$checknil_double");         //end
}

void generate_checkzero_function_int(prog_t *dst){
    //->[b,a]
    generate_start_function(dst,"$OP$checkzero_int");   //function write()
    generate_parameter(dst,"$TEMP_CHECKZERO$");

    app_instr(dst,"PUSHS %s$TEMP_CHECKZERO$",VAR_FORMAT);
    app_instr(dst,"PUSHS %s$TEMP_CHECKZERO$",VAR_FORMAT);
    app_instr(dst,"PUSHS int@%i",0);
    app_instr(dst,"JUMPIFNEQS $CHECKZERO_int$");
    app_instr(dst,"EXIT int@9");
    app_instr(dst,"LABEL $CHECKZERO_int$");

    generate_end_function(dst,"$OP$checkzero_int");         //end

}

void generate_checkzero_function_float(prog_t *dst){
    //->[b,a]
    generate_start_function(dst,"$OP$checkzero_float");   //function write()
    generate_parameter(dst,"$TEMP_CHECKZERO$");

    app_instr(dst,"PUSHS %s$TEMP_CHECKZERO$",VAR_FORMAT);
    app_instr(dst,"PUSHS %s$TEMP_CHECKZERO$",VAR_FORMAT);
    app_instr(dst,"PUSHS float@%a",0.0f);
    app_instr(dst,"JUMPIFNEQS $CHECKZERO_float$");
    app_instr(dst,"EXIT int@9");
    app_instr(dst,"LABEL $CHECKZERO_float$");

    generate_end_function(dst,"$OP$checkzero_float");         //end

}

void generate_unaryminus_function(prog_t *dst){
    //start
    generate_start_function(dst,"$OP$unaryminus");
    
    generate_call_function(dst,"$OP$checknil_single");

    generate_parameter(dst,"$TEMP$");
    //define string for type
    app_instr(dst,"DEFVAR TF@$TEMP$type");
    app_instr(dst,"TYPE TF@$TEMP$type %s$TEMP$",VAR_FORMAT);
    
    //generate 
    app_instr(dst,"PUSHS TF@$TEMP$type");
    app_instr(dst,"PUSHS string@int");
    app_instr(dst,"JUMPIFNEQS $UNARY$FLOAT$");
        app_instr(dst,"PUSHS int@-1");
        app_instr(dst,"JUMP $UNARY$END$");
    app_instr(dst,"LABEL $UNARY$FLOAT$");
        app_instr(dst,"PUSHS float@%a",-1.0f);
    app_instr(dst,"LABEL $UNARY$END$");
    app_instr(dst,"PUSHS %s$TEMP$",VAR_FORMAT);
    app_instr(dst,"MULS");
    generate_end_function(dst,"$OP$unaryminus");
}

void generate_same_types(prog_t *dst){
    // if(a == int && b == float)
    //     "int2floats a"
    // else if(a == float && b == int)
    //     "int2floats b"
    // else
    //     //do nothing
    // "push a"
    // "push b"

    generate_start_function(dst,"$BUILTIN$sametypes");

    //get param B
    generate_parameter(dst,"$TEMP$B");
    app_instr(dst,"DEFVAR TF@$TEMP$typeB");
    app_instr(dst,"TYPE TF@$TEMP$typeB %s$TEMP$B",VAR_FORMAT);
    
    //get param A
    generate_parameter(dst,"$TEMP$A");
    app_instr(dst,"DEFVAR TF@$TEMP$typeA");
    app_instr(dst,"TYPE TF@$TEMP$typeA %s$TEMP$A",VAR_FORMAT);
    
    //if a == int && b === float then int2floats a
    app_instr(dst,"PUSHS TF@$TEMP$typeA");
    app_instr(dst,"PUSHS string@int");
    app_instr(dst,"EQS");
    app_instr(dst,"PUSHS TF@$TEMP$typeB");
    app_instr(dst,"PUSHS string@float");
    app_instr(dst,"EQS");
    
    app_instr(dst,"ANDS");
    app_instr(dst,"PUSHS bool@true");
    app_instr(dst,"JUMPIFEQS $ATOFLOAT$");

    //if a == int && b === float then int2floats b
    app_instr(dst,"PUSHS TF@$TEMP$typeA");
    app_instr(dst,"PUSHS string@float");
    app_instr(dst,"EQS");
    app_instr(dst,"PUSHS TF@$TEMP$typeB");
    app_instr(dst,"PUSHS string@int");
    app_instr(dst,"EQS");
    
    app_instr(dst,"ANDS");
    app_instr(dst,"PUSHS bool@true");
    app_instr(dst,"JUMPIFEQS $BTOFLOAT$");


    app_instr(dst,"JUMP $BUILTIN$sametypes$END");
    //CONVERT A to FLOAT
    app_instr(dst,"LABEL $ATOFLOAT$");
    app_instr(dst,"PUSHS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"INT2FLOATS");
    app_instr(dst,"POPS %s$TEMP$A",VAR_FORMAT);
    
    app_instr(dst,"JUMP $BUILTIN$sametypes$END");
    app_instr(dst,"LABEL $BTOFLOAT$");
    //CONVERT B to FLOAT
    app_instr(dst,"PUSHS %s$TEMP$B",VAR_FORMAT);
    app_instr(dst,"INT2FLOATS");
    app_instr(dst,"POPS %s$TEMP$B",VAR_FORMAT);

    //end
    app_instr(dst,"LABEL $BUILTIN$sametypes$END");
    app_instr(dst,"PUSHS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"PUSHS %s$TEMP$B",VAR_FORMAT);
    generate_end_function(dst,"$BUILTIN$sametypes");
}


void generate_force_floats(prog_t *dst){
    generate_start_function(dst,"$BUILTIN$forcefloats");

    //get param B
    generate_parameter(dst,"$TEMP$B");
    app_instr(dst,"DEFVAR TF@$TEMP$typeB");
    app_instr(dst,"TYPE TF@$TEMP$typeB %s$TEMP$B",VAR_FORMAT);

    //get param A
    generate_parameter(dst,"$TEMP$A");
    app_instr(dst,"DEFVAR TF@$TEMP$typeA");
    app_instr(dst,"TYPE TF@$TEMP$typeA %s$TEMP$A",VAR_FORMAT);
    
    //check if A is INT
    app_instr(dst,"PUSHS string@int");
    app_instr(dst,"PUSHS TF@$TEMP$typeA");
    app_instr(dst,"JUMPIFNEQS $SKIPCONVA2FLOAT$");
    
    //CONVERT A to FLOAT
    app_instr(dst,"PUSHS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"INT2FLOATS");
    app_instr(dst,"POPS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"LABEL $SKIPCONVA2FLOAT$");

    //check if B is INT
    app_instr(dst,"PUSHS string@int");
    app_instr(dst,"PUSHS TF@$TEMP$typeB");
    app_instr(dst,"JUMPIFNEQS $SKIPCONVB2FLOAT$");
    
    //CONVERT B to FLOAT
    app_instr(dst,"PUSHS %s$TEMP$B",VAR_FORMAT);
    app_instr(dst,"INT2FLOATS");
    app_instr(dst,"POPS %s$TEMP$B",VAR_FORMAT);
    app_instr(dst,"LABEL $SKIPCONVB2FLOAT$");

    //end
    app_instr(dst,"PUSHS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"PUSHS %s$TEMP$B",VAR_FORMAT);
    generate_end_function(dst,"$BUILTIN$forcefloats");
}

void generate_force_ints(prog_t *dst){
    generate_start_function(dst,"$BUILTIN$forceints");

    //get param B
    generate_parameter(dst,"$TEMP$B");
    app_instr(dst,"DEFVAR TF@$TEMP$typeB");
    app_instr(dst,"TYPE TF@$TEMP$typeB %s$TEMP$B",VAR_FORMAT);

    //get param A
    generate_parameter(dst,"$TEMP$A");
    app_instr(dst,"DEFVAR TF@$TEMP$typeA");
    app_instr(dst,"TYPE TF@$TEMP$typeA %s$TEMP$A",VAR_FORMAT);
    
    //check if A is FLOAT
    app_instr(dst,"PUSHS string@float");
    app_instr(dst,"PUSHS TF@$TEMP$typeA");
    app_instr(dst,"JUMPIFNEQS $SKIPCONVA$2INT");
    
    //CONVERT A to INT
    app_instr(dst,"PUSHS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"FLOAT2INTS");
    app_instr(dst,"POPS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"LABEL $SKIPCONVA$2INT");

    //check if B is FLOAT
    app_instr(dst,"PUSHS string@float");
    app_instr(dst,"PUSHS TF@$TEMP$typeB");
    app_instr(dst,"JUMPIFNEQS $SKIPCONVB$2INT");
    
    //CONVERT B to INT
    app_instr(dst,"PUSHS %s$TEMP$B",VAR_FORMAT);
    app_instr(dst,"FLOAT2INTS");
    app_instr(dst,"POPS %s$TEMP$B",VAR_FORMAT);
    app_instr(dst,"LABEL $SKIPCONVB$2INT");

    //end
    app_instr(dst,"PUSHS %s$TEMP$A",VAR_FORMAT);
    app_instr(dst,"PUSHS %s$TEMP$B",VAR_FORMAT);
    generate_end_function(dst,"$BUILTIN$forceints");
}

void generate_tobool(prog_t *dst){

    generate_start_function(dst,"$BUILTIN$tobool");
    generate_parameter(dst, "TMP");


    app_instr(dst, "DEFVAR TF@$TYPE$");

    app_instr(dst, "TYPE TF@$TYPE$ %sTMP",VAR_FORMAT);
    
    app_instr(dst, "PUSHS TF@$TYPE$");
    app_instr(dst, "PUSHS string@bool");
    app_instr(dst, "JUMPIFEQS $toboolORIGINAL$");

    app_instr(dst, "PUSHS TF@$TYPE$");
    app_instr(dst, "PUSHS string@nil");
    app_instr(dst, "JUMPIFEQS $toboolFALSE$");
    
    //return true
    app_instr(dst, "PUSHS bool@true");
    app_instr(dst, "JUMP $toboolEND$");

    //return original value
    app_instr(dst, "LABEL $toboolORIGINAL$");
    app_instr(dst, "PUSHS %sTMP",VAR_FORMAT);
    app_instr(dst, "JUMP $toboolEND$");
    
    //return false
    app_instr(dst, "LABEL $toboolFALSE$");
    app_instr(dst, "PUSHS bool@false");

    app_instr(dst, "LABEL $toboolEND$");
    generate_end_function(dst,"$BUILTIN$tobool");
}

void generate_int2num(prog_t *dst_code) {
    generate_start_function(dst_code, "$IMPLICIT$int2num");
    generate_parameter(dst_code, "TMP");

    app_instr(dst_code, "DEFVAR TF@$TYPE$");
    
    app_instr(dst_code, "TYPE TF@$TYPE$ %sTMP", VAR_FORMAT);
    
    app_instr(dst_code, "PUSHS %sTMP", VAR_FORMAT);
    app_instr(dst_code, "PUSHS TF@$TYPE$");
    app_instr(dst_code, "PUSHS string@nil");
    app_instr(dst_code, "JUMPIFEQS $int2numSKIPCONVERSION$");

    app_instr(dst_code, "INT2FLOATS");

    app_instr(dst_code, "LABEL $int2numSKIPCONVERSION$");


    generate_end_function(dst_code, "$IMPLICIT$int2num");
}


/**
 * *---------VARIOUS---------
 */ 


void impl_int2num(prog_t *dst_code) {
    app_instr(dst_code, "PUSHFRAME");
    app_instr(dst_code, "CALL $FUN$$IMPLICIT$int2num");
    app_instr(dst_code, "POPFRAME");
}


void generate_clears(prog_t *dst_code) {
    app_instr(dst_code,"CLEARS");
}


/**
 * *---------UTILITY---------
 */ 

void code_print(prog_t *dst, const char *const _Format, ...) {
    //get the arguments
    va_list args;
    va_start(args, _Format);
    vfprintf(stderr, _Format, args);

    // app_instr(dst,_Format,args);
    va_end(args);
    //use variable argument printf
    // fprintf(stdout,"\n");
}

string_t get_unique_name( void *sym_stack,symtab_t *symtab , token_t *var_id, scanner_t * scanner ){
    
    //get it from the token name;
    char *name = get_attr(var_id,scanner);

    //search it in the table
    tree_node_t *name_element = deep_search(sym_stack, symtab, name);
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



/***                            End of generator.c                         ***/
