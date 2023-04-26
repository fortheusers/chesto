# this script looks at appstore.bin, gets a list of all the DLLs it depends on, and copies them to the same directory as appstore.bin, then finally renames appstore.bin to appstore.exe

import os
import shutil
import subprocess
import sys

cmd = "ldd appstore.bin"
output = os.popen(cmd).read()
dlls = []
for line in output.split("\n"):
    if "=>" in line:
        dll_path = line.split("=>")[1].split(" ")[1].strip()
        # skip Windows DLLs
        if dll_path.startswith("/c/Windows"):
            continue
        dlls.append(dll_path)
    
for dll in dlls:
    os.system("cp " + dll + " .")

os.system("cp appstore.bin appstore.exe")

print("Done! Copied " + str(len(dlls)) + " DLLs to the current folder.")