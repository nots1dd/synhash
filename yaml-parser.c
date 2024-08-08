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
void load_syntax(const char *path, HashTable *keywords, HashTable *singlecomments, HashTable *multicomments1, HashTable *multicomments2, HashTable *strings, HashTable *functions, HashTable *symbols, HashTable *operators) {
    FILE *fh = fopen(path, "r");
    yaml_parser_t parser;
    yaml_event_t event;
    int in_keywords = 0;
    int in_singlecomments = 0;
    int in_multcomment1 = 0;
    int in_multcomment2 = 0;
    int in_strings = 0;
    int in_functions = 0;
    int in_symbols = 0;
    int in_operators = 0;

    // Initialize parser
    if (!yaml_parser_initialize(&parser)) {
        fputs("Failed to initialize parser!\n", stderr);
        exit(EXIT_FAILURE);
    }
    if (fh == NULL) {
        fputs("Failed to open file!\n", stderr);
        exit(EXIT_FAILURE);
    }

    // Set input file
    yaml_parser_set_input_file(&parser, fh);

    do {
        if (!yaml_parser_parse(&parser, &event)) {
            printf("Parser error %d\n", parser.error);
            exit(EXIT_FAILURE);
        }

        switch (event.type) {
            case YAML_NO_EVENT:
                puts("No event!");
                break;
            case YAML_STREAM_START_EVENT:
                break;
            case YAML_STREAM_END_EVENT:
                break;
            case YAML_DOCUMENT_START_EVENT:
                break;
            case YAML_DOCUMENT_END_EVENT:
                break;
            case YAML_SEQUENCE_START_EVENT:
                break;
            case YAML_SEQUENCE_END_EVENT:
                in_keywords = 0;
                in_singlecomments = 0;
                in_strings = 0;
                in_functions = 0;
                in_symbols = 0;
                in_operators = 0;
                in_multcomment1 = 0;
                in_multcomment2 = 0;
                break;
            case YAML_MAPPING_START_EVENT:
                break;
            case YAML_MAPPING_END_EVENT:
                in_singlecomments = 0;
                break;
            case YAML_ALIAS_EVENT:
                printf("Got alias (anchor %s)\n", event.data.alias.anchor);
                break;
            case YAML_SCALAR_EVENT:
                if (in_keywords) {
                    insert(keywords, (char *)event.data.scalar.value);
                } else if (in_singlecomments) {
                    insert(singlecomments, (char *)event.data.scalar.value);
                } else if (in_multcomment1) {
                    multicomments1_length += strlen((char *)event.data.scalar.value);
                    insert(multicomments1, (char *)event.data.scalar.value);
                } else if (in_multcomment2) {
                    multicomments2_length += strlen((char *)event.data.scalar.value);
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
                    }
                }
                break;
        }

        if (event.type != YAML_STREAM_END_EVENT)
            yaml_event_delete(&event);

    } while (event.type != YAML_STREAM_END_EVENT);

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);
    fclose(fh); 
}

void colorLine(WINDOW* win, int color_pair, int y, int x, char *buffer) {
  wattron(win, COLOR_PAIR(color_pair));
  mvwprintw(win, y, x, "%s", buffer);
  wattroff(win, COLOR_PAIR(color_pair));
}

// Function to highlight code snippet
void highlight_code(WINDOW *win, int start_y, int start_x, const char *code, HashTable *keywords, HashTable *singlecomments, HashTable *multicomments1, HashTable *multicomments2, HashTable *strings, HashTable *functions, HashTable *symbols, HashTable *operators) {
    const char *cursor = code;
    char buffer[256];
    int in_string = 0;
    int in_comment = 0; // Single-line comment
    int in_multiline_comment = 0; // Multi-line comment
    int buffer_index = 0;
    int x = start_x, y = start_y;

    while (*cursor != '\0') {
        if (hash_table_contains(strings, cursor)) {
            in_string = !in_string;
            if (in_string && buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(strings, buffer)) {
                    colorLine(win, 3, y, x, buffer);
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            colorLine(win, 3, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;
        } else if (hash_table_contains(singlecomments, cursor) && hash_table_contains(singlecomments, cursor + 1)) {
            in_comment = 1;
            cursor++;
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(keywords, buffer)) {
                    colorLine(win, 4, y, x, buffer);
                } else if (search(symbols, buffer)) {
                    colorLine(win, 6, y, x, buffer);
                } else if (search(functions, buffer)) {
                    colorLine(win, 2, y, x, buffer);
                } else if (search(operators, buffer)) {
                    colorLine(win, 5, y, x, buffer);
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            colorLine(win, 1, y, x, (char[]){ *cursor, '\0' });
            x++;
        } else if (hash_table_contains(multicomments1, cursor) && hash_table_contains(multicomments2, cursor + 1)) {
            in_multiline_comment = 1;
            cursor++;
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(keywords, buffer)) {
                    colorLine(win, 4, y, x, buffer);
                } else if (search(symbols, buffer)) {
                    colorLine(win, 6, y, x, buffer);
                } else if (search(functions, buffer)) {
                    colorLine(win, 2, y, x, buffer);
                } else if (search(operators, buffer)) {
                    colorLine(win, 5, y, x, buffer);
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            colorLine(win, 1, y, x, (char[]){ *(cursor-1), *cursor, '\0' });
            x += 2;
            cursor++;
        } else if (in_string) {
            colorLine(win, 3, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;
        } else if (in_comment || in_multiline_comment) {
            colorLine(win, 1, y, x, (char[]){ *cursor, '\0' });
            if (*cursor == '\n' && in_comment) {
                in_comment = 0;
            } else if (hash_table_contains(multicomments2, cursor) && hash_table_contains(multicomments1, cursor + 1)) { // End of multi-line comment
                cursor++;
                mvwprintw(win, y, x + 1, "%c", *cursor);
                wattroff(win, COLOR_PAIR(1));
                x++;
                in_multiline_comment = 0;
            }
            x++;
            cursor++;
        } else if (isspace(*cursor)) {
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(keywords, buffer)) {
                    colorLine(win, 4, y, x, buffer);
                } else if (search(symbols, buffer)) {
                    colorLine(win, 6, y, x, buffer);
                } else if (search(functions, buffer)) {
                    colorLine(win, 2, y, x, buffer);
                } else if (search(operators, buffer)) {
                    colorLine(win, 5, y, x, buffer);
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            mvwprintw(win, y, x, "%c", *cursor);
            if (*cursor == '\n') {
                x = start_x;
                in_comment = 0;
                y++;
            } else {
                x++;
            }
            cursor++;
        } else if (hash_table_contains(symbols, cursor)) {
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(strings, buffer)) {
                    colorLine(win, 3, y, x, buffer);
                } else if (search(keywords, buffer)) {
                    colorLine(win, 4, y, x, buffer);
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            colorLine(win, 6, y, x, (char[]){ *cursor, '\0' });
            x++;
            cursor++;
        } else {
            buffer[buffer_index++] = *cursor;
            cursor++;
        }
    }

    if (buffer_index > 0) {
        buffer[buffer_index] = '\0';
        if (search(keywords, buffer)) {
            colorLine(win, 4, y, x, buffer);
        } else if (search(symbols, buffer)) {
            colorLine(win, 6, y, x, buffer);
        } else if (search(functions, buffer)) {
            colorLine(win, 2, y, x, buffer);
        } else if (search(operators, buffer)) {
            colorLine(win, 5, y, x, buffer);
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
    load_syntax("java.yaml", keywords, singlecomments, multicomments1, multicomments2, strings, functions, symbols, operators);

    // Initialize ncurses
    initscr();
    start_color();
    use_default_colors();
    if (can_change_color()) {
        // Normalize RGB values to the range 0-1000
        init_color(COLOR_CYAN, 70 * 1000 / 255, 70 * 1000 / 255, 70 * 1000 / 255);
    }

    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(4, COLOR_BLUE, -1);
    init_pair(5, COLOR_MAGENTA, -1);
    init_pair(6, 175, 235); // Use the custom color for the foreground
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
    highlight_code(win, 1, 1, java_code, keywords, singlecomments, multicomments1, multicomments2, strings, functions, symbols, operators);
    wrefresh(win);

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
