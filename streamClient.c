#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT "2330" //the PORT we will be connecting to
#define MAXDATASIZE 10000 //max number of bytes we can get at once

//get sockaddr
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo;
    struct addrinfo *p;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }
    printf("client: connect to host [%s]...\n", argv[1]);

    //set hints struct to 0 vals for assurance
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    //loop through all the results and connect to the first we can
    int gai;
    if ((gai = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, 
                p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        

        inet_ntop(p->ai_family, 
            get_in_addr((struct sockaddr *)(p->ai_addr)),
             s, sizeof s);
        printf("client: attempting to connect to %s\n", s);

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    printf("client: connected to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure

    numbytes = 0;
    //lets make client send data instead of just receiving
    char buffer[1000000];
    memset(buffer, 'a', sizeof(buffer)); // set outbound buffer to 1M a's
    write(sockfd, buffer, sizeof(buffer));
    printf("client: sent %ld bytes to server\n", sizeof(buffer));

    shutdown(sockfd, SHUT_WR); //close the write side of the socket to signal we're done sending data

    //drain the read buffer before shutting down the write side of the socket
    int res;
    for(;;) {
        res=read(sockfd, buffer, 4000);
        if(res < 0) {
            perror("reading");
            exit(1);
        }
        printf("client: read %d bytes from server\n", res);
        if(!res)
            break;
    }
    return 0;
}
