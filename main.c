/******************************************************************************
 *                                  IFJ21
 *                                  main.c
 * 
 *          Authors: Radek Marek (xmarek77), Vojtěch Dvořák (xdvora3o), 
 *                   Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 * 
 *                   Purpose: Main function of compiler
 * 
 *                    Last change: 25. 11. 2021
 *****************************************************************************/

/**
 * @file main.c
 * @brief Main function of compiler
 * 
 * @authors Radek Marek (xmarek77), Vojtěch Dvořák (xdvora3o), 
 *          Juraj Dědič (xdedic07), Tomáš Dvořák (xdvora3r)
 */

#include "parser_topdown.h"
#include "scanner.h"

/**
 * @brief User for custom input into the parser
 */ 

int main() {
    scanner_t scanner;
    if(scanner_init(&scanner) != EXIT_SUCCESS) {
        return INTERNAL_ERROR;
    }

    parser_t parser;
    parser_setup(&parser, &scanner);

    int return_value = parse_program(&parser);

    scanner_dtor(&scanner);
    
    return return_value;
}


/**************************       End of main.c      *************************/
