all: testlib

testlib: testlib.o primlib.o
	gcc -fsanitize=undefined -g $^ -o $@ -lm -lSDL2_gfx `sdl2-config --libs`

.c.o: 
	gcc -fsanitize=undefined -g -Wall -pedantic `sdl2-config --cflags` -c  $<

primlib.o: primlib.c primlib.h

testlib.o: testlib.c primlib.h

pieces.o: pieces.c pieces.h

clean:
	-rm primlib.o testlib.o testlib
