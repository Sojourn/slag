CC=gcc
LEX=flex
YACC=bison

# put together object files from lexer and parser, and
# link the yacc and lex libraries (in that order, to pick
# main() from yacc's library rather than lex's)

mgen : mgen.tab.o mgen.lex.o
	$(CC) -o $@ mgen.tab.o mgen.lex.o

# tell make which files yacc will generate
#
# an explanation of the arguments:
# -b mgen  -  name the files mgen.tab.*
# -d        -  generate a .tab.h file too

mgen.tab.h mgen.tab.c : mgen.y
	$(YACC) -d -b mgen $?

# the object file relies on the generated lexer, and
# on the token constants 

mgen.lex.o : mgen.tab.h mgen.lex.c

# can't use the default suffix rule because we're
# changing the name of the output to .lex.c

mgen.lex.c : mgen.l
	$(LEX) -t $? > $@
