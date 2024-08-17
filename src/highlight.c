#include "../include/highlight.h"

extern int multicomments1_length;
extern int multicomments2_length;

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

    if (!yaml_parser_initialize(&parser)) {
        fputs("Failed to initialize parser!\n", stderr);
        exit(EXIT_FAILURE);
    }
    if (fh == NULL) {
        fputs("Failed to open file!\n", stderr);
        exit(EXIT_FAILURE);
    }

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
            case YAML_STREAM_END_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_DOCUMENT_END_EVENT:
            case YAML_SEQUENCE_START_EVENT:
            case YAML_SEQUENCE_END_EVENT:
            case YAML_MAPPING_START_EVENT:
            case YAML_MAPPING_END_EVENT:
            case YAML_ALIAS_EVENT:
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

// Function to highlight code snippet
void highlight_code(WINDOW *win, int start_y, int start_x, const char *code, HashTable *keywords, HashTable *singlecomments, HashTable *multicomments1, HashTable *multicomments2, HashTable *strings, HashTable *functions, HashTable *symbols, HashTable *operators) {
    const char *cursor = code;
    char buffer[256];
    int in_string = 0;
    int in_comment = 0;
    int in_multiline_comment = 0;
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
                buffer_index = 0;
            }
            wattron(win, COLOR_PAIR(2));
            mvwprintw(win, y, x, "%c", *cursor);
            wattroff(win, COLOR_PAIR(2));
        } else if (hash_table_contains(singlecomments, cursor) && !in_string) {
            in_comment = 1;
        } else if (in_comment || in_multiline_comment) {
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, y, x, "%c", *cursor);
            wattroff(win, COLOR_PAIR(1));
        } else {
            buffer[buffer_index++] = *cursor;
        }

        if (*cursor == '\n') {
            y++;
            x = start_x;
            if (in_comment) in_comment = 0;
        } else {
            x++;
        }
        cursor++;
    }

    if (buffer_index > 0) {
        buffer[buffer_index] = '\0';
        if (search(keywords, buffer)) {
            wattron(win, COLOR_PAIR(2));
            mvwprintw(win, y, x, "%s", buffer);
            wattroff(win, COLOR_PAIR(2));
        } else {
            mvwprintw(win, y, x, "%s", buffer);
        }
    }
}
