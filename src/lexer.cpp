#include "lang.hpp"
#include <set>

const std::set<std::string> operators = {
    "+", "-", "*", "/", "++", "--", "%", "=", "~",
    "and", "or", "==", "!=", ">", ">=", "<", "<="
};

bool lex(Node* nodes)
{
    Node* pointer = nodes;
    
    while (pointer != nullptr)
    {
        std::string val = std::any_cast<std::string>(pointer->t.value);

        switch (val.front())
        {
            case '[':
                pointer->t.type = TokenType::LIST_START;
                break;
            case ']':
                pointer->t.type = TokenType::LIST_END;
                break;
            case '{':
                pointer->t.type = TokenType::SUB_LIST_START;
                break;
            case '}':
                pointer->t.type = TokenType::SUB_LIST_END;
                break;
            case '(':
                if (val.back() == ')')
                    pointer->t.type = TokenType::USER_FUNCTION;
                break;
            case '_':
                if (val.back() == '_')
                    pointer->t.type = TokenType::CONSTANT;
                break;
            default:
                break;
        }

        if (pointer->t.type == TokenType::NULL_TOKEN)
        {
            if (val.front() == '\'' && val.back() == '\'')
                pointer->t.type = TokenType::DATA_Char;
            else if (val.front() == '"' && val.back() == '"')
            {
                pointer->t.type = TokenType::DATA_String;
                val.erase(0, 1);
                val.erase(val.length() - 1, 1);
                pointer->t.value = val;
            } else if (val == "true" || val == "false")
                pointer->t.type = TokenType::DATA_Bool;
            else if (val.back() == '>' && val.length() > 1)
            {
                if (val.find("<..") != std::string::npos)
                    pointer->t.type = TokenType::TAG_LOCAL;
                else if (val.find("<.") != std::string::npos)
                    pointer->t.type = TokenType::TAG_MEMBER;
                else if (val.find("<!") != std::string::npos)
                    pointer->t.type = TokenType::TAG_BLOCK;
                else if (val.find("<") != std::string::npos)
                    pointer->t.type = TokenType::TAG_GLOBAL;
                else
                {
                    error_msg(pointer, "Incomplete TAG");
                    return false;
                    break;
                }
            } else {
                if (isdigit(val.front()) || (val.front() == '-' && val.length() > 1))
                {
                    bool not_num = false;
                    for (char c : val)
                    {
                        if (!isdigit(c) && c != '.' && c != '-')
                        {
                            not_num = true;
                            break;
                        }
                    }

                    if (not_num == false)
                        pointer->t.type = TokenType::DATA_Number;
                }

                if (pointer->t.type != TokenType::NULL_TOKEN)
                {
                    int i = 0;
                    for (char c : val)
                    {
                        if (!isdigit(c) && c != '.' && c != '-')
                        {
                            error_msg(pointer, "Numeric values cannot include letters.");
                            return false;
                        }
                        i++;
                    }
                } else {
                    if (operators.find(val) != operators.end())
                        pointer->t.type = TokenType::OPERATOR;
                    else
                        pointer->t.type = TokenType::COMMAND;
                }
            }
        }

        pointer = pointer->default_next;
    }

    pointer = nodes;

    while (pointer != nullptr)
    {
        std::string val = std::any_cast<std::string>(pointer->t.value);
        
        switch (pointer->t.type)
        {
            case TokenType::NULL_TOKEN:
                error_msg(pointer, "Unidentified token");
                return false;
                break;
            
            case TokenType::DATA_Char:
            {
                char c;
                std::string char_data = val;
                char_data.erase(char_data.begin());
                char_data.erase(char_data.end() - 1);

                if (char_data.length() > 1)
                {
                    if (char_data.front() != '\\')
                    {
                        error_msg(pointer, "A Char can only be 2 character long with the exception of a leading \\");
                        return false;
                    }
                }
                
                c = char_data.front();
                pointer->t.value = std::any_cast<char>(c);
                break;
            }

            case TokenType::DATA_Number:
            {
                pointer->t.value = std::any_cast<float>(std::stof(val));
                break;
            }

            case TokenType::DATA_Bool:
            {
                if (val == "true")
                    pointer->t.value = true;
                else if (val == "false")
                    pointer->t.value = false;
                else
                {
                    error_msg(pointer, "Unknown boolean value.");
                    return false;
                }
            }
            default:
                break;
        }
        
        pointer = pointer->default_next;
    }
    
    return true;
}
