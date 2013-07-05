SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
CFLAGS := -std=gnu99 -O3

%.o:%.c
	gcc -c -o $@ $< $(CFLAGS)

nframe-to-bmp: $(OBJ)
	gcc $(OBJ) $(CFLAGS) $(LDFLAGS) -o nframe-to-bmp
