#pragma once
#include <any>
#include <string>
#include <vector>
#define VERSION "2024.4.4"
#define EXTENSION ".msol"

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
    USER_FUNCTION,
    CONSTANT,
    COMMAND,
    OPERATOR,
    FUNCTION_CALL,
    CONDITION_BLOCK,
    LOOP_BLOCK,
    BLOCK,
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
    CACHE,
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
    STRLEN,
    LEN,
    NOPOP,
    NOPUSH,
    GUARD,
    EXIT
};

struct Token
{
    TokenType type = TokenType::NULL_TOKEN;
    std::any value;
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

Node* tokenize(const std::string &executable_path, const std::string &program_path, const std::string &code);
bool lex(Node* Nodes);
bool parse(Node* Nodes);
bool interpret(const std::string &executable_path, const std::string &program_path, Node* program, std::vector<Token> &backup_stack);

std::string load_file(const std::string &path);
void delete_nodes(Node* pointer);
CommandEnum get_command_enum(const std::string &val);
std::string get_token_string(const Token &t = {.type = TokenType::NULL_TOKEN});
std::string trim_num_string(const std::string &num);

int find_tag(const std::vector<Token> &list, const Token &tag);
void error_msg(const int &line, const std::string &token_string, const std::string &explanation);
bool is_valid_extension(const std::string &file, const std::string &extension);
std::string getexepath();
std::string get_base_path(const std::string &file);

inline bool is_tag(const Token &t)
{
    return (t.type >= TokenType::TAG_GLOBAL && t.type <= TokenType::TAG_MEMBER);
}

inline bool is_value(const Token &t)
{
    return (t.type >= TokenType::DATA_String && t.type <= TokenType::DATA_Bool);
}

