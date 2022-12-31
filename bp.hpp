#ifndef EX5_CODE_GEN
#define EX5_CODE_GEN

#include <vector>
#include <string>

//this enum is used to distinguish between the two possible missing labels of a conditional branch in LLVM during backpatching.
//for an unconditional branch (which contains only a single label) use FIRST.
enum branch_label_index { FIRST, SECOND };

class code_buffer
{
    std::vector<std::string> buffer;
    std::vector<std::string> global_defs;

    code_buffer();
    code_buffer(code_buffer const&);
    void operator=(code_buffer const&);

    public:

    static code_buffer& instance();

    // ******** Methods to handle the code section ******** //

    //generates a jump location label for the next command, writes it to the buffer and returns it
    std::string generate_label();

    //writes command to the buffer, returns its location in the buffer
    int emit(const std::string& command);

    //gets a pair<int,branch_label_index> item of the form {buffer_location, branch_label_index} and creates a list for it
    static std::vector<std::pair<int, branch_label_index>> make_list(std::pair<int, branch_label_index> item);

    //merges two lists of {buffer_location, branch_label_index} items
    static std::vector<std::pair<int, branch_label_index>> merge(const std::vector<std::pair<int, branch_label_index>>& l1, const std::vector<std::pair<int, branch_label_index>>& l2);

    /* accepts a list of {buffer_location, branch_label_index} items and a label.
    For each {buffer_location, branch_label_index} item in address_list, backpatches the branch command
    at buffer_location, at index branch_label_index (FIRST or SECOND), with the label.
    note - the function expects to find a '@' char in place of the missing label.
    note - for unconditional branches (which contain only a single label) use FIRST as the branch_label_index.
    example #1:
    int loc1 = emit("br label @");  - unconditional branch missing a label. ~ Note the '@' ~
    backpatch(make_list({loc1,FIRST}),"my_label"); - location loc1 in the buffer will now contain the command "br label %my_label"
    note that index FIRST referes to the one and only label in the line.
    example #2:
    int loc2 = emit("br i1 %cond, label @, label @"); - conditional branch missing two labels.
    backpatch(make_list({loc2,SECOND}),"my_false_label"); - location loc2 in the buffer will now contain the command "br i1 %cond, label @, label %my_false_label"
    backpatch(make_list({loc2,FIRST}),"my_true_label"); - location loc2 in the buffer will now contain the command "br i1 %cond, label @my_true_label, label %my_false_label"
    */
    void backpatch(const std::vector<std::pair<int, branch_label_index>>& address_list, const std::string& label);

    //prints the content of the code buffer to stdout
    void print_code_buffer() const;

    // ******** Methods to handle the data section ******** //
    //write a line to the global section
    void emit_global(const std::string& data_line);

    //print the content of the global buffer to stdout
    void print_global_buffer() const;
};

#endif

