SET(CMAKE_CXX_FLAGS "-stdlib=libc++")
SET(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++ -L/usr/lib/llvm-7/lib -lc++ -lc++abi")
SET(CMAKE_SHARED_LINKER_FLAGS "-stdlib=libc++ -L/usr/lib/llvm-7/lib -lc++ -lc++abi")
SET(CMAKE_CXX_COMPILER "clang++-7")
SET(CMAKE_C_COMPILER "clang-7")