SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
CFLAGS := -O3
LDFLAGS := -lncurses

%.o:%.c
	gcc -c -o $@ $< $(CFLAGS)

nframe-viewer: $(OBJ)
	gcc $(OBJ) $(CFLAGS) $(LDFLAGS) -o nframe-viewer
