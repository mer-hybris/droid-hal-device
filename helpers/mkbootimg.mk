# Makefile for mkbootimg

SRCS+= mkbootimg.c

VPATH+= ../libmincrypt
SRCS+= dsa_sig.c
SRCS+= p256.c
SRCS+= p256_ec.c
SRCS+= p256_ecdsa.c
SRCS+= rsa.c
SRCS+= sha.c
SRCS+= sha256.c

CPPFLAGS+= -I.
CPPFLAGS+= -I../include

#LIBS+=

OBJS=$(SRCS:.c=.o)

all: mkbootimg

mkbootimg: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) mkbootimg

