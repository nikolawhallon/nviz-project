SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
CFLAGS := -O3
LDFLAGS := -lncurses -lpthread

%.o:%.c
	gcc -c -o $@ $< $(CFLAGS)

nviz-player: $(OBJ)
	gcc $(OBJ) $(CFLAGS) $(LDFLAGS) -o nviz-player
