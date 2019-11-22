/**
 * @file
 *
 * Unix utility that recursively searches for matching words in text files,
 * similar to the grep command (specifically with flags -Rnw). Only entire words
 * are matched, not partial words (i.e., searching for 'the' does not match
 * 'theme'). The line number where the matching search term was found is also
 * printed.
 *
 * The tool makes use of multiple threads running in parallel to perform the
 * search. Each file is searched by a separate thread.
 */

#define _GNU_SOURCE

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"

/* Preprocessor directives */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


/* Function prototypes */
void print_usage(char *argv[]);


/* Globals */
char *g_search_dir = NULL;
bool g_exact = false;
unsigned int g_num_terms = 0;
char **g_search_terms;

void print_usage(char *argv[])
{
    printf("Usage: %s [-eh] [-d directory] [-t threads] "
            "search_term1 search_term2 ... search_termN\n" , argv[0]);
    printf("\n");
    printf("Options:\n"
           "    * -d directory    specify start directory (default: CWD)\n"
           "    * -e              print exact name matches only\n"
           "    * -h              show usage information\n"
           "    * -t threads      set maximum threads (default: # procs)\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    int max_threads = 1;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "d:eht:")) != -1) {
        switch (c) {
            case 'd':
                g_search_dir = optarg;
                break;
            case 'e':
                g_exact = true;
                break;
            case 'h':
                print_usage(argv);
                return 0;
            case 't':
                max_threads = atoi(optarg);
                break;
            case '?':
                if (optopt == 'd' || optopt == 't') {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", optopt);
                }
                return 1;
            default:
                abort();
        }
    }

    g_num_terms = argc - optind;
    g_search_terms = &argv[optind];

    LOG("Searching '%s' recursively using %d threads\n",
           g_search_dir, max_threads);
    int i;
    for (i = 0; i < g_num_terms; ++i) {
        LOG("Search term: '%s'\n", g_search_terms[i]);
    }

    return 0;
}
