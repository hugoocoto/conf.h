LUA_PKG := $(shell pkg-config --exists lua 2>/dev/null && echo lua)

TARGET   = a.out
TARGET_CPP = a_cpp.out
CFLAGS   = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE -I.
CXXFLAGS = -Wall -Wextra -std=c++17 -D_DEFAULT_SOURCE -I.
LDLIBS   = -llua -lm

ifdef LUA_PKG
  CFLAGS   += $(shell pkg-config --cflags lua)
  CXXFLAGS += $(shell pkg-config --cflags lua)
  LDLIBS    = $(shell pkg-config --libs lua) -lm
endif

test_all: test test_cpp

test: $(TARGET)
	cd test && ../$(TARGET)

test_cpp: $(TARGET_CPP)
	cd test && ../$(TARGET_CPP)

$(TARGET): test/test.c conf.h
	$(CC) $(CFLAGS) -o $@ test/test.c $(LDLIBS)

$(TARGET_CPP): test/test.cpp conf.h
	$(CXX) $(CXXFLAGS) -o $@ test/test.cpp $(LDLIBS)

clean:
	rm -f $(TARGET) $(TARGET_CPP)
