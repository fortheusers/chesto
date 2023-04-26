# helper script for windows to find pkg-config dependencies

import os

cmd = "pkg-config -libs -static"
path = "--with-path=/c/MSYS2/mingw64/lib/pkgconfig/"
deps = "sdl2 sdl2_image sdl2_ttf sdl2_mixer libcurl"

# run command and get output
output = os.popen(cmd + " " + path + " " + deps).read()

# remove all duplicates (isn't pkg-config supposed to do this?)
seen = set()
libs = []
for lib in output.split():
    if lib not in seen:
        libs.append(lib)
        seen.add(lib)

print(" ".join(libs))