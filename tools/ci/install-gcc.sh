#!/bin/bash -ex

WORK_DIR=/home/travis/opt
INSTALL_DIR=/home/travis/opt/gcc-install
URL=ftp://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-8.2.0/gcc-8.2.0.tar.gz


test ! -d "${INSTALL_DIR}" && mkdir "${INSTALL_DIR}"

if [ -f "${INSTALL_DIR}/bin/g++" ]; then
  exit 0
fi

wget -qO- "${URL}" | tar xz -C "${WORK_DIR}"
cd "${WORK_DIR}"/gcc-?.?.?* 
./contrib/download_prerequisites
./configure --prefix="${INSTALL_DIR}" --disable-multilib
make -j2 install
rm -rf "${WORK_DIR}"/gcc-?.?.?* 
