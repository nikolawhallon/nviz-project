SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
CFLAGS := -std=gnu99 -O3
LDFLAGS := -lncurses -lpthread -lrt

%.o:%.c
	gcc -c -o $@ $< $(CFLAGS)

nviz-player: $(OBJ)
	gcc $(OBJ) $(CFLAGS) $(LDFLAGS) -o nviz-player
