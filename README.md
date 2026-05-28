# RAT Interpreter

RAT Interpreter is a small learning project where I built a tiny programming
language interpreter in C. The main point was not to create a production
language, but to understand what happens between a text file full of code and a
program that actually runs.

I used this project to learn the classic interpreter pipeline:

1. Turn raw source text into tokens.
2. Parse those tokens into a structured tree.
3. Walk that tree and evaluate the program.

That means this repo is partly an interpreter and partly a set of notes in code
form. The comments in `RatInterpret.c` are intentionally detailed because I was
using the implementation to learn how tokenisers, parsers, ASTs, symbol tables,
and evaluation passes fit together.

## What It Supports

The language currently supports:

- integer values
- string literals for printing
- variables and assignment
- arithmetic with `+`, `-`, `*`, and `/`
- comparisons with `==`, `!=`, `<`, `>`, `<=`, and `>=`
- `print` statements
- `if` / `else` blocks
- `while` loops
- function definitions
- function calls
- `return` statements
- recursion

Here is the sample program in `test.txt`:

```rat
func rat(n) {
    if (n <= 1) {
        return n;
    } else {
        return rat(n-1) + rat(n-2);
    }
}

print rat(1);
```

## How The Interpreter Works

The project is organised around three main passes.

### 1. Lexer / Tokeniser

The lexer reads the source code one character at a time and groups it into
tokens. For example:

```rat
x = 5 + 3;
```

becomes roughly:

```text
IDENTIFIER(x) ASSIGN NUMBER(5) PLUS NUMBER(3) SEMICOLON
```

This was the first useful lesson: before code can be parsed, it has to be
split into meaningful pieces. The lexer handles things like numbers,
identifiers, keywords, operators, braces, parentheses, strings, and the end of
the input.

### 2. Parser / AST Builder

The parser consumes tokens and builds an Abstract Syntax Tree. I used a
recursive descent parser, where each function maps to part of the grammar.

For example, arithmetic is split into precedence levels:

- `factor` handles numbers, strings, variables, grouped expressions, and
  function calls
- `term` handles multiplication and division
- `comparison` handles addition and subtraction
- `expr` handles comparison operators

That structure helped me understand how interpreters avoid reading expressions
from left to right blindly. The parser gives `*` and `/` higher precedence than
`+` and `-`, and it builds nested binary operation nodes to represent that
structure.

### 3. Evaluator / Tree Walker

The evaluator walks the AST and executes each node. Assignment updates the
symbol table, `print` evaluates and displays an expression, `if` chooses a
branch, `while` repeats a block, and function calls look up function
definitions before evaluating their bodies.

This pass made the earlier stages click for me. The lexer does not run the
program, and the parser does not calculate results. They just prepare the data
structure that the evaluator can walk.

## Build And Run

This is a CMake project, so it can be built from the command line:

```sh
cmake -S . -B build
cmake --build build
```

Then run it with the sample program:

```sh
./build/RAT_interpreter test.txt
```

You can also compile the single C file directly:

```sh
cc -std=c11 RatInterpret.c -o RAT_interpreter
./RAT_interpreter test.txt
```

Expected output for the current `test.txt` is:

```text
1
```

## Example Syntax

Variables:

```rat
x = 10;
y = x * 2;
print y;
```

Conditionals:

```rat
if (x > 5) {
    print x;
} else {
    print 0;
}
```

Loops:

```rat
i = 0;
while (i < 3) {
    print i;
    i = i + 1;
}
```

Functions:

```rat
func add(a, b) {
    return a + b;
}

print add(2, 3);
```

## Learning Notes

Some of the most useful things I learned while building this:

- Tokenising is its own real phase, not just a helper function.
- Parser design becomes easier when the grammar is split into small functions.
- Operator precedence is easier to model by parsing expressions in layers.
- An AST is a bridge between syntax and execution.
- A tree-walking interpreter is slow compared with compiled code, but it is a
  great way to understand language implementation.
- Functions need more than just a name and a body. They also need argument
  binding, return handling, and some idea of scope.
- Even a tiny language quickly raises real language design questions.

## Current Limitations

This is deliberately small and experimental. A few rough edges are still part
of the learning process:

- values are mostly integers
- strings are only useful in limited print scenarios
- there is no garbage collection or full memory cleanup
- arrays, booleans, comments, and richer types are not implemented
- error messages are simple
- scope handling is basic
- fixed-size arrays are used for variables, functions, arguments, and blocks

Those limitations are useful because they show where a toy interpreter starts
turning into a bigger language runtime.

## Project Files

- `RatInterpret.c` contains the lexer, parser, AST definitions, symbol tables,
  evaluator, file loading, and program entry point.
- `test.txt` contains a small RAT program that exercises functions, recursion,
  conditionals, returns, and printing.
- `CMakeLists.txt` defines the CMake build target.

## Why This Exists

I made this as a learning exercise: to get closer to the machinery behind
programming languages instead of only using them from the outside.

The project helped me see how source code moves through a series of passes,
with each pass adding structure:

```text
source code -> tokens -> AST -> evaluated result
```

That pipeline is the main idea of the repo.
