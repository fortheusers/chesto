# this script looks at the .bin, gets a list of all the DLLs it depends on, and copies them to the same directory as the bin, then finally renames it to exe

import os
import sys

# get first argument
if len(sys.argv) < 2:
    print("Usage: python win_copy_dlls.py <name of binary>")
    exit(1)

# get name to binary
bin_name = sys.argv[1]

cmd = "ldd " + bin_name
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

# get bin name without extension
bin_short = bin_name.split(".")[0]

# copy to exe file
os.system("cp " + bin_name + " " + bin_short + ".exe")

print("Done! Copied " + str(len(dlls)) + " DLLs to the current folder.")