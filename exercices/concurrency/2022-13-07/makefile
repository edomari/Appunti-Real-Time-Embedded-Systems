# Usa la wildcard per selezionare tutti i file .c nella directory corrente
PROGS := $(basename $(wildcard *.c))

OBJS := $(addsuffix .o, $(PROGS))

all: $(PROGS)

clean:
	rm -rf $(PROGS)

%: %.c
	gcc -Wall $< -o $@ -lpthread