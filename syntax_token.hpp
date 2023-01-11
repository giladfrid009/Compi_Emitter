#ifndef _SYNTAX_TOKEN_HPP_
#define _SYNTAX_TOKEN_HPP_

#include <string>

class syntax_token
{
    public:

    const int type;
    const int position;
    const std::string text;

    syntax_token(int type, int position, const std::string& text): type(type), position(position), text(text)
    {

    }
};

#endif