#include "lang.hpp"
#include <any>
#include <iostream>
#include <fstream>
#include <string>

std::string load_file(const char* path)
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
    else {
        delete_nodes(pointer->default_next);
        delete pointer;
    }
}

CommandEnum get_command_enum(const std::string val)
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
        return CommandEnum::CHACHE;
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

std::string get_token_string(const Token t)
{
    std::string result;

    switch (t.type)
    {
        case TokenType::NULL_TOKEN:
            result = "NULL_TOKEN";
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
            result = std::any_cast<std::string>(t.value);
            break;

        case TokenType::DATA_Char:
            result = std::any_cast<char>(t.value);
            break;
        
        case TokenType::DATA_Number:
            result = trim_num_string(std::to_string(std::any_cast<float>(t.value)));
            break;
        
        case TokenType::DATA_Bool:
            result = (std::any_cast<bool>(t.value) == true) ? "true" : "false";
            break;
        
        case TokenType::LIST_START:
            result = "[";
            break;
        
        case TokenType::LIST_END:
            result = "]";
            break;
        
        case TokenType::SUB_LIST_START:
            result = "{";
            break;
        
        case TokenType::SUB_LIST_END:
            result = "}";
            break;

        default:
            result = TokenTypeString[t.type];
            break;
    }

    return result;
}

std::string trim_num_string(const std::string num)
{
    std::string res = "";

    int end = num.length() - 1;

    for (int i = end; i > 0; i--)
    {
        if (num.at(i) != '0')
        {
            end = (num.at(i) == '.') ? i - 1 : i;
            break;
        }
    }

    for (int j = 0; j <= end; j++)
        res += num.at(j);

    return res;
}

int find_tag(const std::vector<Token> list, Token tag)
{
    if (list.empty())
        return -1;

    switch (tag.type)
    {
        case TokenType::TAG_GLOBAL:
        {
            std::string value = get_token_string(tag);
            for (int i = 0; i < list.size(); i++)
            {
                switch (list.at(i).type)
                {
                    case TokenType::TAG_GLOBAL:
                        if (get_token_string(list.at(i)) == value)
                            return i;
                    default:
                        break;
                }
            }
            break;
        }
    
        case TokenType::TAG_LOCAL:
        {
            std::string value = get_token_string(tag);
            int pos = -1;

            for (int i = list.size() - 1; i >= 0; i--)
            {
                switch (list.at(i).type)
                {
                    case TokenType::TAG_LOCAL:
                        if (get_token_string(list.at(i)) == value)
                            pos = i;
                        break;
                    case TokenType::FUNCTION_CALL:
                        return pos;
                    default:
                        break;
                }
            }

            return pos;
            break;
        }

        case TokenType::TAG_BLOCK:
        {
            std::string value = std::any_cast<std::string>(tag.value);
            int pos = -1;
            for (int i = list.size() - 1; i >= 0; i--)
            {
                switch (list.at(i).type)
                {
                    case TokenType::TAG_BLOCK:
                        pos = (get_token_string(list.at(i)) == value) ? i : pos;
                        break;
                    case TokenType::LOOP_BLOCK:
                    case TokenType::CONDITION_BLOCK:
                    case TokenType::BLOCK:
                        return pos;
                    default:
                        break;
                }
            }
            break;
        }
    
        case TokenType::TAG_MEMBER:
        {
            std::string value = std::any_cast<std::string>(tag.value);
            for (int i = list.size() - 1; i >= 0; i--)
            {
                switch (list.at(i).type)
                {
                    case TokenType::TAG_MEMBER:
                        if (get_token_string(list.at(i)) == value)
                            return i;
                        break;
                    case TokenType::LIST_START:
                        return -1;
                    default:
                        break;
                }
            }
            break;
        }

        default:
            break;
    }

    return -1;
}

void error_msg(Node* node, const char* explanation)
{
    std::cout << "[ERROR]" << std::endl;
    std::cout << "Line " << node->line << std::endl;
    std::cout << get_token_string(node->t) << "\n" << std::endl;
    std::cout << explanation << std::endl;
}
