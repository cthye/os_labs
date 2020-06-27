CC?=gcc
CFLAGS=-g
BINARIES=myfind
SOURCEDIR=./src
EXECUTABLEDIR=./bin


all: default
default: $(BINARIES)

%: $(SOURCEDIR)/%.c 
	gcc $(CFLAGS) $^ -o $(EXECUTABLEDIR)/$@ 

clean:
	rm -f $(EXECUTABLEDIR)/*