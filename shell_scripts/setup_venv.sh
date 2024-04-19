#!/bin/bash

VENV_DIR="./venv"

# Check if virtual environment directory exists
if [ -d "$VENV_DIR" ]; then
    echo "Virtual environment already exists in the current directory."
else
    echo "Creating a new virtual environment..."
    # Create a new virtual environment
    python3 -m venv "$VENV_DIR"
    if [ $? -eq 0 ]; then
        echo "Virtual environment created successfully."
        $VENV_DIR/bin/pip install -r requirements.txt
    else
        echo "Error creating virtual environment."
        exit 1
    fi
fi
