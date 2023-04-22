%{
#include <stdio.h>

extern FILE* yyout;

/* declarations to fix warnings from sloppy
   yacc/byacc/bison code generation. For instance,
   the code should have a declaration of yylex. */

int yylex(void);

/* The POSIX specification says yyerror should return
   int, although bison documentation says the value is
   ignored. We match POSIX just in case. */

int yyerror(const char *s);
%}

/* tokens our lexer will produce */
%token NUM

%%

/* The first rule is the final goal. Yacc will work
   backward trying to arrive here. This "results" rule
   is a stub we use to print the value from "number." */

results :
  number { fprintf(yyout, "%d\n", $1); }
;

/* as the lexer produces more NUMs, keep adding them */

number :

  /* this is a common pattern for saying number is one or
     more NUMs.  Notice we specify "number NUM" and not
     "NUM number". In yacc recursion, think "right is wrong
     and left is right." */

  number NUM { $$ = $1 + $2; }

  /* base case, using default rule of $$ = $1 */

| NUM
;

%%

int yyerror(const char* s) {
    fprintf(yyout, "Error: %s\n", s);
    return 0;
}

int main(int argc, char** argv) {
    yylex();
    return 0;
}
