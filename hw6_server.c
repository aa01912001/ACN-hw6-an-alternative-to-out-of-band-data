#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <poll.h>

#define MAXLINE 4096

int main(int argc, char *argv[])
{
    printf("================waiting for client====================\n");
    int listenfd, connfd , sockfd;
    struct sockaddr_in servaddr, clientaddr, controladdr;
    char buff[MAXLINE];

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) { //create a socket for listening
        printf("create socket error\n");
        return 0;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // create socket
        printf("create socket error\n");
	return 0;
    }

    printf("listenfd: %d, sockfd: %d\n", listenfd, sockfd);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // receive messages which comes from different interface
    servaddr.sin_port = htons(8888); // server port number

    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) { // bind socket with server socket pair
        printf("bind socket error\n");
        return 0;
    }

    if(listen(listenfd, 10) == -1){ // listen for socket connection
        printf("listen socket error\n");
        return 0;
    }

    unsigned int clientaddrlen = sizeof(clientaddr);
    connfd = accept(listenfd, (struct sockaddr*) &clientaddr, &clientaddrlen);
    recv(connfd, buff, MAXLINE, 0); // get client's listenfd port number
    printf("%d receives %s from client\n",listenfd ,buff);

    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&clientaddr;
    struct in_addr ipAddr = pV4Addr->sin_addr; // get client's IPv4
    char str[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

    send(connfd, "ack", sizeof("ack"), 0);

    memset(&controladdr, 0, sizeof(controladdr)); // clean controladdr
    controladdr.sin_family = AF_INET; // for IPv4
    controladdr.sin_port = htons(atoi(buff)); // destination port
    controladdr.sin_addr.s_addr = inet_addr(str); // destination ipv4

    recv(connfd, buff, MAXLINE, 0); // get unique message from client
    printf("%d receives %s from client\n",listenfd ,buff);

    send(connfd, "ack", sizeof("ack"), 0);


    if(connect(sockfd,(struct sockaddr *)&controladdr,sizeof(controladdr)) == -1){ // connect to client
        printf("Connection error\n");
	return 0;
    }

    char message[100];
    strncpy(message, buff, sizeof(buff));
    printf("%d sends %s to client\n", sockfd, message);
    send(sockfd, message, sizeof(message), 0); // send unique message to client


    // specify pollfd
    struct pollfd fds[2];
    fds[0].fd = connfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    fds[1].fd = sockfd;
    fds[1].events = POLLIN;
    fds[1].revents = 0;

    while(1) {
        poll(fds, 2, -1); // poll sockfd and listenfd
        for(int i=0; i<2; i++) {
            if(fds[i].revents & POLLIN) { // if socket has data to read
                if(i == 0) { // if connfd
                    recv(connfd, buff, MAXLINE, 0); // receive normal data from client
                    printf("%d receives %s from client\n",listenfd ,buff);
                    send(connfd, "ack", sizeof("ack"), 0); // send ack to client
                }else if(i == 1) { // id sockfd
                    char receiveMessage[100] = {};
                    recv(sockfd, receiveMessage, sizeof(receiveMessage),0); // receive priority data from client
                    printf("%d receives %s from client\n",sockfd ,receiveMessage);
                    send(sockfd, "ack", sizeof("ack"), 0); // send ack to client
                }

            }
        }

    }

    close(connfd);
    close(listenfd);
    close(sockfd);

	return 0;
}
