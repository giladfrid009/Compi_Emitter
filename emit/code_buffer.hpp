#ifndef _BP_HPP_
#define _BP_HPP_

#include "ir_builder.hpp"
#include <list>
#include <vector>
#include <string>
#include <fstream>

class code_buffer
{
    private:

    int _indent;
    std::vector<std::string> _buffer;
    std::vector<std::string> _global_defs;

    code_buffer();

    code_buffer(code_buffer const&) = delete;

    void operator=(code_buffer const&) = delete;

    public:

    static code_buffer& instance();

    void increase_indent();

    void decrease_indent();

    std::string emit_label();

    size_t emit(const std::string& line);

    template<typename ... Args>
    size_t emit(const std::string& line, Args ... args)
    {
        std::string formatted = ir_builder::format_string(line, args ...);

        return emit(formatted);
    }

    void new_line();

    void comment(std::string line);

    size_t emit_from_file(std::string file_path);

    void backpatch(const std::list<size_t>& patch_list, const std::string& label);

    void print() const;

    void emit_global(const std::string& line);

    void print_globals() const;
};

#endif

