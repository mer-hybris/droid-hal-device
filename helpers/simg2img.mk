# Makefile for simg2img

SIMG2IMG_SOURCES?= nosourcessupplied

CXXFLAGS+= -std=gnu++14

CPPFLAGS+= -Iinclude
CPPFLAGS+= -I../base/include -I../../libbase/include

TMP_OBJS=$(SIMG2IMG_SOURCES:.cpp=.o)
OBJS=$(TMP_OBJS:.c=.o)

LIBS=-lz

all: simg2img

simg2img: $(OBJS)
	$(CXX) -o $@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) simg2img
