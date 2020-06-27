CC?=gcc
CFLAGS=-g -pthread
BINARIES=shell 
SOURCEDIR=./src
SRCS=$(SOURCEDIR)/shell.c $(SOURCEDIR)/tokenizer.c
EXECUTABLEDIR=./bin
OBJS=$(SRCS:.c=.o)

all: default
default: $(BINARIES)

$(BINARIES): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXECUTABLEDIR)/$@ 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXECUTABLEDIR)/*