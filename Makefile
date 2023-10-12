objects = src/*.c
libs = 

run: $(objects)
	@gcc -o main  -Wall -Werror -Wextra -Os $(objects) $(libs)
	./main
