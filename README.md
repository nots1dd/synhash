# SYNHAS - Code Syntax Highlighting with NCurses

This project provides a terminal-based syntax highlighter for code snippets using the NCurses library in C. It supports syntax highlighting for different elements such as keywords, single-line comments, multi-line comments, strings, functions, symbols, and operators. The syntax rules are loaded from a YAML configuration file.

## PURPOSE 

This project is meant to be integrated with [LITEFM](https://github.com/nots1dd/litefm)

The goal is to create a **MODULAR** and **FAST** syntax highlighting system that can be used to preview files in LITEFM.

For this purpose, I use `hastables` to get the average time complexity of **O(1)** [space complexity is a bit bad tho]

`libyaml` is also a VERY fast YAML parsing library that works on async events rather than a standard parsing system.

All in all, this project can be made better but this has been tried and tested and it works really well to my surprise.

The below sections talk in depth about how this project is structured.

For more info on the integration with **LiteFM**, check out [this](https://github.com/nots1dd/litefm/blob/main/README.md)

## Overview

### Key Components

1. **YAML Parsing**: Loads syntax rules from a YAML file.
2. **Syntax Highlighting**: Applies syntax highlighting to code snippets.
3. **NCurses Interface**: Displays the highlighted code in a terminal window.

## Components

### `load_syntax`

- **Purpose**: Parses a YAML file to load syntax rules into hash tables.
- **Parameters**:
  - `path`: Path to the YAML file.
  - `keywords`, `singlecomments`, `multicomments1`, `multicomments2`, `strings`, `functions`, `symbols`, `operators`: Hash tables for different syntax elements.
  - `singlecommentslen`: Pointer to an integer representing the length of single-line comments.

### `highlightLine`

- **Purpose**: Highlights a line of text in a specific color.
- **Parameters**:
  - `win`: NCurses window to draw on.
  - `color_pair`: Color pair for highlighting.
  - `y`, `x`: Coordinates for the text.
  - `buffer`: The text to be highlighted.

### `highlight_code`

- **Purpose**: Applies syntax highlighting to a code snippet.
- **Parameters**:
  - `win`: NCurses window to draw on.
  - `start_y`, `start_x`: Starting coordinates for the code.
  - `code`: The code snippet to highlight.
  - `keywords`, `singlecomments`, `multicomments1`, `multicomments2`, `strings`, `functions`, `symbols`, `operators`: Hash tables with syntax elements.
  - `singlecommentslen`: Length of single-line comments.

### `main`

- **Purpose**: Initializes the program, loads syntax rules, and displays highlighted code.
- **Steps**:
  1. Initialize hash tables.
  2. Load syntax from the YAML file.
  3. Set up NCurses and color pairs.
  4. Highlight a sample code snippet.
  5. Clean up and exit.

## Detailed Explanation

### YAML File Structure

The YAML file should contain sections for different syntax elements:
- `keywords`: List of keywords.
- `singlecomments`: List of single-line comment indicators.
- `multicomments1`: Start indicators for multi-line comments.
- `multicomments2`: End indicators for multi-line comments.
- `strings`: List of string delimiters.
- `functions`: List of function names.
- `symbols`: List of symbols (e.g., `+`, `-`, `*`, `/`).
- `operators`: List of operators.
- `singlecommentslen`: Length of single-line comments.

### Syntax Highlighting Rules

- **Multiline Comments**: Highlighted based on the start and end indicators specified.
- **Strings**: Highlighted when delimiters are encountered.
- **Single-line Comments**: Highlighted if the length matches `singlecommentslen`.
- **Functions**: Highlighted if recognized in the code.
- **Symbols and Operators**: Highlighted based on hash table entries.

### NCurses Setup

- **Color Pairs**: Custom color pairs are defined for different syntax elements:
  - `21`: Multiline comments.
  - `22`: Strings.
  - `23`: Single-line comments.
  - `24`: Keywords.
  - `25`: Symbols.
  - `26`: Functions.
  - `27`: Numbers.

## Usage

1. **Compile the Program**: Ensure the necessary libraries (e.g., libyaml, ncurses) are installed.
2. **Run the Program**: Execute the compiled binary. It will read the YAML file, apply syntax highlighting to the code snippet, and display it in the terminal.

```sh
gcc -o syntax_highlighter syntax_highlighter.c -lyaml -lncurses
./syntax_highlighter
```

## Dependencies

- `libyaml`: For parsing YAML files. (`libyaml-dev` for debian)
- `ncurses`: For terminal-based UI and color handling. (`libncurses-dev` or `libncursesw5-dev` for debian)

## Notes

- Ensure the YAML file follows the correct structure.
- Adjust the color pairs and highlighting rules as needed.

## License

This project is licensed under the GNU GPL v3 License. See the `LICENSE` file for more details.
