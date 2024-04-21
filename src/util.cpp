#include "lang.hpp"
#include <any>
#include <iostream>
#include <fstream>
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

    return res;
}

void delete_nodes(Node* pointer)
{
    if (pointer == nullptr)
        return;
    
    delete_nodes(pointer->default_next);
    delete pointer;
    pointer = nullptr;
}

CommandEnum get_command_enum(const std::string &val)
{
    if (val == "print")
        return CommandEnum::PRINT;
    if (val == "println")
        return CommandEnum::PRINTLN;
    if (val == "input")
        return CommandEnum::INPUT;
    if (val == "print-stack")
        return CommandEnum::PRINT_STACK;
    if (val == "drop")
        return CommandEnum::DROP;
    if (val == "drop-list")
        return CommandEnum::DROP_LIST;
    if (val == "at")
        return CommandEnum::AT;
    if (val == "get")
        return CommandEnum::GET;
    if (val == "get-list")
        return CommandEnum::GET_LIST;
    if (val == "get-list-values")
        return CommandEnum::GET_LIST_VALUES;
    if (val == "merge")
        return CommandEnum::MERGE;
    if (val == "merge-x")
        return CommandEnum::MERGE_X;
    if (val == "int")
        return CommandEnum::INT;
    if (val == "if")
        return CommandEnum::IF;
    if (val == "?")
        return CommandEnum::ERROR_HANDLER;
    if (val == "begin")
        return CommandEnum::BEGIN;
    if (val == "loop")
        return CommandEnum::LOOP;
    if (val == "while")
        return CommandEnum::WHILE;
    if (val == "for")
        return CommandEnum::FOR;
    if (val == "defunc")
        return CommandEnum::DEFUNC;
    if (val == "$")
        return CommandEnum::CACHE;
    if (val == "return")
        return CommandEnum::RETURN;
    if (val == "end")
        return CommandEnum::END;
    if (val == "break")
        return CommandEnum::BREAK;
    if (val == "continue")
        return CommandEnum::CONTINUE;
    if (val == "swap")
        return CommandEnum::SWAP;
    if (val == "swap-list")
        return CommandEnum::SWAP_LIST;
    if (val == "dup")
        return CommandEnum::DUP;
    if (val == "dup-x")
        return CommandEnum::DUP_X;
    if (val == "dup-list")
        return CommandEnum::DUP_LIST;
    if (val == "define")
        return CommandEnum::DEFINE;
    if (val == "include")
        return CommandEnum::INCLUDE;
    if (val == "strlen")
        return CommandEnum::STRLEN;
    if (val == "len")
        return CommandEnum::LEN;
    if (val == "cat")
        return CommandEnum::CAT;
    if (val == "exit")
        return CommandEnum::EXIT;

    return CommandEnum::UNKNOWN_COMMAND;
}

const char* get_command_string(CommandEnum c)
{
    switch (c)
    {
        case CommandEnum::PRINT:
            return "print";
        case CommandEnum::PRINTLN:
            return "println";
        case CommandEnum::INPUT:
            return "input";
        case CommandEnum::PRINT_STACK:
            return "print-stack";
        case CommandEnum::DROP:
            return "drop";
        case CommandEnum::DROP_LIST:
            return "drop-list";
        case CommandEnum::AT:
            return "at";
        case CommandEnum::GET:
            return "get";
        case CommandEnum::GET_LIST:
            return "get-list";
        case CommandEnum::GET_LIST_VALUES:
            return "get-list-values";
        case CommandEnum::MERGE:
            return "merge";
        case CommandEnum::MERGE_X:
            return "merge-x";
        case CommandEnum::INT:
            return "int";
        case CommandEnum::IF:
            return "if";
        case CommandEnum::ERROR_HANDLER:
            return "?";
        case CommandEnum::BEGIN:
            return "begin";
        case CommandEnum::LOOP:
            return "loop";
        case CommandEnum::WHILE:
            return "while";
        case CommandEnum::FOR:
            return "for";
        case CommandEnum::DEFUNC:
            return "defunc";
        case CommandEnum::CACHE:
            return "$";
        case CommandEnum::RETURN:
            return "return";
        case CommandEnum::END:
            return "end";
        case CommandEnum::BREAK:
            return "break";
        case CommandEnum::CONTINUE:
            return "continue";
        case CommandEnum::SWAP:
            return "swap";
        case CommandEnum::SWAP_LIST:
            return "swap-list";
        case CommandEnum::DUP:
            return "dup";
        case CommandEnum::DUP_X:
            return "dup-x";
        case CommandEnum::DUP_LIST:
            return "dup-list";
        case CommandEnum::DEFINE:
            return "define";
        case CommandEnum::INCLUDE:
            return "include";
        case CommandEnum::STRLEN:
            return "strlen";
        case CommandEnum::LEN:
            return "len";
        case CommandEnum::CAT:
            return "cat";
        case CommandEnum::EXIT:
            return "exit";
        default:
            return "UNKNOWN_COMMAND";
    }
}

OperatorEnum get_operator_enum(const std::string &val)
{
    if (val == "+")
        return OperatorEnum::ADDITION;
    if (val == "-")
        return OperatorEnum::SUBTRACTION;
    if (val == "*")
        return OperatorEnum::MULTIPLICATION;
    if (val == "/")
        return OperatorEnum::DIVISION;
    if (val == "++")
        return OperatorEnum::INCREMENT;
    if (val == "--")
        return OperatorEnum::DECREMENT;
    if (val == "%")
        return OperatorEnum::MODULO;
    if (val == "=")
        return OperatorEnum::ASSIGNMENT;
    if (val == "~")
        return OperatorEnum::NEGATE;
    if (val == "and")
        return OperatorEnum::AND;
    if (val == "or")
        return OperatorEnum::OR;
    if (val == "==")
        return OperatorEnum::EQUAL;
    if (val == "!=")
        return OperatorEnum::NOT_EQUAL;
    if (val == ">")
        return OperatorEnum::GREATER_THAN;
    if (val == ">=")
        return OperatorEnum::GREATER_THAN_EQUAL;
    if (val == "<")
        return OperatorEnum::LESS_THAN;
    if (val == "<=")
        return OperatorEnum::LESS_THAN_EQUAL;

    return OperatorEnum::UNKNOWN_OPERATOR;
}

const char* get_operator_string(const OperatorEnum &op)
{
    switch (op)
    {
        case OperatorEnum::ADDITION:
            return "+";
        case OperatorEnum::SUBTRACTION:
            return "-";
        case OperatorEnum::MULTIPLICATION:
            return "*";
        case OperatorEnum::DIVISION:
            return "/";
        case OperatorEnum::INCREMENT:
            return "++";
        case OperatorEnum::DECREMENT:
            return "--";
        case OperatorEnum::MODULO:
            return "%";
        case OperatorEnum::ASSIGNMENT:
            return "=";
        case OperatorEnum::NEGATE:
            return "~";
        case OperatorEnum::AND:
            return "and";
        case OperatorEnum::OR:
            return "or";
        case OperatorEnum::EQUAL:
            return "==";
        case OperatorEnum::NOT_EQUAL:
            return "!=";
        case OperatorEnum::GREATER_THAN:
            return ">";
        case OperatorEnum::GREATER_THAN_EQUAL:
            return ">=";
        case OperatorEnum::LESS_THAN:
            return "<";
        case OperatorEnum::LESS_THAN_EQUAL:
            return "<=";
        default:
            return "UNKNOWN_OPERATOR";
    }
}

std::string get_token_string(const Token &t)
{
    switch (t.type)
    {
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
            return std::string(get_command_string(std::any_cast<CommandEnum>(t.value)));
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

                if (list.at(i).type == TokenType::FUNCTION_CALL)
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

