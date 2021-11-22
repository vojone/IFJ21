/******************************************************************************
 *                                  IFJ21
 *                               generator.h
 * 
 *          Authors: Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 *                   Purpose: Header file of code generator
 * 
 *                    Last change:
 *****************************************************************************/

/**
 * @file generator.h
 * @brief Header file of code generator
 * 
 * @authors Radek Marek, Vojtech Dvorak, Juraj Dedic, Tomas Dvorak
 */ 

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dstring.h"
#include "symtable.h"
#include "scanner.h"

typedef struct gen {
    void *(declare_variable);
} gen_t;

void declare_variable(const char * name, char dtype);

#endif

/***                            End of generator.h                         ***/
