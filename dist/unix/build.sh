#!/bin/sh

. /etc/os-release

case $ID in
	"arch" | "parabola" | "manjarolinux" )
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
		
		echo "You can now install it by type:"
		echo "cd .."
		echo "dpkg -i crow_*-amd64.deb"
	;;
	
	"fedora" )
	;;
	
	*)
	;;
esac
