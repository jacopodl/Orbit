# LiftOff: The Orbit Compiler

LiftOff is the default implementation of the compiler for the Orbit programming language, supporting all the language features.

## Project Structure

The LiftOff compiler is divided into three main components:

- **Scanner**: Responsible for lexical analysis, breaking down the source code into tokens.
- **Parser**: Performs syntactic analysis, constructing an Abstract Syntax Tree (AST) from the tokens.
- **Compiler**: Generates the final executable code from the AST.

The `grammar.ebnf` file in the directory contains the formal grammar specification for the Orbit language.
