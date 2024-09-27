#!/bin/bash

# Get list of executable files from build directory
exec_files=$(find ./build -maxdepth 1 -type f -perm +111)

# Check if any executable files were found
if [ -z "$exec_files" ]; then
    echo "No executable files found in the build directory."
    exit 1
fi

# Display the list of executable files to choose from
echo "Available executable files:"
select exec_file in $exec_files; do
    if [ -n "$exec_file" ]; then
        break
    else
        echo "Invalid selection. Please try again."
    fi
done

# Prompt the user to enter the number of processors to use
read -p "Enter the number of processors to use: " num_procs

# Prompt the user to enter additional arguments
read -p "Enter any additional arguments to pass to the executable: " args

# Run the selected executable with mpirun, specified number of processors, and additional arguments
mpirun -np "$num_procs" --oversubscribe "$exec_file" $args