#ifndef _GENERIC_SYNTAX_HPP_
#define _GENERIC_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "abstract_syntax.hpp"
#include <list>
#include <vector>
#include <string>
#include <type_traits>

template<typename element_type> class list_syntax final: public syntax_base
{
    private:

    std::list<element_type*> elements;

    public:

    list_syntax(): elements()
    {
        static_assert(std::is_base_of<syntax_base, element_type>::value, "must be of type syntax_base");
    }

    list_syntax(element_type* element): elements{ element }
    {
        static_assert(std::is_base_of<syntax_base, element_type>::value, "must be of type syntax_base");

        element->set_parent(this);
    }

    list_syntax(const list_syntax& other) = delete;

    list_syntax& operator=(const list_syntax& other) = delete;

    list_syntax<element_type>* push_back(element_type* element)
    {
        elements.push_back(element);
        return this;
    }

    list_syntax<element_type>* push_front(element_type* element)
    {
        elements.push_front(element);
        return this;
    }

    const std::vector<element_type*> get_elements() const
    {
        return std::vector<element_type*>(elements.begin(), elements.end());
    }

    std::vector<syntax_base*> get_children() const override
    {
        return std::vector<syntax_base*>(elements.begin(), elements.end());
    }

    std::vector<syntax_token*> get_tokens() const override
    {
        return std::vector<syntax_token*>();
    }

    std::size_t size() const
    {
        return elements.size();
    }

    typename std::list<element_type*>::const_iterator begin() const
    {
        return elements.begin();
    }

    typename std::list<element_type*>::const_iterator end() const
    {
        return elements.end();
    }

    void emit() override
    {
        
    }

    ~list_syntax()
    {
        for (syntax_base* child : get_children())
        {
            delete child;
        }

        for (syntax_token* token : get_tokens())
        {
            delete token;
        }
    }
};

class type_syntax final: public syntax_base
{
    public:

    syntax_token* const type_token;
    const fundamental_type type;

    type_syntax(syntax_token* type_token);

    type_syntax(const type_syntax& other) = delete;

    type_syntax& operator=(const type_syntax& other) = delete;

    bool is_numeric() const;

    bool is_special() const;

    void emit() override;

    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~type_syntax();
};

class formal_syntax final: public syntax_base
{
    public:

    type_syntax* const type;
    syntax_token* const identifier_token;
    const std::string identifier;

    formal_syntax(type_syntax* type, syntax_token* identifier_token);

    formal_syntax(const formal_syntax& other) = delete;

    formal_syntax& operator=(const formal_syntax& other) = delete;

    void emit() override;

    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~formal_syntax();
};

class function_declaration_syntax final: public syntax_base
{
    public:

    type_syntax* const return_type;
    syntax_token* const identifier_token;
    const std::string identifier;
    list_syntax<formal_syntax>* const formal_list;
    list_syntax<statement_syntax>* const body;

    function_declaration_syntax(type_syntax* return_type, syntax_token* identifier_token, list_syntax<formal_syntax>* formal_list, list_syntax<statement_syntax>* body);

    function_declaration_syntax(const function_declaration_syntax& other) = delete;

    function_declaration_syntax& operator=(const function_declaration_syntax& other) = delete;

    void emit() override;

    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~function_declaration_syntax();
};

class root_syntax final: public syntax_base
{
    public:

    list_syntax<function_declaration_syntax>* const function_list;

    root_syntax(list_syntax<function_declaration_syntax>* function_list);

    root_syntax(const root_syntax& other) = delete;

    root_syntax& operator=(const root_syntax& other) = delete;

    void emit() override;

    std::vector<syntax_base*> get_children() const override;

    std::vector<syntax_token*> get_tokens() const override;

    ~root_syntax();
};

#endif
