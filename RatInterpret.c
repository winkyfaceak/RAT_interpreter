/*
 * CUSTOM PROGRAMMING LANGUAGE RAT INTERPRETER
 *
 * This interpreter implements a complete programming language with:
 * - Variables and arithmetic
 * - If/else conditionals
 * - While loops
 * - Functions with parameters and return values
 * - Comparison operators
 *
 * The interpreter works in three phases:
 * 1. LEXER: Converts source code text into tokens
 * 2. PARSER: Builds an Abstract Syntax Tree (AST) from tokens
 * 3. EVALUATOR: Walks the AST and executes the program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* ============================================================================
 * TOKEN DEFINITIONS
 * ============================================================================
 */

typedef enum {
    TOK_NUMBER,      // Numbers like 42, 123
    TOK_IDENTIFIER,  // Variable/function names like x, myVar
    TOK_STRING,      // String literals like "hello"

    // Operators
    TOK_ASSIGN,      // =
    TOK_PLUS,        // +
    TOK_MINUS,       // -
    TOK_MUL,         // *
    TOK_DIV,         // /

    // Delimiters
    TOK_LPAREN,      // (
    TOK_RPAREN,      // )
    TOK_LBRACE,      // {
    TOK_RBRACE,      // }
    TOK_SEMICOLON,   // ;
    TOK_COMMA,       // ,

    // Keywords
    TOK_PRINT,       // print
    TOK_IF,          // if
    TOK_ELSE,        // else
    TOK_WHILE,       // while
    TOK_FUNC,        // func
    TOK_RETURN,      // return

    // Comparison operators
    TOK_EQ,          // ==
    TOK_NEQ,         // !=
    TOK_LT,          // <
    TOK_GT,          // >
    TOK_LTE,         // <=
    TOK_GTE,         // >=

    TOK_EOF,         // End of file/input
    TOK_UNKNOWN      // Unknown/invalid token
} TokenType;

/* Token structure - holds type and optional value */
typedef struct {
    TokenType type;
    char *value;  // Stores actual text for numbers, identifiers, strings
} Token;

/* ============================================================================
 * LEXER (TOKENIZER)
 * ============================================================================
 * The lexer reads source code character by character and groups them into
 * meaningful tokens. For example: "x = 5 + 3;" becomes tokens:
 * [IDENTIFIER:x] [ASSIGN] [NUMBER:5] [PLUS] [NUMBER:3] [SEMICOLON]
 */

typedef struct {
    char *input;        // The source code string
    int pos;            // Current position in the input
    char current_char;  // Character we are currently looking at
} Lexer;

/* Initialize the lexer with source code */
void lexer_init(Lexer *lex, char *input) {
    lex->input = input;
    lex->pos = 0;
    lex->current_char = input[0];  // Start at first character
}

/* Move to the next character in the input */
void lexer_advance(Lexer *lex) {
    lex->pos++;
    if (lex->pos < strlen(lex->input))
        lex->current_char = lex->input[lex->pos];
    else
        lex->current_char = '\0';  // End of input
}

/* Skip over whitespace characters (spaces, tabs, newlines) */
void skip_whitespace(Lexer *lex) {
    while (lex->current_char != '\0' && isspace(lex->current_char))
        lexer_advance(lex);
}

/*
 * Get the next token from the input
 * This is the main lexer function - it identifies what kind of token
 * we are looking at and creates the appropriate Token structure
 */
Token *lexer_get_next_token(Lexer *lex) {
    skip_whitespace(lex);

    Token *tok = malloc(sizeof(Token));
    tok->value = NULL;

    // Check for end of input
    if (lex->current_char == '\0') {
        tok->type = TOK_EOF;
        return tok;
    }

    // NUMBER: If current char is a digit, read the entire number
    if (isdigit(lex->current_char)) {
        char num[32];
        int i = 0;
        while (isdigit(lex->current_char)) {
            num[i++] = lex->current_char;
            lexer_advance(lex);
        }
        num[i] = '\0';
        tok->type = TOK_NUMBER;
        tok->value = strdup(num);  // Store the number as a string
        return tok;
    }

    // STRING: If we see a quote, read until the closing quote
    if (lex->current_char == '"') {
        lexer_advance(lex);  // Skip opening quote
        char str[256];
        int i = 0;
        while (lex->current_char != '"' && lex->current_char != '\0') {
            str[i++] = lex->current_char;
            lexer_advance(lex);
        }
        str[i] = '\0';
        lexer_advance(lex);  // Skip closing quote
        tok->type = TOK_STRING;
        tok->value = strdup(str);
        return tok;
    }

    // IDENTIFIER or KEYWORD: If current char is a letter, read the word
    if (isalpha(lex->current_char)) {
        char id[32];
        int i = 0;
        // Read alphanumeric characters and underscores
        while (isalnum(lex->current_char) || lex->current_char == '_') {
            id[i++] = lex->current_char;
            lexer_advance(lex);
        }
        id[i] = '\0';

        // Check if it's a keyword, otherwise it's an identifier
        if (strcmp(id, "print") == 0) tok->type = TOK_PRINT;
        else if (strcmp(id, "if") == 0) tok->type = TOK_IF;
        else if (strcmp(id, "else") == 0) tok->type = TOK_ELSE;
        else if (strcmp(id, "while") == 0) tok->type = TOK_WHILE;
        else if (strcmp(id, "func") == 0) tok->type = TOK_FUNC;
        else if (strcmp(id, "return") == 0) tok->type = TOK_RETURN;
        else {
            tok->type = TOK_IDENTIFIER;
            tok->value = strdup(id);
        }
        return tok;
    }

    // OPERATORS and DELIMITERS: Single or double character tokens
    char c = lex->current_char;
    lexer_advance(lex);

    switch (c) {
        case '=':
            // Could be '=' or '=='
            if (lex->current_char == '=') {
                lexer_advance(lex);
                tok->type = TOK_EQ;
            } else {
                tok->type = TOK_ASSIGN;
            }
            break;
        case '!':
            // Must be '!='
            if (lex->current_char == '=') {
                lexer_advance(lex);
                tok->type = TOK_NEQ;
            }
            break;
        case '<':
            // Could be '<' or '<='
            if (lex->current_char == '=') {
                lexer_advance(lex);
                tok->type = TOK_LTE;
            } else {
                tok->type = TOK_LT;
            }
            break;
        case '>':
            // Could be '>' or '>='
            if (lex->current_char == '=') {
                lexer_advance(lex);
                tok->type = TOK_GTE;
            } else {
                tok->type = TOK_GT;
            }
            break;
        // Single-character tokens
        case '+': tok->type = TOK_PLUS; break;
        case '-': tok->type = TOK_MINUS; break;
        case '*': tok->type = TOK_MUL; break;
        case '/': tok->type = TOK_DIV; break;
        case '(': tok->type = TOK_LPAREN; break;
        case ')': tok->type = TOK_RPAREN; break;
        case '{': tok->type = TOK_LBRACE; break;
        case '}': tok->type = TOK_RBRACE; break;
        case ';': tok->type = TOK_SEMICOLON; break;
        case ',': tok->type = TOK_COMMA; break;
        default: tok->type = TOK_UNKNOWN;
    }

    return tok;
}

/* ============================================================================
 * ABSTRACT SYNTAX TREE (AST) DEFINITIONS
 * ============================================================================
 * The AST represents the structure of the program. Each node represents
 * a different kind of language construct (expression, statement, etc.)
 */

// Types of AST nodes
typedef enum {
    NODE_NUMBER,      // Numeric literal: 42
    NODE_STRING,      // String literal: "hello"
    NODE_VARIABLE,    // Variable reference: x
    NODE_BINOP,       // Binary operation: x + y
    NODE_ASSIGN,      // Assignment: x = 5
    NODE_PRINT,       // Print statement: print x
    NODE_BLOCK,       // Block of statements: { stmt1; stmt2; }
    NODE_IF,          // If statement: if (cond) { ... }
    NODE_WHILE,       // While loop: while (cond) { ... }
    NODE_FUNC_DEF,    // Function definition: func foo(x) { ... }
    NODE_FUNC_CALL,   // Function call: foo(5)
    NODE_RETURN       // Return statement: return x
} NodeType;

// Forward declaration of ASTNode (needed for self-referential structures)
typedef struct ASTNode ASTNode;

/* Block of statements - used for function bodies, if bodies, etc. */
typedef struct {
    ASTNode **statements;  // Array of statement nodes
    int count;             // Number of statements
} Block;

/* If statement node - contains condition and branches */
typedef struct {
    ASTNode *condition;      // The condition to test
    ASTNode *then_branch;    // Code to execute if true
    ASTNode *else_branch;    // Code to execute if false (can be NULL)
} IfNode;

/* Function definition node */
typedef struct {
    char *name;           // Function name
    char **params;        // Parameter names
    int param_count;      // Number of parameters
    ASTNode *body;        // Function body (block of statements)
} FuncDef;

/* Function call node */
typedef struct {
    char *name;           // Function name
    ASTNode **args;       // Argument expressions
    int arg_count;        // Number of arguments
} FuncCall;

/*
 * Main AST Node structure
 * Uses a union to store different types of node data efficiently
 * Only one of the union members is valid at a time, depending on 'type'
 */
struct ASTNode {
    NodeType type;
    union {
        int number;                    // For NODE_NUMBER
        char *str_value;               // For NODE_STRING
        char *var_name;                // For NODE_VARIABLE

        // Binary operation (x + y, x == y, etc.)
        struct {
            ASTNode *left;
            ASTNode *right;
            char op;  // Operation: +, -, *, /, e(==), n(!=), <, >, l(<=), g(>=)
        } binop;

        // Assignment (x = expr)
        struct {
            char *var_name;
            ASTNode *expr;
        } assign;

        // Print statement
        struct {
            ASTNode *expr;
        } print;

        Block block;                   // For NODE_BLOCK
        IfNode if_node;                // For NODE_IF

        // While loop
        struct {
            ASTNode *condition;
            ASTNode *body;
        } while_node;

        FuncDef func_def;              // For NODE_FUNC_DEF
        FuncCall func_call;            // For NODE_FUNC_CALL

        // Return statement
        struct {
            ASTNode *expr;
        } return_node;
    } data;
};

/* ============================================================================
 * SYMBOL TABLE
 * ============================================================================
 * The symbol table stores variables and functions during execution
 */

#define MAX_VARS 100   // Maximum number of variables
#define MAX_FUNCS 50   // Maximum number of functions

/* Variable entry in symbol table */
typedef struct {
    char *name;
    int value;
} Variable;

/* Function entry in symbol table */
typedef struct {
    char *name;
    FuncDef *func;
} Function;

// Global symbol tables
Variable symbol_table[MAX_VARS];
int var_count = 0;

Function func_table[MAX_FUNCS];
int func_count = 0;

/* ============================================================================
 * PARSER
 * ============================================================================
 * The parser takes tokens from the lexer and builds an Abstract Syntax Tree.
 * It uses recursive descent parsing, which means each grammar rule becomes
 * a function that calls other functions for sub-expressions.
 *
 * Grammar (simplified):
 * program    → statement*
 * statement  → assignment | print | if | while | funcdef | return
 * assignment → IDENTIFIER = expr ;
 * print      → print expr ;
 * if         → if ( expr ) block (else block)?
 * while      → while ( expr ) block
 * funcdef    → func IDENTIFIER ( params ) block
 * block      → { statement* }
 * expr       → comparison
 * comparison → term ((== | != | < | > | <= | >=) term)?
 * term       → factor ((+ | -) factor)*
 * factor     → atom ((* | /) atom)*
 * atom       → NUMBER | STRING | IDENTIFIER | ( expr ) | func_call
 */

typedef struct {
    Lexer *lexer;
    Token *current_token;  // Token we're currently processing
} Parser;

/*
 * Consume a token of expected type and move to next token
 * If the token doesn't match, it's a syntax error
 */
void parser_eat(Parser *p, TokenType type) {
    if (p->current_token->type == type) {
        free(p->current_token->value);
        free(p->current_token);
        p->current_token = lexer_get_next_token(p->lexer);
    } else {
        fprintf(stderr, "Syntax error: unexpected token\n");
        exit(1);
    }
}

// Forward declarations for recursive parsing functions
ASTNode *parse_factor(Parser *p);
ASTNode *parse_term(Parser *p);
ASTNode *parse_comparison(Parser *p);
ASTNode *parse_expr(Parser *p);
ASTNode *parse_statement(Parser *p);

/*
 * Parse a factor (highest precedence level)
 * factor → NUMBER | STRING | IDENTIFIER | ( expr ) | func_call
 */
ASTNode *parse_factor(Parser *p) {
    Token *tok = p->current_token;

    // NUMBER: Create a number node
    if (tok->type == TOK_NUMBER) {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_NUMBER;
        node->data.number = atoi(tok->value);  // Convert string to integer
        parser_eat(p, TOK_NUMBER);
        return node;
    }
    // STRING: Create a string node
    else if (tok->type == TOK_STRING) {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_STRING;
        node->data.str_value = strdup(tok->value);
        parser_eat(p, TOK_STRING);
        return node;
    }
    // IDENTIFIER: Could be a variable or function call
    else if (tok->type == TOK_IDENTIFIER) {
        char *name = strdup(tok->value);
        parser_eat(p, TOK_IDENTIFIER);

        // If followed by '(', it's a function call
        if (p->current_token->type == TOK_LPAREN) {
            parser_eat(p, TOK_LPAREN);
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = NODE_FUNC_CALL;
            node->data.func_call.name = name;
            node->data.func_call.args = malloc(sizeof(ASTNode*) * 10);
            node->data.func_call.arg_count = 0;

            // Parse arguments (if any)
            if (p->current_token->type != TOK_RPAREN) {
                node->data.func_call.args[node->data.func_call.arg_count++] = parse_expr(p);
                // Parse additional arguments separated by commas
                while (p->current_token->type == TOK_COMMA) {
                    parser_eat(p, TOK_COMMA);
                    node->data.func_call.args[node->data.func_call.arg_count++] = parse_expr(p);
                }
            }
            parser_eat(p, TOK_RPAREN);
            return node;
        }
        // Otherwise it's a variable reference
        else {
            ASTNode *node = malloc(sizeof(ASTNode));
            node->type = NODE_VARIABLE;
            node->data.var_name = name;
            return node;
        }
    }
    // PARENTHESES: Parse the expression inside
    else if (tok->type == TOK_LPAREN) {
        parser_eat(p, TOK_LPAREN);
        ASTNode *node = parse_expr(p);  // Parse expression recursively
        parser_eat(p, TOK_RPAREN);
        return node;
    }

    fprintf(stderr, "Syntax error in factor\n");
    exit(1);
}

/*
 * Parse a term (multiplication and division)
 * term → factor ((* | /) factor)*
 * This handles left-associativity: 2 * 3 * 4 = (2 * 3) * 4
 */
ASTNode *parse_term(Parser *p) {
    ASTNode *node = parse_factor(p);

    // Keep parsing * and / operations
    while (p->current_token->type == TOK_MUL || p->current_token->type == TOK_DIV) {
        Token *tok = p->current_token;
        char op = (tok->type == TOK_MUL) ? '*' : '/';
        parser_eat(p, tok->type);

        // Create binary operation node
        ASTNode *binop = malloc(sizeof(ASTNode));
        binop->type = NODE_BINOP;
        binop->data.binop.left = node;  // Previous result becomes left side
        binop->data.binop.right = parse_factor(p);
        binop->data.binop.op = op;
        node = binop;  // New node becomes current
    }

    return node;
}

/*
 * Parse comparison (addition and subtraction)
 * comparison → term ((+ | -) term)*
 * Lower precedence than multiplication/division
 */
ASTNode *parse_comparison(Parser *p) {
    ASTNode *node = parse_term(p);

    // Keep parsing + and - operations
    while (p->current_token->type == TOK_PLUS || p->current_token->type == TOK_MINUS) {
        Token *tok = p->current_token;
        char op = (tok->type == TOK_PLUS) ? '+' : '-';
        parser_eat(p, tok->type);

        ASTNode *binop = malloc(sizeof(ASTNode));
        binop->type = NODE_BINOP;
        binop->data.binop.left = node;
        binop->data.binop.right = parse_term(p);
        binop->data.binop.op = op;
        node = binop;
    }

    return node;
}

/*
 * Parse expression (comparison operators)
 * expr → comparison ((== | != | < | > | <= | >=) comparison)?
 * Lowest precedence - evaluated last
 */
ASTNode *parse_expr(Parser *p) {
    ASTNode *node = parse_comparison(p);

    // Check for comparison operators
    TokenType cmp_types[] = {TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LTE, TOK_GTE};
    char cmp_ops[] = {'e', 'n', '<', '>', 'l', 'g'};

    for (int i = 0; i < 6; i++) {
        if (p->current_token->type == cmp_types[i]) {
            char op = cmp_ops[i];
            parser_eat(p, cmp_types[i]);

            ASTNode *binop = malloc(sizeof(ASTNode));
            binop->type = NODE_BINOP;
            binop->data.binop.left = node;
            binop->data.binop.right = parse_comparison(p);
            binop->data.binop.op = op;
            return binop;
        }
    }

    return node;
}

/*
 * Parse a block of statements
 * block → { statement* }
 */
ASTNode *parse_block(Parser *p) {
    parser_eat(p, TOK_LBRACE);
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = NODE_BLOCK;
    node->data.block.statements = malloc(sizeof(ASTNode*) * 100);
    node->data.block.count = 0;

    // Parse all statements until we hit closing brace
    while (p->current_token->type != TOK_RBRACE && p->current_token->type != TOK_EOF) {
        node->data.block.statements[node->data.block.count++] = parse_statement(p);
    }

    parser_eat(p, TOK_RBRACE);
    return node;
}

/*
 * Parse a statement
 * statement → print | if | while | funcdef | return | assignment
 */
ASTNode *parse_statement(Parser *p) {
    // PRINT STATEMENT: print expr;
    if (p->current_token->type == TOK_PRINT) {
        parser_eat(p, TOK_PRINT);
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_PRINT;
        node->data.print.expr = parse_expr(p);
        parser_eat(p, TOK_SEMICOLON);
        return node;
    }
    // IF STATEMENT: if (expr) { ... } else { ... }
    else if (p->current_token->type == TOK_IF) {
        parser_eat(p, TOK_IF);
        parser_eat(p, TOK_LPAREN);
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_IF;
        node->data.if_node.condition = parse_expr(p);
        parser_eat(p, TOK_RPAREN);
        node->data.if_node.then_branch = parse_block(p);

        // Optional else clause
        if (p->current_token->type == TOK_ELSE) {
            parser_eat(p, TOK_ELSE);
            node->data.if_node.else_branch = parse_block(p);
        } else {
            node->data.if_node.else_branch = NULL;
        }
        return node;
    }
    // WHILE LOOP: while (expr) { ... }
    else if (p->current_token->type == TOK_WHILE) {
        parser_eat(p, TOK_WHILE);
        parser_eat(p, TOK_LPAREN);
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_WHILE;
        node->data.while_node.condition = parse_expr(p);
        parser_eat(p, TOK_RPAREN);
        node->data.while_node.body = parse_block(p);
        return node;
    }
    // FUNCTION DEFINITION: func name(params) { ... }
    else if (p->current_token->type == TOK_FUNC) {
        parser_eat(p, TOK_FUNC);
        char *name = strdup(p->current_token->value);
        parser_eat(p, TOK_IDENTIFIER);
        parser_eat(p, TOK_LPAREN);

        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_FUNC_DEF;
        node->data.func_def.name = name;
        node->data.func_def.params = malloc(sizeof(char*) * 10);
        node->data.func_def.param_count = 0;

        // Parse parameter list
        if (p->current_token->type != TOK_RPAREN) {
            node->data.func_def.params[node->data.func_def.param_count++] =
                strdup(p->current_token->value);
            parser_eat(p, TOK_IDENTIFIER);

            while (p->current_token->type == TOK_COMMA) {
                parser_eat(p, TOK_COMMA);
                node->data.func_def.params[node->data.func_def.param_count++] =
                    strdup(p->current_token->value);
                parser_eat(p, TOK_IDENTIFIER);
            }
        }

        parser_eat(p, TOK_RPAREN);
        node->data.func_def.body = parse_block(p);
        return node;
    }
    // RETURN STATEMENT: return expr;
    else if (p->current_token->type == TOK_RETURN) {
        parser_eat(p, TOK_RETURN);
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_RETURN;
        node->data.return_node.expr = parse_expr(p);
        parser_eat(p, TOK_SEMICOLON);
        return node;
    }
    // ASSIGNMENT: identifier = expr;
    else if (p->current_token->type == TOK_IDENTIFIER) {
        char *var_name = strdup(p->current_token->value);
        parser_eat(p, TOK_IDENTIFIER);
        parser_eat(p, TOK_ASSIGN);
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = NODE_ASSIGN;
        node->data.assign.var_name = var_name;
        node->data.assign.expr = parse_expr(p);
        parser_eat(p, TOK_SEMICOLON);
        return node;
    }

    return NULL;
}

/* ============================================================================
 * INTERPRETER (EVALUATOR)
 * ============================================================================
 * The interpreter walks the AST and executes the program.
 * This is also called "tree-walking interpretation".
 */

/* Look up a variable's value in the symbol table */
int get_var(char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0)
            return symbol_table[i].value;
    }
    // Variable not found - return 0 as default
    return 0;
}

/* Set a variable's value in the symbol table */
void set_var(char *name, int value) {
    // Check if variable already exists
    for (int i = 0; i < var_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            symbol_table[i].value = value;
            return;
        }
    }
    // Variable doesn't exist - create new entry
    symbol_table[var_count].name = strdup(name);
    symbol_table[var_count].value = value;
    var_count++;
}

/* Register a function in the function table */
void register_func(char *name, FuncDef *func) {
    func_table[func_count].name = strdup(name);
    func_table[func_count].func = func;
    func_count++;
}

/* Look up a function by name */
FuncDef *get_func(char *name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(func_table[i].name, name) == 0)
            return func_table[i].func;
    }
    return NULL;
}

// Global state for handling return statements
int return_value = 0;      // Value to return from function
bool returned = false;     // Flag indicating we've hit a return

/*
 * Evaluate (execute) an AST node
 * This is the heart of the interpreter - it recursively evaluates
 * the tree structure and performs the appropriate operations
 */
int eval(ASTNode *node) {
    // If we've already returned from a function, stop executing
    if (returned) return return_value;

    switch (node->type) {
        // NUMBER: Just return the numeric value
        case NODE_NUMBER:
            return node->data.number;

        // STRING: Print the string (for now, strings only work in print)
        case NODE_STRING:
            printf("%s", node->data.str_value);
            return 0;

        // VARIABLE: Look up and return its value
        case NODE_VARIABLE:
            return get_var(node->data.var_name);

        // BINARY OPERATION: Evaluate both sides and apply operator
        case NODE_BINOP: {
            int left = eval(node->data.binop.left);
            int right = eval(node->data.binop.right);
            switch (node->data.binop.op) {
                case '+': return left + right;
                case '-': return left - right;
                case '*': return left * right;
                case '/': return left / right;
                // Comparison operators return 1 (true) or 0 (false)
                case 'e': return left == right;
                case 'n': return left != right;
                case '<': return left < right;
                case '>': return left > right;
                case 'l': return left <= right;
                case 'g': return left >= right;
            }
        }

        // ASSIGNMENT: Evaluate expression and store in variable
        case NODE_ASSIGN:
            set_var(node->data.assign.var_name, eval(node->data.assign.expr));
            return 0;

        // PRINT: Evaluate expression and print the result
        case NODE_PRINT:
            printf("%d\n", eval(node->data.print.expr));
            return 0;

        // BLOCK: Execute each statement in sequence
        case NODE_BLOCK:
            for (int i = 0; i < node->data.block.count; i++) {
                eval(node->data.block.statements[i]);
                // Stop if we hit a return statement
                if (returned) break;
            }
            return 0;

        // IF STATEMENT: Evaluate condition and execute appropriate branch
        case NODE_IF:
            // In our language, 0 is false, anything else is true
            if (eval(node->data.if_node.condition)) {
                eval(node->data.if_node.then_branch);
            } else if (node->data.if_node.else_branch) {
                eval(node->data.if_node.else_branch);
            }
            return 0;

        // WHILE LOOP: Keep executing body while condition is true
        case NODE_WHILE:
            while (eval(node->data.while_node.condition)) {
                eval(node->data.while_node.body);
                // Stop if we hit a return statement
                if (returned) break;
            }
            return 0;

        // FUNCTION DEFINITION: Register the function for later use
        case NODE_FUNC_DEF:
            register_func(node->data.func_def.name, &node->data.func_def);
            return 0;

        // FUNCTION CALL: Execute the function with given arguments
        case NODE_FUNC_CALL: {
            // Look up the function
            FuncDef *func = get_func(node->data.func_call.name);
            if (!func) {
                fprintf(stderr, "Error: undefined function '%s'\n", node->data.func_call.name);
                exit(1);
            }

            // Save current variable state (for local scope simulation)
            int saved_vars[MAX_VARS];
            int saved_count = var_count;
            for (int i = 0; i < var_count; i++) {
                saved_vars[i] = symbol_table[i].value;
            }

            // Bind arguments to parameters
            for (int i = 0; i < func->param_count; i++) {
                set_var(func->params[i], eval(node->data.func_call.args[i]));
            }

            // Execute function body
            returned = false;
            eval(func->body);
            int result = return_value;
            returned = false;

            // Restore previous variable state
            var_count = saved_count;
            for (int i = 0; i < saved_count; i++) {
                symbol_table[i].value = saved_vars[i];
            }

            return result;
        }

        // RETURN STATEMENT: Set return value and flag
        case NODE_RETURN:
            return_value = eval(node->data.return_node.expr);
            returned = true;
            return return_value;
    }
    return 0;
}

/* ============================================================================
 * FILE I/O
 * ============================================================================ */

/*
 * Read entire file into a string
 * This allows us to run programs from .txt files
 */
char *read_file(char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        exit(1);
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Read entire file into buffer
    char *content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';  // Null terminate
    fclose(f);

    return content;
}

/* ============================================================================
 * MAIN ENTRY POINT
 * ============================================================================ */

int main(int argc, char **argv) {
    char *code;

    // If a filename is provided, read from file
    if (argc > 1) {
        code = read_file(argv[1]);
    }
    // Otherwise, run built-in example program
    else {
        code =
            "// Example: Factorial function and loop\n"
            "func factorial(n) {"
            "    if (n <= 1) {"
            "        return 1;"
            "    } else {"
            "        return n * factorial(n - 1);"
            "    }"
            "}"
            ""
            "x = 5;"
            "result = factorial(x);"
            "print result;"
            ""
            "i = 0;"
            "while (i < 3) {"
            "    print i;"
            "    i = i + 1;"
            "}";
    }

    // PHASE 1: LEXING
    // Initialize lexer with source code
    Lexer lexer;
    lexer_init(&lexer, code);

    // PHASE 2: PARSING
    // Initialize parser and get first token
    Parser parser;
    parser.lexer = &lexer;
    parser.current_token = lexer_get_next_token(&lexer);

    // PHASE 3: EXECUTION
    // Parse and evaluate each statement until end of file
    while (parser.current_token->type != TOK_EOF) {
        ASTNode *stmt = parse_statement(&parser);
        if (stmt) {
            eval(stmt);  // Execute the statement
            // In a production interpreter, we'd free the AST here
        }
    }

    return 0;
}