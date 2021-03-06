#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "variables.h"
#include "commands.h"
#include "environment.h"
#include "command_parser.h"
#include "file_processing.h"

void cd(char *path) {
    // cd to $HOME if no path or ~ given
    if (path == NULL || strcmp(path, "~") == 0) {
        char *home = lookup_variable("HOME");
        if (chdir(home) != 0) {
            error(path, "No such file or directory");
        }
        free(home);
    }
    // cd to given path if exists, replacing ~ with value of $HOME
    else {
        char buffer[MAX_BUFFER_SIZE + 1];
        int index = 0;
        while (path[index] == ' ' || path[index] == '\t') index++;
        if (path[index] == '~'){
            char *home = lookup_variable("HOME");
            memcpy(buffer, home, strlen(home) + 1);
            strcat(buffer, path + index + 1);
            free(home);
        }
        else {
            memcpy(buffer, path, strlen(path) + 1);
        }
        if (chdir(buffer) != 0) {
            error(path, "No such file or directory");
        }
    }
}

void assign_variable(const char *assignment) {
    // if first char of the variable is not alpha then it's not a valid variable name
    if (!isalpha(assignment[0])) {
        error(assignment, "command not found");
        return;
    }
    // get the key and value (parts before and after =) and store them in variable table
    int index = (int) (strchr(assignment, '=') - assignment);
    char key[index + 1];
    char val[strlen(assignment) - index + 1];
    memcpy(key, assignment, index);
    memcpy(val, assignment + index + 1, strlen(assignment) - index);
    key[index] = '\0';
    val[strlen(assignment) - index] = '\0';
    set_variable(key, val);
}

void printenv(const char *arg) {
    if (arg == NULL) {
        // loops through the environment variables and look them up in the variable table and print them
        for (int i = 0; get_env_variables()[i] != NULL; i++) {
            char *val = lookup_variable(get_env_variables()[i]);
            printf("%s=%s\n", get_env_variables()[i], val);
            free(val);
        }
        printf("\n");
    } else {
        error(arg, "No such file or directory");
    }
}

void export(const char *assignment) {
    // checks for valid assignment
    if (assignment == NULL || !strchr(assignment, '=')) {
        return;
    }
    // checks for valid variable name
    if (!isalpha(assignment[0])) {
        error(assignment, "is not a valid identifier");
        return;
    }
    // store key and value in variable table
    int index = (int) (strchr(assignment, '=') - assignment);
    char *key = malloc(sizeof(char) * (index + 1));
    char val[strlen(assignment) - index + 1];
    memcpy(key, assignment, index);
    memcpy(val, assignment + index + 1, strlen(assignment) - index);
    key[index] = '\0';
    val[strlen(assignment) - index] = '\0';
    set_variable(key, val);
    // add key to environment variables
    add_env_variable(key);
}

void history() {
    char buffer[MAX_BUFFER_SIZE + 1];
    int cnt = 1;
    memset(buffer, 0, sizeof(buffer));
    rewind(get_history_file());
    while (!feof(get_history_file())){
        fgets(buffer, MAX_BUFFER_SIZE, get_history_file());
        printf("%d %s", cnt++, buffer);
        memset(buffer, 0, sizeof(buffer));
    }
}


void error(const char *command, const char *msg) {
    fprintf(stderr, "%s: %s\n", command, msg);
}