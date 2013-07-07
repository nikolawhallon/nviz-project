SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
CFLAGS := -O3

%.o:%.c
	gcc -c -o $@ $< $(CFLAGS)

nviz-to-nframes: $(OBJ)
	gcc $(OBJ) $(CFLAGS) $(LDFLAGS) -o nviz-to-nframes
