CFLAGS=-O3 -g -Wall -Wextra -Werror -std=c++0x -DPROFILE=1 -Wno-sign-compare  -w
LIBS=-lnuma -lpthread -lrt -lprofiler
CXX=g++

SOURCES:=$(wildcard *.cc *.c)
OBJECTS:=$(patsubst %.cc,build/%.o,$(SOURCES))
DEPSDIR:=.deps
DEPCFLAGS=-MD -MF $(DEPSDIR)/$*.d -MP
INCLUDE:=.

all:CFLAGS+=-DTESTING=0 -DUSE_BACKOFF=1
#all:LIBS+=-ltcmalloc_minimal
all:env build/db

test:CFLAGS+=-DTESTING=1
test:env build/tests

-include $(wildcard $(DEPSDIR)/*.d)

build/%.o: %.cc $(DEPSDIR)/stamp GNUmakefile
	@mkdir -p build
	@echo + cc $<
	@$(CXX) $(CFLAGS) $(DEPCFLAGS) -I$(INCLUDE) -c -o $@ $<

$(TESTOBJECTS):$(OBJECTS)

build/db:$(OBJECTS)
	@$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

$(DEPSDIR)/stamp:
	@mkdir -p $(DEPSDIR)
	@touch $@

.PHONY: clean env

clean:
	rm -rf build $(DEPSDIR)
