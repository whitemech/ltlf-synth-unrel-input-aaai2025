from pathlib import Path
import subprocess
import os
import shutil
import sys
import csv
import time
from subprocess import PIPE
import threading
import tempfile

# Lock for writing to CSV
csv_lock = threading.Lock()

print('Collecting tests')
p = Path("./tests")
tests = []
for file in p.rglob("*/*.ltlf"):
    # Check for corresponding .part
    corresponding_partfile = file.with_suffix(".part")
    corresponding_output = file.with_name("expected.txt")
    if (not corresponding_partfile.is_file()) or (not corresponding_output.is_file()):
        print('One of the tests is missing the correct files')
        if not corresponding_partfile.is_file():
            print('Expected to find ' + str(corresponding_partfile))
        if not corresponding_output.is_file():
            print('Expected to find ' + str(corresponding_output))
    tests.append((file, corresponding_partfile, corresponding_output))

print('Collected %d test(s).' % (len(tests)))

def generate_temp_files(input_file, temp_dir):
    with open(input_file) as infile:
        lines = infile.readlines()
        main_ltlf = os.path.join(temp_dir, "temp-main.ltlf")
        backup_ltlf = os.path.join(temp_dir, "temp-backup.ltlf")
        with open(main_ltlf, "w") as main_out:
            main_out.write(lines[0])
        with open(backup_ltlf, "w") as backup_out:
            backup_out.write(lines[1])
    return main_ltlf, backup_ltlf

def generate_dfas(dfa_directory, main_ltlf, backup_ltlf, cordfa):
    try:
        # Change working directory to the DFA directory
        original_cwd = Path.cwd()
        os.chdir(dfa_directory)
        main_mona = os.path.join(dfa_directory, "temp-main.mona")
        backup_mona = os.path.join(dfa_directory, "temp-backup.mona")
        main_dfa = os.path.join(dfa_directory, "temp-main.dfa")
        backup_dfa = os.path.join(dfa_directory, "temp-backup.dfa")

        with open(main_mona, "w") as f:
            result = subprocess.run(
                ["./ltlf2fol", "NNF", main_ltlf],
                stdout=f,
                check=True
            )
            if result.returncode != 0:
                raise subprocess.CalledProcessError(result.returncode, result.args)

        if cordfa == 0:
            with open(backup_mona, "w") as f:
                result = subprocess.run(
                    ["./ltlf2fol", "NNF", backup_ltlf],
                    stdout=f,
                    check=True
                )
                if result.returncode != 0:
                    raise subprocess.CalledProcessError(result.returncode, result.args)
        else:
            with open(backup_ltlf, "r") as infile:
                negated_line = "!({})".format(infile.readline().strip())
            with open(backup_ltlf, "w") as outfile:
                outfile.write(negated_line)
            with open(backup_mona, "w") as f:
                result = subprocess.run(
                    ["./ltlf2pfol", backup_ltlf],
                    stdout=f,
                    check=True
                )
                if result.returncode != 0:
                    raise subprocess.CalledProcessError(result.returncode, result.args)

        with open(main_dfa, "w") as f:
            result = subprocess.run(
                ["/data/coml-whitemech/some5590/MONA/Front/mona", "-u", "-xw", main_mona],
                stdout=f,
                check=True
            )
            if result.returncode != 0:
                raise subprocess.CalledProcessError(result.returncode, result.args)
        with open(backup_dfa, "w") as f:
            result = subprocess.run(
                ["/data/coml-whitemech/some5590/MONA/Front/mona", "-u", "-xw", backup_mona],
                stdout=f,
                check=True
            )
            if result.returncode != 0:
                raise subprocess.CalledProcessError(result.returncode, result.args)

        # Restore original working directory
        os.chdir(original_cwd)

        return main_dfa, backup_dfa
    except subprocess.CalledProcessError as e:
        print(f"Error generating DFA: {e}")
        os.chdir(original_cwd)
        return None, None

CONST_TIMEOUT = 1800
failed = 0
runTests = 0
cordfa = int(sys.argv[1]) if len(sys.argv) > 1 else 0  # Default to 0 if no argument provided
testtoRun = sys.argv[2] if len(sys.argv) > 2 else None

if not testtoRun:
    csv_file = "results.csv"
    with open(csv_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Test Name", "DFA Construction Time (ms)", "Synthesis Time (ms)", "Full Time (ms)"])

def run_test(test):
    global runTests, failed
    if testtoRun and testtoRun not in str(test):
        return
    if not testtoRun:
        print(test)
    else:
        print("Running selected test now")

    with tempfile.TemporaryDirectory() as temp_dir:
        if cordfa != 3:
            start_dfa_time = time.perf_counter()
            main_ltlf, backup_ltlf = generate_temp_files(test[0], temp_dir)
            shutil.copyfile(test[1], os.path.join(temp_dir, "temp.part"))
            main_dfa, backup_dfa = generate_dfas(temp_dir, main_ltlf, backup_ltlf, cordfa)
            end_dfa_time = time.perf_counter()
            dfa_construction_time = end_dfa_time - start_dfa_time

            if not main_dfa or not backup_dfa:
                failed += 1
                return

            # Run the test
            try:
                syft_command = f"./Syft {main_dfa} {backup_dfa} {temp_dir}/temp.part 0 partial "
                syft_command += "cordfa" if cordfa else "dfa"
                start_syn_time = time.perf_counter()
                print(syft_command)
                l = subprocess.check_output(
                    syft_command,
                    timeout=CONST_TIMEOUT,
                    cwd="/data/coml-whitemech/some5590/synthesis-with-backup/product-construction/Syft/build/bin/",
                    shell=True
                )
                end_syn_time = time.perf_counter()
                dfa_syn_time = end_syn_time - start_syn_time
                runTests += 1
                with csv_lock:
                    with open(csv_file, mode='a', newline='') as file:
                        writer = csv.writer(file)
                        writer.writerow([test[0], dfa_construction_time, dfa_syn_time, dfa_construction_time + dfa_syn_time])

            except subprocess.TimeoutExpired:
                failed += 1
                print("Timeout when running test %s " % (test[0]))
                with csv_lock:
                    with open(csv_file, mode='a', newline='') as file:
                        writer = csv.writer(file)
                        writer.writerow([test[0], -1, -1, -1])

                return

            rel = None
            if "realizable" in str(l):
                rel = True
            if "unrealizable" in str(l):
                rel = False
            if rel is None:
                print(str(l))
                print("Another error has occurred. Unexpected output when running test %s " % (test[0]))

            # Open outfile
            with open(test[2], "r") as read_file:
                expected_output = int(next(read_file).split()[0])
                if expected_output == 1 and not rel:
                    print("Failed test %s (expected %d and got %d)" % (test[0], expected_output, rel))
                    print(l)
                    failed += 1
                elif expected_output == 0 and rel:
                    print("Failed test %s (expected %d and got %d)" % (test[0], expected_output, rel))
                    print(l)
                    failed += 1
                else:
                    print("ok")
        else:
            print("Running Syft version")
            start = time.perf_counter()
            try:
                l = subprocess.run(["./Syft %s %s 0" % (os.path.abspath(test[0]), os.path.abspath(test[1]))],
                                   cwd="Syft/build/bin/", shell=True, stdout=PIPE, stderr=PIPE, timeout=CONST_TIMEOUT)
                end = time.perf_counter()
                #with csv_lock:
                #    with open(csv_file, mode='a', newline='') as file:
                #        writer = csv.writer(file)
                #        writer.writerow([test[0], -1, -1, end - start])
            except subprocess.TimeoutExpired:
                failed += 1
                print("Timeout when running test %s " % (test[0]))
                with csv_lock:
                    with open(csv_file, mode='a', newline='') as file:
                        writer = csv.writer(file)
                        writer.writerow([test[0], -1, -1, -1])

                return

            runTests += 1
            rel = None
            if "Realizable" in str(l):
                rel = True
            if "Unrealizable" in str(l):
                rel = False
            if rel is None:
                print(str(l))
                print("Another error has occurred. Unexpected output when running test %s " % (test[0]))
            if "token too large" in str(l.stderr):
                print(str(l))
                print("Another error has occurred. Unexpected output when running test %s " % (test[0]))
                return

        if testtoRun:
            sys.exit(0)

        with open(test[2], "r") as read_file:
            expected_output = int(next(read_file).split()[0])
            if expected_output == 1 and not rel:
                print("Failed test %s (expected %d and got %d)" % (test[0], expected_output, rel))
                print(l)
                failed += 1
            elif expected_output == 0 and rel:
                print("Failed test %s (expected %d and got %d)" % (test[0], expected_output, rel))
                print(l)
                failed += 1
            else:
                print("ok")

threads = []

for test in tests:
    t = threading.Thread(target=run_test, args=(test,))
    t.start()
    threads.append(t)

for t in threads:
    t.join()

if failed:
    print("%d out of %d tests have failed" % (failed, runTests))
else:
    print("All tests successful.")
