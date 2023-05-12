# this is a helper script to build the project for PC
# across various platforms

# Usage: ./build_pc.sh <name> <platform> <sdl_version>

# <platform> can be one of the following: ubuntu, macos, windows
# <sdl_version> can be one of the following: sdl2, sdl1

if [ $# -ne 3 ]; then
    echo "Usage: ./build_pc.sh <name> <platform> <sdl_version>"
    exit 1
fi

NAME=$1
PLATFORM=$2
SDL_VERSION=$3

# install deps for the current platform
SDL2_CMDS=""
SDL1_CMDS=""

if [ "$PLATFORM" = "ubuntu" ]; then
    sudo apt-get update
    sudo apt-get install -y zlib1g-dev gcc g++ libcurl4-openssl-dev wget git libmpg123-dev libfreetype-dev
    SDL2_CMDS="sudo apt-get install -y libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-gfx-dev libsdl2-mixer-dev"
    SDL1_CMDS="sudo apt-get install -y libsdl1.2-dev libsdl-ttf2.0-dev libsdl-image1.2-dev libsdl-gfx1.2-dev libsdl-mixer1.2-dev"
elif [ "$PLATFORM" = "macos" ]; then
    brew install wget git mpg123 freetype
    SDL2_CMDS="brew install sdl2 sdl2_ttf sdl2_image sdl2_gfx sdl2_mixer"
    SDL1_CMDS="brew install sdl sdl_ttf sdl_image sdl_gfx sdl_mixer"
elif [ "$PLATFORM" = "windows" ]; then
    choco install -y make wget git zip
    wget https://repo.msys2.org/distrib/x86_64/msys2-x86_64-20230318.exe
    ./msys2-x86_64-20230318.exe install --confirm-command --root /c/MSYS2
    export PATH="/c/MSYS2/usr/bin:/c/MSYS2/mingw64/bin:${PATH}"
    pacman --noconfirm -S mingw-w64-x86_64-curl mingw-w64-x86_64-gcc
    SDL2_CMDS="pacman --noconfirm -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_gfx mingw-w64-x86_64-SDL2_mixer"
    SDL1_CMDS="pacman --noconfirm -S mingw-w64-x86_64-SDL mingw-w64-x86_64-SDL_ttf mingw-w64-x86_64-SDL_image mingw-w64-x86_64-SDL_gfx mingw-w64-x86_64-SDL_mixer"
else
    echo "Unknown platform: $PLATFORM"
    exit 1
fi

MAKE_COMMAND="make pc"
EXT="bin"

# run the commands to install SDL, depending on the version
if [ "$SDL_VERSION" = "sdl2" ]; then
    $SDL2_CMDS
elif [ "$SDL_VERSION" = "sdl1" ]; then
    $SDL1_CMDS
    MAKE_COMMAND="make pc-sdl1"
    EXT="bin-sdl1"
else
    echo "Unknown SDL version: $SDL_VERSION"
    exit 1
fi

# call the right make command, (the makefile should take care of platform-dependent stuff)
$MAKE_COMMAND

# package the binary into a zip, alongside assets
SYSTEM_SPECIFIC=""
if [ "$PLATFORM" = "ubuntu" ]; then
    echo "cd \$(dirname \$0) && ./${NAME}.${EXT}" > run.sh
    chmod +x run.sh
    SYSTEM_SPECIFIC="run.sh"
elif [ "$PLATFORM" = "macos" ]; then
    python3 ./libs/chesto/helpers/mac_copy_libs.py ${NAME}.${EXT} # creates the .app
    EXT="app"
elif [ "$PLATFORM" = "windows" ]; then
    python3 ./libs/chesto/helpers/win_copy_dlls.py ${NAME}.${EXT} # creates the .exe
    EXT="bat"   # the batch script is the main now
    echo "start .\\contents\\${NAME}.exe" > ${NAME}.${EXT}
    mkdir -p contents
    mv ${NAME}.exe *.dll contents
    SYSTEM_SPECIFIC="contents"
fi

# error out if we don't have the binary! (or folder)
if [ ! -f ${NAME}.${EXT} ] && [ ! -d ${NAME}.${EXT} ]; then
    echo "Binary not found: ${NAME}.${EXT}, build error?"
    exit 1
fi

chmod +x ${NAME}.${EXT}
zip -r -9 "${NAME}_${PLATFORM}_${SDL_VERSION}.zip" ${NAME}.${EXT} resin ${SYSTEM_SPECIFIC}


# depending on the OS, package the resulting binary in a zip file
# if [ "$PLATFORM" = "ubuntu" ]; then
# elif [ "$PLATFORM" = "macos" ]; then
#     zip -r -j -9 "${NAME}_${PLATFORM}_${SDL_VERSION}.zip" ${NAME}.${EXT}
# elif [ "$PLATFORM" = "windows" ]; then
#     zip -r -j -9 "${NAME}_${PLATFORM}_${SDL_VERSION}.zip" ${NAME}.${EXT}
# fi

# TODO: on macos, bundle into a .app folder and add an info.plist