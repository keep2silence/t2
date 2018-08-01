CPPFLAGS=-g -I./api/ctp/latest/include/ -std=c++11 -Wall -Wextra -Wno-unused-parameter

all: libengine.a md_engine_test

libengine.a: ctp_md_engine.o
	ar rv $@ $^
	ranlib $@

md_engine_test: md_engine_test.o libengine.a 
	g++ -o $@ $^ -g ./api/ctp/latest/lib/libthostmduserapi.so

clean:
	rm -f *.o libengine.a