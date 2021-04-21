hunter_config(Protobuf
VERSION 3.14.0-4a09d77-p0-local
URL "https://${URL_BASE}/cpp-pm/protobuf/archive/v3.14.0-4a09d77-p0.tar.gz"
SHA1 3553ff3bfd7d0c4c1413b1552064b3dca6fa213e
CMAKE_ARGS protobuf_BUILD_TESTS=OFF
)

hunter_config(Microsoft.GSL
VERSION 2.0.0-p0-local
URL "https://${URL_BASE}/hunter-packages/Microsoft.GSL/archive/v2.0.0-p0.tar.gz"
SHA1 a94c9c1e41edf787a1c080b7cab8f2f4217dbc4b
CMAKE_ARGS GSL_TEST=OFF
)

hunter_config(wedpr-crypto
VERSION 1.1.0-local
URL "https://${URL_BASE}/WeBankBlockchain/WeDPR-Lab-Crypto/archive/78a0927e65c5e730e8aa9cca09120411853bb817.tar.gz"
SHA1 027041c12f2897ed52331ffee9ec6beea8a3cc8e
CMAKE_ARGS
    CMAKE_INSTALL_LIBDIR=lib
    CMAKE_INSTALL_BINDIR=bin
    CMAKE_INSTALL_INCLUDEDIR=include
)