#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <vector>
#include <string>

namespace output
{
    [[noreturn]] void error_lex(int lineno);

    [[noreturn]] void error_syn(int lineno);

    [[noreturn]] void error_undef(int lineno, const std::string& id);

    [[noreturn]] void error_def(int lineno, const std::string& id);

    [[noreturn]] void error_undef_func(int lineno, const std::string& id);

    [[noreturn]] void error_mismatch(int lineno);

    [[noreturn]] void error_prototype_mismatch(int lineno, const std::string& id, std::vector<std::string>& arg_types);

    [[noreturn]] void error_unexpected_break(int lineno);

    [[noreturn]] void error_unexpected_continue(int lineno);

    [[noreturn]] void error_main_missing();

    [[noreturn]] void error_byte_too_large(int lineno, const std::string& value);
}

#endif