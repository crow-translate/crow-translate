#!/bin/bash -e

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
        cd pacman
    
        makepkg
        rm -r pkg
        rm -r src
        cd ../../..
        cleanDir
    
        echo -e "\x1b[1;32mNow you can install Crow Translate by running the following commands:\x1b[0m"
        echo -e "\x1b[1;37mcd pacman\x1b[0m"
        echo -e "\x1b[1;37msudo pacman -U crow-git-*.pkg.tar.xz\x1b[0m"
        ;;
    "ubuntu" | "linuxmint" | "elementary" | "debian" | "devuan" )
        echo "Installing dependencies..."
        sudo apt install debhelper devscripts qt5-default qt5-qmake libqt5x11extras5-dev qttools5-dev-tools qtmultimedia5-dev qtbase5-dev qtbase5-dev-tools
        
        cd ../..
        cp -r dist/unix/apt/debian debian
        ver=$(awk 'NR==1 {print $2}' debian/changelog | sed 's/[()]//g')
        tar czfv ../crow-translate_{$ver%-*}.orig.tar.gz .github *
        debuild -i -us -uc
        rm -r debian
        cleanDir
        
        echo -e "\x1b[1;32mNow you can install Crow Translate by running the following commands:\x1b[0m"
        echo -e "\x1b[1;37mcd ../../..\x1b[0m"
        echo -e "\x1b[1;37msudo dpkg -i crow-translate_{$ver}_amd64.deb\x1b[0m"
        echo -e "\x1b[1;37msudo apt install -f\x1b[0m"
        ;;
    "fedora" )
        echo -e "\x1b[1;31mSorry, package generation for your distribution temporally unsupported\x1b[0m"
        ;;
    *)
        echo -e "\x1b[1;37mCan't determine your distribution. The project will be compiled and installed in the \"install\" folder.\x1b[0m"
        
        qmake
        make
        mkdir ../install
        PREFIX=../install make -j$(nproc) install
        cleanDir
        echo -e "\x1b[1;32mCrow Translate was successfully installed in the \"install\" folder\x1b[0m"
    ;;
esac
