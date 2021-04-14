
srcdir = ./src
outdir = ./build
incdir = ./include

CC = gcc
cflags = -I$(incdir) -g

targets = 1a 1b 1c 1d

_targets = $(patsubst %, $(outdir)/%, $(targets))

all:$(outdir)/$(_targets)

$(outdir)/%: $(srcdir)/%.c $(outdir) 
	$(CC) $< -o $@ $(cflags)

$(outdir):
	-mkdir $@

.PHONY: clean
clean:
	-rm $(outdir)/*