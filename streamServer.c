/*
A simple stream server that listens for incoming connections 
and sends a welcome message. This is meant to test sending 
1MB of data to the client, to test the performance of the stream server.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>

#define PORT "2330"
#define BACKLOG 10
#define MSG_SIZE 1000000

//get sockaddr
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr); //cast to sockaddr_in ptr, grab sin_addr field
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void){  

    int sockfd, new_fd; // listen on sockfd, new connection on new_fd
    struct addrinfo hints, *servinfo;
    struct addrinfo *p; //pointer used to traverse the linked list of addrinfo structs returned by getaddrinfo()
    struct sockaddr_storage their_addr; // connector's address information; sockaddr_storage is large enough to hold either sockaddr_in or sockaddr_in6
    socklen_t sin_size;
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN]; //char buffer to hold the string form of the client's IP address on ln117
    int yes=1;

    //memset to zero out the hints struct before filling it in
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    int gai;
    if ((gai = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai));
        return 1;
    }

    //loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        //create a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        //set socket options to interact at socket level, reuseaddr allows us reuse of port immediately after server terminates
        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        //bind the socket
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) { //i.e. we went through the whole list and couldn't bind to any
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    char myaddr[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, 
        get_in_addr((struct sockaddr *)(p->ai_addr)),
        myaddr, sizeof myaddr);

    freeaddrinfo(servinfo); // done with this structure

    //listen on the socket
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: listening on %s...\n", myaddr);
    printf("server: waiting for connections...\n");

    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
        continue;
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);

    //send welcome message to client
    write(new_fd, "220 Welcome\r\n", 13);
    char buf[4096];
    int numbytes = 0;
    while(1) {
        int res;
        if ((res = read(new_fd, buf, sizeof(buf)-1)) == -1) {
            perror("recv");
            exit(1);    
        }
        numbytes += res;
        if (!res) {
            printf("server: client closed connection\n");
            break;
        }
    }

    printf("message was %d bytes long\n", numbytes);
    close(new_fd);
    return 0;
}















