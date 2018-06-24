#!/usr/bin/env bash

. /etc/os-release

cleanDir()
{
		make clean
        rm crow
        rm .qmake.stash
        rm Makefile
}

case $ID in
	"arch" | "parabola" | "manjarolinux" )
        cd archlinux
	
        makepkg
        rm -r pkg
        rm -r src
        cd ../../..
        cleanDir
	
        echo -e "\x1b[1;32mNow you can install Crow Translate by running the following commands:\x1b[0m"
        echo -e "\x1b[1;37mcd archlinux\x1b[0m"
        echo -e "\x1b[1;37msudo pacman -U crow-git-*.pkg.tar.xz\x1b[0m"
    ;;
	
	"ubuntu" | "linuxmint" | "elementary" | "debian" | "devuan" )
		if [ -z $(which debuild) ]; then
			echo "Package devsripts REQUIRED for building:"
			sudo apt install devscripts qt5-default qt5-qmake libqt5x11extras5-dev qttools5-dev-tools qtmultimedia5-dev qtbase5-dev qtbase5-dev-tools
			
			[ "$?" -eq 1 ] && echo "You can still install it by yourself." && exit 0
		fi
		
		cd ../..
		VER=$(awk 'NR==1 {print $2}' dist/unix/debian/changelog | sed 's/-[1-9]//g;s/[()]//g')
		tar czfv crow-translate_$VER.orig.tar.gz *
		mv crow-translate_*.orig.tar.gz ..
		cp -r dist/unix/debian .
		debuild -i -us -uc
		rm -rf debian
		cleanDir
		
        echo -e "\x1b[1;32mNow you can install Crow Translate by running the following commands:\x1b[0m"
        echo -e "\x1b[1;37mcd ../../..\x1b[0m"
        echo -e "\x1b[1;37msudo dpkg -i crow-translate_$VER-1_amd64.deb\x1b[0m"
        echo -e "\x1b[1;37msudo apt install -f\x1b[0m"
	;;
	
	"fedora" )
	echo -e "\x1b[1;31mSorry, package generation for your distribution temporally unsupported\x1b[0m"
	;;
	
	*)
		echo -e "\x1b[1;37mCan't determine your distribution. The project will be compiled and installed in the \"install\" folder.\x1b[0m"
		
		export MAKEFLAGS="-j$(nproc)"
		qmake
		make
		mkdir ../install
		PREFIX=../install make install
		cleanDir
		echo -e "\x1b[1;32mCrow Translate was successfully installed in the \"install\" folder\x1b[0m"
	;;
esac
