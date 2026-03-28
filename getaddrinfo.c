#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main() {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;       //points to the results

    memset(&hints, 0, sizeof hints); //make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     //don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
    hints.ai_flags = AI_CANONNAME;     

    if ((status = getaddrinfo("www.google.com", "80", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }   

    printf("%d\n", servinfo->ai_family); // print the family of the first result
    printf("%d\n", servinfo->ai_socktype); // print the family of the first result
    printf("%s\n", servinfo->ai_canonname); // print the canonical name of the first result
    
    struct addrinfo *p = servinfo->ai_next;
    printf("%p\n", &servinfo->ai_next); // print the address of the next result
    printf("%d\n", p->ai_family); // print the family of the second result
    printf("%s\n", p->ai_addr->sa_data); // print the sockaddr data of the second result

    freeaddrinfo(servinfo); // free the linked list
    return 0;
}