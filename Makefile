EXTENSION = q3c
EXTVERSION := $(shell grep default_version $(EXTENSION).control | \
		 sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")


DOCS = README.md
OBJS = dump.o q3c.o q3c_poly.o q3cube.o
MODULE_big = q3c
DATA = $(wildcard scripts/*sql)
PG_CONFIG = pg_config
SHLIB_LINK += $(filter -lm, $(LIBS))
EXTRA_CLEAN = dump.c prepare prepare.o gen_data.o \
			results/join.out results/cone.out results/ellipse.out \
			results/version.out results/poly.out results/area.out \
			gen_data

OPT = -O3
OPT_LOW = -O2
#DEBUG = -g3 -ggdb -DQ3C_DEBUG
PG_CPPFLAGS = $(DEBUG) $(OPT) -D_GNU_SOURCE -D__STDC_FORMAT_MACROS -DQ3C_VERSION='"'$(EXTVERSION)'"'
CPPFLAGS = $(CPPFLAGS) -D$(Q3CVERSION)

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
PGVER := $(shell echo $(VERSION) | sed "s/^\([^\.]\+\)\..*/\1/" )
PGVERNEW := $(shell if [ $(PGVER) -ge 12 ] ; then echo N ; else echo O ; fi )

ifeq ($(PGVERNEW), N)
	PG_LIBS += -L$(shell $(PG_CONFIG) --pkglibdir)
	LIBS := $(filter-out -lpam -lxml2 -lxslt -lselinux -ledit, $(LIBS))
	MYBINLIBS := $(LIBS) $(PG_LIBS) -lm
else
	MYBINLIBS := $(PG_LIBS) -lm
endif

dump.c: prepare
	./prepare

prepare: prepare.o q3cube.o q3c_poly.o
	$(CC) prepare.o q3cube.o q3c_poly.o $(PG_LDFLAGS) $(LDFLAGS) $(MYBINLIBS) -o $@

gen_data: gen_data.c
	$(CC) $< $(CPPFLAGS) $(PG_LFGLAGS) $(LDFLAGS) $(MYBINLIBS) -o $@

test: gen_data all
	createdb q3c_test
	psql q3c_test -c "CREATE TABLE test (ra double precision, dec double precision)"
	psql q3c_test -c "CREATE TABLE test1 (ra double precision, dec double precision)"
	psql q3c_test -c "CREATE TABLE test_pm0 (ra double precision, dec double precision, pmra real, pmdec real, epoch real)"
	psql q3c_test -c "CREATE TABLE test_pm1 (ra double precision, dec double precision, pmra real, pmdec real, epoch real)"
	psql q3c_test -c "CREATE TABLE test_small (ra double precision, dec double precision)"
	./gen_data 1 1000000 | psql q3c_test -c "COPY test FROM STDIN WITH DELIMITER ' '"
	./gen_data 2 1000000 | psql q3c_test -c "COPY test1 FROM STDIN WITH DELIMITER ' '"
	./gen_data 3 100000 | psql q3c_test -c "COPY test_small FROM STDIN WITH DELIMITER ' '"

	./gen_data 4 1000000 --withpm --pmscale=0 --randomepoch | psql q3c_test -c "COPY test_pm0 FROM STDIN WITH DELIMITER ' '"
	./gen_data 5 1000000 --withpm --pmscale=1000 --epoch=2015 | psql q3c_test -c "COPY test_pm1 FROM STDIN WITH DELIMITER ' '"

	psql q3c_test -c 'create extension q3c'
	psql q3c_test -c 'CREATE INDEX q3c_idx1 ON test1 (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'CREATE INdex ON test_pm0 (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'CREATE INDEX on test_pm1 (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'CREATE INDEX q3c_idx ON test (q3c_ang2ipix(ra,dec))'

	psql q3c_test -c 'CREATE INDEX q3c_idx_small ON test_small (q3c_ang2ipix(ra,dec))'
	psql q3c_test -c 'ANALYZE test'
	psql q3c_test -c 'ANALYZE test1'
	psql q3c_test -c 'ANALYZE test_pm0'
	psql q3c_test -c 'ANALYZE test_pm1'
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
	cat sql/join_pm1.sql | psql q3c_test > results/join_pm1.out
	diff results/join_pm1.out expected/join_pm1.expected
	cat sql/join_pm2.sql | psql q3c_test > results/join_pm2.out
	diff results/join_pm2.out expected/join_pm2.expected
	cat sql/poly.sql | psql q3c_test > results/poly.out
	diff results/poly.out expected/poly.expected
	cat sql/poly1.sql | psql q3c_test > results/poly1.out
	diff results/poly1.out expected/poly.expected
	cat sql/version.sql | psql q3c_test > results/version.out
	diff results/version.out expected/version.expected
	cat sql/area.sql | psql q3c_test > results/area.out
	diff results/area.out expected/area.expected
	dropdb q3c_test
