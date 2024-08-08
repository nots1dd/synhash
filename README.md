# Syntax Highlighting Tool for LiteFM

## Overview

This project adds syntax highlighting functionality to LiteFM, a terminal-based file manager. The highlighting system is implemented in C using hash tables to efficiently store and manage syntax rules, which are defined in YAML files. Currently, the tool supports C and Java languages, but it is designed to be easily extendable to other languages by adding new YAML configuration files.

## How It Works

### Syntax Highlighting

The syntax highlighting engine uses hash tables to map syntax elements like keywords, functions, operators, and symbols to specific styles (like colors). The key-value pairs in the hash table consist of the syntax elements (e.g., keywords) as keys and their corresponding style codes (e.g., ANSI escape codes for colors) as values.

#### YAML Configuration

Each programming language is represented by a YAML file, which contains lists of keywords, functions, operators, symbols, and comment delimiters. The YAML file is parsed at runtime, and the elements are loaded into the hash tables.

##### YAML FILE Structure

> [!NOTE]
> 
> This structure and implementation may NOT seem intuitive for the following params:
> 
> 1. multicomments1/2 
> 2. symbols (not that often)
> 
> Kindly test out the new YAML file with synhash and modify these fields accordingly

```yaml
keywords:
 - ...

functions:
 - ...

operators:
 - ...

symbols:
 - ...

singlecomments:
 - ...

multicomments1:
 - ...

multicomments2:
 - ...

strings:
 - ...

```

Take a look at `python.yaml`, `java.yaml` and `c.yaml` for a better understanding of how **Object Oriented** and **Structured Oriented** languages are being highlighted

### Code Structure

The project has a modular structure:

- **src/**: Contains the source code.
  - **hashtable.c**: Implements the hash table for storing syntax elements.
  - **syntax.c**: Contains functions to parse the YAML files and apply syntax highlighting.
  - **yaml-parser.c**: The entry point for the program, handling file loading and initialization.
  
- **include/**: Contains the header files.
  - **hashtable.h**: Header for the hash table implementation.
  - **syntax.h**: Header for syntax highlighting functions.

- **yaml/**: Contains YAML files defining syntax elements for different languages.
  - **c.yaml**
  - **java.yaml**
  - **python.yaml**

### Building the Project

You can build the project using either `make` or `CMake`. Both methods are supported.

#### Building with Make

1. Navigate to the project directory.
2. Run `make` to compile the project.

The `Makefile` compiles all `.c` files in the `src/` directory and outputs the executable.

#### Building with CMake

1. Navigate to the project directory.
2. Create a build directory: `mkdir build && cd build`
3. Run `cmake ..` to generate the build files.
4. Run `make` to compile the project.

Alternate CMAKE method (**RECOMMENDED**)

1. Navigate to the project directory.
2. Run `cmake -S . -B build/` to link all libraries and headers
3. Run `cmake --build build/` to get a `./build/syntax_highlighter` executable
4. Simply run the executable

The CMake setup allows for more flexible project management and easier integration with IDEs.

### Future Plans

- **Integration with LiteFM**: The primary goal is to integrate this syntax highlighting engine into LiteFM, enhancing its file preview capabilities.
- **Support for More Languages**: Adding support for more programming languages by creating additional YAML configuration files.
- **Customization**: Allowing users to customize syntax highlighting rules directly from LiteFM.
- **Performance Optimization**: Further optimize the hash table implementation to handle larger files and more complex syntax rules efficiently.

### Known Drawbacks

- **Limited Language Support**: Currently, only a few basic YAML files have been created, this is where you come in. If you are interested, look at CONTRIBUTION.md
- **YAML Parsing Overhead**: Parsing YAML files at runtime introduces some overhead, though this is minimized by using hash tables.
- **Terminal Dependency**: The tool relies on ANSI escape codes, so it may not render correctly in all terminal environments.

## Conclusion

This syntax highlighting tool is a powerful addition to LiteFM, providing clear visual distinctions in code files. While there are some limitations, such as language support and terminal dependency, the framework is solid and easily extendable. Future updates will focus on expanding language support and improving performance, making it a versatile tool for developers.

---

Thank you for using this tool! Contributions, suggestions, and bug reports are always welcome.
