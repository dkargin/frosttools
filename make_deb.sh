#!/bin/bash

MAKE_CMD="make -C build install"

REQUIRES=""
PROVIDES="libfrosttools"
NAME="libfrosttools"

checkinstall -D --pkgname $NAME --provides $PROVIDES --nodoc --pkggroup "robotics" --pakdir ~/deb $MAKE_CMD
