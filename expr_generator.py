import random as rand
import array as arr

max_appr_length = 500

rules = [
    ["i"],
    ["(E)"],
    ["E+E"],
    ["E-E"],
    ["E*E"],
    ["E/E"],
    #["E//E"]
]

expression = ""

expression += rand.choice(rules)[0]

only_terminals = False
while not only_terminals:
    if len(expression) > max_appr_length:
        expression = expression.replace("E", rules[0][0])

    if expression.find("E") >= 0:
        expression = expression.replace("E", rand.choice(rules)[0], 1)
    else:
        only_terminals = True

only_imm_values = False
while not only_imm_values:
    if expression.find("i") >= 0:
        expression = expression.replace("i", str(rand.randint(1, 100)), 1)
    else:
        only_imm_values = True
    

print(expression)


print(float.hex(float(eval(expression))))

