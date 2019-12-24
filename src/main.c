#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "lists.h"
#include "hashmap.h"

#define BUFSIZE 1024

struct TestSuite {
    char *init;
    char *clean;
    char *setUp;
    char *tearDown;
    struct HashMap tests;
};

void init_test_suite(struct TestSuite *suite) {
    suite->init = NULL;
    suite->clean = NULL;
    suite->setUp = NULL;
    suite->tearDown = NULL;
    HM_init(&suite->tests, NULL, 0);
}

void free_test_suite(struct TestSuite *suite) {
    HM_free(&suite->tests);
}

#define INIT 0
#define CLEAN 1
#define CASE 2
#define SETUP 3
#define TEARDOWN 4

#define BEFORE_COMMAND 0
#define IN_COMMAND_NAME 1
#define AFTER_COMMAND_NAME 2
#define IN_COMMAND_PARAMS 3
#define IN_INT 4
#define IN_STRING 5
#define AFTER_COMMAND 6
#define IN_VOID 7
#define AFTER_VOID  8
#define IN_FNAME 9
#define AFTER_FNAME 10
#define END_OF_STATEMENT 11


void parse_file(FILE *f, const char *filename, struct HashMap *suites) {
    // TODO handle before command for a__test_init__ not to trigger
    char buff[BUFSIZE];
    char pref[] = "__test_";
    char *macros[] = {
        "__test_init__",
        "__test_clean__",
        "__test_case__",
        "__test_set_up__",
        "__test_tear_down__",
    };
    char escaped = 0, pos, command;
    size_t line = 1, col = 1;
    struct Queue *params = NULL;
    struct TestSuite *suite_data = NULL;
    size_t i, j, param_length = 1024, fname_length = 1024, n_brackets;
    char *fname = NULL, *suite_name = NULL, *test_name = NULL, *param = NULL;

    pos = BEFORE_COMMAND;
    while (fgets(buff, BUFSIZE, f) != NULL) {
        for (i = 0; i < BUFSIZE && buff[i] != '\0'; ++i) {
            if (buff[i] == '\n') {
                line += 1;
                col = 1;
            }
            else {
                col += 1;
            }

            switch (pos) {

                case BEFORE_COMMAND:
                    switch (buff[i]) {
                        case ' ':
                        case '\t':
                        case '\n':
                        case '\r':
                            break;
                        case '_':
                            pos = IN_COMMAND_NAME;
                            j = 1;
                            break;
                    }
                    break;

                case IN_COMMAND_NAME:
                    if (j < 7) {
                        if (buff [i] == pref[j]) {
                            j += 1;
                        }
                        else {
                            goto eos_err;
                        }
                    }
                    else if (j == 7) {
                        switch (buff[i]) {
                            case 'i':
                                command = INIT;
                                break;
                            case 'c':
                                command = CLEAN;
                                break;
                            case 's':
                                command = SETUP;
                                break;
                            case 't':
                                command = TEARDOWN;
                                break;
                            default:
                                goto eos_err;
                                break;
                        }
                        j += 1;
                    }
                    else if (command == CLEAN && j == 8){
                        switch (buff[i]) {
                            case 'l':
                                command = CLEAN;
                                break;
                            case 'a':
                                command = CASE;
                                break;
                            default:
                                goto eos_err;
                                break;
                        }
                        j += 1;
                    }
                    else {
                        if (macros[command][j] == '\0') {
                            if (buff[i] == '(')
                                goto after_comm;
                            else
                                pos = AFTER_COMMAND_NAME;
                        }
                        else if (buff[i] == macros[command][j])
                            j += 1;
                        else {
                            goto eos_err;
                        }
                    }
                    break;

                case AFTER_COMMAND_NAME:
                    switch (buff[i]) {
                        case ' ':
                        case '\t':
                        case '\n':
                        case '\r':
                            break;
                        case '(':
after_comm:
                            pos = IN_COMMAND_PARAMS;
                            j = 0;
                            param = malloc(param_length * sizeof(char));
                            Queue_push(&params, param);
                            break;
                        default:
                            goto eos_err;
                            break;
                    }
                    break;

                case IN_COMMAND_PARAMS:
                    switch (buff[i]) {
                        case ' ':
                        case '\t':
                        case '\n':
                        case '\r':
                            break;
                            // beginning of param
                        case '\"':
                            pos = IN_STRING;
                            break;
                        case ')':
                            pos = AFTER_COMMAND;
                        case ',':
                            param[j] = '\0';
                            j = 0;
                            param = malloc(param_length * sizeof(char));
                            Queue_push(&params, param);
                            break;
                        default:
                            goto eos_err;
                            break;
                    }
                    break;

                case IN_STRING:
                    // make space if needed
                    if (j == param_length) {
                        param_length = param_length < 1;
                        params->val = realloc(param, param_length * sizeof(char));
                    }
                    if (escaped == 0 && buff[i] == '\\') {
                        escaped = 1;
                        param[j] = buff[i];
                        j += 1;
                    }
                    // if we have reached the end of the string
                    else if (escaped == 0 && buff[i] == '\"') {
                        pos = IN_COMMAND_PARAMS;
                    }
                    else {
                        param[j] = buff[i];
                        j += 1;
                    }
                    break;

                case AFTER_COMMAND:
                    switch (buff[i]) {
                        case ' ':
                        case '\t':
                        case '\n':
                        case '\r':
                            break;
                        case 'v':
                            pos = IN_VOID;
                            j = 1;
                            break;
                        case 'i':
                            pos = IN_INT;
                            j = 1;
                            break;
                        default:
                            goto eos_err;
                    }
                    break;

                case IN_VOID:
                    if (j == 4)
                        pos = AFTER_VOID;
                    else if (buff[i] == "void"[j])
                        j += 1;
                    break;

                case IN_INT:
                    if (j == 3)
                        pos = AFTER_VOID;
                    else if (buff[i] == "int"[j])
                        j += 1;
                    break;

                case AFTER_VOID :
                    if (('A' <= buff[i] && buff[i] <= 'Z') || buff[i] == '_' || ('a' <= buff[i] && buff[i] <= 'z')) {
                        pos = IN_FNAME;
                        fname = malloc(fname_length * sizeof(char));
                        fname[0] = buff[i];
                        j = 1;
                    }
                    else if (buff[i] == ' ' || buff[i] == '\t' || buff[i] == '\n' || buff[i] == '\r') {}
                    else
                        goto eos_err;
                    break;

                case IN_FNAME:
                    if (('A' <= buff[i] && buff[i] <= 'Z') || buff[i] == '_' || ('a' <= buff[i] && buff[i] <= 'z') || ('0' <= buff[i] && buff[i] <= '9')) {
                        pos = IN_FNAME;
                        fname[j] = buff[i];
                        j += 1;
                        if (j > fname_length) {
                            fname_length = fname_length < 1;
                            fname = realloc(fname, fname_length * sizeof(char));
                        }
                    }
                    else {
                        fname[j] = '\0';
                        pos = AFTER_FNAME;
                        n_brackets = 0;
                    }
                    break;

                case AFTER_FNAME:
                    if (buff[i] == ';' && n_brackets == 0) {
                        goto eos;
                    }
                    if (buff[i] == '{') {
                        n_brackets += 1;
                    }
                    if (buff[i] == '}') {
                        n_brackets -= 1;
                        if (n_brackets == 0)
                            goto eos;
                    }
                    break;

                case END_OF_STATEMENT:
eos:
                    suite_name = Queue_pop(&params);
                    if (!HM_is_in(suites, suite_name)) {
                        suite_data = malloc(sizeof(struct TestSuite));
                        init_test_suite(suite_data);
                        HM_set(suites, suite_name, suite_data);
                    }
                    else {
                        suite_data = HM_get(suites, suite_name);
                    }
                    switch (command) {
                        case INIT:
                            suite_data->init = fname;
                            break;
                        case CLEAN:
                            suite_data->clean = fname;
                            break;
                        case SETUP:
                            suite_data->setUp = fname;
                            break;
                        case TEARDOWN:
                            suite_data->tearDown = fname;
                            break;
                        case CASE:
                            test_name = Queue_pop(&params);
                            if (!HM_is_in(&suite_data->tests, test_name)) {
                                HM_set(&suite_data->tests, test_name, fname);
                            }
                            else {
                                fprintf(stderr, "%s:%d Error, test name \"%s\"", fname, line, test_name);
                            }
                            break;
                    }
                    fname = NULL;
eos_err:
                    j = 0;
                    while (params != NULL) {
                        free(Queue_pop(&params));
                    }
                    if (fname != NULL) {
                        free(fname);
                        fname = NULL;
                    }
                    pos = BEFORE_COMMAND;
                    break;
            }
        }
    }
}
#undef INIT
#undef CLEAN
#undef SETUP
#undef TEARDOWN
#undef CASE

#undef BEFORE_COMMAND
#undef IN_COMMAND_NAME
#undef AFTER_COMMAND_NAME
#undef IN_COMMAND_PARAMS
#undef IN_INT
#undef IN_STRING
#undef AFTER_COMMAND
#undef IN_VOID
#undef AFTER_VOID
#undef IN_FNAME
#undef AFTER_FNAME
#undef END_OF_STATEMENT

#undef ESCAPE

void print_summary(FILE *f, struct HashMap *suites) {
    size_t i, j;
    struct TestSuite *suite = NULL;
    char *buff = NULL;
    struct _hashmapelem *it = NULL, *itt = NULL;

    fprintf(stdout, "void create_suites(void) {\n");
    fprintf(stdout, "    CU_pSuite pSuite = NULL;\n\n");

    for (i = 0; i < suites->n_buckets; ++i) {
        for (it = suites->buckets[i]; it != NULL; it = it->next) {
            suite = it->val;
            fprintf(stdout, "    pSuite = CU_add_suite_with_setup_and_teardown(\"%s\", ", it->name);
            fprintf(stdout, "%s, ", (suite->init == NULL) ? "NULL" : suite->init);
            fprintf(stdout, "%s, ", (suite->clean == NULL) ? "NULL" : suite->clean);
            fprintf(stdout, "%s, ", (suite->setUp == NULL) ? "NULL" : suite->setUp);
            fprintf(stdout, "%s);\n", (suite->tearDown == NULL) ? "NULL" : suite->tearDown);

            for (j = 0; j < suite->tests.n_buckets; ++j) {
                for (itt = suite->tests.buckets[j]; itt != NULL; itt = itt->next) {
                    buff = itt->val;
                    fprintf(stdout, "    CU_add_test(pSuite, \"%s\", %s);\n", itt->name, buff);
                }
            }
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "}\n");
}

int main(int argc, char *argv[]) {
    // TODO handle command line arugments
    fd_set set;
    struct timeval timeout = {0, 0};
    FILE *f;
    struct HashMap suites;
    struct Queue *files = NULL;
    char *buf;

    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    HM_init(&suites, NULL, 0);

    // reading input pipe
    if (select(FD_SETSIZE, &set, NULL, NULL, &timeout) != 0) {
        parse_file(stdin, "stdin", &suites);
    }

    char i;
    for (i = 1; i < argc; ++i) {
        f = fopen(argv[i], "r");
        parse_file(f, argv[i], &suites);
        fclose(f);
        Queue_push(&files, argv[i]);
    }

    while ((buf = Queue_pop(&files)) != NULL) {
        fprintf(stdout, "#include \"%s\"\n", buf);
    }
    fprintf(stdout, "\n");

    print_summary(stdout, &suites);

    return EXIT_SUCCESS;
}
