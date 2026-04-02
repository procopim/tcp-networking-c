# tcp-network-c

To more deeply understand TCP,  I've ventured to learn and test the system calls and underlying structures used to forge a TCP connection between client and server. 

The ultimate goal is to create a server and client and observe 1MB of data being sent between them; from some reading and interacting with content creators like Tsoding on youtube, I expect to find that we will need some config partial to allowing the kernel to clear out network adapter buffers so that data is not "lost".

I am doing this in C to rekindle my intutition with memory management and thinking about variables, values and references to them. This in turn is helping me better understand the more abstracted python implementations of these tools, since much of Python is compiled to C.

The below files can be compiled using gcc on linux, e.g. `gcc -o streamserver ./streamServer.c`

---
#streamServer.c

This is the server side implementation. 

Inline comments are meant to reinforce things I studied/learned on the fly. We have error checking throughout and are spawning child processes to handle incoming socket connections.

`addrinfo` struct does alot of heavy lifting for us so that we dont have to manually stuff info into our socket declarations. I switched to using `write()` from `send()` just to save passing the flags parameter.

---
#streamClient.c

This is the client implementation. We are using `read()` instead of `recv()`. It is reading the entire contents from server and printing out how many bytes. 
I want to test this on a remote machine, not my localhost, and observe whether we lose some bytes in this basic implementation.

---
# Interesting learning - we must drain the buffers!

If on any side of the tcp connection we do not empty read/write buffers, we cannot have our socket close as we instruct using `close()`. 

In our example, `streamServer` accepts a connection from `streamClient` and immediately sends a welcome message; `streamClient` **does not** `read()` the message, and only sends 1MB of data to the `streamServer`. 
Per RFC 1122, a `close()` with data pending in read/write queue, should send a RST to its counterpart. This is exactly what we see:
```
$ ./streamserver
server: listening on 0.0.0.0...
server: waiting for connections...
server: got connection from 18.219.236.160
recv: Connection reset by peer
```

Switching `close()` system call for `shutdown()` does not fair better for `streamServer`; yes `streamClient` terminates and logs that it has asked the kernel to write 1MB to the network card, but `streamServer` still reports an error: `recv: Connection reset by peer`

Once we drain the `read` buffer the kernel is holding for us in `streamClient`, do we see the expected behaviour. We can also combine this practice with `shutdown(sockfd, SHUT_WR)` prior to the read drain; this has `streamClient` send a `FIN` packet to `streamServer`, where the latter comprehends that the socket has been shut by the client as it breaks its loop with nothing further read. 

`streamClient`:
```
...
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
```

`streamServer`:
```
...
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
```
```
$ ./streamclient X.X.X.X
client: connect to host [X.X.X.X]...
client: attempting to connect to X.X.X.X
client: connected to X.X.X.X
client: sent 1000000 bytes to server
client: read 26 bytes from server
client: read 0 bytes from server
$
---
$ ./streamserver
server: listening on 0.0.0.0...
server: waiting for connections...
server: got connection from X.X.X.X
client: server closed connection
message was 1000000 bytes long
$
```

#architecture:
This was tested on AWS ec2 instances in separate VPCs using public IPs; the server allowed the client's connection via SecurityGroup inbound rules on the port specified in the programs. There was en expectation to lose data in the transmission but this was not observed and a test with separate networks should be conducted to vet this phenomenon.
