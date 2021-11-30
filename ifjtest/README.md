# ifjtest

Testing suite for IFJ project, VUT FIT.

### Installation

- Clone the repository to your linux machine
- Edit the `config` file to match your project structure
- Set the `COMPILER` to path to you executable. This is mandatory.
- Set the `MAKE_DIR` to directory, where you can execute make to build your project. 
This is optional, but but convenient. Your project will build automatically before the testing.

### Running the script

To run the test, just type:
```
./ifjtest
```
The output might look something like:
```
✓ Ondroid 001
✓ Ondroid 002
✗ Ondroid 003
Invalid assignment of float value to integer variable.
Program was supposed to fail, but with different exit code
Expected return: 5
Compiler return: 0
Interpreter return: 53
✓ Ondroid 004
✓ Ondroid 005
✓ Ondroid 006
✓ Ondroid 007

✗ SOME TESTS FAILED (6/7 passed)
```
To show description of all tests, run
```
./ifjtest show
```
To create your own test interactively, run
```
./ifjtest generate "YourName 001"
```
The test will ask you to enter the program to run. Then it will ask for your simple description. After you have entered the description, script will start generating the reference outputs.

Note: If you do this on merlin server, default editor is set to vim. Run `export EDITOR=nano` to set the environment variable.

This command can also be used to generate new reference output.

### File structure

If you take look into the `test_cases` directory, you can see all the tests. Each has its own directory with these files:
```
description.txt
input
output
program.tl
return
```
- `description.txt` is a short explanation of the test. 
- `program.tl` is the source code, that gets compiled with your compiler. 
- If the compilation or interpretation of this code should fail, the return code will be checked against `return` file. 
- If the compilation is successful, program is then run with interpreter with `input` file as stdin. 
- If the interpreter finishes successfully, its output is then checked against `output file`.

All the temporary files and stderr outputs are stored in `./tmp` folder by default


### Contributing

There are not many test cases yet. If you write some of your own, please make a pull request, so others can try them as well. Please follow the naming scheme. When creating a test case, name it `YOUR_NAME 3_DIGIT_NUMBER` (for example `Ondroid 420`). Keep your descriptions short and simple - one line and ideally under 80 characters.
