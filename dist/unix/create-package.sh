#!/bin/sh

. /etc/os-release

[ -z $(which git) ] && [ -z $(which awk) ] && [ -z $(which sed) ] && echo "Install git, awk and sed first." && exit 0

cd ../..
git pull
git submodule init
git submodule update

checkDepends()
{
		[ ! -z $(which qmake) ] && [ ! -z $(which make) ] && [ ! -z $(which gcc) ] && [ ! -z $(which g++) ] && exit 0
	
		echo "You need to install following depends:"
		echo "qt5"
		echo "make"
		echo "gcc4.9 and g++4.9 or above"
		echo "Then just run script again."
		exit 0
}

cleanDir()
{
		make clean
        rm crow
        rm .qmake.stash
}

case $ID in
	"arch" | "parabola" | "manjarolinux" )
        cd archlinux
	
        makepkg
        rm -r pkg
        rm -r src
        cd ..
        cleanDir
	
        echo -e "\x1b[1;32mNow you can install Crow by running the following commands:\x1b[0m"
        echo -e "\x1b[1;37mcd archlinux\x1b[0m"
        echo -e "\x1b[1;37msudo pacman -U crow-git-*.pkg.tar.xz\x1b[0m"
    ;;
	
	"ubuntu" | "linuxmint" | "elementary" | "debian" | "devuan" )
		if [ -z $(which debuild) ]; then
			echo "Package devsripts REQUIRED for building:"
			sudo apt install devscripts
			
			[ "$?" -eq 1 ] && echo "You can still install it by yourself." && exit 0
		fi
		
		tar czfv crow_$(awk 'NR==1 {print $2}' dist/unix/debian/changelog | sed 's/-[1-9]//g;s/[()]//g').orig.tar.gz *
		mv crow_*.orig.tar.gz ..
		cp -r dist/unix/debian .
		debuild -i -us -uc
		rm -rf debian
		cleanDir
		
        echo -e "\x1b[1;32mNow you can install Crow by running the following commands:\x1b[0m"
        echo -e "\x1b[1;37mcd ../../..\x1b[0m"
        echo -e "\x1b[1;37msudo dpkg -i crow-*-amd64.deb\x1b[0m"
        echo -e "\x1b[1;37msudo apt install -f\x1b[0m"
	;;
	
	"fedora" )
	;;
	
	*)
		checkDepends
		
		CORES="$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | wc -l)"
		
		qmake
		make -j$CORES		
		mkdir ../crow_install
		PREFIX=../crow_install make install
		cleanDir
	;;
esac
