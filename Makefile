CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -I/usr/include/mysql
LDLIBS = -lmysqlclient

all: control

control: control.cpp
	$(CXX) $(CXXFLAGS) $< $(LDLIBS) -o $@

clean:
	rm -f control