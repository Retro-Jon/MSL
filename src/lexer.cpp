#include "lang.hpp"
#include <any>
#include <cctype>

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
                pointer->t.type = (val.back() == ')') ? TokenType::USER_FUNCTION : pointer->t.type;
                break;
            case '_':
                pointer->t.type = (val.back() == '_') ? TokenType::CONSTANT : pointer->t.type;
                break;
            default:
                break;
        }

        if (pointer->t.type == TokenType::NULL_TOKEN)
        {
            if (val.front() == '\'' && val.back() == '\'')
            {
                pointer->t.type = TokenType::DATA_Char;
                std::string char_data = val;
                char_data.erase(char_data.begin());
                char_data.erase(char_data.end() - 1);

                if (char_data.length() > 1)
                {
                    if (char_data.front() != '\\')
                    {
                        error_msg(pointer, "A Char can only be 1 character long with the exception of a leading \\");
                        return false;
                    }
                }
                
                pointer->t.value = std::any_cast<char>(char_data.front());

            } else if (val.front() == '"' && val.back() == '"')
            {
                pointer->t.type = TokenType::DATA_String;
                val.erase(0, 1);
                val.erase(val.length() - 1, 1);
                pointer->t.value = val;

            } else if (val == "true" || val == "false")
            {
                pointer->t.type = TokenType::DATA_Bool;
                
                if (val == "true")
                    pointer->t.value = true;
                else if (val == "false")
                    pointer->t.value = false;
                else
                {
                    error_msg(pointer, "Unknown boolean value.");
                    return false;
                }
 
            } else if (val.length() > 2 && val.front() == '<' && val.back() == '>')
            {
                if (val.find(".local>") != std::string::npos || val.find(".l>") != std::string::npos)
                    pointer->t.type = TokenType::TAG_LOCAL;
                else if (val.find(".member>") != std::string::npos || val.find(".m>") != std::string::npos)
                    pointer->t.type = TokenType::TAG_MEMBER;
                else if (val.find(".block>") != std::string::npos || val.find(".b>") != std::string::npos)
                    pointer->t.type = TokenType::TAG_BLOCK;
                else if (val.find(".global>") != std::string::npos || val.find(".g>") != std::string::npos || val.find(">") != std::string::npos)
                    pointer->t.type = TokenType::TAG_GLOBAL;
                else
                {
                    error_msg(pointer, std::string("Incomplete TAG: " + val).c_str());
                    return false;
                    break;
                }

            } else {
                if ((val != "--" && val != "++") && (isdigit(val.front()) || (val.front() == '-') && val.length() > 1))
                {
                    if (is_num(val))
                    {
                        pointer->t.type = TokenType::DATA_Number;
                        pointer->t.value = std::any_cast<float>(std::stof(val));
                    }
                }

                if (pointer->t.type != TokenType::NULL_TOKEN)
                {
                    int i = 0;
                    for (char c : val)
                    {
                        if (!isdigit(c) && c != '.' && c != '-')
                        {
                            pointer->t.type = TokenType::DATA_String;
                            error_msg(pointer, "Numeric values cannot include letters.");
                            return false;
                        }
                        i++;
                    }
                } else {
                    OperatorEnum o = get_operator_enum(val);
                    CommandEnum c = get_command_enum(val);

                    if (o != OperatorEnum::UNKNOWN_OPERATOR)
                    {
                        pointer->t.value = o;
                        pointer->t.type = TokenType::OPERATOR;
                    } else if (c != CommandEnum::UNKNOWN_COMMAND)
                    {
                        pointer->t.value = c;
                        pointer->t.type = TokenType::COMMAND;
                    } else {
                        error_msg(pointer, "Unknown command.");
                        return false;
                    }
                }
            }
        }

        pointer = pointer->default_next;
    }

    pointer = nodes;

    while (pointer != nullptr)
    {
        std::string val = get_token_string(pointer->t);
        
        switch (pointer->t.type)
        {
            case TokenType::NULL_TOKEN:
                error_msg(pointer, "Unidentified token");
                return false;
                break;

            default:
                break;
        }
        
        pointer = pointer->default_next;
    }
    
    return true;
}
