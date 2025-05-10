# Email: realyoavperetz@gmail.com
# Makefile – builds both console demo and SFML GUI.

Main:
	g++ -std=c++20 -Wall -Wextra -pedantic \
	    src/*.cpp src/roles/*.cpp src/gui/*.cpp main.cpp \
	    -Iinclude \
	    -lsfml-graphics -lsfml-window -lsfml-system \
	    -o Main
	time ./Main
	$(MAKE) clean

test:
	g++ -std=c++20 -Wall -Wextra -pedantic \
	    src/*.cpp src/roles/*.cpp tests/*.cpp \
	    -Iinclude \
	    -o tests.out
	./tests.out

valgrind: Main
	valgrind --leak-check=full ./Main

clean:
	rm -f Main tests.out