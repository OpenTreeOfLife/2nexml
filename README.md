This repo contains an NCL-dependent file format conversion tool written in C++
to be used within the Open Tree of Life project for creating NeXML docs.

Why another NEXUS to NeXML conversion when there are already options (see 
https://github.com/nexml/nexml/wiki/NeXML-Manual#wiki-Converting_a_NEXUS_file_to_NeXML ) 
in existence?  Because NCL has had some NeXML output for quite a while, so this
is not really new.





Installation of NCL and 2nexml
==============================
Prerequisites:
* make
* a C++ compiler
* autoconf (tested on version 2.69)
* automake (tested on version 1.11.5)

(1) Choose an installation prefix that will serve as the root of tools installed.
In the docs below this will be called OPEN_TREE_TOOL_DIR, but you can actually
choose any variable name that you like. Here we assume that you are using bash:

 export OPEN_TREE_TOOL_DIR="${HOME}/opentree-tools"
 
(2) Download and install NCL. Note that these commands should probably be 
executed from some directory that is *not* under $OPEN_TREE_TOOL_DIR

*Option A:* use git:

    cd my-build-dir
    git clone https://github.com/mtholder/ncl
    cd ncl
    sh bootstrap.sh
    mkdir build
    ../configure --prefix="${OPEN_TREE_TOOL_DIR}" --with-constfuncs=yes --disable-shared --with-reserveid
    make -j2 && make check && make install && make installcheck


*Option B:* build NCL from a posted source archive. *Instructions coming soon*.

*Option C:* download the NCL libraries. *Coming soon*.

In addition to putting the libraries you need in $OPEN_TREE_TOOL_DIR/lib, this
set of commands will add 3 executables to $OPEN_TREE_TOOL_DIR/bin. These will be
named NCLconverter, NEXUSnormalizer, NEXUSvalidator. They can be deleted, if you 
do not want them.


(3) Create a build directory, configure, build and install 2nexml.

    mkdir build
    sh bootstrap.sh
    cd build
    ../configure --prefix="${OPEN_TREE_TOOL_DIR}" --with-ncl="${OPEN_TREE_TOOL_DIR}"
    make -j2 && make check && make install && make installcheck

If all goes well, you should have a 2nexml commandline tool in ${OPEN_TREE_TOOL_DIR}/bin


Usage
=====
You can invoke the 2nexml tool with -h to see command line flags to alter the
behavior of the tool. The basic usage is 

    2nexml in.nex > out.xml

To produce a NeXML file called out.xml by redirecting the standard output of the
2nexml program.


Bug reports
===========
Please report bugs via the github issue tracker at https://github.com/OpenTreeOfLife/2nexml/issues
