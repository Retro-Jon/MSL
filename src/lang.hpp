#pragma once
#include <any>
#include <string>
#include <vector>
#define VERSION "1"
#define EXTENSION ".msl"
#ifdef DEBUG
#include <iostream>
#include <chrono>
#endif

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
    ROOT
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
    DUP_LIST,
    DEFINE,
    INCLUDE,
    STRLEN,
    LEN,
    CAT,
    OPEN_LIB,
    EXECUTE,
    EXIT
};

enum OperatorEnum
{
    UNKNOWN_OPERATOR,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    INCREMENT,
    DECREMENT,
    MODULO,
    ASSIGNMENT,
    NEGATE,
    AND,
    OR,
    EQUAL,
    NOT_EQUAL,
    GREATER_THAN,
    GREATER_THAN_EQUAL,
    LESS_THAN,
    LESS_THAN_EQUAL
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
    int file_source;
    Node* default_next = nullptr;
    Node* alt_next = nullptr;
};

struct Function
{
    Node* location = nullptr;
    Token* argument_tags = nullptr;
    int arg_count = 0;
};

extern std::vector<std::string> included_files;
extern const char* TokenTypeString[];

Node* tokenize(const std::string& executable_path, const std::string& program_path, const std::string& code, const std::string& file_name = "user session");
bool lex(Node* Nodes);
bool parse(Node* Nodes);
bool interpret(const std::string& executable_path, const std::string& program_path, Node* program, std::vector<Token>& backup_stack);

std::string load_file(const std::string& path);
void delete_nodes(Node* pointer);
void delete_sub_list(Node* start, Node* end);
CommandEnum get_command_enum(const std::string& val);
std::string get_token_string(const Token& t = {.type = TokenType::NULL_TOKEN});
OperatorEnum get_operator_enum(const std::string& val);
std::string get_operator_string(const OperatorEnum& val);
const char* get_command_string(const CommandEnum& c);
std::string trim_num_string(const std::string& num);
bool is_num(const std::string& val);

int find_tag(const std::vector<Token>& list, const Token& tag);
void error_msg(const Node* node, const char* explanation);
bool is_valid_extension(const std::string& file, const std::string& extension);
std::string getexepath();
std::string get_base_path(const std::string& file);

inline bool is_tag(const Token& t)
{
    return !(t.type < TokenType::TAG_GLOBAL || t.type > TokenType::TAG_MEMBER);
}

inline bool is_value(const Token& t)
{
    return !(t.type < TokenType::DATA_String || t.type > TokenType::DATA_Bool);
}

inline bool is_stack_break(const Token& tok)
{
    return tok.type >= TokenType::FUNCTION_CALL || tok.type == TokenType::LIST_START;
}

class InterpreterException : public std::exception
{
    private:
        const std::string message;

    public:
        InterpreterException(const std::string& msg) : message(msg) {}

        const char* what()
        {
            return message.c_str();
        }
};

