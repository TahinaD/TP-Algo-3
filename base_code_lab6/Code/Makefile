CC = gcc
REPPORTFILENAME=
CFLAGS = -std=c99 -Wextra -Wall -Werror -pedantic
LDFLAGS =

ECHO = @
ifeq ($(VERBOSE),1)
	ECHO=
endif

ifeq ($(DEBUG),yes)
	CFLAGS += -g
	LDFLAGS +=
else
	CFLAGS += -O3 -DNDEBUG
	LDFLAGS +=
endif

EXEC=bstreetest
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

all:
ifeq ($(DEBUG),yes)
	@echo "Generating in debug mode"
else
	@echo "Generating in release mode"
endif
	@$(MAKE) $(EXEC)

$(EXEC): $(OBJ)
	$(ECHO)$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(ECHO)$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	$(ECHO)rm -rf *.o

mrproper: clean
	$(ECHO)rm -rf $(EXEC) documentation/html *.dot *.pdf

doc: bstree.h queue.h main.c
	$(ECHO)doxygen documentation/TP5

pdf : $(EXEC)
	$(ECHO)dot -Tpdf *.dot -O

queue.o : queue.h
bstree.o : bstree.h queue.h
main.o : bstree.h
doc : bstree.h queue.h main.c
