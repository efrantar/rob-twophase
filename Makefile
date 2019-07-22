C=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-std=c++11 -lpthread -O3 -DQTM
LDFLAGS=
LDLIBS=-lpthread

SRCS=main.cc coord.cc cubie.cc face.cc moves.cc prun.cc solve.cc sym.cc
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
