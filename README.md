# IFJ21
IFJ21 compiler, that creates IFJcode21 from IFJ21 (the superset of Teal) source.

## About
This project was assigned in Formal Languages and Compilers subject (IFJ) at VUT FIT.


Final evaluation:

Lexical analysis (error detection): 94 % (164/174)

Parsing (error detection): 97 % (188/192)

Semantic analysis (error detection): 99 % (292/294)

Semantic/runtime errors (detection): 100 % (62/62)

Interpretation of compiled code (basics): 96 % (295/305)

Interpretation of compiled code (expressions, builtin functions): 95 % (189/198)

Interpretation of compiled code (complex): 72 % (247/343)


Extensions:

BOOLTHEN 0 % (0/170)

CYCLES 0 % (0/150)

FUNEXP 83 % (125/150)

OPERATORS 84 % (42/50)


Total evaluation (without extensions): 91 % (1437/1568)


## Authors
Radek Marek (xmarek77)

Vojtěch Dvořák (xdvora3o)

Juraj Dědič (xdedic07)

Tomáš Dvořák (xdvora3r)


Authors of `ic21int` are teachers in IFJ course.

## Compilation of project
Use `make` and enclosed Makefile command to build compiler. GCC is used wihtout any optimization. 
If you are going to compile program manually, please ensure that shortcut evaluation is turned on.

## Usage
After compilation you can run program by:

`./IFJ21 < your_code.tl > result.ifj`

Then you can run compiled code by `ic21int`.

## Return values
If everything goes well compiler returns `0`.

`1` - Lexical Error (e. g. invalid character)

`2` - Syntax Error

`3`-`7` - Semantic Errors

`8` - Bad usage of `nil` 

`9` - Zero division (when operation `/`, `//`, `%` are used)

Errors `8`, `9` can ocurr during compilation as well as during interpretation.
There are simple `nil` and zero division detection in compiler. To turn it off check `precedence_parser.h`. 

## Warnings
Our compiler also prints warnings to stderr, if performs implicit actions (at least some of them).
You can turn this feature off in files  `precedence_parser.h`, `parser_topdown.h`, `generator.h`.

## Files

`*.cpp` files - unit tests of compiler modules (impelemented by GoogleTest)

`*.c`, `*.h` files - source files of compiler and its header files

`Makefile` - file with target for make program (it was used during the development)

`ic21int` - interpreter of target language (IFJcode21), made by teachers in our course

`rozdeleni` - partition points in our team

`rozsireni` - list of implemented extensions of our compiler

`expr_generator.py` - simple generator of basic expressions (useful for creating test cases)

`perftest_generator.py` - generator of perfomace test cases

## Folders

`documentace` - documentation files of our project

`examples` - examples of codes in source language and test cases (compatible with https://github.com/ondrej-mach/ifjtest.git)

`grammar` - formal grammar created by us and used to build compiler

