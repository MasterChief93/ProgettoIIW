#!/bin/bash
# My first script

echo "Hello World!"

sudo apt-get install libapr1-dev

cd apr-1.5.2
./configure
make
sudo make install

cd ../httpd-2.2.31
./cofngiure --with-included-apr
make
sudo make install

cd ../libwurfl-master
./configure --with-apache='/usr/local/apache2'
make
sudo make install

sudo ldconfig 

sudo apt-get install libmagickwand-dev