# Makefile for img2simg

SRCS+= backed_block.c
SRCS+= output_file.c
SRCS+= sparse.c
SRCS+= sparse_crc32.c
SRCS+= sparse_err.c
SRCS+= sparse_read.c
SRCS+= img2simg.c

CPPFLAGS+= -Iinclude

OBJS=$(SRCS:.c=.o)

LIBS=-lz

all: img2simg

img2simg: $(OBJS)
	$(CC) -o $@ $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) img2simg
