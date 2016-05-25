RM = rm -f
CXXSTDFLAGS ?= -std=c++1z
CXXFLAGS := $(CXXFLAGS) -Wall -Wextra $(CXXSTDFLAGS)

all: Basic basic-lab2

SRCS = \
	basic-lab2.cpp \
	compiler.cpp \
	interactive_console.cpp \
	interactive_machine.cpp \
	linker.cpp \
	machine.cpp

OBJS = $(SRCS:.cpp=.o)

ELFS = \
	basic-lab2

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CXXFLAGS) -MM $^ >> ./.depend

basic-lab2: $(OBJS)
	$(CXX) $(LDFLAGS) -o basic-lab2 $(OBJS) $(LDLIBS)

Basic: basic-lab2
	ln -sf basic-lab2 Basic

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) *~ .depend

include .depend
