#ifndef _GENERIC_SYNTAX_HPP_
#define _GENERIC_SYNTAX_HPP_

#include "syntax_token.hpp"
#include "abstract_syntax.hpp"
#include <list>
#include <vector>
#include <string>

template<typename element_type> class list_syntax final: public syntax_base
{
    private:

    std::list<element_type*> _elements;

    public:

    list_syntax(): _elements()
    {
        static_assert(std::is_base_of<syntax_base, element_type>::value, "must be of type syntax_base");
    }

    list_syntax(element_type* element): list_syntax()
    {
        push_back(element);
    }

    list_syntax(const list_syntax& other) = delete;

    list_syntax& operator=(const list_syntax& other) = delete;

    element_type* front() const
    {
        return _elements.front();
    }

    element_type* back() const
    {
        return _elements.back();
    }

    list_syntax<element_type>* push_back(element_type* element)
    {
        _elements.push_back(element);

        add_child(element);

        return this;
    }

    list_syntax<element_type>* push_front(element_type* element)
    {
        _elements.push_front(element);

        add_child_front(element);

        return this;
    }

    std::size_t size() const
    {
        return _elements.size();
    }

    typename std::list<element_type*>::const_iterator begin() const
    {
        return _elements.begin();
    }

    typename std::list<element_type*>::const_iterator end() const
    {
        return _elements.end();
    }

    ~list_syntax()
    {
        for (syntax_base* child : children())
        {
            delete child;
        }
    }

    void analyze() const override
    {
    }

    void emit() override
    {
        for (element_type* element : _elements)
        {
            element->emit();
        }
    }
};

class type_syntax final: public syntax_base
{
    public:

    const syntax_token* const type_token;
    const type_kind kind;

    type_syntax(syntax_token* type_token);
    ~type_syntax();

    type_syntax(const type_syntax& other) = delete;
    type_syntax& operator=(const type_syntax& other) = delete;

    bool is_numeric() const;
    bool is_special() const;

    void analyze() const override;
    void emit() override;
};

class parameter_syntax final: public syntax_base
{
    public:

    type_syntax* const type;
    const syntax_token* const identifier_token;
    const std::string identifier;

    parameter_syntax(type_syntax* type, syntax_token* identifier_token);
    ~parameter_syntax();

    parameter_syntax(const parameter_syntax& other) = delete;
    parameter_syntax& operator=(const parameter_syntax& other) = delete;

    void analyze() const override;
    void emit() override;
};

class function_header_syntax final: public syntax_base
{
    public:

    type_syntax* const return_type;
    const syntax_token* const identifier_token;
    const std::string identifier;
    list_syntax<parameter_syntax>* const parameters;

    function_header_syntax(type_syntax* return_type, syntax_token* identifier_token, list_syntax<parameter_syntax>* parameters);
    ~function_header_syntax();

    function_header_syntax(const function_header_syntax& other) = delete;
    function_header_syntax& operator=(const function_header_syntax& other) = delete;

    void analyze() const override;
    void emit() override;
};

class function_declaration_syntax final: public syntax_base
{
    public:

    function_header_syntax* const header;
    list_syntax<statement_syntax>* const body;

    function_declaration_syntax(function_header_syntax* header, list_syntax<statement_syntax>* body);
    ~function_declaration_syntax();

    function_declaration_syntax(const function_declaration_syntax& other) = delete;
    function_declaration_syntax& operator=(const function_declaration_syntax& other) = delete;

    void analyze() const override;
    void emit() override;
};

class root_syntax final: public syntax_base
{
    public:

    list_syntax<function_declaration_syntax>* const functions;

    root_syntax(list_syntax<function_declaration_syntax>* functions);
    ~root_syntax();

    root_syntax(const root_syntax& other) = delete;
    root_syntax& operator=(const root_syntax& other) = delete;

    void analyze() const override;
    void emit() override;
};

#endif
