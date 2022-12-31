#ifndef EX5_CODE_GEN
#define EX5_CODE_GEN

#include <vector>
#include <string>

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

    std::string generate_label();

    int emit(const std::string& command);

    static std::vector<patch_record> make_list(patch_record item);

    static std::vector<patch_record> merge(const std::vector<patch_record>& first, const std::vector<patch_record>& second);

    void backpatch(const std::vector<patch_record>& patch_list, const std::string& label);

    void print_code_buffer() const;

    void emit_global(const std::string& line);

    void print_global_buffer() const;
};

#endif

