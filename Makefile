LUA_CFLAGS := $(shell pkg-config --cflags lua)
LUA_LDLIBS := $(shell pkg-config --libs lua)

TARGET   = a.out
TARGET_CPP = a_cpp.out
CFLAGS   = -Wall -Wextra -std=c99 -I. -D_DEFAULT_SOURCE $(LUA_CFLAGS)
CXXFLAGS = -Wall -Wextra -std=c++11 -I. -D_DEFAULT_SOURCE $(LUA_CFLAGS)
LDLIBS   = $(LUA_LDLIBS)

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
