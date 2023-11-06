objects = src/*.c
libs = 
flags = -Wall -Werror -Wextra -Os

run: $(objects)
	@gcc -o main $(flags) -Os $(objects) $(libs)
	@./main

debug-run: $(objects)
	@gcc -g -o debug-build $(flags) $(objects) -DDEBUG
	./debug-build

clean: 
	@rm ./debug-build
	@rm ./main
