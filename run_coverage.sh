#!/bin/bash
wget https://ftp.postgresql.org/pub/source/v11.8/postgresql-11.8.tar.bz2
tar xfj postgresql-11.8.tar.bz2
mv postgresql-11.8 ../
cd ../postgresql-11.8/
./configure --enable-coverage --prefix=$PWD/../pg_install/
make install
cd ../q3c/
export PATH=$PWD/../pg_install/bin/:$PATH
make clean
../pg_install/bin/initdb -D ../pg_install/data
../pg_install/bin/postgres -D ../pg_install/data &
sleep 2
make install
make test
