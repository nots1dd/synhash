#include <stdio.h>
#include <yaml.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <unistd.h>
#include "include/hashtable.h"

/*#define TABLE_SIZE 1000*/
int multicomments1_length = 0;
int multicomments2_length = 0;

// Load syntax from YAML file
bool load_syntax(const char *path, HashTable *keywords, HashTable *singlecomments, HashTable *multicomments1, HashTable *multicomments2, HashTable *strings, HashTable *functions, HashTable *symbols, HashTable *operators, int *singlecommentslen) {
    FILE *fh = fopen(path, "r");
    yaml_parser_t parser;
    yaml_event_t event;
    bool success = true;

    int in_keywords = 0;
    int in_singlecomments = 0;
    int in_multcomment1 = 0;
    int in_multcomment2 = 0;
    int in_strings = 0;
    int in_functions = 0;
    int in_singlecommentslen = 0;
    int in_symbols = 0;
    int in_operators = 0;

    // Initialize parser
    if (!yaml_parser_initialize(&parser)) {
        fprintf(stderr, " [LIBYAML] Failed to initialize parser for synhash: %s\n", path);
        return false;
    }
    if (fh == NULL) {
        fprintf(stderr, " [LIBYAML] Failed to open file for synhash: %s\n", path);
        yaml_parser_delete(&parser);
        return false;
    }

    // Set input file
    yaml_parser_set_input_file(&parser, fh);

    do {
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, " [LIBYAML] Parser error %d in file: %s\n", parser.error, path);
            success = false;
            break;
        }

        switch (event.type) {
            case YAML_NO_EVENT:
                fprintf(stderr, " [LIBYAML] No event in file: %s\n", path);
                break;
            case YAML_STREAM_START_EVENT:
            case YAML_STREAM_END_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_MAPPING_START_EVENT:
            case YAML_MAPPING_END_EVENT:
            case YAML_SEQUENCE_START_EVENT:
            case YAML_ALIAS_EVENT:
                // Handle these cases as needed
                break;
            case YAML_SEQUENCE_END_EVENT:
                in_keywords = 0;
                in_singlecomments = 0;
                in_singlecommentslen = 0;
                in_strings = 0;
                in_functions = 0;
                in_symbols = 0;
                in_operators = 0;
                in_multcomment1 = 0;
                in_multcomment2 = 0;
                break;
            case YAML_SCALAR_EVENT:
                if (in_keywords) {
                    insert(keywords, (char *)event.data.scalar.value);
                } else if (in_singlecomments) {
                    insert(singlecomments, (char *)event.data.scalar.value);
                } else if (in_singlecommentslen) {
                    *singlecommentslen = atoi((char *)event.data.scalar.value);
                } else if (in_multcomment1) {
                    insert(multicomments1, (char *)event.data.scalar.value);
                } else if (in_multcomment2) {
                    insert(multicomments2, (char *)event.data.scalar.value);
                } else if (in_strings) {
                    insert(strings, (char *)event.data.scalar.value);
                } else if (in_functions) {
                    insert(functions, (char *)event.data.scalar.value);
                } else if (in_symbols) {
                    insert(symbols, (char *)event.data.scalar.value);
                } else if (in_operators) {
                    insert(operators, (char *)event.data.scalar.value);
                } else {
                    if (strcmp((char *)event.data.scalar.value, "keywords") == 0) {
                        in_keywords = 1;
                    } else if (strcmp((char *)event.data.scalar.value, "singlecomments") == 0) {
                        in_singlecomments = 1;
                    } else if (strcmp((char *)event.data.scalar.value, "multicomments1") == 0) {
                        in_multcomment1 = 1;
                    } else if (strcmp((char *)event.data.scalar.value, "multicomments2") == 0) {
                        in_multcomment2 = 1;
                    } else if (strcmp((char *)event.data.scalar.value, "strings") == 0) {
                        in_strings = 1;
                    } else if (strcmp((char *)event.data.scalar.value, "functions") == 0) {
                        in_functions = 1;
                    } else if (strcmp((char *)event.data.scalar.value, "symbols") == 0) {
                        in_symbols = 1;
                    } else if (strcmp((char *)event.data.scalar.value, "operators") == 0) {
                        in_operators = 1;
                    } else if (strcmp((char *)event.data.scalar.value, "singlecommentslen") == 0) {
                        in_singlecommentslen = 1;
                    }
                }
                break;
            default:
                fprintf(stderr, " [LIBYAML] Unhandled YAML event type: %d\n", event.type);
                break;
        }

        if (event.type != YAML_STREAM_END_EVENT)
            yaml_event_delete(&event);

    } while (event.type != YAML_STREAM_END_EVENT);

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(fh);

    if (success) {
        fprintf(stdout, " [LIBYAML] Successfully parsed file: %s\n", path);
    } else {
        fprintf(stderr, " [LIBYAML] Parsing failed for file: %s\n", path);
    }

    return success;
}


void highlightLine(WINDOW* win, int color_pair, int y, int x, char *buffer) {
  wattron(win, COLOR_PAIR(color_pair));
  mvwprintw(win, y, x, "%s", buffer);
  wattroff(win, COLOR_PAIR(color_pair));
}

// Function to highlight code snippet
void highlight_code(WINDOW *win, int start_y, int start_x, const char *code, HashTable *keywords, HashTable *singlecomments, HashTable *multicomments1, HashTable *multicomments2, HashTable *strings, HashTable *functions, HashTable *symbols, HashTable *operators, int *singlecommentslen) {
    const char *cursor = code;
    char buffer[256];
    int in_string = 0;
    int in_comment = 0;
    int in_multiline_comment = 0;
    int buffer_index = 0;
    int x = start_x, y = start_y;

    while (*cursor != '\0') {
        // Check for multiline comments first
        if (hash_table_contains(multicomments1, cursor) && hash_table_contains(multicomments2, cursor + 1)) {
            in_multiline_comment = 1;
            cursor++;
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                mvwprintw(win, y, x, "%s", buffer);
                x += buffer_index;
                buffer_index = 0;
            }
            highlightLine(win, 21, y, x, (char[]){ *(cursor-1), *cursor, '\0' });
            x += 2;
            cursor++;
        } else if (in_multiline_comment) {
            highlightLine(win, 21, y, x, (char[]){ *cursor, '\0' });
            if (hash_table_contains(multicomments2, cursor) && hash_table_contains(multicomments1, cursor + 1)) {
                highlightLine(win, 21, y, x + 1, (char[]){ *(cursor+1), '\0' });
                x++;
                cursor++;
                in_multiline_comment = 0;
            } else if (*cursor == '\n') {
                x = start_x-1;
                y++;
                in_comment = 0;
            }
            x++;
            cursor++;
        } 
        // Then check for strings
        else if (hash_table_contains(strings, cursor)) {
            in_string = !in_string;
            if (in_string && buffer_index > 0) {
                buffer[buffer_index] = '\0';
                mvwprintw(win, y, x, "%s", buffer);
                x += buffer_index;
                buffer_index = 0;
            }
            highlightLine(win, 22, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;
        } 
        // Then check for single line comments
        else if (hash_table_contains(singlecomments, cursor) && !in_string) {
            int comment_track = 1;

            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                mvwprintw(win, y, x, "%s", buffer);
                x += buffer_index;
                buffer_index = 0;
            }
            cursor++;
            char *tmp = cursor;

            // Check if the length of the comment matches singlecommentslen
            while (comment_track < *singlecommentslen && *cursor != '\0' && *cursor != ' ') {
                if (hash_table_contains(singlecomments, cursor)) {
                    comment_track++;
                    cursor++;
                    x++;
                } else {
                    break;
                }
            }

            if (comment_track == *singlecommentslen) {
                // Highlight the comment
                in_comment = 1;
                cursor = tmp;  // Reset to the beginning of the comment
                if (*singlecommentslen > 1) {
                  x--;
                }
                while (*cursor != '\0' && *cursor != '\n') {
                    highlightLine(win, 21, y, x, (char[]){ *(cursor-1), '\0' });
                    x++;
                    cursor++;
                }
                if (*cursor == '\n') {
                    // Handle newline character
                    highlightLine(win, 21, y, x, cursor-1);
                    x = start_x;
                    y++;
                    cursor++;
                    in_comment = 0;
                }
            } else {
                // Not a comment, so move the cursor back and skip the first character
                cursor = tmp;
                mvwprintw(win, y, x, "%c", *(cursor-1));
                x++;
            }
        } 
        // Handle ongoing string state
        else if (in_string) {
            highlightLine(win, 22, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;
        } 
        // Handle ongoing comment state
        else if (in_comment) {
            highlightLine(win, 21, y, x, (char[]){ *cursor, '\0' });
            if (*cursor == '\n') {
                x = start_x;
                y++;
                in_comment = 0;
            }
            x++;
            cursor++;
        } 
        // Handle the '(' character, which typically follows a function name
        else if (*cursor == '(') {
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                // Color code the function name with color pair 28
                highlightLine(win, 28, y, x, buffer);
                x+=buffer_index;
                buffer_index = 0;
            }

            // Highlight the '(' symbol with its corresponding color pair (if needed)
            highlightLine(win, 25, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;
        }
        // Handle dot notation like "System.out.println"
        else if (*cursor == '.') {
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(functions, buffer)) {
                    highlightLine(win, 26, y, x, buffer);
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }

            highlightLine(win, 25, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;

            // Handle function or method after the dot
            while (isalnum(*cursor) || *cursor == '_') {
                buffer[buffer_index++] = *cursor;
                cursor++;
            }
            buffer[buffer_index] = '\0';
            if (search(functions, buffer)) {
                highlightLine(win, 26, y, x, buffer);
            } else {
                mvwprintw(win, y, x, "%s", buffer);
            }
            x += buffer_index;
            buffer_index = 0;
        }

        // Handle whitespace
        else if (isspace(*cursor)) {
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(keywords, buffer)) {
                    highlightLine(win, 24, y, x, buffer);
                } else if (search(functions, buffer)) {
                    highlightLine(win, 26, y, x, buffer);
                } else if (search(symbols, buffer)) {
                    highlightLine(win, 25, y, x, buffer);
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            mvwprintw(win, y, x, "%c", *cursor);
            if (*cursor == '\n') {
                x = start_x;
                y++;
                in_comment = 0;
            } else {
                x++;
            }
            cursor++;
        } 
        // Handle numbers
        else if (isdigit(*cursor)) {
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                mvwprintw(win, y, x, "%s", buffer);
                x += buffer_index;
                buffer_index = 0;
            }
            highlightLine(win, 27, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;
        }
        else if (hash_table_contains(operators, cursor)) {
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                // Print buffer content before handling the operator
                if (search(keywords, buffer)) {
                    highlightLine(win, 24, y, x, buffer);
                } else if (search(functions, buffer)) {
                    highlightLine(win, 26, y, x, buffer);
                } else if (search(symbols, buffer)) {
                    highlightLine(win, 25, y, x, buffer);
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            // Highlight the operator
            highlightLine(win, 23, y, x, (char[]){ *cursor, '\0' });  // Using color pair 23 for operators
            x++;
            cursor++;
        }
        // Handle symbols
        else if (hash_table_contains(symbols, cursor) && *cursor != '(') {
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                mvwprintw(win, y, x, "%s", buffer);
                x += buffer_index;
                buffer_index = 0;
            }
            highlightLine(win, 25, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;
        } 
        // Continue building buffer
        else {
            buffer[buffer_index++] = *cursor;
            cursor++;
        }
    }

    // Print any remaining buffer content
    if (buffer_index > 0) {
        buffer[buffer_index] = '\0';
        if (search(keywords, buffer)) {
            highlightLine(win, 24, y, x, buffer);
        } else if (search(functions, buffer)) {
            highlightLine(win, 26, y, x, buffer);
        } else if (search(symbols, buffer)) {
            highlightLine(win, 25, y, x, buffer);
        } else {
            mvwprintw(win, y, x, "%s", buffer);
        }
    }
}

int main() {
    // Initialize hash tables
    HashTable *keywords = create_table();
    HashTable *singlecomments = create_table();
    HashTable *multicomments1 = create_table();
    HashTable *multicomments2 = create_table();
    HashTable *strings = create_table();
    HashTable *functions = create_table();
    HashTable *symbols = create_table();
    HashTable *operators = create_table();

    // Load syntax elements from YAML file
    int singlecommentslen = 0;
    bool syntaxLoad = load_syntax("java.yaml", keywords, singlecomments, multicomments1, multicomments2, strings, functions, symbols, operators, &singlecommentslen);

    // Initialize ncurses
    initscr();
    start_color();
    use_default_colors();
    if (can_change_color()) {
        // Normalize RGB values to the range 0-1000
        init_color(COLOR_CYAN, 70 * 1000 / 255, 70 * 1000 / 255, 70 * 1000 / 255);
    }

    init_pair(21, COLOR_CYAN, -1);
    init_pair(22, COLOR_GREEN, -1);
    init_pair(23, COLOR_YELLOW, -1);
    init_pair(24, COLOR_BLUE, -1);
    init_pair(25, COLOR_MAGENTA, -1);
    init_pair(26, 175, 235); // Use the custom color for the foreground
    init_pair(27, COLOR_RED, -1);
    init_pair(28, 108, 235);
    cbreak();
    refresh();

    // Create a new window for displaying highlighted code
    WINDOW *win = newwin(LINES, COLS, 0, 0);
    box(win, 0, 0);
    wrefresh(win);

    // Example code to highlight
    const char *code = "#include <stdio.h>\n /* yoo does this comment exist? */\n int main() {\n  printf(\"Hello, World!\");\n  const int y.x = 100/5-9.0+44.347-ok;\n  return 0;\n } //hello \n";
    const char *pycode = "import ** ## this is an import\n\ndef example_func(x,y):\n  \"\"\"This is some func\"\"\"\n  if x > 0 and y < 0:\n   return True\n else: \n   return False";

    const char* java_code = 
    "public class HelloWorld {\n"
    "    public static void main(String[] args) {\n"
    "        System.out.println(\"Hello, World!\");\n"
    "    }\n"
    "}";


    // Highlight the code
    if (syntaxLoad) {
      highlight_code(win, 1, 1, java_code, keywords, singlecomments, multicomments1, multicomments2, strings, functions, symbols, operators, &singlecommentslen);
      wrefresh(win);
    } else {
      mvwprintw(win, 3, 1, "NO YAML FILE FOUND!");
    } 

    // sleep for sometime (debugging step)
    sleep(3);

    // Clean up
    delwin(win);
    endwin();
    //printf("%d %d", multicomments1_length, multicomments2_length);
    free_table(keywords);
    free_table(singlecomments);
    free_table(strings);
    free_table(functions);
    free_table(symbols);
    free_table(operators);

    return 0;
}

