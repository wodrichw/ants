

# Build and Install
1. Get vcpkg up and running by doing `git submodule init; git submodule update; ./vcpkg/bootstrap-vcpkg.sh ; ./vcpkg/vcpkg install `
2. Get cmake to generate make file from CMakeLists.txt `cmake . `
3. Build ants executable `make` -- this currently builds ants with debug symbols as specified in CMakeLists.txt file

