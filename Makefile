objects = src/*.c
libs = 
flags = -Wall -Werror -Wextra -Os

run: $(objects)
	@gcc -o main $(flags) $(objects) $(libs)
	@./main

debug-run: $(objects)
	@gcc -g -o debug-build $(flags) $(objects) -DDEBUG
	./debug-build

clean: 
	@rm ./debug-build
	@rm ./main
	@rm ./termstop

install:
	# TODO: these values are quite hardcoded. Fix this
	# TODO: make sure all the directories exist
	gcc -o termstop $(flags) $(objects) $(libs)
	cp ./termstop /usr/bin/
	chmod 755 /usr/bin/termstop
	cp ./termstop.1 /usr/share/man/man1/
	chmod 644 /usr/share/man/man1/termstop.1

uninstall:
	rm -f /usr/bin/termstop
	rm -f /usr/share/man/man1/termstop.1
