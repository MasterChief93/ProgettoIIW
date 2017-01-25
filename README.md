#A simple multi-process multi-threaded Web Server
*meant to deliver adapted static contents according device User Agent*

Preliminar instruction:
These 3 folders need to be present inside the main folder of the projectt in order to compile correctly the libwurfl library:
- apr-1.5.2 (folder obtained by extracting the file downloadable here: http://apache.panu.it//apr/apr-1.5.2.tar.gz
- httpd-2.2.31 (folder obtained by extracting the file downloadable here: http://mirrors.muzzy.it/apache//httpd/httpd-2.2.31.tar.gz
- libwurfl-master (You can download the folder here, because we modified a file in order to work with our project: https://www.dropbox.com/s/r2ci8qg96bhqlt1/libwurfl-master.zip?dl=0

The file wurfl.xml needs to be present inside the main folder, you can download it from the wurfl official site or you can take it from the "etc" folder inside "libwurfl-master" folder.

Installation instruction:

- open a terminal into this folder
- ./INSTALL.sh 
- if the command does not work, give permissions to this file ("chmod 755 INSTALL.sh") and retry
- if the INSTALL work, let it does its work
- Once the installation finishes, exec the command "make" into the folder
- Launch "./main" once the compilation has finished, in order to launch the server menu.
