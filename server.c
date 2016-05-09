/*
 * This is a server that uses AF_INET sockets.
 * The server sets itself up then runs and waits for a 
 * client connect. When a client connects and sends a message,
 * the server logs the message to stdout and then echos back the
 * message that the client sent. The only argument for the program is
 * a port number that must be specified for the program to run correctly.
 *
 * RELATED:
 * - client.c
 * - ourshell.c
 *
 * Author: Dan Reinke (@djreinke), Ryan Carl (rpcarl)
 * Date: 4/24/16
 *
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>     /* For AF_INET domain */
#include <string.h>        /* For bzero */
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_PROTOCOL   0		/* default for socket function */
#define MAX_BUF 300					/* max read/write message length */
#define SHELL_PROMPT "[myshell]$"	/* prompt */

/****************************************************************/

void error(char *msg){
    perror(msg);
    exit(1);
}
void runServer(int);

int main (int argc, char *argv[]){
    char msg[MAX_BUF];
    int serverFd, status;
    if(argc < 2){
        fprintf(stderr, "SERVER: ERROR no port provided\n");
        exit(1);
    }
    serverFd = setupServer(atoi(argv[1]));
    runServer(serverFd);
}

/* 
 * Start the server.
 * Accepts: port number
 * Returns: client file descriptor 
 * ****************************************************************************
 * */
int setupServer(int portno){

    int serverFd, clientFd, serverLen, clientLen;
    struct sockaddr_in serverINETAddress;	/* Server address */
    struct sockaddr* serverSockAddrPtr; 	/* Ptr to server address */
    
    serverSockAddrPtr = (struct sockaddr*) &serverINETAddress;
    serverLen = sizeof (serverINETAddress);
    serverFd = socket( AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    
    serverINETAddress.sin_family = AF_INET;
    serverINETAddress.sin_addr.s_addr = INADDR_ANY;
    serverINETAddress.sin_port = htons(portno);
    
    if(serverFd < 0)
       error("SERVER: ERROR opening socket");

    bind(serverFd, serverSockAddrPtr, serverLen); /* bind a name to the socket */
    listen(serverFd, 5); /* listen for a socket connection */
    fprintf(stderr, "SERVER: listening for connections on port %d...\n", portno);
    return serverFd;
}

int acceptClient(int serverFd){
    int clientFd, clientLen;
    struct sockaddr_in clientINETAddress; /* Client address */
    struct sockaddr* clientSockAddrPtr; /* Ptr to client address */
    clientSockAddrPtr = (struct sockaddr*) &clientINETAddress;
    clientLen = sizeof (clientINETAddress);
    clientFd = accept(serverFd, clientSockAddrPtr, &clientLen);
    if(clientFd < 0)
        error("ERROR on accept");
    return clientFd;
}

/* 
 * Runs the server.
 * Accepts: client socket file descriptor
 * ****************************************************************************
 * */
void runServer(int serverFd){
    char *line;
    size_t length; 
    int status, isParent, pid, clientFd;
    
    isParent = 1;
    while(isParent){
        clientFd = acceptClient(serverFd);
        if(clientFd < 0)
            error("SERVER: ERROR accepting client");
        else{
            printf("Client connected.\n", clientFd);
        }
        
        pid = fork();
        if(pid == 0){ /* child */
            isParent = 0;
            runShell(clientFd);
            close(clientFd);
            exit(0);
        }
    }
    close(clientFd);
    exit(0);
}
