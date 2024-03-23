#pragma once
#include <any>
#include <string>
#include <vector>

enum TokenType
{
    NULL_TOKEN,
    DATA_String,
    DATA_Char,
    DATA_Number,
    DATA_Bool,
    TAG_GLOBAL,
    TAG_LOCAL,
    TAG_BLOCK,
    TAG_MEMBER,
    LIST_START,
    LIST_END,
    SUB_LIST_START,
    SUB_LIST_END,
    FUNCTION_CALL,
    USER_FUNCTION,
    CONSTANT,
    CONDITION_BLOCK,
    LOOP_BLOCK,
    BLOCK,
    COMMAND,
    OPERATOR
};

enum CommandEnum
{
    UNKNOWN_COMMAND,
    PRINT,
    PRINTLN,
    INPUT,
    PRINT_STACK,
    DROP,
    DROP_LIST,
    AT,
    GET,
    GET_LIST,
    GET_LIST_VALUES,
    MERGE,
    MERGE_X,
    INT,
    IF,
    ERROR_HANDLER,
    BEGIN,
    LOOP,
    WHILE,
    FOR,
    DEFUNC,
    CHACHE,
    RETURN,
    END,
    BREAK,
    CONTINUE,
    SWAP,
    SWAP_LIST,
    DUP,
    DUP_X,
    DEFINE,
    INCLUDE,
    EXIT
};

struct Token
{
    std::any value;
    TokenType type = TokenType::NULL_TOKEN;
    CommandEnum command = CommandEnum::UNKNOWN_COMMAND;
};

struct Node
{
    Token t;
    int line;
    Node* default_next = nullptr;
    Node* alt_next = nullptr;
};

const std::string TokenTypeString[] = {
    "NULL_TOKEN",
    "DATA_String",
    "DATA_Char",
    "DATA_Number",
    "DATA_Bool",
    "TAG_GLOBAL",
    "TAG_LOCAL",
    "TAG_BLOCK",
    "TAG_MEMBER",
    "LIST_START",
    "LIST_END",
    "SUB_LIST_START",
    "SUB_LIST_END",
    "FUNCTION_CALL",
    "USER_FUNCTION",
    "CONSTANT",
    "CONDITION_BLOCK",
    "LOOP_BLOCK",
    "BLOCK",
    "COMMAND",
    "OPERATOR"
};

struct Function
{
    Node* location = nullptr;
    Token* argument_tags = nullptr;
    int arg_count = 0;
};

Node* tokenize(const char* code);
bool lex(Node* Nodes);
bool parse(Node* Nodes);
bool interpret(std::string executable_path, std::string program_path, Node* program, std::vector<Token> &backup_stack);

std::string load_file(const char* path);
void delete_nodes(Node* pointer);
CommandEnum get_command_enum(Token t);
std::string get_token_string(Token t);
std::string trim_num_string(std::string num);

int find_tag(std::vector<Token> list, Token tag);
bool is_tag(Token t);
bool is_value(Token t);
void error_msg(Node* node, const char* explanation);
