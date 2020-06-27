CC?=gcc
CFLAGS=-pthread
BINARIES=an_ch2_1a an_ch2_1b lab lab_1
SOURCEDIR=./src
EXECUTABLEDIR=./bin


all: default
default: $(BINARIES)

%: $(SOURCEDIR)/%.c 
	gcc $(CFLAGS) $^ -o $(EXECUTABLEDIR)/$@ 

clean:
	rm -f $(EXECUTABLEDIR)/*