/*
******************************************************************************
 *                                  IFJ21
 *                        ifj21-grammar-bottomup.xml
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 * Purpose: File for grammar definition for bottom-up expression parsing
 * 
 *                    Last change: 14. 11. 2021
 *****************************************************************************

 * @file ifj21-grammar-bottomup.xml
 * @brief File for grammar definition for bottom-up expression parsing
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
*/

/* 'i' is a token (terminal).*/
%token i identifier
%% /* LL(1) */

E   : '(' E ')'
    | i

/*Arithmetic rules*/
    | E '+' E
    | E '-' E
    | E '*' E
    | E '/' E


/*Logical rules*/
/* E     : ! E */
    | E '<' E
    | E '>' E
    | E '<=' E
    | E '>=' E
    | E '==' E
    | E '~=' E

/*Concat rule*/
    | E '..' E

/*String length rule*/
    | '#'E;

/*Rules for function call parsing*/
    | identifier '(' ArgumentList ')' ;
ArgumentList    :
                | Argument ArgumentList;
Argument : E