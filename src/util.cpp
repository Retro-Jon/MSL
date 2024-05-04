#include "lang.hpp"
#include <any>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <unistd.h>

std::string load_file(const std::string &path)
{
    std::string res;
    std::fstream file;
    file.open(path);

    if (file.is_open())
    {
        std::string line;

        while (std::getline(file, line))
            res += line + '\n';
        
        file.close();
    }

    if (res.empty())
        res = "[\"Could not open " + path + "\"] print";

    return res;
}

void delete_nodes(Node* pointer)
{
    for (Node* current = pointer; current != nullptr; current = current->default_next)
        delete current;
}

void delete_sub_list(Node* start, Node* end)
{
    Node* last;
    
    if (end != nullptr)
        last = end->default_next;
    else
        last = nullptr;

    for (Node* current = start->default_next; current != nullptr && current != last; current = current->default_next)
        delete current;

    start->default_next = last;
}

bool is_num(const std::string &val)
{
    bool is_num = false;
    if (isdigit(val.front()) || (val.front() == '-' && val.length() > 1))
    {
        for (char c : val)
        {
            is_num = (isdigit(c) || c == '.' || c == '-');

            if (!is_num)
                break;
        }
    }

    return is_num;
}

const std::map<const std::string, const CommandEnum> command_enum_map = {
    {"print", CommandEnum::PRINT},
    {"println", CommandEnum::PRINTLN},
    {"input", CommandEnum::INPUT},
    {"print-stack", CommandEnum::PRINT_STACK},
    {"drop", CommandEnum::DROP},
    {"drop-list", CommandEnum::DROP_LIST},
    {"at", CommandEnum::AT},
    {"get", CommandEnum::GET},
    {"get-list", CommandEnum::GET_LIST},
    {"get-list-values", CommandEnum::GET_LIST_VALUES},
    {"merge", CommandEnum::MERGE},
    {"merge-x", CommandEnum::MERGE_X},
    {"int", CommandEnum::INT},
    {"if", CommandEnum::IF},
    {"?", CommandEnum::ERROR_HANDLER},
    {"begin", CommandEnum::BEGIN},
    {"loop", CommandEnum::LOOP},
    {"while", CommandEnum::WHILE},
    {"for", CommandEnum::FOR},
    {"defunc", CommandEnum::DEFUNC},
    {"$", CommandEnum::CACHE},
    {"return", CommandEnum::RETURN},
    {"end", CommandEnum::END},
    {"break", CommandEnum::BREAK},
    {"continue", CommandEnum::CONTINUE},
    {"swap", CommandEnum::SWAP},
    {"swap-list", CommandEnum::SWAP_LIST},
    {"dup", CommandEnum::DUP},
    {"dup-x", CommandEnum::DUP_X},
    {"dup-list", CommandEnum::DUP_LIST},
    {"define", CommandEnum::DEFINE},
    {"include", CommandEnum::INCLUDE},
    {"strlen", CommandEnum::STRLEN},
    {"len", CommandEnum::LEN},
    {"cat", CommandEnum::CAT},
    {"open-lib", CommandEnum::OPEN_LIB},
    {"execute", CommandEnum::EXECUTE},
    {"exit", CommandEnum::EXIT}
};

const std::map<const CommandEnum, const std::string> enum_command_map = {
    {CommandEnum::PRINT, "print"},
    {CommandEnum::PRINTLN, "println"},
    {CommandEnum::INPUT, "input"},
    {CommandEnum::PRINT_STACK, "print-stack"},
    {CommandEnum::DROP, "drop"},
    {CommandEnum::DROP_LIST, "drop-list"},
    {CommandEnum::AT, "at"},
    {CommandEnum::GET, "get"},
    {CommandEnum::GET_LIST, "get-list"},
    {CommandEnum::GET_LIST_VALUES, "get-list-values"},
    {CommandEnum::MERGE, "merge"},
    {CommandEnum::MERGE_X, "merge-x"},
    {CommandEnum::INT, "int"},
    {CommandEnum::IF, "if"},
    {CommandEnum::ERROR_HANDLER, "?"},
    {CommandEnum::BEGIN, "begin"},
    {CommandEnum::LOOP, "loop"},
    {CommandEnum::WHILE, "while"},
    {CommandEnum::FOR, "for"},
    {CommandEnum::DEFUNC, "defunc"},
    {CommandEnum::CACHE, "$"},
    {CommandEnum::RETURN, "return"},
    {CommandEnum::END, "end"},
    {CommandEnum::BREAK, "break"},
    {CommandEnum::CONTINUE, "continue"},
    {CommandEnum::SWAP, "swap"},
    {CommandEnum::SWAP_LIST, "swap-list"},
    {CommandEnum::DUP, "dup"},
    {CommandEnum::DUP_X, "dup-x"},
    {CommandEnum::DUP_LIST, "dup-list"},
    {CommandEnum::DEFINE, "define"},
    {CommandEnum::INCLUDE, "include"},
    {CommandEnum::STRLEN, "strlen"},
    {CommandEnum::LEN, "len"},
    {CommandEnum::CAT, "cat"},
    {CommandEnum::OPEN_LIB, "open-lib"},
    {CommandEnum::EXECUTE, "execute"},
    {CommandEnum::EXIT, "exit"}
};

CommandEnum get_command_enum(const std::string val)
{
    if (command_enum_map.count(val) > 0)
        return command_enum_map.at(val);

    return CommandEnum::UNKNOWN_COMMAND;
}

const char* get_command_string(const CommandEnum &c)
{
    if (enum_command_map.count(c) > 0)
        return enum_command_map.at(c).c_str();

    return "UNKNOWN_COMMAND";
}

const std::map<const std::string, const OperatorEnum> operator_enum_map = {
    {"+", OperatorEnum::ADDITION},
    {"-", OperatorEnum::SUBTRACTION},
    {"*", OperatorEnum::MULTIPLICATION},
    {"/", OperatorEnum::DIVISION},
    {"++", OperatorEnum::INCREMENT},
    {"--", OperatorEnum::DECREMENT},
    {"%", OperatorEnum::MODULO},
    {"=", OperatorEnum::ASSIGNMENT},
    {"~", OperatorEnum::NEGATE},
    {"and", OperatorEnum::AND},
    {"or", OperatorEnum::OR},
    {"==", OperatorEnum::EQUAL},
    {"!=", OperatorEnum::NOT_EQUAL},
    {">", OperatorEnum::GREATER_THAN},
    {">=", OperatorEnum::GREATER_THAN_EQUAL},
    {"<", OperatorEnum::LESS_THAN},
    {"<=", OperatorEnum::LESS_THAN_EQUAL}
};

const std::map<const OperatorEnum, const std::string> enum_operator_map = {
    {OperatorEnum::ADDITION, "+"},
    {OperatorEnum::SUBTRACTION, "-"},
    {OperatorEnum::MULTIPLICATION, "*"},
    {OperatorEnum::DIVISION, "/"},
    {OperatorEnum::INCREMENT, "++"},
    {OperatorEnum::DECREMENT, "--"},
    {OperatorEnum::MODULO, "%"},
    {OperatorEnum::ASSIGNMENT, "="},
    {OperatorEnum::NEGATE, "~"},
    {OperatorEnum::AND, "and"},
    {OperatorEnum::OR, "or"},
    {OperatorEnum::EQUAL, "=="},
    {OperatorEnum::NOT_EQUAL, "!="},
    {OperatorEnum::GREATER_THAN, ">"},
    {OperatorEnum::GREATER_THAN_EQUAL, ">="},
    {OperatorEnum::LESS_THAN, "<"},
    {OperatorEnum::LESS_THAN_EQUAL, "<="}
};

OperatorEnum get_operator_enum(const std::string &val)
{
    if (operator_enum_map.count(val) > 0)
        return operator_enum_map.at(val);

    return OperatorEnum::UNKNOWN_OPERATOR;
}

std::string get_operator_string(const OperatorEnum &val)
{
    if (enum_operator_map.count(val) > 0)
        return enum_operator_map.at(val);

    return "UNKNOWN_OPERATOR";
}

const char* TokenTypeString[] = {
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
    "OPERATOR",
    "ROOT",
};

std::string get_token_string(const Token &t)
{
    switch (t.type)
    {
        case TokenType::ROOT:
            return "ROOT";
            break;

        case TokenType::NULL_TOKEN:
            return "NULL_TOKEN";
            break;
        
        case TokenType::TAG_GLOBAL:
        case TokenType::TAG_LOCAL:
        case TokenType::TAG_BLOCK:
        case TokenType::TAG_MEMBER:
        case TokenType::USER_FUNCTION:
        case TokenType::DATA_String:
        case TokenType::CONSTANT:
            return std::any_cast<std::string>(t.value);
            break;

        case TokenType::COMMAND:
            return get_command_string(std::any_cast<CommandEnum>(t.value));
            break;

        case TokenType::OPERATOR:
            return std::string(get_operator_string(std::any_cast<OperatorEnum>(t.value)));
            break;

        case TokenType::DATA_Char:
            return std::string("") + std::any_cast<char>(t.value);
            break;
        
        case TokenType::DATA_Number:
            return trim_num_string(std::to_string(std::any_cast<float>(t.value)));
            break;
        
        case TokenType::DATA_Bool:
            return (std::any_cast<bool>(t.value) == true) ? "true" : "false";
            break;
        
        case TokenType::LIST_START:
            return "LIST_START";
            break;
        
        case TokenType::LIST_END:
            return "LIST_END";
            break;
        
        case TokenType::SUB_LIST_START:
            return "SUB_LIST_START";
            break;
        
        case TokenType::SUB_LIST_END:
            return "SUB_LIST_END";
            break;

        default:
            return TokenTypeString[t.type];
            break;
    }

    return "";
}

std::string trim_num_string(const std::string &num)
{
    std::string res = "";

    for (int i = num.length() - 1; i > 0; i--)
    {
        if (num.at(i) != '0')
        {
            i += num.at(i) == '.' ? -1 : 0;

            for (int j = 0; j <= i; j++)
                res += num.at(j);

            break;
        }
    }

    return res;
}

int find_tag(const std::vector<Token> &list, const Token &tag)
{
    if (list.empty() || !is_tag(tag))
        return -1;

    std::string value = get_token_string(tag);
    int pos = -1;

    switch (tag.type)
    {
        case TokenType::TAG_GLOBAL:
        {
            for (int i = 0; i < list.size(); i++)
            {
                if (list.at(i).type == tag.type && get_token_string(list.at(i)) == value)
                    return i;
            }
            break;
        }
    
        case TokenType::TAG_LOCAL:
        {
            for (int i = list.size() - 1; i >= 0; i--)
            {
                if (list.at(i).type == TokenType::TAG_LOCAL && get_token_string(list.at(i)) == value)
                    pos = i;
                else if (list.at(i).type == TokenType::FUNCTION_CALL)
                    return pos;
            }

            return pos;
            break;
        }

        case TokenType::TAG_BLOCK:
        {
            for (int i = list.size() - 1; i >= 0; i--)
            {
                if (list.at(i).type == TokenType::TAG_BLOCK)
                    pos = (get_token_string(list.at(i)) == value) ? i : pos;
                else if (list.at(i).type == TokenType::LOOP_BLOCK || list.at(i).type == TokenType::CONDITION_BLOCK || list.at(i).type == TokenType::BLOCK)
                    return pos;
            }
            break;
        }
    
        case TokenType::TAG_MEMBER:
        {
            for (int i = list.size() - 1; i >= 0; i--)
            {
                if (list.at(i).type == TokenType::TAG_MEMBER)
                {
                    if (get_token_string(list.at(i)) == value)
                        return i;
                } else if (list.at(i).type == TokenType::LIST_START)
                    return -1;
            }
            break;
        }

        default:
            break;
    }

    return pos;
}

void error_msg(const Node* node, const char* explanation)
{
    std::cout << "[ERROR] " << included_files[node->file_source] << ":" << node->line << std::endl;
    std::cout << get_token_string(node->t) << std::endl;
    std::cout << explanation << std::endl;
}

bool is_valid_extension(const std::string &file, const std::string &extension)
{
    bool result = false;

    if (file.length() >= extension.length())
        result = file.substr(file.length() - extension.length()) == extension;

    return result;
}

std::string get_base_path(const std::string &file)
{
    std::string result = file;
    while (!result.empty() && result.back() != '/' && result.back() != '\\')
        result.erase(result.length() - 1, 1);

    return result;
}

#ifdef LINUX
#include <linux/limits.h>

std::string getexepath()
{
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) -1);
    std::string result = std::string(buffer, len);
    return get_base_path(result);
}
#endif

#ifdef WINDOWS
#include <windows.h>

std::string getexepath()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string result = std::string(buffer, sizeof buffer);
    return get_base_path(result);
}
#endif

