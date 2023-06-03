%code requires {
    namespace ast {
        class Context;
    }
}

%param { ast::Context& context }

%{
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstddef>
#include "ast.h"
#include "ast_context.h"

// FIXME: figure out how to alter the signature of yylex
extern int yylex();
inline int yylex(ast::Context&) {
    return yylex();
}

void yyerror(ast::Context& context, const char* text);
    
%}

%union {
    int                       token;
    std::vector<std::string>* strings;
    std::string*              string;
    ast::Stmt*                stmt;
    std::vector<ast::Stmt*>*  stmts;
    ast::Decl*                decl;
    std::vector<ast::Decl*>*  decls;
    ast::Type*                type;
    std::vector<ast::Type*>*  types;
}

%token <string> TIDENT TSTRING TINTEGER
%token <token>  TPLUS TMINUS TMULTIPLY TDIVIDE
%token <token>  TSTRUCT TENUM TUNION TTUPLE TLIST TMAP

%type <strings> enum_values;
%type <stmt> stmt unit decl_stmt struct_field
%type <stmts> stmts struct_fields
%type <decl> decl struct_decl enum_decl
%type <type> type named_type union_type tuple_type list_type map_type
%type <types> type_list

%left TPLUS TMINUS
%left TMULTIPLY TDIVIDE

%start unit

%%

unit : stmts { $$ = &context.new_file_stmt(context.new_compound_stmt(*$1)); }
     ;

stmts : stmts ';' stmt { $1->push_back($3); }
      | stmt           { $$ = new std::vector<ast::Stmt*>; $$->push_back($1); }
      | %empty         { $$ = new std::vector<ast::Stmt*>; }
      ;

stmt : decl_stmt
     ;

decl_stmt : decl { $$ = &context.new_decl_stmt(*$1); }
          ;

decl : struct_decl
     | enum_decl
     ;

struct_decl : TSTRUCT TIDENT '{' struct_fields '}' { $$ = &context.new_struct_decl(*$2, context.new_compound_stmt(*$4)); }

struct_fields : struct_fields struct_field { $1->push_back($2); }
              | struct_field               { $$ = new std::vector<ast::Stmt*>; $$->push_back($1); }
              | %empty                     { $$ = new std::vector<ast::Stmt*>; }
              ;

struct_field : TIDENT ':' type ';' { $$ = &context.new_decl_stmt(context.new_variable_decl(*$1, *$3)); }
             ;

enum_decl : TENUM TIDENT '{' enum_values '}'          { $$ = &context.new_enum_decl(*$2, context.new_named_type("u64"), *$4); }
          | TENUM TIDENT ':' type '{' enum_values '}' { $$ = &context.new_enum_decl(*$2, *$4, *$6); }
          ;

enum_values : enum_values ',' TIDENT { $1->push_back(*$3); }
            | TIDENT                 { $$ = new std::vector<std::string>; $$->push_back(*$1); }
            | %empty                 { $$ = new std::vector<std::string>; }
            ;

type : named_type
     | union_type
     | tuple_type
     | list_type
     | map_type
     ;

named_type : TIDENT { $$ = &context.new_named_type(*$1); }
           ;

type_list : type_list ',' type { $1->push_back($3); }
          | type               { $$ = new std::vector<ast::Type*>; $$->push_back($1); }
          | %empty             { $$ = new std::vector<ast::Type*>; }
          ;

union_type : TUNION '[' type_list ']' { $$ = &context.new_union_type(*$3); }
           ;

tuple_type : TTUPLE '[' type_list ']' { $$ = &context.new_tuple_type(*$3); }
           ;

list_type : TLIST '[' type ']' { $$ = &context.new_list_type(*$3); }
          ;

map_type : TMAP '[' type ',' type ']' { $$ = &context.new_map_type(*$3, *$5); }
         ;

%%
