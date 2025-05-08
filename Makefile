# Email: realyoavperetz@gmail.com
# Makefile – explicit commands, now including main.cpp instead of Demo.

Main:
	g++ -std=c++20 -Wall -Wextra -pedantic \
         src/*.cpp src/roles/*.cpp gui/GameWindow.cpp main.cpp \
         -Iinclude \
         -lsfml-graphics -lsfml-window -lsfml-system \
         -o Main
	time ./Main
test:
	g++ -std=c++20 -Wall -Wextra -pedantic src/*.cpp src/roles/*.cpp tests/*.cpp -Iinclude -o tests.out
	./tests.out

valgrind: Main
	valgrind --leak-check=full ./Main

clean:
	rm -f Main tests.out
