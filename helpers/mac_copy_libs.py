# this script looks at the .bin, gets a list of all the libs it depends on, and copies them to the same directory as the bin, then bundles it into a .app folder

import os
import sys

# get first argument
if len(sys.argv) < 2:
    print("Usage: python mac_copy_libs.py <name of binary>")
    exit(1)

# get name to binary
bin_name = sys.argv[1]

cmd = "otool -L " + bin_name
output = os.popen(cmd).read()
libs = []

for line in output.split("\n"):
    if "(" in line:
        lib_path = line.split("(")[0].strip()
        # skip non-opt libs
        if not lib_path.startswith("/opt/"):
            continue
        libs.append(lib_path)

# copy them out into this folder
# for lib in libs:
#     os.system("cp " + lib + " .")

# print("Done! Copied " + str(len(libs)) + " libs to the current folder.")

# use otool to change the paths of the libs
for lib in libs:
    lib_name = lib.split("/")[-1]
    os.system("install_name_tool -change " + lib + " @executable_path/" + lib_name + " " + bin_name)

# create .app folder structure for macos
bin_short = bin_name.split(".")[0]
os.system("mkdir -p " + bin_short + ".app/Contents/MacOS")
# os.system("mkdir " + bin_short + ".app/Contents/Resources")
# os.system("mkdir " + bin_short + ".app/Contents/Frameworks")

# copy the binary into the .app folder
os.system("cp " + bin_name + " " + bin_short + ".app/Contents/MacOS")

# create Info.plist file
plist = """<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN"
"http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
    <dict>
        <key>CFBundleExecutable</key>
        <string>""" + bin_name + """</string>
        <key>CFBundleGetInfoString</key>
        <string>Created by Chesto</string>
    </dict>
</plist>
"""

# copy the dylibs in
for lib in libs:
    lib_name = lib.split("/")[-1]
    os.system("cp " + lib + " " + bin_short + ".app/Contents/MacOS")

# copy in the resinf older
os.system("cp -r resin " + bin_short + ".app/Contents/Resources")

# write to file
f = open(bin_short + ".app/Contents/Info.plist", "w")
f.write(plist)
f.close()
