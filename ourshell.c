/*
Author:		Ryan Carl & Daniel Reinke
ID:	        1694731, 1697289
Course:		CSIS-381
Assignment:	Lab 11

ourshell - simple shell program with piping capability

Note: "dopipe" code adapted from Glass & Ables text
*/

#include <stdio.h>		/* printf, fgets */
#include <stdlib.h>		/* malloc, execvp, wait */
#include <string.h>		/* strlen, strcat, strcmp */
#define MAXLN 100		/* max length of command line */
#define MAXARGS 20		/* maximum number of command args */
#define DELIM " "		/* command args separator */
#define CMDNF 149		/* arbitrary error exit code */
#define EXIT "exit"		/* exit command */
#define PIPE "|"		/* pipe command */
#define READ   0		/* reader fd */
#define WRITE  1		/* writer fd */
#define MAX_BUF 300

int readMessage(int, char*);
void writeMessage(int, char*);

void dopipe(char **wargs, char **rargs) {
    int fd[2];
    pipe(fd); /* Create an unamed pipe */
    if (fork() != 0) { /* writer */
        close(fd[READ]); 		/* Close unused end */
        dup2(fd[WRITE], 1); 	/* Duplicate used end to stdout */
        close(fd[WRITE]); 		/* Close original used end */
        execvp(wargs[0], wargs); /* Execute writer program */
        fprintf(stderr, "%s could not be executed\n", wargs[0]);
        fprintf(stdout, "%s could not be executed\n", wargs[0]);
        exit(CMDNF);
    } else { 	/* reader */
        close(fd[WRITE]); 		/* Close unused end */
        dup2(fd[READ], 0); 		/* Duplicate used end to stdin */
        close(fd[READ]); 		/* Close original used end */
        execvp(rargs[0], rargs); /* Execute reader program */
        fprintf(stderr, "%s could not be executed\n", rargs[0]);
        fprintf(stdout, "%s could not be executed\n", rargs[0]);
        exit(CMDNF);
    }
}

/* 
 * Read from client, get arguments to execute 
 * Accepts: file descriptor 
 * Returns: number of arguments
 * */
int getargs(int fd, char **args) {
    int argsIndex;
    char *line = (char *) malloc(MAXLN);
    if (readMessage(fd, line) <= 0){/* get command */
        fprintf(stderr, "Error reading from client.\n");
        exit(0);
    }
    *args = strtok(line, DELIM);	/* split command */
    argsIndex = 0;
    /* chop string */
    while (args[argsIndex] != NULL)
        args[++argsIndex] = strtok(NULL, DELIM);
    free(line);
    return argsIndex; /* number of arguments */
}

void zeroArgs(int argc, char **args){
    int i;
    for(i = 0; i < argc; i++)
        bzero(args[i], 256);
}

void redirectOutput(int fd){
    dup2(fd, 1);
    close(fd);
}

/* 
 * Writes shell prompt to client, then reads args to exec from client.
 * */
runShell(int fd) {
    char **args;
    int pid, argc, childPid, status, i;
    char *p;

    /* Build shell prompt */
    if (getenv("LOGNAME") != NULL) {
        p = getenv("LOGNAME");
        strcat(p, "$ ");
    } 
    else
        p = "$ ";

    args = (char **) malloc(MAXARGS * sizeof(char *));	
    
    while (1) {	/* flag-controlled loop */
        
        /* Write shell prompt to client */
        writeMessage(fd, p);

        argc = getargs(fd, args);
        if (strcmp(args[0], EXIT) == 0){
            fprintf(stderr, "Client disconnected.\n");
            exit(0); /* no error exit */
        }
        else {
            pid = fork();
            if (pid != 0) { /* parent */
                childPid = wait(&status); /* wait for child */

                zeroArgs(argc, args);
            } 
            else { /* child */
                redirectOutput(fd);
                i = 0;
                /* search for pipe */
                while (i++ < argc) {
                    if (args[i] != NULL && strcmp(args[i], PIPE) == 0) {
                        args[i] = NULL;	/* replace pipe */
                        dopipe(args, args+i+1);	/* split */
                        break;
                    }
                }
                if (i >= argc) {
                    execvp(args[0], args); /* execute command */
                    fprintf(stderr, "%s could not be executed\n", args[0]);
                    fprintf(stdout, "%s could not be executed\n", args[0]);
                    exit(CMDNF); /* exit w/ error */
                }
            }
        }
    }
}

/*
 * Writes message to file
 * Accepts: a file descriptor, message char array
 * ****************************************************************************
 */
void writeMessage(int fd, char *message){
    write(fd, message, strlen(message + 1));
}

/*
 * Reads message from file
 * Returns: 1 if read something, 0 if not
 * ****************************************************************************
 */
int readMessage(int fd, char *line){
    return read(fd, line, MAX_BUF); 
}
