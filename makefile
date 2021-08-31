#! /usr/bin/make

INCLUDEDIR=/usr/include/
LIBDIR=/usr/lib/

SOURCE= FruitTetris3D.cpp include/InitShader.cpp

CC= g++
CFLAGS= -O3 -g -Wall -pedantic -DGL_GLEXT_PROTOTYPES -std=c++11
EXECUTABLE= FruitTetris3D
LDFLAGS = -lGL -lglut -lGLEW -lXext -lX11 -lm
INCLUDEFLAG= -I. -I$(INCLUDEDIR) -Iinclude/
LIBFLAG= -L$(LIBDIR)
OBJECT= $(SOURCE:.cpp=.o)

all: $(OBJECT) depend
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(LIBFLAG) $(OBJECT) -o $(EXECUTABLE) $(LDFLAGS)
	make clean_object
depend:
	$(CC) -std=c++11 -M $(SOURCE) > depend

$(OBJECT):
	$(CC) $(CFLAGS) $(INCLUDEFLAG) -c -o $@ $(@:.o=.cpp)

clean_object:
	rm -f $(OBJECT)

clean:
	rm -f $(OBJECT) depend $(EXECUTABLE)

include depend
