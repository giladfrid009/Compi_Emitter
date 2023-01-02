#ifndef _IR_BUILDER_HPP_
#define _IR_BUILDER_HPP_

#include "types.hpp"
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

class ir_builder
{
    public:

    template<typename ... Args>
    static std::string format_string(const std::string& format, Args ... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), as_ctype(args) ...) + 1;

        if (size_s <= 0)
        {
            throw std::runtime_error("Error during formatting.");
        }

        auto size = static_cast<size_t>(size_s);

        std::unique_ptr<char[]> buffer(new char[size]);

        std::snprintf(buffer.get(), size, format.c_str(), as_ctype(args) ...);

        return std::string(buffer.get(), buffer.get() + size - 1);
    }

    static std::string fresh_register();

    static std::string fresh_label();

    static std::string get_type(type_kind type);

    private:

    template<class T>
    static T const& as_ctype(T const& arg)
    {
        return arg;
    }

    static const char* as_ctype(std::string const& arg)
    {
        return arg.c_str();
    }
};

#endif