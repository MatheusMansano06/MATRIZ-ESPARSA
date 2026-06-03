CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11 -g
TARGET  = aeroportos
SRCS    = main.c aeroporto.c matriz_esparsa.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
