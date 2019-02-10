#!/bin/sh
# Copyright (C) 2019 by Shining the Master of Warders <shining@linuxcondom.net>                            *
# This file is part of "icmpengine"
# Read COPYING.LESSER for license details

CMAKE_INSTALL_PREFIX="/usr"

if [ "x$1" = "xuninstall" ] ; then
	sudo rm $CMAKE_INSTALL_PREFIX/lib/x86_64-linux-gnu/qt5/plugins/plasma/dataengine/plasma_engine_icmp.so $CMAKE_INSTALL_PREFIX/share/kservices5/plasma-engine-icmp.desktop
	exit $?
fi
[ -d "build" ] || mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX .. && make && sudo make install && kbuildsycoca5 && kquitapp5 plasmashell && kstart5 plasmashell
exit $?
