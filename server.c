/*Rory Bresnahan
* bresnahr@onid.oregonstate.edu
* CS372
* Project 1
* server.c	this program is the server side of a chat program.  The server waits for a client request on
* 		a port that was specified on the command line.  It waits for a message from the client and begins
* 		chat, sending and receiving messages in turn
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

char copyHandle[500];
char myHandle[10];
char clientMessage[500];
char c;
int yes = 1;

// SIGINT derived from code at http://www.thegeekstuff.com/2012/03/catch-signals-sample-c-code/
void sig_handler(int signo){
    if(signo == SIGINT){
        printf("Received SIGINT, terminating application\n");
	exit(0);
    }
}
void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){
	return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
// sends messages to the client
void sendMessage(int fd){

    char message[500];
    
    while((c = getchar()) != '\n' && c != EOF);// testing ways to clear STDIN/STDOUT
    fflush(stdout);

    //strcat(copyHandle, message);

    if(fgets(message, sizeof(message), stdin) != NULL){// make sure the char array terminates with ending character
        char *line = strchr(message, '\n');
        if(line != NULL){
            *line = '\0';
        }
    }

    if(strcmp("\\quit", message) == 0){
            printf("Closing connection\n");
            exit(0);
    }

    strcat(copyHandle, message);   

    int i = send(fd, copyHandle, strlen(copyHandle), 0);
    if(i < 0){
        perror("Error sending message\n");
	exit(0);
    }

    return;
}
// receives messages from the client, then prints it for display
void getMessage(int fd){
    
    int i;
    i = recv(fd, clientMessage, 500, 0);
    if(i < 0){
        perror("Error receiving message\n");
	exit(0);
    }
    else if(i > 0){
	
	printf("%s", clientMessage);
/* this would have closed the connection if a message containing just '\quit' was received
	if(strcmp("\\quit", clientMessage) == 0){
	    close(fd);
	
	}else{
	    printf("%s \n", clientMessage);
	    //bzero(clientMessage, strlen(clientMessage));
	}
*/
    }
    else if(i == 0){
	printf("Connection closed by client\n");
	close(fd);
    }

    return;   
}
// this functions starts the connection and waits for a client response
// connect code derived from http://beej.us/guide/bgnet/output/html/multipage/clientserver.html#simpleserver
int start_up(int sockfd, char *argvOne){

    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, argvOne, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, SOCK_STREAM, p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }

        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
	     
	}

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    
    freeaddrinfo(servinfo);
    return sockfd;

}
int main(int argc, char *argv[]){

    int sockfd, newfd;
    struct sockaddr_storage their_addr; // client's address info
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    // command line must have port number as argument
    if(argc != 2){
        fprintf(stderr, "Please run program with port number as an argument\n");
        exit(1);
    }
   
    sockfd = start_up(sockfd, argv[1]);

    if(listen(sockfd, 10) == -1){// listen for up to 10 connections
	perror("listen");
    }

    printf("server is listening for connections...\n");

    signal(SIGINT, sig_handler);// checks for SIGINT throughout application
       
    while(1){
        sin_size = sizeof their_addr;
        newfd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);

        if(newfd == -1){
            perror("accept");
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof(s));
        printf("server: got connection from %s\n", s);

	if(!fork()){
	    close(sockfd);// child process closes listener
	    
            printf("Enter a handle name 10 characters or less: ");
            scanf("%s", myHandle);
  	
            // make sure handle is 10 characters or less
            while(strlen(myHandle) > 10){

                printf("Handle name must be 10 characters or less- try again: ");
                while((c = getchar()) != '\n' && c != EOF);
                scanf("%s", myHandle);
            }

            strcat(myHandle, "> ");
            printf("Close the connection during chat by typing: \\quit\nPlease wait for a message from the client.\n");
	
	    while(1){

	        //bzero(clientMessage, 500);
	        getMessage(newfd);
	        strcpy(copyHandle, myHandle);
                printf("%s", myHandle);
	        sendMessage(newfd);
 
            }
    	    close(newfd);
	}
        //close(newfd); not sure about needing to close sockets, left as a comment for the future
    }
   // close(sockfd);
    return 0;
}
