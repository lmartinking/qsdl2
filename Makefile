CC := g++

CFLAGS := -std=c++14 -O2 -DNDEBUG -Wno-writable-strings
CFLAGS_32 := -m32
CFLAGS_64 := -m64

CFLAGS_M := -bundle -undefined dynamic_lookup `pkg-config sdl2 --cflags --libs`
CFLAGS_L := -shared -fPIC -lstdc++ `pkg-config sdl2 --cflags --libs`

SRC = qsdl2.cpp

m32: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_32) $(CFLAGS_M) $(SRC) -o qsdl2_m32.so

m64: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_64) $(CFLAGS_M) $(SRC) -o qsdl2_m64.so

l32: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_32) $(CFLAGS_L) $(SRC) -o qsdl2_l32.so

l64: $(SRC)
	$(CC) $(CFLAGS) $(CFLAGS_64) $(CFLAGS_L) $(SRC) -o qsdl2_l64.so

clean:
	rm -f qsdl2_*.so

.PHONY: clean
