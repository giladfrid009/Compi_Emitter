#ifndef _BP_HPP_
#define _BP_HPP_

#include "ir_builder.hpp"
#include <list>
#include <vector>
#include <string>
#include <fstream>

enum class label_index { first, second };

struct patch_record
{
    public:

    size_t line;
    label_index index;

    patch_record(size_t line, label_index index);
};

class code_buffer
{
    private:

    std::vector<std::string> buffer;
    std::vector<std::string> global_defs;

    code_buffer();

    code_buffer(code_buffer const&) = delete;

    void operator=(code_buffer const&) = delete;

    public:

    static code_buffer& instance();

    std::string emit_label();

    size_t emit(const std::string& line);

    template<typename ... Args>
    size_t emit(const std::string& line, Args ... args)
    {
        std::string formatted = ir_builder::format_string(line, args ...);

        buffer.push_back(formatted);

        return buffer.size() - 1;
    }

    size_t emit_from_file(std::ifstream file);

    static std::list<patch_record> make_list(patch_record item);

    static std::list<patch_record> merge(const std::list<patch_record>& first, const std::list<patch_record>& second);

    void backpatch(const std::list<patch_record>& patch_list, const std::string& label);

    void print() const;

    void emit_global(const std::string& line);

    void print_globals() const;
};

#endif

