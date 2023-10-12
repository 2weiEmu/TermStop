objects = src/*.c
libs = 

run: $(objects)
	@gcc -o main $(objects) $(libs)
	./main
