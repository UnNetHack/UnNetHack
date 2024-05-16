#!/bin/bash -xe

# packages for Debian
# dpkg --add-architecture i386 && apt-get update
# sudo apt-get install gcc-mingw-w64-i686 wine wine32 wine-binfmt imagemagick
#
# Arch:
# mingw-w64-gcc wine imagemagick


if [ ! -d "liblua" ]; then
    git clone -b v5.4 --depth 1 https://github.com/lua/lua.git liblua

    cd liblua
    sed -i -e 's/ -DLUA_USE_LINUX -DLUA_USE_READLINE//' makefile
    sed -i -e 's/^MYLIBS=.*/MYLIBS=/' makefile
    make AR='i686-w64-mingw32-ar rc' CC=i686-w64-mingw32-gcc RANLIB=i686-w64-mingw32-ranlib
    cd ..
fi

export LUA=`pwd`/liblua/lua.exe
export LUA_INCLUDE=-I`pwd`/liblua
export LUA_LIB="`pwd`/liblua/liblua.a -lm"

INSTALL=/tmp/unnethack_win32
DESTDIR=/tmp/unnethack_destdir
mkdir -p $DESTDIR $INSTALL

function compile_unnethack {
	env CFLAGS='-O2 -Wall -Wno-unused' ./configure \
		--host i686-w64-mingw32 \
		--prefix=$INSTALL \
		--with-owner="`id -un`" \
		--with-group="`id -gn`" \
		--build=i686-pc-mingw32 \
		--without-compression --disable-file-areas \
		--disable-status-color --enable-score-on-botl --enable-realtime-on-botl \
		$GRAPHICS \
		&& make --trace install
}

rm -rf $INSTALL/share/unnethack $DESTDIR/unnethack-win32-*

GRAPHICS="--disable-mswin-graphics --enable-tty-graphics"
compile_unnethack
mv $INSTALL/share/unnethack/unnethack.exe $INSTALL/share/unnethack/UnNetHack.exe

GRAPHICS="--enable-mswin-graphics --disable-tty-graphics"
compile_unnethack
mv $INSTALL/share/unnethack/unnethack.exe $INSTALL/share/unnethack/UnNetHackW.exe

rm -f $INSTALL/share/unnethack/unnethack.exe.old

make win32_release
make release_archive
