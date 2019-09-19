FROM ubuntu:18.04

RUN apt-get update &&               \
    apt-get upgrade -y &&           \
    apt-get dist-upgrade -y &&      \
    apt-get install -y              \
        qt5-default                 \
        qt5-qmake                   \
        libqt5x11extras5-dev        \
        qttools5-dev-tools          \
        qtmultimedia5-dev           \
        qtbase5-dev                 \
        qtbase5-dev-tools         &&\
    apt-get clean

ENTRYPOINT ["/bin/bash"] 
