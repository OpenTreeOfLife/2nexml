This repo contains an NCL-dependent file format conversion tool written in C++
to be used within the Open Tree of Life project for creating NeXML docs.

Why another NEXUS to NeXML conversion when there are already options (see 
https://github.com/nexml/nexml/wiki/NeXML-Manual#wiki-Converting_a_NEXUS_file_to_NeXML ) 
in existence?  Because NCL has had some NeXML output for quite a while, so this
is not really new.





Installation
============

(1) Choose an installation prefix that will serve as the root of tools installed.
In the docs below this will be called OPEN_TREE_TOOL_DIR, but you can actually
choose any variable name that you like. Here we assume that you are using bash:

 export OPEN_TREE_TOOL_DIR="${HOME}/opentree-tools"
 
(2) Download and install NCL. Note that these commands should probably be 
executed from some directory that is *not* under $OPEN_TREE_TOOL_DIR

*Option A* use git:

    cd my-build-dir
    git clone https://github.com/mtholder/ncl
    cd ncl
    sh bootstrap.sh
    mkdir build
    ../configure --prefix="${OPEN_TREE_TOOL_DIR}" --with-constfuncs=yes --disable-shared --with-reserveid
    make -j2 && make check && make install && make installcheck




In addition to putting the libraries you need in $OPEN_TREE_TOOL_DIR/lib, this
set of commands will add 3 executables to $OPEN_TREE_TOOL_DIR/bin. These will be
named NCLconverter, NEXUSnormalizer, NEXUSvalidator. They can be deleted, if you 
do not want them.


