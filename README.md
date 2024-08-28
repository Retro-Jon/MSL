# Compiling MSL

## Required Software
- Make
- MingW (g++)

## Running Make
- Open your terminal and navigate to the root directory of the repository.
- Execute the following command `make release`. This will compile MSL for general use and result in a lightweight and fast executable.
    - `make debug` will compile a debug build of the interpreter with g++ debug symbols and allow you to run MSL in gdb.
    - `make performance` will include code to indicate the time spent on the following steps of execution: Lexing, Parsing, and Execution.

# Running MSL Programs
- Navigate to the root directory of the repository and enter the following `./build/msl <program path>`. Replace <program path> with the path to your MSL program.
    - If you have MSL added to your path on your system, you may enter `msl <program path>`. Replace <program path> with the path to your MSL program.

# Running MSL REPL
- Launch MSL without any arguments to enter REPL mode.
- This mode will allow you to enter commands in the language and have them execute instantly.
- While in REPL Mode, you may enter `exit` to exit the interpreter.

