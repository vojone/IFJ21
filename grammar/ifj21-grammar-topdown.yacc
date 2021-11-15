/* ******************************************************************************
 *                                  IFJ21
 *                        ifj21-grammar-topdown.xml
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 * Purpose: File for grammar definition for recursive descent parsing 
 * 
 *                    Last change: 14. 11. 2021
 *****************************************************************************

 * @file ifj21-grammar-topdown.xml
 * @brief File for grammar definition for recursive descent parsing
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak */
 
%token string type expression id
%% /* LL(1) */


S : GlobalStatementList;  /*Only require and functions basically*/


GlobalStatementList : 
                    | GlobalStatement GlobalStatementList;
GlobalStatement     : 'require' string
                    | FunctionCall
                    | 'function' id '(' ParamList ')' TypeList StatementList 'end';


StatementList       : 
                    | Statement StatementList;
/* VARIABLES */
Statement           : 'global' id ':' type ValueAssignment
                    | 'local'  id ':' type ValueAssignment
                    | Assignment
                    | 'if' expression 'then' StatementList ElseBranch 'end'
                    | 'while' expression 'do' StatementList 'end'
                    | FunctionCall;

/*Assignment when declaring*/
ValueAssignment     : 
                    | '=' expression;

/* Assignment to variable(s) */
Assignment          : IdList '=' ExpressionList;

IdList      :  id IdList1;
IdList1     : 
            | ',' id IdList1;


/* multiple-assignment   : = 
multiple-assignment   : , id multiple-assignment expression , */

/*ELSE*/

ElseBranch          :
                    | 'else' StatementList;           

/*FUNCTIONS CALL*/
FunctionCall        : id '(' ArgumentList ')';
ArgumentList        : 
                    | argument ArgumentList1 ;
ArgumentList1       : 
                    | ',' argument ArgumentList1;
argument            : expression;

/*FUNCTIONS DEFINITION*/
FunctionTypeDef     :
                    | TypeList;

/*Functions parameters*/
ParamList           : id ':' type ParamList1;
ParamList1          : 
                    | ',' id ':' type ParamList1;

/*Function types*/
TypeList            : 
                    | ':' type TypeList1;
TypeList1           : 
                    | ',' type TypeList1;

/*Function return*/
ReturnStatement     : 'return' ExpressionListVoluntary;

ExpressionListVoluntary : 
                        | ExpressionList;

/* Expression lists are used in RETURN or in ASSIGNMENT */
ExpressionList      :  expression ExpressionList1;
ExpressionList1     : 
                    | ',' expression ExpressionList1;