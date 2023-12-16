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
    TAG_MEMBER,
    LIST_START,
    LIST_END,
    SUB_LIST_START,
    SUB_LIST_END,
    FUNCTION_CALL,
    CONDITION_BLOCK,
    LOOP_BLOCK,
    COMMAND,
    OPERATOR
};

struct Token
{
    std::any value;
    TokenType type = TokenType::NULL_TOKEN;
};

struct Node
{
    Token t;
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
    "TAG_MEMBER",
    "LIST_START",
    "LIST_END",
    "SUB_LIST_START",
    "SUB_LIST_END",
    "FUNCTION_CALL",
    "CONDITION_BLOCK",
    "LOOP_BLOCK",
    "COMMAND",
    "OPERATOR"
};

Node* tokenize(const char* code);
bool lex(Node* Nodes);
bool parse(Node* Nodes);
bool interpret(Node* program);

std::string load_file(const char* path);
void delete_nodes(Node* pointer);
std::string get_token_string(Token t);
std::string trim_num_string(std::string num);

int find_tag(std::vector<Token> list, Token tag);
bool is_tag(Token t);
bool is_value(Token t);
void error_msg(const char* cause, const char* explanation);
