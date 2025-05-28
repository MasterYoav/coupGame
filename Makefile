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
	    src/*.cpp src/roles/*.cpp tests.cpp \
	    -Iinclude \
	    -o tests.out
	./tests.out

valgrind:
	@echo "⮞ building test runner"
	g++ -std=c++20 -g -O0 -Wall -Wextra -pedantic \
	    tests.cpp src/*.cpp src/roles/*.cpp \
	    -Iinclude -o tests_val
	@echo "⮞ Valgrinding tests_val …"
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 ./tests_val

	@echo "⮞ building game simulation runner"
	g++ -std=c++20 -g -O0 -Wall -Wextra -pedantic \
	    main_for_valgrind.cpp src/*.cpp src/roles/*.cpp \
	    -Iinclude -o game_val
	@echo "⮞ Valgrinding game_val …"
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --error-exitcode=1 ./game_val

clean:
	rm -f Main tests.out             \
	      tests_val game_val         \
	      *.o