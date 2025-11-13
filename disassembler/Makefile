CC=clang
APR_CONFIG=apr-1-config
LIB_ELF=/usr/local/Cellar/libelf/0.8.13
CFLAGS=-Wall $(shell $(APR_CONFIG) --cppflags --cflags --includes) -I$(LIB_ELF)/include
LDFLAGS=$(shell $(APR_CONFIG)  --ldflags --link-ld --libs) -L$(LIB_ELF)/lib -lelf

SOURCES=*.c
OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=dissassembler

all: $(EXECUTABLE)

$(EXECUTABLE):
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o ../Debug/$@

clean: 
	rm -f ..\Debug\dissassembler    
