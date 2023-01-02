#ifndef _BP_HPP_
#define _BP_HPP_

#include <list>
#include <vector>
#include <string>
#include <fstream>
#include "formatter.hpp"

enum class label_index { first, second };

struct patch_record
{
    public:

    int line = 0;
    label_index index;

    patch_record(int line, label_index index);
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

    std::string register_name() const;

    std::string label_name() const;

    std::string emit_label();

    int emit(const std::string& line);

    template<typename ... Args>
    int emit(const std::string& line, Args ... args)
    {
        std::string formatted = formatter::format(line, args ...);

        buffer.push_back(formatted);

        return buffer.size() - 1;
    }

    int emit_from_file(std::ifstream file);

    static std::list<patch_record> make_list(patch_record item);

    static std::list<patch_record> merge(const std::list<patch_record>& first, const std::list<patch_record>& second);

    void backpatch(const std::list<patch_record>& patch_list, const std::string& label);

    void print_code_buffer() const;

    void emit_global(const std::string& line);

    void print_global_buffer() const;
};

#endif

