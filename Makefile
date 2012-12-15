MODULE_big=q3c
OBJS=q3c.o dump.o q3cube.o q3c_poly.o
DATA_built=q3c.sql
DOCS=README.q3c

OPT=-O3
OPT_LOW=-O2
#DEBUG=-g3 -ggdb -DQ3C_DEBUG

GITEXISTS := $(shell which git)
ifeq ($(GITEXISTS),)
Q3C_VERSION='""'
else
Q3C_VERSION='"'"`git describe`"'"'
endif

PG_CPPFLAGS = -DQ3C_VERSION=$(Q3C_VERSION) $(DEBUG) $(OPT) -D_GNU_SOURCE -D__STDC_FORMAT_MACROS
SHLIB_LINK += $(filter -lm, $(LIBS))
EXTRA_CLEAN=dump.c prepare prepare.o gen_data.o \
			tests/join.out tests/cone.out tests/ellipse.out \
			gen_data

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
CPPFLAGS = $(CPPFLAGS) -D$(Q3CVERSION)


prepare: prepare.o q3cube.o q3c_poly.o

dump.c: prepare
	./prepare

prepare: prepare.o q3cube.o q3c_poly.o
	$(CC) $(CPPFLAGS) $(CFLAGS) prepare.o q3cube.o q3c_poly.o $(PG_LIBS) $(LDFLAGS) $(LIBS) -o $@
              
oldclean: 
	rm -f *~ tests/*~

gen_data: gen_data.c
	$(CC) $< $(CPPFLAGS) $(PG_LIBS) $(LDFLAGS) $(LIBS) -o $@

test: gen_data all
	createdb q3c_test
	psql q3c_test -c "CREATE TABLE test (ra double precision, dec double precision)"
	psql q3c_test -c "CREATE TABLE test1 (ra double precision, dec double precision)"
	psql q3c_test -c "CREATE TABLE test_small (ra double precision, dec double precision)"
	./gen_data 1 1000000 | psql q3c_test -c "COPY test FROM STDIN WITH DELIMITER ' '"
	./gen_data 2 1000000 | psql q3c_test -c "COPY test1 FROM STDIN WITH DELIMITER ' '"
	./gen_data 3 10000 | psql q3c_test -c "COPY test_small FROM STDIN WITH DELIMITER ' '"

	psql q3c_test -c '\i q3c.sql'
	psql q3c_test -c 'CREATE INDEX q3c_idx ON test (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'ANALYZE test'
	cat tests/cone.sql | psql q3c_test > tests/cone.out
	diff tests/cone.out tests/cone.expected
	cat tests/cone_join_rev.sql | psql q3c_test > tests/cone.out
	diff tests/cone.out tests/cone.expected
	cat tests/ellipse.sql | psql q3c_test > tests/ellipse.out
	diff tests/ellipse.out tests/ellipse.expected
	cat tests/join.sql | psql q3c_test > tests/join.out
	diff tests/join.out tests/join.expected
	cat tests/version.sql | psql q3c_test > tests/version.out
	diff tests/version.out tests/version.expected
	dropdb q3c_test

dist: clean
	mkdir -p dist
	cp *.c *.h *.sql.in README.q3c COPYING dist
	cat Makefile | sed 's/^Q3C_VERSION=.*$$/Q3C_VERSION="'`git describe`'"/'  > dist/Makefile
	mkdir -p dist/tests
	cp tests/*.expected tests/*.sql dist/tests
	cat q3c.sql.in | perl utils/create_drops.pl > dist/drop_q3c.sql

