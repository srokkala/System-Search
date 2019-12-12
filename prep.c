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

/* Structures */
/**
 * This structure holds the arguments to the thread.
 */
typedef struct
{
    /* Path of the file to check */
    char file_path[PATH_MAX];
    
    /* Stats of the file */
    struct stat file_stat;
    
    /* Term to search */
    char *term;
} ThreadTask;


/* Function prototypes */
void print_usage(char *argv[]);
void run_search(int max_threads);

/* Globals */

/* Directory we should search in */
char *g_search_dir = NULL;

/* Does the case should match exactly? */
bool g_exact = false;

/* Count of search terms in the input */
unsigned int g_num_terms = 0;

/* Terms that will be searched */
char **g_search_terms;

/* Semaphore that is used for limiting the threads count */
sem_t g_threads_sem;

/* Mutex that is used for protecting the output */
pthread_mutex_t g_output_mutex;

/* Flag that shows if the threads done  */

/**
 * Prints the usage and help for this program.
 * @param argv - command line arguments.
 */
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

/**
 * Outputs the mode of the file into string buffer.
 * @param file_stat - properties of the file.
 * @param buffer - string buffer where the mode string goes.
 */
void output_mode(struct stat file_stat, char *buffer)
{
    strcpy(buffer, "----");
    
    /* Check if the user or group is the owner */
    if (file_stat.st_uid == getuid()) {
        /* Output user's rights */
        buffer[0] = 'o';
        if (file_stat.st_mode & S_IRUSR) {
            buffer[1] = 'r';
        }
        if (file_stat.st_mode & S_IWUSR) {
            buffer[2] = 'w';
        }
        if (file_stat.st_mode & S_IXUSR) {
            buffer[3] = 'x';
        }
    } else if (file_stat.st_gid == getgid()) {
        /* Output group rights */
        if (file_stat.st_mode & S_IRGRP) {
            buffer[1] = 'r';
        }
        if (file_stat.st_mode & S_IWGRP) {
            buffer[2] = 'w';
        }
        if (file_stat.st_mode & S_IXGRP) {
            buffer[3] = 'x';
        }
    } else {
        /* Output "other" rights */
        if (file_stat.st_mode & S_IROTH) {
            buffer[1] = 'r';
        }
        if (file_stat.st_mode & S_IWOTH) {
            buffer[2] = 'w';
        }
        if (file_stat.st_mode & S_IXOTH) {
            buffer[3] = 'x';
        }
    }
}

/**
 * Creates a copy of the string trimmed from both sides.
 * @param str - string to copy.
 * @returns processed string copy.
 */
char *sanitize_string(char *str)
{
    int i;
    char *result;
    
    /* Allocate the copy */
    result = (char *) malloc((strlen(str) + 1) * sizeof(char));
    result[0] = '\0';
    
    /* Skip all the spaces */
    i = 0;
    while (str[i] != '\0' && isspace(str[i])) {
        i++;
    }
    
    /* Copy the content */
    strcpy(result, str + i);
    
    /* Trim the ending spaces */
    i = strlen(result);
    while (i > 0 && isspace(result[i - 1])) {
        result[i - 1] = '\0';
        i--;
    }
    
    /* Return created string */
    return result;
}

/**
 * Checks if the supplied character is a word character.
 * @param c - character to check.
 * @returns true if word character, false otherwise.
 */
bool is_word_char(char c)
{
    /* Spaces are not word characters */
    if (isspace(c) || c == '\0') {
        return false;
    }
    
    /* Punctuation is not word characters */
    if (strchr("\t\r\n.,:?!`()[]-/\'\"<>", c) != NULL) {
        return false;
    }
    
    /* All other are word characters */
    return true;
}

/**
 * Searches the line for a term and prints result if found.
 * @param task - thread argument with path, stats, and term.
 * @param line - text of the line.
 * @param line_no - number of this line in the file.
 */
void search_line(ThreadTask *task, char *line, int line_no)
{
    int i, j;
    bool found = false;
    
    /* LOG("Searching line '%s' for '%s'\n", line, task->term); */
    
    /* Iterate over all possible term start position */
    i = 0;
    while (line[i] != '\0') {
        /* Skip all the characters which are not letters */
        while (line[i] != '\0' && !is_word_char(line[i])) {
            i++;
        }
        
        /* Compare this word to the term */
        j = 0;
        while (line[i] != '\0' && task->term[j] != '\0' &&
               ((g_exact && line[i] == task->term[j]) ||
                (!g_exact && tolower(line[i]) == tolower(task->term[j])))) {
                   i++;
                   j++;
               }
        
        /* Stop if the term was found */
        if (task->term[j] == '\0' && !is_word_char(line[i])) {
            found = true;
            break;
        }
        
        /* Else, ignore all letters in the line */
        while (line[i] != '\0' && is_word_char(line[i])) {
            i++;
        }
    }
    
    /* If the term was found, print the result */
    if (found) {
        char mode_str[5];
        char *print_line;
        
        /* Prepare the mode and string for output */
        output_mode(task->file_stat, mode_str);
        print_line = sanitize_string(line);
        
        /* Ensure only this thread is printing */
        pthread_mutex_lock(&g_output_mutex);
        
        /* Print the filename and line index */
        printf("[ %s | %d | %s | %s ]\n", task->file_path, line_no,
               mode_str, print_line);
        
        /* Release the lock */
        pthread_mutex_unlock(&g_output_mutex);
        
        /* Clean the memory for sanitized line */
        free(print_line);
    }
}

/**
 * Reads a line from the file.
 * @param file - file to read the line from.
 * @returns allocated line.
 */
char *read_line(FILE *file)
{
    char *result = NULL, *result_old;
    int result_length, result_capacity;
    int c;
    
    /* Read first character and check if the end of the file reached */
    if ((c = fgetc(file)) == EOF) {
        return NULL;
    }
    
    /* Init the response with this character */
    result_capacity = 1024;
    result_length = 1;
    result = (char *) malloc(result_capacity);
    result[0] = '\0';
    
    /* Check if that's empty line */
    if (c == '\n') {
        return result;
    }
    
    /* Else, init the string with this char*/
    result[0] = c;
    result[1] = '\0';
    result_length = 2;
    
    /* Read the characters and add until '\n' is found or EOF reached */
    while (1) {
        /* Read the character and stop if it's EOF or '\n' */
        c = fgetc(file);
        if (c == EOF || c == '\n') {
            break;
        }
        
        /* Check if the resulting string should be resized */
        if (result_length >= result_capacity) {
            /* Resize the string */
            result_capacity += 1024;
            result_old = result;
            result = (char *) realloc(result, result_capacity);
            if (result == NULL) {
                free(result_old);
                return NULL;
            }
        }
        
        /* Add the character to the string */
        result[result_length - 1] = c;
        result[result_length++] = '\0';
    }
    
    /* Return created string */
    return result;
}

/**
 * Code for a thread that searches for a term inside the file.
 * @param arg - pointer argument of the thread call.
 * @returns thread exit code.
 */
void *search_thread(void *arg)
{
    ThreadTask *task;
    FILE *file;
    
    /* Cast the supplied argument to the correct type */
    task = (ThreadTask *) arg;
    
    /* LOG("Searching '%s' for token '%s'\n", task->file_path, task->term); */
    
    /* Open the file for reading */
    file = fopen(task->file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file '%s'", task->file_path);
        perror("");
    } else {
        int line_no = 0;
        char *line;
        
        /* Process each line */
        while ((line = read_line(file)) != NULL) {
            /* Increase index */
            line_no++;
            
            /* Search the term inside this line */
            search_line(task, line, line_no);
            
            /* Finally, free this line */
            free(line);
        }
        
        /* Close the file */
        fclose(file);
    }
    
    /* Release the semaphore so the next thread can start */
    sem_post(&g_threads_sem);
    
    /* Free the argument's memory */
    free(arg);
    
    /* Detach this thread to have no "potentially lost" memory */
    pthread_detach(pthread_self());
    
    /* Finished successfuly */
    return NULL;
}

/**
 * Creates a thread to search the term in the given file.
 * @param file_path - path to file in which we should search.
 * @param file_stat - properties of the file.
 * @param term  - term to search.
 */
void search_in_file(char *file_path, struct stat file_stat, char *term)
{
    ThreadTask *task;
    pthread_t thread_id;
    
    /* Create and fill a structure for a thread to use */
    task = (ThreadTask *) malloc(sizeof(ThreadTask));
    memcpy(task->file_path, file_path, strlen(file_path) + 1);
    memcpy(&(task->file_stat), &file_stat, sizeof(file_stat));
    task->term = term;
    
    /* Lock the semaphore before running the thread */
    sem_wait(&g_threads_sem);
    
    /* Create a new thread */
    pthread_create(&thread_id, NULL, search_thread, task);
}

/**
 * Searches the term in the directory tree starting from given directory.
 * @param dir_path - path to directory which is the root of the subtree.
 * @param term - term to search.
 */
void search_in_directory(char *dir_path, char *term)
{
    DIR *dir;
    struct dirent *ent;
    
    /* LOG("Searching inside directory '%s'\n", dir_path); */
    
    /* Open the directory */
    dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "Failed to open the directory '%s'", dir_path);
        perror("");
        return;
    }
    
    /* Process all entries inside the directory */
    while ((ent = readdir(dir)) != NULL) {
        char entry_path[PATH_MAX];
        struct stat entry_stat;
        
        /* If it's relative entries, ignore them */
        if ((strcmp(".", ent->d_name) == 0) ||
            (strcmp("..", ent->d_name) == 0)) {
            continue;
        }
        
        /* Create a full path */
        sprintf(entry_path, "%s/%s", dir_path, ent->d_name);
        
        /* Check the file's stats */
        if (stat(entry_path, &entry_stat) < 0) {
            fprintf(stderr, "Failed to get stats of '%s'", entry_path);
            perror("");
            continue;
        }
        
        /* Use the stat to check if it's directory or a file */
        if (S_ISDIR(entry_stat.st_mode)) {
            search_in_directory(entry_path, term);
        } else {
            search_in_file(entry_path, entry_stat, term);
        }
    }
    
    /* Close the directory */
    closedir(dir);
}

/**
 * Executes the search.
 * @param max_threads - maximum count of threads to use.
 */
void run_search(int max_threads)
{
    DIR *dir;
    char search_dir[PATH_MAX];
    char abs_dir[PATH_MAX];
    
    /* If the directory was not specified, use current working directory */
    if (g_search_dir == NULL) {
        if (getcwd(search_dir, sizeof(search_dir)) == NULL) {
            perror("Failed to get current working directory");
            exit(1);
        }
        g_search_dir = search_dir;
    }
    LOG("Using directory '%s'\n", g_search_dir);
    
    /* Try to open the directory to check if it's possible */
    dir = opendir(g_search_dir);
    if (dir == NULL) {
        perror("Failed to open the start directory");
        exit(1);
    }
    closedir(dir);
    
    /* Resolve the relative path to the absolute path */
    if (realpath(g_search_dir, abs_dir) == NULL) {
        perror("Failed to resolve start directory path");
        exit(1);
    }
    LOG("Absolute path to search directory is '%s'\n", abs_dir);
    
    /* Create the semaphore which is used to limit the count of threads */
    sem_init(&g_threads_sem, 0, max_threads);
    
    /* Create the mutex for output */
    pthread_mutex_init(&g_output_mutex, NULL);
    
    /* Do the search */
    LOG("Searching '%s' recursively using %d threads\n",
        abs_dir, max_threads);
    int i;
    for (i = 0; i < g_num_terms; ++i) {
        LOG("Search term: '%s'\n", g_search_terms[i]);
        search_in_directory(abs_dir, g_search_terms[i]);
    }
    
    /* Lock the semaphore for maximum capacity to know that threads ended */
    for (i = 0; i < max_threads; i++) {
        sem_wait(&g_threads_sem);
    }
    for (i = 0; i < max_threads; i++) {
        sem_post(&g_threads_sem);
    }
    
    /* Free the semaphore and mutex */
    sem_destroy(&g_threads_sem);
    pthread_mutex_destroy(&g_output_mutex);
}

/**
 * This is the program's entry point.
 * @param argc - count of command-line arguments.
 * @param argv - command-line arguments.
 * @returns program exit code.
 */
int main(int argc, char *argv[])
{
    int max_threads;
    
    /* Set default max threads count*/
    max_threads = get_nprocs();
    
    /* Parse-process the command line arguments */
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
                if (max_threads < 1) {
                    fprintf(stderr, "Invalid threads count (has to be > 0)\n");
                    return 1;
                }
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
    
    /* See how much and which search terms we have */
    g_num_terms = argc - optind;
    g_search_terms = &argv[optind];
    
    /* If no terms specified, just print usage and exit */
    if (g_num_terms == 0) {
        print_usage(argv);
        return 1;
    }
    
    /* Do the search */
    run_search(max_threads);
    
    return 0;
}
