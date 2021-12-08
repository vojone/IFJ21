# IFJ21
IFJ21 compiler, that creates IFJcode21 from IFJ21 (the superset of Teal) source.

## Authors
Radek Marek (xmarek77)

Vojtěch Dvořák (xdvora3o)

Juraj Dědič (xdedic07)

Tomáš Dvořák (xdvora3r)

## Compilation (of compiler)
Use `make` and enclosed Makefile command to build compiler. GCC is used wihtout any optimization. 
If you are going to compile program manually, please endure that shortcut evaluation is turned on.

## Usage
`./IFJ21 < your_code.tl > result.ifj`

## Return values
If everything goes well compiler returns `0`.

`1` - Lexical Error (e. g. invalid character)

`2` - Syntax Error

`3`-`7` - Semantic Errors

`8` - Bad usage of `nil` 

`9` - Zero division (when operation `/`, `//`, `%` are used)

Errors `8`, `9` can ocurrs during compilation as well as during interpretation.
There are simple `nil` and zero division detection in compiler. To turn it off check `precedence_parser.h`. 

## Warnings
Our compiler also prints warnings to stderr, if performs implicit actions (at least some of them).
You can turn this feature off in files  `precedence_parser.h`, `parser_topdown.h`, `generator.h`.
