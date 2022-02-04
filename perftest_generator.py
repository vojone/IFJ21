#Generator of huge amount of functions and its declarations to make
#performance tests of compiler (especially symbol table)

import array as arr
import random

print("require \"ifj\"")
print("")

number_of_func_declarations = 10000
number_of_only_def_functions = 100
max_num_of_returns = 100
max_num_of_params = 200
max_num_of_locals = 200
data_types = ["integer", "nil", "string", "number"]


fname = "a"
#Function declaration generator
functions = []
for i in range(0, number_of_func_declarations):
    function = []
    print("global", fname, ": function(", end="")
    function.append(fname)

    num_of_params = random.randint(0, max_num_of_params)
    function_p = []
    #Parameter generator
    for u in range(0, num_of_params):
        if u != 0:
            print(", ", end="")

        cur_type = random.choice(data_types)
        print(cur_type, end="")
        function_p.append(cur_type)

    print(")", end="")
    function.append(function_p)

    #Generator of return values
    num_of_ret = random.randint(0, max_num_of_returns)
    function_r = []
    for u in range(0, num_of_ret):
        if u != 0:
            print(", ", end="")
        else:
            print(" : ", end="")

        cur_type = random.choice(data_types)
        print(cur_type, end="")
        function_r.append(cur_type)
    
    function.append(function_r)

    #Local variables generator
    num_of_locals = random.randint(0, max_num_of_locals)
    function_l = []
    l_index = 0
    for u in range(0, num_of_locals):
        cur_loc = []
        cur_name = "locvar" + str(l_index)
        cur_type = random.choice(data_types)
        cur_loc.append(cur_name)
        cur_loc.append(cur_type)
        l_index += 1
        function_l.append(cur_loc)

    function.append(function_l)

    #Generating new func name
    new_fname = ""
    char_cnt = 1
    for c in fname:
        if ord(c) < ord("z"):
            c = chr(ord(c) + 1)
            new_fname += c
        else:
            new_fname += c
            if char_cnt == len(fname):
                new_fname = fname + "a"

        char_cnt += 1

    fname = new_fname

    #Saving function with its paramters and returns for print definition and fcall
    functions.append(function)

    print("")

print("\n\n")

#Generator of function definitions
for func in functions:
    if random.randint(0,1) == 1 and number_of_only_def_functions > 0:
        print("function f" + str(number_of_only_def_functions) + "() \nend \n")
        print("")
        number_of_only_def_functions -= 1

    print("function " + func[0] + "(", end="")

    param_ind = 0
    for param in func[1]:
        if param_ind != 0:
            print(", ", end="")

        print("p" + str(param_ind) + " : ", end="")
        print(param, end="")
        param_ind += 1

    print(")", end="")
    
    return_ind = 0
    for return_val in func[2]:
        if return_ind != 0:
            print(", ", end="")
        else:
            print(" : ", end="")

        print(return_val, end="")
        return_ind += 1

    print("")

    for local in func[3]:
        print("local " + local[0] + " : " + local[1])

    print("end")
    print()

#Define the rest of only defined functions
while number_of_only_def_functions > 0:
    print("function f" + str(number_of_only_def_functions) + "() \nend \n")
    number_of_only_def_functions -= 1
