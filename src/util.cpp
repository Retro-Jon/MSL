#include "lang.hpp"
#include <iostream>
#include <fstream>

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

std::string get_token_string(Token t)
{
    std::string result;

    switch (t.type)
    {
        case TokenType::NULL_TOKEN:
            result = "NULL_TOKEN";
            break;
        
        case TokenType::TAG_GLOBAL:
        case TokenType::TAG_LOCAL:
        case TokenType::TAG_MEMBER:
        case TokenType::OPERATOR:
        case TokenType::COMMAND:
        case TokenType::USER_FUNCTION:
        case TokenType::DATA_String:
            result = std::any_cast<std::string>(t.value);
            break;

        case TokenType::DATA_Char:
            result = std::any_cast<char>(t.value);
            break;
        
        case TokenType::DATA_Number:
            result = trim_num_string(std::to_string(std::any_cast<float>(t.value)));
            break;
        
        case TokenType::DATA_Bool:
            result = std::to_string(std::any_cast<bool>(t.value));
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

std::string trim_num_string(std::string num)
{
    std::string res = "";

    int i = num.length() - 1;
    for (i; i > 0; i--)
    {
        if (num.at(i) == '0')
            continue;

        if (num.at(i) == '.')
            i--;
        
        // Break if 0 is not found at the current position.
        break;
    }

    for (int j = 0; j <= i; j++)
        res += num.at(j);

    return res;
}

int find_tag(std::vector<Token> list, Token tag)
{
    if (list.size() == 0)
        return -1;

    int pos = -1;

    if (tag.type == TokenType::TAG_GLOBAL)
    {
        for (int i = 0; i < list.size() - 1; i++)
        {
            if (list.at(i).type != tag.type)
                continue;
            
            if (std::any_cast<std::string>(list.at(i).value) == std::any_cast<std::string>(tag.value))
            {
                pos = i;
                break;
            }
        }
    } else if (tag.type == TokenType::TAG_LOCAL)
    {
        for (int i = list.size() - 1; i >= 0; i--)
        {
            if (list.at(i).type == TokenType::FUNCTION_CALL)
            {
                pos = i;
                break;
            }
        }

        if (pos < 0)
            return pos;

        for (int i = pos; i < list.size() - 1; i++)
        {
            if (list.at(i).type != tag.type)
                continue;
            
            if (std::any_cast<std::string>(list.at(i).value) == std::any_cast<std::string>(tag.value))
            {
                pos = i;
                break;
            }
        }
    } else if (tag.type == TokenType::TAG_MEMBER)
    {
        for (int i = list.size() - 1; i >= 0; i--)
        {
            if (list.at(i).type == TokenType::LIST_START)
            {
                pos = i;
                break;
            }
        }

        for (int i = pos; i < list.size() - 1; i++)
        {
            if (list.at(i).type != tag.type)
                continue;
            
            if (std::any_cast<std::string>(list.at(i).value) == std::any_cast<std::string>(tag.value))
            {
                pos = i;
                break;
            }
        }
    }

    return pos;
}

bool is_tag(Token t)
{
    if (t.type == TokenType::TAG_GLOBAL || t.type == TokenType::TAG_LOCAL || t.type == TokenType::TAG_MEMBER)
        return true;

    return false;
}

bool is_value(Token t)
{
    if (t.type > TokenType::NULL_TOKEN && t.type <= TokenType::DATA_Bool)
        return true;
    return false;
}

void error_msg(Node* node, const char* explanation)
{
    std::cout << "[ERROR]" << std::endl;
    std::cout << "Line " << node->line << std::endl;
    std::cout << get_token_string(node->t) << "\n" << std::endl;
    std::cout << explanation << std::endl;
}
