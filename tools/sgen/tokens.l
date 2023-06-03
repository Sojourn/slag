%{
#include <string>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cassert>

#include "ast.h"
#include "ast_context.h"
#include "parser.hpp"

#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)   (yylval.token = t)
%}

%option noyywrap

%%

[ \t\r\n]                 ;
"struct"                  return TOKEN(TSTRUCT);
"enum"                    return TOKEN(TENUM);
"union"                   return TOKEN(TUNION);
"tuple"                   return TOKEN(TTUPLE);
"list"                    return TOKEN(TLIST);
"map"                     return TOKEN(TMAP);
[a-zA-Z_][a-zA-Z0-9_]*    SAVE_TOKEN; return TIDENT;
[0-9]+                    SAVE_TOKEN; return TINTEGER;
\".*\"                    SAVE_TOKEN; return TSTRING;

"{"                       return '{';
"}"                       return '}';
"("                       return '(';
")"                       return ')';
"["                       return '[';
"]"                       return ']';
"."                       return '.';
","                       return ',';
":"                       return ':';
";"                       return ';';

"+"                       return TOKEN(TPLUS);
"-"                       return TOKEN(TMINUS);
"*"                       return TOKEN(TMULTIPLY);
"/"                       return TOKEN(TDIVIDE);


.                         printf("Unknown token!\n"); yyterminate();

%%
