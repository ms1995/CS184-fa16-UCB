# CS184 Assignment 2: Yet Another Renderer II
More details about my submission are available at:
https://inst.eecs.berkeley.edu/~cs184-adp/renderer2.html

# How to Build
`cd` into the assignment directory, type into the Terminal the following step by step:

* mkdir build
* cd build
* cmake -DCMAKE_BUILD_TYPE=Release
* make

Currently I've only built `libjpeg.a` (yes that means it was linked statically to eliminate the disgusting runtime problem) for OS X and Linux x64. If you need to make your own, you might find the following tutorial I wrote useful (it demonstrates how I built `libjpeg.a` for my submission).

* Clone LibJPEG's source codes from [here](https://github.com/LuaDist/libjpeg) into some folder.
* Go to that folder (let's call it root folder). Run `cmake . && make`.
* Open `CMakeFiles/jpeg.dir/link.txt`. Starting from some point to the end, there's a list of `.o` files.
* Run `ar rcs libjpeg.a LIST_OF_DOT_O_FILES` from root folder. Copy `libjpeg.a`, as well as `jpeglib.h`, `jerror.h`, `jconfig.h`, `jmorecfg.h` into somewhere of the project folder, like `libjpeg/lib/`.
* Modify CMakeLists.txt in the project by adding the following lines at the end.

```
file(GLOB LIBRARIES PATH_TO_LIBJPEG_DOT_A)
target_link_libraries(PROJECT_NAME ${LIBRARIES})
```

Include `jpeglib.h` in the code and it should be working.
 
Note that LibJPEG and the project should be compiled on the same arch, because the generated `libjpeg.a` is arch-specific.