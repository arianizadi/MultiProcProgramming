#!/bin/bash

# Get list of executable files from build directory
exec_files=$(find ./build -maxdepth 1 -type f -perm +111)

# Display the list of executable files to choose from
echo "Available executable files:"
select exec_file in $exec_files; do
    if [ -n "$exec_file" ]; then
        break
    else
        echo "Invalid selection. Please try again."
    fi
done

# Run the selected executable with mpirun
mpirun -np 5 --oversubscribe "$exec_file"