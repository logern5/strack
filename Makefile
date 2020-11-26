CC=gcc
CFLAGS=-lcurl -Wall
CFLAGS+=-g #Debugging flags for gdb
CFLAGS+=-lgd -lshp -lm
DEFAULT_MAP=ne_110m_land.shp
all: default
clean:
	rm *.o
default:
	$(CC) -c lib/asciiworld/asciiworld.c -DDEFAULT_MAP='"$(DEFAULT_MAP)"'
	$(CC) -c lib/cJSON/cJSON.c
	$(CC) -c lib/curl/curllib.c
	$(CC) -c main.c
	$(CC) -c utils.c
	$(CC) -c api.c
	$(CC) main.o \
	utils.o cJSON.o api.o asciiworld.o curllib.o \
	-o strack $(CFLAGS)
