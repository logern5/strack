CC=gcc
CFLAGS=-lcurl -Wall
#CFLAGS+=-g #Debugging flags for gdb, can be safely commented out
ASCIIWORLDCFLAGS=-lgd -lshp -lm
DEFAULT_MAP =`readlink -f ne_110m_land.shp`#readlink not supported on non-Cygwin Windows
ASCIIWORLDCFLAGS += -DDEFAULT_MAP=\"$(DEFAULT_MAP)\"
USRFLAGS=
all: default
default:
	$(CC) main.c -o strack $(CFLAGS) $(ASCIIWORLDCFLAGS) $(USRFLAGS)
autojson:
	$(CC) main.c -o strack $(CFLAGS) $(ASCIIWORLDCFLAGS) $(USRFLAGS) -DAUTO_JSON_OFFSET -DJFILE=\"example.json\"
