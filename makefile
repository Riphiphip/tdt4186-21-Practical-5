
srcdir = ./src
outdir = ./build
incdir = ./include

CC = gcc
cflags = -I$(incdir) -g

targets = 1a

all:$(outdir)/$(targets)

$(outdir)/%: $(srcdir)/%.c $(outdir) 
	$(CC) $< -o $@ $(cflags)

$(outdir):
	-mkdir $@

.PHONY: clean
clean:
	-rm $(outdir)/*