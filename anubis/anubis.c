#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h> 
#include <fcntl.h>
#include <assert.h>
#include <stdarg.h>


// Use this _DEBUG variable to control the error messages printed
// by the ERROR function.
//  _DEBUG=0: print only one error message; 
//  _DEBUG=1: print custom messages.
// Make sure you set _DEBUG to 0 for the submitted version. 

int _DEBUG = 0;
struct string_list * paths; // shell paths

void ERROR(int errnum, const char * format, ... )
{
    if(_DEBUG) {
        va_list args;
        va_start (args, format);
        vfprintf(stderr, format, args);
        va_end (args);
        if(errnum > 0) fprintf(stderr, " : %s", strerror(errnum));
        fprintf(stderr, "\n"); 
        return; 
    }

    fprintf(stderr, "An error has occurred\n"); 
}


// A list of string associated with some helper functions
struct string_list{
    char ** string_array; // A null-terminated array of strings
    int current_size; // number of element stored
    int max_size; // Actual size of the array
};

// A list of string associated with some helper functions
struct pid_list{
    int * pid_array; // A null-terminated array of int
    int current_size; // number of element stored
    int max_size; // Actual size of the array
};

// Create a int list
struct pid_list * pid_list_create(){
    int size = 32;
    int * pid_array = malloc(size * sizeof(pid_t)); // allocate an array consisting of 32 pointers
    if(!pid_array) {
        ERROR(errno, "malloc()");
        exit(EXIT_FAILURE);} 
    memset(pid_array, 0, size * sizeof(int)); // initialise the array
    struct pid_list* list = malloc(sizeof(struct pid_list));
    list->pid_array = pid_array;
    list->current_size = 0;
    list->max_size = size;
    return list;
}

// Free a int list
void pid_list_free(struct pid_list * list){
    free(list->pid_array);
    free(list);
}

// Add an element to the int list
void pid_list_add(struct pid_list * list, int num){
    if(list->current_size == list->max_size) { // extent the token array
        list->max_size += 10;
        list->pid_array = realloc(list->pid_array, list->max_size * sizeof(pid_t));
    }
    list->pid_array[list->current_size] = num;
    list->current_size++;
}

// Print the string list
void pid_list_print(struct pid_list * list){
    for (int i = 0; i < list->current_size; i++) {
        printf("[%i] %i\n", i, list->pid_array[i]);
    }
}

// Create a string list
struct string_list * string_list_create(){
    int size = 32;
    char ** string_array = malloc(size * sizeof(char *)); // allocate an array consisting of 32 pointers
    if(!string_array) {
        ERROR(errno, "malloc()");
        exit(EXIT_FAILURE);} 
    memset(string_array, 0, size * sizeof(char*)); // initialise the array
    struct string_list* list = malloc(sizeof(struct string_list));
    list->string_array = string_array;
    list->current_size = 0;
    list->max_size = size;
    return list;
}

// Free a string list
void string_list_free(struct string_list * list){
    for(int i = 0; i < list->current_size; i++) // free each element (string)
    {
        if(list->string_array[i]) free(list->string_array[i]);
        list->string_array[i] = NULL;
    }
    free(list->string_array);
    free(list);
}

// Add an element to the string list
void string_list_add(struct string_list * list, char * str){
    if(list->current_size + 1 == list->max_size) { // extent the token array
        list->max_size += 10;
        list->string_array = realloc(list->string_array, list->max_size * sizeof(char*));
    }
    list->string_array[list->current_size] = strdup(str);
    list->current_size++;
    list->string_array[list->current_size] = NULL;
}

// Get a sublist of the string list
struct string_list * string_list_sublist(struct string_list * list, int start, int end){
    struct string_list * sub_list = string_list_create();
    for (int i = start; i < end && i < list->current_size; i++){
        string_list_add(sub_list, list->string_array[i]);
    }
    return sub_list;
}


// Print the string list
void string_list_print(struct string_list * list){
    for (int i = 0; list->string_array[i] != NULL; i++) {
        printf("[%i] %s\n", i, list->string_array[i]);
    }
}


// Modified From Lab 3
// inputs: command line
// output: an array of tokens where the last element is NULL

struct string_list * tokenise(char *line)
{
    char * delim1 = " \t\n";
    char * delim2 = "&>|";
    struct string_list * tokens = string_list_create(); // create a list to hold all tokens
    char * token;

        token = line;
        int len = strlen(line);
        int i = 0;
        while(i < len){
            if (line[i] == '\0'){
                if (strlen(token) > 0) string_list_add(tokens, token);
                i++;
            } else if (strchr(delim1, line[i])){
                line[i] = '\0';
                if (strlen(token) > 0) string_list_add(tokens, token);
                i++;
                token = &line[i];
            } else if (strchr(delim2, line[i])) {
                char * separator = malloc(2);
                separator[0] = line[i];
                separator[1] = '\0';
                line[i] = '\0';
                if (strlen(token) > 0) string_list_add(tokens, token);
                string_list_add(tokens, separator);
                free(separator);
                i++;
                token = &line[i];
            } else {
                i++;
            }
    }
    return tokens;
}

// run a built-in command given a list of tokens
// 0: run secessfully
// -1: not a built-in command
int run_cmd_built_in (struct string_list * tokens){
    char ** argv = tokens->string_array;

    // Build-in command: exit
    if(strcmp(argv[0], "exit") == 0){
        if (tokens->current_size == 1){
            exit(EXIT_SUCCESS);
        } else {
            ERROR(0, "A exit command should only contain an exit");
            return 0;
        }
    }

    // Build-in command: path
    if(strcmp(argv[0], "path") == 0){
        string_list_free(paths); // clear all previous path
        paths = string_list_create();
        for (int i = 1; i < tokens->current_size; i++){
            string_list_add(paths, argv[i]);
        }
        return 0;
    }

    // Build-in command: cd
    if(strcmp(argv[0], "cd") == 0) {
        if (tokens->current_size == 2){
            char * directory = argv[1];
            if (chdir(directory)){
                ERROR(0, "Change directory fail!");
            }
            return 0;
        } else {
            ERROR(0, "A cd command should have exactly one argument");
            return 0;
        }
    }
    return -1;
}

// run the command given a list of tokens
void run_cmd (struct string_list * tokens){
    char ** argv = tokens->string_array;
    execv(argv[0], argv);
    // If no program is found, search for paths
    char * path;
    for (int i = 0; i < paths->current_size; i++){
        path = paths->string_array[i];
        int mem_size = strlen(path) + strlen(argv[0]) + 2; // '/' and '\0' so + 2
        char * combined_path = malloc(mem_size);
        strcpy(combined_path, path);
        strcat(combined_path, "/");
        strcat(combined_path, argv[0]);
        if (!access(combined_path, X_OK)){
            execv(combined_path, argv);
        } else {
            free(combined_path);
        }
    }
    ERROR(errno,"execv()");
    exit(EXIT_FAILURE);
}


// Split a string_list into an array of string_list with a delimiter
struct string_list ** splitter (struct string_list * tokens, char * delimiter){
    int size = 32;
    struct string_list ** a = malloc(size * sizeof(struct string_list *)); 
    assert(a != NULL); 
    memset(a, 0, size * sizeof(char*));

    int start = 0; // start index of a subarray
    int end = 0; // end index of a subarray
    int i = 0; // index of a
    while(end <= tokens->current_size){
        if (tokens->string_array[end] == NULL || strcmp(tokens->string_array[end], delimiter) == 0){ 
            // find a delimiter
            if(i >= size - 1) {
                size += 10; 
                a = realloc(a, size * sizeof(char*));
            }
            a[i] = string_list_sublist(tokens, start, end);
            i++;
            end++; // skip the token
            start = end;
            continue;
        }
        end++;
    }
    a[i] = NULL;
    return a;
}


// Split a command into an array of parallel commands using the delimiter ">"
// e.g. tokens = ["cmd1", "|", "cmd2", "|", "cmd3"]
// -> ["cmd1", "cmd2", "cmd3"]
void parse_pipe (struct string_list * command){
    struct string_list ** pipeline_commands = splitter (command, "|");
    string_list_free(command);

    // No pipeline
    if (pipeline_commands[1] == NULL){
        run_cmd(pipeline_commands[0]);
        string_list_free(pipeline_commands[0]);
        free(pipeline_commands);
    }


    int first_program = 0; // index of the first program
    int last_program = 0; // index of the last program
    for(int i = 0; pipeline_commands[i] != NULL; i++){last_program++;}
    last_program--;
    int pipe_num = last_program; // number of pipe needed

    // An array of pipe, each pipe contains two file descriptor
    // e.g. [pipe_1_out, pipe_1_in, pipe_2_out, pipe_2_in...]
    int * pipe_array = malloc(pipe_num * 2 * sizeof(int));
    for(int i = 0; i < pipe_num * 2; i += 2){
        pipe(&pipe_array[i]);
    }

    // Loop through each command in tokens_array and build up a pipeline from them
    struct pid_list * parallel_processes = pid_list_create();
    pid_t pid;
    for(int i = 0; pipeline_commands[i] != NULL; i++){
        if (pipeline_commands[i]->current_size == 0){
            // ERROR(0, "Error: one of the parallel command is empty!");
            continue;
        }
        pid = fork();
        if (pid == 0) { // child process
            // using address of address of parallel_commands[i] because address of parallel_commands[i] will be redirected.
            // run_cmd (pipeline_commands[i]);

            // Set up a pipeline. There are 3 cases
            if (i == first_program){
                int pipe_in = pipe_array[1];
                dup2(pipe_in, STDOUT_FILENO);
                // Close all other pipe entry
                for (int i = 0; i < pipe_num * 2; i++){close(pipe_array[i]);}
                run_cmd (pipeline_commands[i]);
            } else if (i == last_program){
                int pipe_out = pipe_array[(i - 1) * 2];
                dup2(pipe_out, STDIN_FILENO);
                // Close all other pipe entry
                for (int i = 0; i < pipe_num * 2; i++){close(pipe_array[i]);}
                run_cmd (pipeline_commands[i]);
            } else{
                int pipe_in = pipe_array[i * 2 + 1];
                int pipe_out = pipe_array[(i - 1) * 2];
                dup2(pipe_out, STDIN_FILENO);
                dup2(pipe_in, STDOUT_FILENO);
                // Close all other pipe entry
                for (int i = 0; i < pipe_num * 2; i++){close(pipe_array[i]);}
                run_cmd (pipeline_commands[i]);
            }

        } else if (pid < 0){
            ERROR(errno, "fork()");
            exit(EXIT_FAILURE);
        } else{ // parent process
            pid_list_add(parallel_processes, pid); // parent records the child process
        }
    }
    // Parent close all it open pipe fd. Otherwise children will hang.
    for (int i = 0; i < pipe_num * 2; i++){
        close(pipe_array[i]);
    }
    // Reap finished parallel processes
    int wstatus;
    // printf("%i\n", parallel_processes->current_size);
    for (int i = 0; i < parallel_processes->current_size; i++){
        int w = waitpid(parallel_processes->pid_array[i], &wstatus, 0);
        if(w == -1) {
            ERROR(errno,"wait()");
            exit(EXIT_FAILURE);
        }
        // printf("reap:%i\n", i);
    }
    // Free pipe
    // Free pipe commands
    for (int i = 0; pipeline_commands[i] != NULL; i++){
        string_list_free(pipeline_commands[i]);
    }
    free(pipe_array);
    free(pipeline_commands);
    exit(0);
}

// Split a command into an array of parallel commands using the delimiter ">"
// The array size should be exactly 2 or 1, array[0] is the program argument and array[1] is the redirected output destination
// e.g. tokens = ["cmd1", "|", "cmd2", ">", "output1"]
// -> ["cmd1", "|", "cmd2", ">", "output1"]
void parse_redirect (struct string_list * command){
    struct string_list ** arguments_and_output = splitter (command, ">");
    string_list_free(command); // we no longer use it and use "arguments_and_output" instead

    // Calculate size
    int size = 0;
    for (int i = 0; arguments_and_output[i] != NULL; i++){size++;}

    // Case 1: No redirect and command is non-empty
    if (size == 1 && arguments_and_output[0]->current_size != 0){
        parse_pipe (arguments_and_output[0]);

    // Case 2: list size of program arguments is not empty and list size of output is exactly 1;
    } else if (size == 2 && arguments_and_output[0]->current_size != 0 && arguments_and_output[1]->current_size == 1){
        char * filepath = arguments_and_output[1]->string_array[0];
        int fd = open(filepath, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
        if (fd < 0) ERROR(errno, "open()");
        dup2(fd, STDOUT_FILENO);
        parse_pipe (arguments_and_output[0]);
    // Case 3: Invalid Syntyax of > symbol
    } else {
        ERROR(0, "Invalid used of > symbol");
        exit(EXIT_FAILURE);
    }
    free(arguments_and_output);
}

// Split a command into an array of parallel commands using the delimiter "&" and execute them separately but in parallel.
// e.g. tokens = ["cmd1", "|", "cmd2", ">", "output1", "&", "cmd3", ">", "ouput2", "&", "cmd4"]
// -> [["cmd1", "|", "cmd2", ">", "output1"], ["cmd3", ">", "ouput2"], ["cmd4"]]
void parse_parallel (struct string_list * command){
    struct string_list ** parallel_commands = splitter (command, "&");
    string_list_free(command); // original tokens it no longer used, so free it

    // Loop through each command in tokens_array and execute them in parallel.
    struct pid_list * parallel_processes = pid_list_create();
    pid_t pid;
    for(int i = 0; parallel_commands[i] != NULL; i++){
        if (parallel_commands[i]->current_size == 0){
            // ERROR(0, "Error: one of the parallel command is empty!");
            continue;
        }
        pid = fork();
        if (pid == 0) { // child process
            // using address of address of parallel_commands[i] because address of parallel_commands[i] will be redirected.
            parse_redirect (parallel_commands[i]);
        } else if (pid < 0){
            ERROR(errno, "fork()");
            exit(EXIT_FAILURE);
        } else{ // parent process
            pid_list_add(parallel_processes, pid); // parent records the child process
        }
    }
    // Reap finished parallel processes
    int wstatus;
    // printf("%i\n", parallel_processes->current_size);
    for (int i = 0; i < parallel_processes->current_size; i++){
        int w = waitpid(parallel_processes->pid_array[i], &wstatus, 0);
        if(w == -1) {
            ERROR(errno,"wait() in parse_parallel");
            exit(EXIT_FAILURE);
        }
        // printf("reap:%i\n", i);
    }
    free(parallel_commands);
    pid_list_free(parallel_processes);
}

// TODO: implement your shell!
int main(int argc, char *argv[])
{
    // Set up initial shell path
    paths = string_list_create();
    string_list_add(paths, "/bin");

    // Set to batch mode or interactive mode
    FILE * fp = stdin;
    if (argc == 1){}
    else {
        if (argc == 2){
            fp = fopen(argv[1], "r");
            if (fp == NULL) {
                ERROR(errno,"fopen()");
                exit(EXIT_FAILURE);
        }
        }
        else{
            ERROR(0, "Argument number should be one or two");
            exit(EXIT_FAILURE);
        }
    }

    char *input=NULL; // input buffer
    size_t n; 

    while (1) 
        {
        // Print anubis if in interactive mode
        if (fp == stdin) printf("anubis> ");
        
        // Get for user's command line
        if (getline(&input, &n, fp) < 1) break;  // exit if e.g. the user enter Ctrl+D and getline return -1

        // Turn the command line into an array of tokens
        struct string_list * tokens = tokenise(input);
        // string_list_print(tokens);
        // printf("%i, %i\n", tokens->current_size, tokens->max_size);

        // Skip empty command
        if (tokens->current_size == 0){
            continue;
        }
        // Try running the command line as a built-in command
        if (run_cmd_built_in(tokens)){ // If fail, run it as a external command.
            parse_parallel(tokens);
        } else { // If sucessful, go to the next while loop
            continue;
        }

        // Run command if there is at least one token
        // if(tokens->current_size > 0) run_cmd(tokens);

        // Clear the input and token buffer
        input[0] = '\0';
    }
    string_list_free(paths);
    return 0;
}
