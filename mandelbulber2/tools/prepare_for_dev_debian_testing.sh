#!/bin/sh
#

echo "Warning! This script pulls, compiles and installs development version of mandelbulber2
THERE IS NO WARRANTY THAT THE PROGRAM WILL WORK PROPERLY,
because the code can contain partially developed functions

This script will ask about root password to perform installation of system packages and the program

To update the program do not remove already downloaded files. This script will pull only the latest changes."

export CXXFLAGS="-march=native -msse2"
MANDELBULBER_SHARE="/usr/share/mandelbulber2"

set -e # if any of the commands fail the script will exit immediately

echo "
installing all needed packages

"

sudo apt-get install build-essential libqt5gui5 qt5-default libpng16-16 libpng-dev qttools5-dev qttools5-dev-tools qtmultimedia5-dev libgsl-dev libgomp1 git qtcreator

echo "
getting fresh source files from git repository

"

if [ ! -d mandelbulber2 ]; then
	git clone https://github.com/buddhi1980/mandelbulber2
	cd mandelbulber2/mandelbulber2
else
	cd mandelbulber2
	git pull
	cd mandelbulber2
fi

echo "
compiling the program

"

cd Release
qmake mandelbulber.pro
make -j4

echo "
installing the program

"

sudo install mandelbulber2 /usr/bin

echo "
creating links to files from qt_data and deploy 
folders in /usr/share/mandelbulber2 directory
if you change anything in that folder you will
not need to reinstall the program
You have to remember to not delete mandelbulber2 folder located here
"

sudo rm -f -r $MANDELBULBER_SHARE
sudo mkdir $MANDELBULBER_SHARE
cd ..
sudo ln -s ${PWD}/qt_data $MANDELBULBER_SHARE/qt_data
sudo ln -s ${PWD}/data $MANDELBULBER_SHARE/data
sudo ln -s ${PWD}/language $MANDELBULBER_SHARE/language
sudo ln -s ${PWD}/deploy/share/mandelbulber2/materials $MANDELBULBER_SHARE/materials
sudo ln -s ${PWD}/deploy/share/mandelbulber2/examples $MANDELBULBER_SHARE/examples
sudo ln -s ${PWD}/deploy/share/mandelbulber2/icons $MANDELBULBER_SHARE/icons
sudo ln -s ${PWD}/deploy/share/mandelbulber2/textures $MANDELBULBER_SHARE/textures
sudo ln -s ${PWD}/deploy/share/mandelbulber2/toolbar $MANDELBULBER_SHARE/toolbar

echo "To start development of Mandelbulber you can open
mandelbulber2/mandelbulber2/Release/mandelbulber.pro
with QtCreator which is already in your system"

echo "To run the program you need to launch folowing file:
mandelbulber2/mandelbulber2/Release/mandelbulber2"


