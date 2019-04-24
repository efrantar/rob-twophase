C=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-std=c++14 -lpthread -O
LDFLAGS=
LDLIBS=-lpthread

SRCS=main.cc coord.cc cubie.cc face.cc misc.cc moves.cc prun.cc solve.cc sym.cc
OBJS=$(subst .cc,.o,$(SRCS))

all: tool

tool: $(OBJS)
	$(CXX) $(LDFLAGS) -o twophase $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) *~ .depend

include .depend
