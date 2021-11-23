#include "generator.h"

int main(){
    generate_init();

    //generate prebuild functions

    //function main(a,b)
    generate_start_function("main");
    const char *p[] = {"a","b"};
    generate_parameters("a&b&",2);

    generate_value_push(VAR,INT,"a");
    generate_value_push(VAR,INT,"b");
    generate_operation(ADD);

    generate_call_function("write");

    generate_end_function("main");

    // generate_main();
    //anotha function

    generate_value_push(VAL,INT,"10");
    generate_value_push(VAL,INT,"5");
    generate_call_function("main");
    
}