#pragma once
#include <any>
#include <string>
#include <vector>
#include <map>
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
bool parse(Node* Nodes, std::vector<Token>& stack);
bool interpret(const std::string& executable_path, const std::string& program_path, Node* program, std::vector<Token>& backup_stack);

std::string load_file(const std::string& path);
void delete_nodes(Node* pointer);
void delete_sub_list(Node* start, Node* end);

extern const std::map<const std::string, const CommandEnum> command_enum_map;
extern const std::map<const CommandEnum, const std::string> enum_command_map;

__attribute__((always_inline))
inline CommandEnum get_command_enum(const std::string& val)
{
    return command_enum_map.count(val) > 0 ? command_enum_map.at(val) : CommandEnum::UNKNOWN_COMMAND;
}

__attribute__((always_inline))
inline const char* get_command_string(const CommandEnum& c)
{
    return enum_command_map.count(c) > 0 ? enum_command_map.at(c).c_str() : "UNKNOWN_COMMAND";
}

std::string get_token_string(const Token& t = {.type = TokenType::NULL_TOKEN});
std::string trim_num_string(const std::string& num);
bool is_num(const std::string& val);

extern const std::map<const std::string, const OperatorEnum> operator_enum_map;
extern const std::map<const OperatorEnum, const std::string> enum_operator_map;

__attribute__((always_inline))
inline OperatorEnum get_operator_enum(const std::string& val)
{
    return (operator_enum_map.count(val) > 0) ? operator_enum_map.at(val) : OperatorEnum::UNKNOWN_OPERATOR;
}

__attribute__((always_inline))
inline std::string get_operator_string(const OperatorEnum& val)
{
    return (enum_operator_map.count(val) > 0) ? enum_operator_map.at(val) : "UNKNOWN_OPERATOR";
}

int find_tag(const std::vector<Token>& list, const Token& tag);
void error_msg(const Node* node, const char* explanation);
bool is_valid_extension(const std::string& file, const std::string& extension);
std::string getexepath();
std::string get_base_path(const std::string& file);

__attribute__((always_inline))
inline bool is_tag(const Token& t)
{
    return t.type >= TokenType::TAG_GLOBAL && t.type <= TokenType::TAG_MEMBER;
}

__attribute__((always_inline))
inline bool is_value(const Token& t)
{
    return t.type >= TokenType::DATA_String && t.type <= TokenType::DATA_Bool;
}

__attribute__((always_inline))
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

