/*
 * This is a client program that communicates with a sever.
 * The arguments passed to this program, client, are the
 * IP address of the server to connect to and the port number.
 * Using this information the client creates an internet socket connection
 * to the specified server. Once the connection is made the client is
 * prompted to send a message to the server. If successful, the server will
 * echo back the message.
 *
 * RELATED:
 * - server.c
 *
 * Authors: Dan Reinke (@djreinke), Ryan Carl (rpcarl)
 * Date: 4/22/16
 *
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>     /* For AFINET sockets */
#include <strings.h>        /* For bzero */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#define DEFAULT_PROTOCOL    0	/* default for socket method */
#define MAX_BUF 1000			/* max read/write message length */

unsigned long nameToAddr ();
int setupConnection(char *, char *);

void error(char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    char buffer[MAX_BUF];
    int serverFd, result;

	/* invalid call */
    if (argc < 3){
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(-1);
    }
    
    serverFd = setupConnection(argv[1], argv[2]);

    while(1){
        /* read from server */
        bzero(buffer, MAX_BUF);
        if(read(serverFd, buffer, MAX_BUF) <= 0){
            fprintf(stderr,"Server disconnected.\n");
    		exit(0);
		}

	    printf("%s ", buffer);				/* show prompt */
    	bzero(buffer, MAX_BUF);				/* clean buffer */
	    fgets(buffer, MAX_BUF, stdin);		/* get command */
    	buffer[strlen(buffer)-1] = '\0'; 	/* remove newline */
	    /* send command to server */
		result = write(serverFd,buffer,strlen(buffer));
	
		if(strcmp(buffer, "exit") == 0){
			exit(0);
		}
        
	    if(read(serverFd,buffer,MAX_BUF) <= 0){
    		fprintf(stderr,"Error reading from server.\n");
        	exit(0);
	    } 
    
		printf("%s",buffer);
    }
    close(serverFd);
    exit(0);
    return 0;
}

/* 
 * Initializes connection to the server.
 * Accepts: server host name (e.g. linux2), port number
 * Returns: Server file descriptor
 * ****************************************************************************
 * */
int setupConnection(char *inetAddr, char *port){
    int serverFd, serverLen, portno, result;
    struct sockaddr_in serverINETAddress;
    struct sockaddr* serverSockAddrPtr;
    unsigned long inetAddress;

    serverSockAddrPtr = (struct sockaddr*) &serverINETAddress;
    serverLen = sizeof(serverINETAddress);

    /* Get IP and port num */
    inetAddress = nameToAddr(inetAddr);
    if(inetAddress == 0)
        error("ERROR host name not found");
    portno = atoi(port);	/* convert to int */

    bzero((char *) &serverINETAddress, serverLen);
    serverINETAddress.sin_family = AF_INET;
    serverINETAddress.sin_addr.s_addr = inetAddress; /* IP */
    serverINETAddress.sin_port = htons(portno);
    
    serverFd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL); 
    if (serverFd < 0) 
        error("ERROR opening socket");
    do {
        result = connect(serverFd, (struct sockaddr *)&serverINETAddress, serverLen);
        if(result == -1) {
            sleep(1);
        }
    }
    while(result == -1);
    return serverFd; 
}


/* 
 * Manages the assignment of the host to connect to
 * Accepts: host name (e.g. linux2)
 * Returns: IP address
 * ****************************************************************************
#define MAX_BUF 300
 * */
unsigned long nameToAddr (char *name){
	char hostName [100];
	struct hostent* hostStruct;
	struct in_addr* hostNode;
	
	/* If name begins with a digit, assume it's a valid numeric */
	/* Internet address of the form A.B.C.D and convert directly */
	if (isdigit (name[0])) return (inet_addr (name));

	if (strcmp (name, "s") == 0){ /* Get host name from database */
		gethostname (hostName,100);
		printf ("Self host name is %s\n", hostName);
	} else /* Assume name is a valid symbolic host name */
		strcpy (hostName, name);

	/* obtain address information from database */
	hostStruct = gethostbyname (hostName);
	if (hostStruct == NULL) return (0); /* Not Found */
	/* Extract the IP Address from the hostent structure */
	hostNode = (struct in_addr*) hostStruct->h_addr;
	return (hostNode->s_addr); /* Return IP address */
}
