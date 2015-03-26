MODULE_big=q3c
OBJS=q3c.o dump.o q3cube.o q3c_poly.o
DATA_built=q3c.sql
DOCS=README.q3c

OPT=-O3
OPT_LOW=-O2
#DEBUG=-g3 -ggdb -DQ3C_DEBUG

GITEXISTS := $(shell which git 2>/dev/null)
ifeq ($(GITEXISTS),)
Q3C_VERSION='""'
else
Q3C_VERSION='"'"`git describe --tags`"'"'
endif

PG_CPPFLAGS = -DQ3C_VERSION=$(Q3C_VERSION) $(DEBUG) $(OPT) -D_GNU_SOURCE -D__STDC_FORMAT_MACROS
SHLIB_LINK += $(filter -lm, $(LIBS))
EXTRA_CLEAN=dump.c prepare prepare.o gen_data.o \
			results/join.out results/cone.out results/ellipse.out \
			results/version.out results/poly.out results/area.out \
			gen_data

ifdef NO_PGXS
subdir = contrib/q3c
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
else
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
ifndef PGXS
$(error You should have `pg_config` program in your PATH or compile Q3C with\
'make NO_PGXS=1' \
after putting it in the contrib subdirectory of Postgres sources)
endif
include $(PGXS)
endif
CPPFLAGS = $(CPPFLAGS) -D$(Q3CVERSION)
MYBINLIBS = -lm
# I have to use this instead of PG_LIBS, because PG_LIBS brings a
# bunch of libraries which are often not installed

prepare: prepare.o q3cube.o q3c_poly.o

dump.c: prepare
	./prepare

prepare: prepare.o q3cube.o q3c_poly.o
	$(CC) prepare.o q3cube.o q3c_poly.o $(MYBINLIBS) -o $@
              
oldclean: 
	rm -f *~ sql/*~

gen_data: gen_data.c
	$(CC) $< $(MYBINLIBS) -o $@

test: gen_data all
	createdb q3c_test
	psql q3c_test -c "CREATE TABLE test (ra double precision, dec double precision)"
	psql q3c_test -c "CREATE TABLE test1 (ra double precision, dec double precision)"
	psql q3c_test -c "CREATE TABLE test_small (ra double precision, dec double precision)"
	./gen_data 1 1000000 | psql q3c_test -c "COPY test FROM STDIN WITH DELIMITER ' '"
	./gen_data 2 1000000 | psql q3c_test -c "COPY test1 FROM STDIN WITH DELIMITER ' '"
	./gen_data 3 100000 | psql q3c_test -c "COPY test_small FROM STDIN WITH DELIMITER ' '"
	psql q3c_test -c '\i q3c.sql'
	psql q3c_test -c 'CREATE INDEX q3c_idx ON test (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'CREATE INDEX q3c_idx1 ON test1 (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'CREATE INDEX q3c_idx_small ON test_small (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'ANALYZE test'
	psql q3c_test -c 'ANALYZE test1'
	psql q3c_test -c 'ANALYZE test_small'
	mkdir -p results
	cat sql/ang2ipix.sql | psql q3c_test > results/ang2ipix.out
	diff results/ang2ipix.out expected/ang2ipix.expected
	cat sql/cone.sql | psql q3c_test > results/cone.out
	diff results/cone.out expected/cone.expected
	cat sql/cone_join_rev.sql | psql q3c_test > results/cone.out
	diff results/cone.out expected/cone.expected
	cat sql/ellipse.sql | psql q3c_test > results/ellipse.out
	diff results/ellipse.out expected/ellipse.expected
	cat sql/join.sql | psql q3c_test > results/join.out
	diff results/join.out expected/join.expected
	cat sql/poly.sql | psql q3c_test > results/poly.out
	diff results/poly.out expected/poly.expected
	cat sql/version.sql | psql q3c_test > results/version.out
	diff results/version.out expected/version.expected
	cat sql/area.sql | psql q3c_test > results/area.out
	diff results/area.out expected/area.expected
	dropdb q3c_test

dist: clean
	mkdir -p dist
	cp *.c *.h *.sql.in README.q3c COPYING dist
	cat Makefile | sed 's/^Q3C_VERSION=.*$$/Q3C_VERSION='"'"$(Q3C_VERSION)"'"'/'  > dist/Makefile
	mkdir -p dist/sql
	mkdir -p dist/expected	
	cp expected/*.expected dist/expected
	cp sql/*.sql dist/sql
	cat q3c.sql.in | perl utils/create_drops.pl > dist/drop_q3c.sql

