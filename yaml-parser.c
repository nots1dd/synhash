#include <stdio.h>
#include <yaml.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <unistd.h>

#define TABLE_SIZE 1000

// Define hash table entry structure
typedef struct Entry {
    char *key;
    struct Entry *next;
} Entry;

// Define hash table structure
typedef struct {
    Entry *entries[TABLE_SIZE];
} HashTable;

// Function prototypes
unsigned int hash(const char *key);
HashTable* create_table();
void free_table(HashTable *table);
void insert(HashTable *table, const char *key);
int search(HashTable *table, const char *key);
void load_syntax(HashTable *keywords, HashTable *singlecomments, HashTable *multicomments1, HashTable *multicomments2, HashTable *strings, HashTable *functions, HashTable *symbols, HashTable *operators);
void highlight_code(WINDOW *win, int start_y, int start_x, const char *code, HashTable *keywords, HashTable *singlecomments, HashTable *multicomments1, HashTable *multicomments2, HashTable *strings, HashTable *functions, HashTable *symbols, HashTable *operators);

// Hash function
unsigned int hash(const char *key) {
    unsigned long int value = 0;
    unsigned int i = 0;
    unsigned int key_len = strlen(key);

    // Perform bitwise operations
    for (; i < key_len; ++i) {
        value = value * 37 + key[i];
    }

    value = value % TABLE_SIZE;

    return value;
}

// Create a new hash table
HashTable* create_table() {
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));

    for (int i = 0; i < TABLE_SIZE; ++i) {
        table->entries[i] = NULL;
    }

    return table;
}

// Free the hash table
void free_table(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        Entry *entry = table->entries[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp);
        }
    }
    free(table);
}

// Insert a key into the hash table
void insert(HashTable *table, const char *key) {
    unsigned int slot = hash(key);

    Entry *entry = table->entries[slot];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return; // Key already exists
        }
        entry = entry->next;
    }

    Entry *new_entry = (Entry *)malloc(sizeof(Entry));
    new_entry->key = strdup(key);
    new_entry->next = table->entries[slot];
    table->entries[slot] = new_entry;
}

// Search for a key in the hash table
int search(HashTable *table, const char *key) {
    unsigned int slot = hash(key);

    Entry *entry = table->entries[slot];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return 1; // Key found
        }
        entry = entry->next;
    }

    return 0; // Key not found
}

// Load syntax from YAML file
void load_syntax(HashTable *keywords, HashTable *singlecomments, HashTable *multicomments1, HashTable *multicomments2, HashTable *strings, HashTable *functions, HashTable *symbols, HashTable *operators) {
    FILE *fh = fopen("syntax.yaml", "r");
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

int hash_table_contains(HashTable *table, const char *key) {
    char key_str[2] = {*key, '\0'};
    unsigned int slot = hash(key_str);
    Entry *entry = table->entries[slot];

    while (entry != NULL) {
        if (strcmp(entry->key, key_str) == 0) {
            return 1;
        }
        entry = entry->next;
    }
    return 0;
}

int is_two_char_sequence_in_table(HashTable *table, const char *cursor) {
    char two_char_sequence[2]; // 2 characters + null terminator
    two_char_sequence[0] = *cursor;
    two_char_sequence[1] = *(cursor + 1);

    return hash_table_contains(table, two_char_sequence);
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
                    wattron(win, COLOR_PAIR(3));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(3));
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            wattron(win, COLOR_PAIR(3));
            mvwprintw(win, y, x, "%c", *cursor);
            wattroff(win, COLOR_PAIR(3));
            x++;
            cursor++;
        } else if (hash_table_contains(singlecomments, cursor) && hash_table_contains(singlecomments, cursor+1)) {
            in_comment = 1;
            cursor++;
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(keywords, buffer)) {
                    wattron(win, COLOR_PAIR(4));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(4));
                } else if (search(symbols, buffer)) {
                    wattron(win, COLOR_PAIR(6));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(6));
                } else if (search(functions, buffer)) {
                    wattron(win, COLOR_PAIR(2));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(2));
                } else if (search(operators, buffer)) {
                    wattron(win, COLOR_PAIR(5));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(5));
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, y, x, "%c", *cursor);
            x++;
        } else if (hash_table_contains(multicomments1, cursor) && hash_table_contains(multicomments2, cursor+1)) {
            in_multiline_comment = 1;
            cursor++;
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                if (search(keywords, buffer)) {
                    wattron(win, COLOR_PAIR(4));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(4));
                } else if (search(symbols, buffer)) {
                    wattron(win, COLOR_PAIR(6));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(6));
                } else if (search(functions, buffer)) {
                    wattron(win, COLOR_PAIR(2));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(2));
                } else if (search(operators, buffer)) {
                    wattron(win, COLOR_PAIR(5));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(5));
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, y, x, "%c%c", *(cursor-1), *cursor);
            x += 2;
            cursor++;
        } else if (in_string) {
            wattron(win, COLOR_PAIR(3));
            mvwprintw(win, y, x, "%c", *cursor);
            wattroff(win, COLOR_PAIR(3));
            x++;
            cursor++;
        } else if (in_comment || in_multiline_comment) {
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, y, x, "%c", *cursor);
            if (*cursor == '\n' && in_comment) {
                in_comment = 0;
            } else if (hash_table_contains(multicomments2, cursor) && hash_table_contains(multicomments1, cursor+1)) { // implementation of /* */ (See syntax.yaml for more)
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
                    wattron(win, COLOR_PAIR(4));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(4));
                } else if (search(symbols, buffer)) {
                    wattron(win, COLOR_PAIR(6));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(6));
                } else if (search(functions, buffer)) {
                    wattron(win, COLOR_PAIR(2));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(2));
                } else if (search(operators, buffer)) {
                    wattron(win, COLOR_PAIR(5));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(5));
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
                    wattron(win, COLOR_PAIR(3));
                    mvwprintw(win, y, x, "%s", buffer);
                    wattroff(win, COLOR_PAIR(3));
                } else {
                    mvwprintw(win, y, x, "%s", buffer);
                }
                x += buffer_index;
                buffer_index = 0;
            }
            wattron(win, COLOR_PAIR(6));
            mvwprintw(win, y, x, "%c", *cursor);
            wattroff(win, COLOR_PAIR(6));
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
            wattron(win, COLOR_PAIR(4));
            mvwprintw(win, y, x, "%s", buffer);
            wattroff(win, COLOR_PAIR(4));
        } else if (search(symbols, buffer)) {
            wattron(win, COLOR_PAIR(6));
            mvwprintw(win, y, x, "%s", buffer);
            wattroff(win, COLOR_PAIR(6));
        } else if (search(functions, buffer)) {
            wattron(win, COLOR_PAIR(2));
            mvwprintw(win, y, x, "%s", buffer);
            wattroff(win, COLOR_PAIR(2));
        } else if (search(operators, buffer)) {
            wattron(win, COLOR_PAIR(5));
            mvwprintw(win, y, x, "%s", buffer);
            wattroff(win, COLOR_PAIR(5));
        } else {
            mvwprintw(win, y, x, "%s", buffer);
        }
    }
}

void print_table(HashTable *table) {
    for (int i = 0; i < 1000; ++i) {
        Entry *entry = table->entries[i];
        if (entry != NULL) {
            while (entry != NULL) {
                printf("  %s\n", entry->key);
                entry = entry->next;
            }
        }
    }
    printf("\n");
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
    load_syntax(keywords, singlecomments, multicomments1, multicomments2, strings, functions, symbols, operators);

    // Initialize ncurses
    initscr();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    cbreak();
    refresh();

    // Create a new window for displaying highlighted code
    WINDOW *win = newwin(LINES, COLS, 0, 0);
    box(win, 0, 0);
    wrefresh(win);

    // Example code to highlight
    const char *code = "#include <stdio.h> /* yoo does this comment exist? */ int main() {   printf(\"Hello, World!\"); const int y.x = 100; return 0; } //hello \n";
    const char *pycode = "def example_func(x,y): \"\"\"This is some func\"\"\" if x > 0 and y < 0: return True else: return False";

    // Highlight the code
    highlight_code(win, 1, 1, code, keywords, singlecomments, multicomments1, multicomments2, strings, functions, symbols, operators);
    wrefresh(win);

    // Wait for user input
    sleep(3);

    // Clean up
    delwin(win);
    endwin();
    print_table(keywords);
    print_table(singlecomments);
    print_table(multicomments1);
    print_table(multicomments2);
    print_table(strings);
    print_table(functions);
    print_table(symbols);
    free_table(keywords);
    free_table(singlecomments);
    free_table(strings);
    free_table(functions);
    free_table(symbols);
    free_table(operators);

    return 0;
}
