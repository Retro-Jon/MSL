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
    if (val == "define")
        return CommandEnum::DEFINE;
    if (val == "include")
        return CommandEnum::INCLUDE;
    if (val == "exit")
        return CommandEnum::EXIT;

    return CommandEnum::UNKNOWN_COMMAND;
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
        case TokenType::OPERATOR:
        case TokenType::COMMAND:
        case TokenType::USER_FUNCTION:
        case TokenType::DATA_String:
        case TokenType::CONSTANT:
            return std::any_cast<std::string>(t.value);
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
    if (list.empty())
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

void error_msg(Node* node, const std::string &explanation)
{
    std::cout << "[ERROR]" << std::endl;
    std::cout << "Line " << node->line << std::endl;
    std::cout << get_token_string(node->t) << "\n" << std::endl;
    std::cout << explanation << std::endl;
}

bool is_valid_extension(const std::string &file, const std::string &extension)
{
    bool result = false;

    if (file.length() >= extension.length())
        result = file.substr(file.length() - extension.length()) == extension;

    return result;
}

#ifdef LINUX
#include <linux/limits.h>

std::string getexepath()
{
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) -1);
    std::string result = std::string(buffer, len);
    while (!result.empty() && result.back() != '/' && result.back() != '\\')
        result.erase(result.length() - 1, 1);

    return result;
}
#endif

#ifdef WINDOWS
#include <windows.h>

std::string getexepath()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string result = std::string(buffer, sizeof buffer);
    while (!result.empty() && result.back() != '/' && result.back() != '\\')
        result.erase(result.length() - 1, 1);

    return result;
}
#endif

