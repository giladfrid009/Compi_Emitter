%{

#include <stdlib.h>
#include <string>
#include "parser.tab.hpp"
#include "hw3_output.hpp"
#include "syntax_token.hpp"

void set_token(yytoken_kind_t kind);

%}

%option yylineno
%option noyywrap

%%

[ \t\r\n]*                         { ; }
\/\/[^\r\n]*[\r|\n|\r\n]?          { ; }
void                               { set_token(VOID); return VOID; }
int                                { set_token(INT); return INT; }
byte                               { set_token(BYTE); return BYTE; }
b                                  { set_token(B); return B; }
bool                               { set_token(BOOL); return BOOL; }
and                                { set_token(AND); return AND; }
or                                 { set_token(OR); return OR; }
not                                { set_token(NOT); return NOT; }
true                               { set_token(TRUE); return TRUE; }
false                              { set_token(FALSE); return FALSE; }
return                             { set_token(RETURN); return RETURN; }
if                                 { set_token(IF); return IF; }
else                               { set_token(ELSE); return ELSE; }
while                              { set_token(WHILE); return WHILE; }
break                              { set_token(BREAK); return BREAK; }
continue                           { set_token(CONTINUE); return CONTINUE; }
;                                  { return SC; }
,                                  { return COMMA; }
\(                                 { return LPAREN; }
\)                                 { return RPAREN; }
\{                                 { return LBRACE; }
\}                                 { return RBRACE; }
=                                  { set_token(ASSIGN); return ASSIGN; }
==|!=                              { set_token(EQOP); return EQOP; }
\<|>|<=|>=                         { set_token(RELOP); return RELOP; }
\+|\-                              { set_token(ADDOP); return ADDOP; }
\*|\/                              { set_token(MULOP); return MULOP; }
[a-zA-Z][a-zA-Z0-9]*               { set_token(ID); return ID; }
0|[1-9][0-9]*                      { set_token(NUM); return NUM; }
\"([^\n\r\"\\]|\\[rnt"\\])+\"      { set_token(STRING); return STRING; }
.                                  { output::errorLex(yylineno); }

%%

void set_token(yytoken_kind_t kind)
{
    yylval.token = new syntax_token(kind, yylineno, yytext);
}