SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
CFLAGS := -std=gnu99 -O3
LDFLAGS := -lm

%.o:%.c
	gcc -c -o $@ $< $(CFLAGS)

wav-to-nviz: $(OBJ)
	gcc $(OBJ) $(CFLAGS) $(LDFLAGS) -o wav-to-nviz
