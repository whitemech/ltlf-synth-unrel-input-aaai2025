# Cleans up all .part files in the directory
import glob
import os

subdirectory = 'tests/*/'
file_ending = '*.part'

fix_naming = False

# Use glob to find all files with the specific ending in the subdirectory
file_paths = glob.glob(os.path.join(subdirectory, file_ending))
for f in file_paths:
    newfile = []
    with open(f, "r") as partfile:
        lines = partfile.readlines()
        if len(lines) != 3:
            print(f"PARTFILE {f} ERROR: expected 3 lines")
        lines = [line.strip().split(" ") for line in lines]
        if lines[0][0] != ".inputs:":
            print(f"PARTFILE {f} ERROR: line 0 starts with {lines[0][0]} instead of .inputs:")
        if lines[1][0] != ".outputs:":
            print(f"PARTFILE {f} ERROR: line 1 starts with {lines[1][0]} instead of .outputs:")
        if lines[2][0] != ".unobservables:":
            print(f"PARTFILE {f} ERROR: line 1 starts with {lines[2][0]} instead of .unobservables:")    
        s1 = set(lines[0][1:])
        s2 = set(lines[1][1:])
        s3 = set(lines[2][1:])
        if s1 & s2:
            print(f"PARTFILE {f} ERROR: input and output have overlap")
        if not (s3.issubset(s1)):
            print(f"PARTFILE {f} ERROR: unobservables is not a subset of the input")  
            print(s3 - s1)
         
        if fix_naming:
            newfile.append(" ".join([".input"] + lines[0][1:]))
            newfile.append(" ".join([".output"] + lines[0][1:]))
            newfile.append(" ".join([".unobservable"] + lines[0][1:]))
            with open("new.txt", "w") as f:
                f.writelines(newfile)
