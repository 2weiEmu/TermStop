objects = src/*.c

run: $(objects)
	@gcc -o main $(objects)
	./main
