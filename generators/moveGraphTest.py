#!/usr/bin/python3
import os
from pathlib import Path
import shutil
import subprocess
import sys

def main(filename, boolean):
    # Step 1: Call `python3 generate.py f`
    subprocess.run(["python3", "graph.py", Path("examples-graph")/Path(filename)], check=True)

    # Step 2: Create directory `../tests/{f}/`
    target_dir = os.path.join("..", "tests", filename)
    os.makedirs(target_dir, exist_ok=True)

    # Step 3: Copy `example.ltlf` to `../tests/{f}/{f}.ltlf`
    shutil.copy("example.ltlf", os.path.join(target_dir, f"{filename}.ltlf"))

    # Step 4: Copy `example.part` to `../tests/{f}/{f}.part`
    shutil.copy("example.part", os.path.join(target_dir, f"{filename}.part"))

    # Step 5: Create `expected.txt` with contents 1 or 0
    with open(os.path.join(target_dir, "expected.txt"), "w") as expected_file:
        expected_file.write(str(boolean))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: script.py <filename> <boolean>")
        sys.exit(1)

    filename = sys.argv[1]
    boolean = sys.argv[2]

    if boolean not in ["0", "1"]:
        print("The boolean argument must be 0 or 1.")
        sys.exit(1)

    boolean = int(boolean)

    main(filename, boolean)
