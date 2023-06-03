all: parser

OBJS = parser.o         \
       main.o           \
       tokens.o         \
       ast_transform.o  \
       code_generator.o \

CPPFLAGS = -std=c++20
LDFLAGS = -lpthread -ldl -lz -rdynamic
LIBS = -lfmt

clean:
	$(RM) -rf parser.cpp parser.hpp parser.output parser tokens.cpp $(OBJS) 

parser.cpp: parser.y
	bison -v -d -o $@ $^
	
parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	flex -o $@ $^

%.o: %.cpp
	g++ -g -c $(CPPFLAGS) -o $@ $<

parser: $(OBJS)
	g++ -g -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

