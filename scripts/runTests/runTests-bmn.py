from pathlib import Path
from colorama import Fore
import subprocess
import os, timeit

print(Fore.YELLOW + 'Collecting tests')
p = Path("./tests")
tests = []
for file in p.rglob("*/*.ltlf"):
    # Check for corresponding .part 
    
    corresponding_partfile = file.with_suffix(".part")
    corresponding_output = file.with_name("expected.txt")
    if (not corresponding_partfile.is_file()) or (not corresponding_output.is_file()):
        print(Fore.RED + 'One of the tests is missing the correct files')
        if not corresponding_partfile.is_file():
            print(Fore.RED + 'Expected to find '+str(corresponding_partfile))
        if not corresponding_output.is_file():
            print(Fore.RED + 'Expected to find '+str(corresponding_output))
    tests.append((file, corresponding_partfile, corresponding_output))

def runTest(test):
    def f():
        l = subprocess.run(["./Syft %s %s 0"%(os.path.abspath(test[0]), os.path.abspath(test[1] ))], cwd="Syft/build/bin/",shell=True,  capture_output=True)
    return f

print(Fore.GREEN + 'Collected %d test(s).'%(len(tests)))
failed = 0
for test in tests:
    # Run the test
    print(test[0])
    t= timeit.Timer(runTest(test))
    print(t.timeit(30))