#!/bin/bash ../.port_include.sh
port=hexdump
version=2.36
useconfigure="true"
files="https://mirrors.edge.kernel.org/pub/linux/utils/util-linux/v${version}/util-linux-${version}.tar.gz util-linux-${version}.tar.gz"
configopts="--target=i686-pc-serenity"
workdir="util-linux-${version}"

build() {
    run make $makeopts hexdump
}

install() {
    run install hexdump "$SERENITY_ROOT/Build/Root/usr/local/bin/hexdump"
}

export CFLAGS="-Wno-deprecated-declarations -Wno-redundant-decls"
