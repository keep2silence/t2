CPPFLAGS=-g -I./api/ctp/latest/include/ -std=c++11 -Wall -Wextra -Wno-unused-parameter

all: libengine.a md_engine_test

libengine.a: ctp_md_engine.o
	ar rv $@ $^
	ranlib $@

md_engine_test: libengine.a md_engine_test.o
	g++ -o $@ $^ -g

clean:
	rm -f *.o libengine.a
