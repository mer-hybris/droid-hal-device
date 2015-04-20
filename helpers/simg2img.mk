# Makefile for simg2img

SRCS+= backed_block.c
SRCS+= output_file.c
SRCS+= sparse.c
SRCS+= sparse_crc32.c
SRCS+= sparse_err.c
SRCS+= sparse_read.c
SRCS+= simg2img.c

# Too smart macros don't work with c99, filter it out.
CFLAGS := $(filter-out -std=c99,$(CFLAGS))

CPPFLAGS+= -Iinclude

OBJS=$(SRCS:.c=.o)

LIBS=-lz

all: simg2img

simg2img: $(OBJS)
	$(CC) -o $@ $(CPPFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) simg2img
