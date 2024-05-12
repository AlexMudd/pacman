all:
	@echo 'build'
	@echo 'clean'
build:
	gcc main.c map.c settings.c pacman.c -lncurses -o game
clean:
	rm game
