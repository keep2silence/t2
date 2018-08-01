CPPFLAGS=-g -I./api/ctp/latest/include/ -std=c++11 -Wall -Wextra

libengine.a: ctp_md_engine.o
	ar rv $@ $^
	ranlib $@

clean:
	rm -f *.o libengine.a
