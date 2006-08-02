MODULE_big=q3c
OBJS=q3c.o dump.o q3cube.o q3c_poly.o
DATA_built=q3c.sql
DOCS=README.q3c

OPT=-O3 #-mcpu=pentium4 -march=pentium4 -msse -msse2 -mfpmath=sse -mmmx
OPT_LOW=-O2
#DEBUG=-g3 -ggdb -DQ3C_DEBUG
PG_CPPFLAGS = -DQ3C_INT8 $(DEBUG) $(OPT) -D_GNU_SOURCE
SHLIB_LINK += $(filter -lm, $(LIBS))
EXTRA_CLEAN=dump.c prepare prepare.o gen_data.o tests/join.out tests/cone.out gen_data

ifdef NO_PGXS
subdir = contrib/q3c
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
else
PGXS := $(shell pg_config --pgxs)
ifndef PGXS
$(error You should have `pg_config` program in your PATH or compile Q3C with\
'make NO_PGXS=1' \
after putting it in the contrib subdirectory of Postgres sources)
endif
include $(PGXS)

endif

prepare: prepare.o q3cube.o q3c_poly.o

dump.c: prepare
	./prepare

prepare: prepare.o q3cube.o q3c_poly.o
	$(CC) $(CFLAGS) prepare.o q3cube.o q3c_poly.o $(PG_LIBS) $(LDFLAGS) $(LIBS) -o $@
              
oldclean: 
	rm -f *~

gen_data: gen_data.c
	$(CC) $< $(CPPFLAGS) $(PG_LIBS) $(LDFLAGS) $(LIBS) -o $@

test: gen_data
	createdb q3c_test
	psql q3c_test -c "CREATE TABLE test (ra double precision, dec double precision)"
	psql q3c_test -c "CREATE TABLE test1 (ra double precision, dec double precision)"
	./gen_data 1 | psql q3c_test -c "COPY test FROM STDIN WITH DELIMITER ' '"
	./gen_data 2 | psql q3c_test -c "COPY test1 FROM STDIN WITH DELIMITER ' '"
	psql q3c_test -c '\i q3c.sql'
	psql q3c_test -c 'CREATE INDEX q3c_idx ON test (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'ANALYZE test'
	cat tests/cone.sql | psql q3c_test > tests/cone.out
	diff tests/cone.out tests/cone.expected
	cat tests/join.sql | psql q3c_test > tests/join.out
	diff tests/join.out tests/join.expected
	dropdb q3c_test

dist: clean
	mkdir -p dist
	cp *.c *.h *.sql.in Makefile README.q3c COPYING dist
	cp -r tests dist
	cat q3c.sql.in | perl create_drops.pl > dist/drop_q3c.sql

#all: q3c test test1 mini_test q3c.sql poly_test

#q3c: q3c.o dump.o q3cube.o q3c_poly.o
#	$(LD) -shared -o lib$@.so $^

#q3c_fast: q3c.o q3cube.o
#	$(LD) -shared -o libq3c.so $^ dump.o

#q3c.o: q3c.c common.h 
#	$(CC) -c $< $(PIC) $(CFLAGS1) -o $@

#q3cube.o: q3cube.c common.h my_bits.h
#	$(CC) -c $< $(PIC) $(CFLAGS) -o $@

#prepare.o: prepare.c common.h
#	$(CC) -c $< $(CFLAGS) -o $@

#q3c_poly.o: q3c_poly.c q3cube.o common.h
#	$(CC) -c $<  $(CFLAGS) -o $@

#prepare: prepare.o q3cube.o q3c_poly.o
#	$(CC) $^ $(LDFLAGS) -o $@

#dump.so: dump.c common.h
#	$(CC) -c $< $(PIC) -shared $(MY_CFLAGS) -o $@ 

#dump.o: dump.c common.h
#	$(CC) -c $< $(PIC) $(MY_CFLAGS) $(OPT) -o $@

#q3c.sql: q3c.sql.in
#	cat q3c.sql.in | sed s/MODULE_PATHNAME/"`echo $(CURDIR)/libq3c|sed 's/\//\\\\\//g'`"/g > q3c.sql
	
#dump.c: prepare
#	./prepare

#clean: 
#	rm -f  *.o test mini_test poly_test test1 prepare *.so  q3c.sql dump.c

#oclean: 
#	rm -f *.o	

#binclean:
#	rm -f test mini_test prepare test test1
		
#test.o: test.c common.h
#	$(CC) -c $< $(CFLAGS2) -o $@

#mini_test.o: mini_test.c common.h
#	$(CC) -c $< $(CFLAGS2) -o $@

#test: test.o q3cube.o dump.o q3c_poly.o
#	$(CC) $^ $(LDFLAGS) -o $@

#poly_test: poly_test.c q3c_poly.o q3cube.o dump.o
#	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	
#test1: test1.c dump.o q3cube.o
#	$(CC) $< $(CFLAGS2) q3cube.o dump.o q3c_poly.o $(LDFLAGS) -o $@

#mini_test: mini_test.o q3cube.o q3c_poly.o
#	$(CC) $< q3cube.o dump.o q3c_poly.o $(LDFLAGS) -o $@
