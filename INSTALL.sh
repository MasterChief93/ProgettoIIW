#!/bin/bash
# My first script

echo "Hello World!"

sudo apt-get install libapr1-dev
sudo apt-get install libmagickwand-dev
sudo apt-get install sqlite3 libsqlite3-dev
sudo apt-get install libxml2
sudo apt-get install automake

cd apr-1.5.2
./configure
make
sudo make install

cd ../httpd-2.2.31
./configure --with-included-apr
make
sudo make install

sudo ldconfig
chmod 777 -R libwurfl-master
cd ../libwurfl-master
./configure --with-apache='/usr/local/apache2'
make
sudo make install

sudo ldconfig
