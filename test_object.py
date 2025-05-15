#!/usr/bin/env python3
"""
Python object generation script for Serpent testing
Creates and keeps various types of basic objects in memory
"""
import os
import sys
import time
import gc
import threading
import random


def print_info():
    """Display process information"""
    print(f"Python {sys.version}")
    print(f"Running PID: {os.getpid()}")
    print(f"Executable: {sys.executable}")
    print(f"Number of objects in memory: {len(gc.get_objects())}")
    print("To display ID and memory address, specify arguments as follows:")
    print("  obj_id = Object ID number (integer) ")


# Create and hold various objects
objects = []

# Integer
int_obj = 12345
objects.append(int_obj)

# String
str_obj = "Serpent Test String"
objects.append(str_obj)

# List
list_obj = [1, 2, 3, 4, 5]
objects.append(list_obj)

# Dictionary
dict_obj = {"name": "Serpent", "type": "Memory Tracer", "version": 0.1}
objects.append(dict_obj)

# Class definition and instance
class TestClass:
    def __init__(self, name):
        self.name = name
        self.data = [random.random() for _ in range(10)]
    
    def get_name(self):
        return self.name

# Create class instance
instance = TestClass("TestInstance")
objects.append(instance)

# Function to display object ID and address
def show_obj_info(obj_id):
    if 0 <= obj_id < len(objects):
        obj = objects[obj_id]
        print(f"Object ID {obj_id}:")
        print(f"  Type: {type(obj).__name__}")
        print(f"  Value: {obj}")
        print(f"  Address: {hex(id(obj))}")
    else:
        print(f"Error: Object ID {obj_id} is out of range")


# Main process
if __name__ == "__main__":
    print_info()
    
    # If arguments are provided, display information for the specified object
    if len(sys.argv) > 1 and sys.argv[1].startswith("obj_id="):
        try:
            obj_id = int(sys.argv[1].split("=")[1])
            show_obj_info(obj_id)
        except (IndexError, ValueError) as e:
            print(f"Error: {e}")
    
    # Display all object information
    print("\nAll test objects:")
    for i, obj in enumerate(objects):
        print(f"{i}: {type(obj).__name__} @ {hex(id(obj))}")
    
    # Sleep to keep the process running
    print("\nRunning until terminated with Ctrl+C...")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nTerminating")
