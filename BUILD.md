git clone --recurse-submodules https://github.com/ideorg/textviewers.git

Prerequisites:

Ubuntu, Mint and similar:
* sudo apt install qt6-base-dev
* sudo apt install libgtest-dev
* sudo apt install libicu-dev libpcre2-dev pkg-config

Build:
* create build direcotry
* cd build
* cmake ..
* make -j4

After build in build directory
will create test_viewer and in examples/viewer
subdirectory will create textViewer 