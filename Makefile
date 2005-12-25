OPT=-O3 #-mcpu=pentium4 -march=pentium4 -msse -msse2 -mfpmath=sse -mmmx
OPT_LOW=-O2
#DEBUG=-g3 -ggdb -DQ3C_DEBUG
#STATIC=-static
#PROF=-pg
#GCOV_PROF=-fprofile-arcs -ftest-coverage

MATH_LINK=-lm

ifdef PROF 
LPROF=-pg -lc_p
MATH_LINK=-lm_p
endif

#PG_PATH=/opt/pgsql/include/server
PG_PATH=`pg_config --includedir-server`
PG_INC=-I$(PG_PATH)
#PG_INC=-I$(PG_PATH) -I/db2/q3cube/pgsql/include/server


MY_CFLAGS= -DQ3C_INT8 $(PG_INC) #-DQ3C_LONG_DOUBLE

PIC=-fpic
CFLAGS=$(OPT) $(DEBUG) $(PROF) $(GCOV_PROF) -Wall $(MY_CFLAGS) $(PG_INC)
CFLAGS1=$(OPT_LOW) $(DEBUG) $(PROF) $(GCOV_PROF) -W -Wall $(MY_CFLAGS) $(PG_INC)
CFLAGS2=$(OPT) $(DEBUG) $(PROF) $(GCOV_PROF) $(MY_CFLAGS) $(PG_INC)

LDFLAGS=$(MATH_LINK) $(LPROF) $(STATIC)

CC = gcc
LD = ld


all: q3c test test1 mini_test q3c.sql poly_test

dist: 
	mkdir -p dist
	cp *.c *.h *.sql.in Makefile README.q3c COPYING dist

q3c: q3c.o dump.o q3cube.o q3c_poly.o
	$(LD) -shared -o lib$@.so $^

q3c_fast: q3c.o q3cube.o
	$(LD) -shared -o libq3c.so $^ dump.o

q3c.o: q3c.c common.h 
	$(CC) -c $< $(PIC) $(CFLAGS1) -o $@

q3cube.o: q3cube.c common.h my_bits.h
	$(CC) -c $< $(PIC) $(CFLAGS) -o $@

prepare.o: prepare.c common.h
	$(CC) -c $< $(CFLAGS) -o $@

q3c_poly.o: q3c_poly.c q3cube.o common.h
	$(CC) -c $<  $(CFLAGS) -o $@

prepare: prepare.o q3cube.o q3c_poly.o
	$(CC) $^ $(LDFLAGS) -o $@

dump.so: dump.c common.h
	$(CC) -c $< $(PIC) -shared $(MY_CFLAGS) -o $@ 

dump.o: dump.c common.h
	$(CC) -c $< $(PIC) $(MY_CFLAGS) $(OPT) -o $@

q3c.sql: q3c.sql.in
	cat q3c.sql.in | sed s/MODULE_PATHNAME/"`echo $(CURDIR)/libq3c|sed 's/\//\\\\\//g'`"/g > q3c.sql
	
dump.c: prepare
	./prepare

clean: 
	rm -f  *.o test mini_test poly_test test1 prepare *.so  q3c.sql dump.c

oclean: 
	rm -f *.o
	
oldclean: 
	rm -f *~

binclean:
	rm -f test mini_test prepare test test1

		
test.o: test.c common.h
	$(CC) -c $< $(CFLAGS2) -o $@

mini_test.o: mini_test.c common.h
	$(CC) -c $< $(CFLAGS2) -o $@

test: test.o q3cube.o dump.o q3c_poly.o
	$(CC) $^ $(LDFLAGS) -o $@

poly_test: poly_test.c q3c_poly.o q3cube.o dump.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	
test1: test1.c dump.o q3cube.o
	$(CC) $< $(CFLAGS2) q3cube.o dump.o q3c_poly.o $(LDFLAGS) -o $@

mini_test: mini_test.o q3cube.o q3c_poly.o
	$(CC) $< q3cube.o dump.o q3c_poly.o $(LDFLAGS) -o $@
