/*Rory Bresnahan
 * bresnahr@onid.oregonstate.edu
 * CS372
 * Project 1
 * client.c     this is the client side of a chat program.  It accepts hostname and port number as command line arguments
 * 		and connects to a server.  A handle will be entered, 10 char or less.  Messages will be sent from client to server
 * 		and from server to client.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

// send messages to server, message has been copied with handle attached and sent here
void send_message(int fd, char msg[500]){

    int x;	
    x = send(fd, msg, 500, 0);// send the copied message with the handle appended
    if(x < 0){
        perror("Error sending message");
	exit(0);
    }
    return;
}
// receive messages from server, message is stored in buffer and cleared with bzero
void get_message(int fd, char buf[500]){

    int y;    
    y = recv(fd, buf, 500, 0);
    if(y < 0){
	perror("Error receiving message\n");
	exit(0);
    }else if(y == 0){
	printf("Server closed connection. Good Bye :)");
	exit(0);
    }
    printf("%s \n", buf);
    bzero(buf, 500);// clears the buffer
    return;
}
// contacts servver to set up a connection using the information (hostname, port number) received from the command line
// this section of code provided by http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleclient
int contact_server(int fd, char *argvOne, char *argvTwo){

    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;    
    
    if((rv = getaddrinfo(argvOne, argvTwo, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
    }


    for(p = servinfo; p != NULL; p = p->ai_next){
        if((fd = socket(p->ai_family, SOCK_STREAM, p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }

        if(connect(fd, p->ai_addr, p->ai_addrlen) == -1){
            close(fd);
            perror("client: connect");
	    continue;
        }

        break;
    }

    if(p == NULL){
        fprintf(stderr, "client: connection failed, exiting program\n");
	return 2;
    }
   
    return fd;
}

int main(int argc, char *argv[]){

    int sockfd;
    
    // command line must have 2 arguments, hostname and port
    if(argc != 3){
        fprintf(stderr, "Please run program with hostname and port number as arguments\n");
        exit(1);
    }
 
    
    sockfd = contact_server(sockfd, argv[1], argv[2]);
    // failsafe to exit program if no connection was made
    if(sockfd == -1){
	printf("Closing program due to failed attempts at connection\n");
	exit(1);
    }

    char myText[500];// for writing message
    char theChat[500];// copy of written message
    char myHandle[10];// handle char array
    char msgHandle[500];// for message and handle 
    char buffer[500];// buffer for received messages
    char c;

    printf("Enter a handle name 10 characters or less: ");
    scanf("%s", myHandle);   

    // make sure handle name is 10 characters or less
    while(strlen(myHandle) > 10){
       
  	printf("Handle name must be 10 characters or less- try again: ");
        while((c = getchar()) != '\n' && c != EOF);
	scanf("%s", myHandle);

    }

    strcat(myHandle, "> ");//appends '>' to the handle (part of requirement"

    printf("Close the connection during chat by typing: \\quit\nEnter a chat message and press Enter.  Please wait for a response before typing again.\n");
   
    while(1){

        strcpy(msgHandle, myHandle);
	printf("%s", myHandle);
	bzero(myText, 500);
	//while((c = getchar()) != '\n' && c != EOF);  testing ways to clear stdin, I'm leaving it in for personal reference
        scanf( " " );
	fgets(myText, 500, stdin);
	if(strcmp("\\quit\n", myText) == 0){
	    close(sockfd);
	    exit(0);
	}

        strcpy(theChat, myText);// copy clients message
	strcat(msgHandle, theChat);// add the clients handle to the copied message

        send_message(sockfd, msgHandle);
        get_message(sockfd, buffer);

    }    
    //close(sockfd); not sure if this is needed, doesn't have sockets closing in beej's guide

return 0;
}  
