# tcp-network-c

To more deeply understand TCP,  I've ventured to learn and test the system calls and underlying structures used to forge a TCP connection between client and server. 

The ultimate goal is to create a server and client and observe 1MB of data being sent between them; from some reading and interacting with content creators like Tsoding on youtube, I expect to find that we will need some config partial to allowing the kernel to clear out network adapter buffers so that data is not "lost".

I am doing this in C to rekindle my intutition with memory management and thinking about variables, values and references to them. This in turn is helping me better understand the more abstracted python implementations of these tools, since much of Python is compiled to C.

---
#streamServer.c

This is the server side implementation. 

Inline comments are meant to reinforce things I studied/learned on the fly. We have error checking throughout and are spawning child processes to handle incoming socket connections.

`addrinfo` struct does alot of heavy lifting for us so that we dont have to manually stuff info into our socket declarations. 
