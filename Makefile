CC=gcc
CFLAGS=-lcurl -Wall -g #-g is debugging symbols for GDB
ASCIIWORLDCFLAGS=-lgd -lshp -lm
DEFAULT_MAP = `readlink -f ne_110m_land.shp`
ASCIIWORLDCFLAGS += -DDEFAULT_MAP=\"$(DEFAULT_MAP)\"
USRFLAGS=
all: default
default:
	$(CC) main.c -o strack $(CFLAGS) $(ASCIIWORLDCFLAGS) $(USRFLAGS)
autojson:
	$(CC) main.c -o strack $(CFLAGS) $(ASCIIWORLDCFLAGS) $(USRFLAGS) -DAUTO_JSON_OFFSET -DJFILE=\"data.json\"
