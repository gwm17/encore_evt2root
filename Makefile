CC=g++
DAQDIR= /usr/opt/nscldaq/11.0/
INCLDIR= $(DAQDIR)include
LIBDIR= $(DAQDIR)lib
CFLAGS= -std=c++11 -c -g -Wall `root-config --cflags`
CPPFLAGS= -I$(INCLDIR)
LDFLAGS = `root-config --glibs`
LIBFLAGS= -L$(LIBDIR) -lurl -lException -ldataformat -lDataFlow -Wl,"-rpath=$(LIBDIR)" 
SOURCES=$(wildcard ./*.cpp)
OBJS=$(SOURCES:%.cpp=%.o)
EXE=evt2root

.PHONY: clean all

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LIBFLAGS)

%.o: %.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
	$(RM) $(EXE) $(OBJS)
