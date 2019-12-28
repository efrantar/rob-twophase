C=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-std=c++11 -lpthread -O3 -DAX
LDFLAGS=
LDLIBS=-lpthread

SRCS=main.cpp coord.cpp cubie.cpp move.cpp prun.cpp sym.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

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
