/**
 * @file parser-topdown.h
 * @brief Header file for recursive descent parser
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dstring.h"
#include "scanner.h"

int main();

void scanner_init();

void statement_list();
void statement();

void value_assigment();
void assigment();

void multiple_assigment();
void else_branch();

void loop_while();

void param_list();
void param_list_1();

void type_list();
void type_list_1();

void expression_list();
void expression_list_1();