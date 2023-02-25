# Makefile for img2simg

IMG2SIMG_SOURCES?= nosourcessupplied

CXXFLAGS+= -std=gnu++14

CPPFLAGS+= -Iinclude
CPPFLAGS+= -I../base/include -I../../libbase/include

TMP_OBJS=$(IMG2SIMG_SOURCES:.cpp=.o)
OBJS=$(TMP_OBJS:.c=.o)

LIBS=-lz

all: img2simg

img2simg: $(OBJS)
	$(CXX) -o $@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS)

clean:
	rm -rf $(OBJS) img2simg
