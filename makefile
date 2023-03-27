.PHONY: all clean

all: clean
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -pedantic -Wall -Wextra -Weffc++ -o hw5 *.c *.cpp syntax/*.cpp emit/*.cpp symbol/*.cpp
clean:
	rm -f lex.yy.c
	rm -f parser.tab.*pp
	rm -f hw5
