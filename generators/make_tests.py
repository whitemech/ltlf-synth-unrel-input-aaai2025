import os
import shutil

def process_files(directory):
    # Ensure the 'tests' subdirectory exists
    tests_dir = os.path.join(directory, 'tests')
    os.makedirs(tests_dir, exist_ok=True)

    # Get all files in the directory
    files = os.listdir(directory)

    # Iterate through the files
    for file in files:
        # Check if the file is a .ltlf file
        if file.endswith('.ltlf'):
            base_name = file[:-5]
            ltlf_file = os.path.join(directory, file)
            part_file = os.path.join(directory, base_name + '.part')

            # Check if the corresponding .part file exists
            if os.path.exists(part_file):
                # Create a new subdirectory in 'tests' with the base name
                new_dir = os.path.join(tests_dir, base_name)
                os.makedirs(new_dir, exist_ok=True)

                # Copy the .ltlf and .part files into the new directory
                shutil.copy(ltlf_file, new_dir)
                shutil.copy(part_file, new_dir)

                # Determine the content of the 'expected' file
                expected_content = '0' if 'unsolv' in base_name else '1'
                
                # Write the 'expected' file
                expected_file = os.path.join(new_dir, 'expected.txt')
                with open(expected_file, 'w') as f:
                    f.write(expected_content)

if __name__ == "__main__":
    # Replace 'your_directory_path' with the path of the directory you want to process
    your_directory_path = '.'
    process_files(your_directory_path)

