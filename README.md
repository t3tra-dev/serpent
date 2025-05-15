# (WIP) Project Serpent

**Serpent** is the world's premier Python debugger and malware analysis tool.

## Use Cases

1. security (forensic malware analysis)
    - Dynamically track Python-based malware and scripts exploited by attackers.
	- Observe the transition of PyObjects in memory and acquire evidence of malicious code injection.
	- Trace code generation by `eval`, `exec`, etc.

2. study the behavior of the Python interpreter
	- Visualizing the life cycle of objects in memory and tracing GC and reference counting behavior.
	- Observable interactions with C extensions, etc.

3. education and visualization tools
	- Real-time GUI visualization of Python object structure (e.g. `PyObject_HEAD`) can be used as an educational tool.

4. debugging support for Python applications
	- Trace memory leaks and object retention status, especially in long-running Python processes (servers, machine learning jobs, etc.).

## dir

```
serpent/
├── .gitignore
├── .vscode/
│   └── settings.json
├── CMakeLists.txt
├── FRAMEWORK.md
├── LOADMAP.md
├── README.md
├── core/
│   ├── CMakeLists.txt
│   ├── include/
│   │   └── serpent/
│   │       └── core/
│   │           ├── AbiFactory.h
│   │           ├── IProcessReader.h
│   │           ├── IPythonABI.h
│   │           └── ReaderFactory.h
│   └── src/
│       ├── AbiFactory.cpp
│       └── Readerfactory.cpp
├── plugins/
│   ├── CMakeLists.txt
│   ├── abi_cp310/
│   │   ├── CMakeLists.txt
│   │   └── Py310ABI.cpp
│   ├── reader_linux/
│   │   ├── CMakeLists.txt
│   │   └── LinuxReader.cpp
│   └── reader_macos/
│       ├── CMakeLists.txt
│       └── MacOSReader.cpp
├── test_object.py
└── tools/
    ├── CMakeLists.txt
    └── hello_serpent/
        ├── CMakeLists.txt
        └── main.cpp
```
