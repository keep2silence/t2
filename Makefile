CPPFLAGS=-g -I./api/ctp/latest/include/ -std=c++11 -Wall -Wextra -Wno-unused-parameter -D__CTP__

all: libengine.a md_engine_test trade_engine_test

libengine.a: ctp_md_engine.o ctp_trade_engine.o
	ar rv $@ $^
	ranlib $@

md_engine_test: md_engine_test.o libengine.a 
	g++ -o $@ $^ -g ./api/ctp/latest/lib/libthostmduserapi.so

trade_engine_test: trade_engine_test.o libengine.a 
	g++ -o $@ $^ -g ./api/ctp/latest/lib/libthosttraderapi.so

clean:
	rm -f *.o libengine.a
