SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
CFLAGS := -O3

%.o:%.c
	gcc -c -o $@ $< $(CFLAGS)

nframe-to-bmp: $(OBJ)
	gcc $(OBJ) $(CFLAGS) $(LDFLAGS) -o nframe-to-bmp
