%{

#include <stdlib.h>
#include <string>
#include "parser.tab.hpp"
#include "errors.hpp"
#include "syntax/syntax_token.hpp"

yytoken_kind_t new_token(yytoken_kind_t kind);

%}

%option yylineno
%option noyywrap
%option nounput

%%

[ \t\r\n]*                         { ; }
\/\/[^\r\n]*[\r|\n|\r\n]?          { ; }
void                               { return new_token(VOID); }
int                                { return new_token(INT); }
byte                               { return new_token(BYTE); }
b                                  { return new_token(B); }
bool                               { return new_token(BOOL); }
and                                { return new_token(AND); }
or                                 { return new_token(OR); }
not                                { return new_token(NOT); }
true                               { return new_token(TRUE); }
false                              { return new_token(FALSE); }
return                             { return new_token(RETURN); }
if                                 { return new_token(IF); }
else                               { return new_token(ELSE); }
while                              { return new_token(WHILE); }
break                              { return new_token(BREAK); }
continue                           { return new_token(CONTINUE); }
;                                  { return SC; }
,                                  { return COMMA; }
\(                                 { return LPAREN; }
\)                                 { return RPAREN; }
\{                                 { return LBRACE; }
\}                                 { return RBRACE; }
=                                  { return new_token(ASSIGN); }
==|!=                              { return new_token(EQOP); }
\<|>|<=|>=                         { return new_token(RELOP); }
\+|\-                              { return new_token(ADDOP); }
\*|\/                              { return new_token(MULOP); }
[a-zA-Z][a-zA-Z0-9]*               { return new_token(ID); }
0|[1-9][0-9]*                      { return new_token(NUM); }
\"([^\n\r\"\\]|\\[rnt"\\])+\"      { return new_token(STRING); }
<<EOF>>                            { return END; }
.                                  { output::error_lex(yylineno); }

%%

yytoken_kind_t new_token(yytoken_kind_t kind)
{
    yylval.token = new syntax_token(kind, yylineno, yytext);
    return kind;
}