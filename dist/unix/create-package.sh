#!/bin/sh

. /etc/os-release

case $ID in
	"arch" | "parabola" | "manjarolinux" )
        cd archlinux
	
        makepkg
        rm -r pkg
        rm -r src
        cd ../../..
        make clean
        rm crow
        rm .qmake.stash
	
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
		
		cd ../..
		
		ln -s dist/unix/debian .
		debuild -i -us -uc
		rm debian
		make clean
		rm crow
        rm .qmake.stash
		
        echo -e "\x1b[1;32mNow you can install Crow by running the following commands:\x1b[0m"
        echo -e "\x1b[1;37mcd archlinux\x1b[0m"
        echo -e "\x1b[1;37msudo pacman -U crow-git-*.pkg.tar.xz\x1b[0m"
	;;
	
	"fedora" )
	;;
	
	*)
	;;
esac
