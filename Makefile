# Simple Makefile

CXX = clang++
CXXFLAGS  += -lboost_system
FILES = $(wildcard ./*.cc)
EXT=.bin
OBJS  = test.bin

all: ${OBJS}

test.bin: test.cc
	@echo $@;
	$(CXX) $(CXXFLAGS) $(LDFLAGS) ${FILES} -o $@

clean:
	rm -f *$(EXT)
