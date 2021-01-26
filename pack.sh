#!/bin/bash

version=1.2

[ -d ./vdiskchain-${version} ] && rm -rf ./vdiskchain-${version}
mkdir -p ./vdiskchain-${version}/doc

cp -a ./Tool/README* ./vdiskchain-${version}/doc/
cp -a ./Tool/ipxe.krn ./vdiskchain-${version}
cp -a ./vdiskchain ./vdiskchain-${version}

tar -czf vdiskchain-${version}.tar.gz vdiskchain-${version}/

rm -rf vdiskchain-${version} 
