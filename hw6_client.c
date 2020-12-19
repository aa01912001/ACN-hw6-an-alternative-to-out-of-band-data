#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    int sockfd, listenfd, connfd;
    int cookie;
    struct sockaddr_in  servaddr, servaddr2; // servaddr2 is for listenfd
    char message[100] = {};
    char receiveMessage[100] = {};
    char receiveControlMessage[100] = {};

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // create socket
        printf("create socket error\n");
    }

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) { //create a socket for listening
        printf("create socket error\n");
        return 1;
    }

    printf("sockfd: %d, listenfd: %d\n", sockfd, listenfd);

    memset(&servaddr, 0, sizeof(servaddr)); // clean servaddr
    memset(&servaddr2, 0, sizeof(servaddr2)); // clean servaddr2


    servaddr.sin_family = AF_INET; // for IPv4
    servaddr.sin_port = htons(8888); // destination port
    servaddr.sin_addr.s_addr = inet_addr("10.0.0.1"); // destination ipv4

    servaddr2.sin_family = AF_INET; // FOR IPv4
    servaddr2.sin_port = htons(0); // destination port
    servaddr2.sin_addr.s_addr = htonl(INADDR_ANY); // receive messages which comes from different interface


    if(bind(listenfd, (struct sockaddr*)&servaddr2, sizeof(servaddr2)) == -1) { // bind socket with client socket pair
        printf("bind socket error\n");
        return 0;
    }

    if(listen(listenfd, 10) == -1){ // listen for socket connection
        printf("listen socket error\n");
        return 0;
    }

    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1){ // connect to server
        printf("Connection error\n");
    }

    struct sockaddr_in my_addr;
    char myIP[16];
	unsigned int myPort;

	// Get my listenfd ip address and port
	bzero(&my_addr, sizeof(my_addr));
	int len = sizeof(my_addr);
	getsockname(listenfd, (struct sockaddr *) &my_addr, &len);
	myPort = ntohs(my_addr.sin_port);
	printf("listenfd port : %u\n", myPort);
	char myPortStr[5];
	sprintf(myPortStr, "%d", myPort);


    printf("%d send %s to server\n",  sockfd, myPortStr);
    send(sockfd, myPortStr, sizeof(myPortStr), 0); // send myPort to server

    recv(sockfd, receiveMessage, sizeof(receiveMessage),0); // receive message from server
    printf("%d receives %s\n", sockfd, receiveMessage);


    sprintf(message, "%d", (int)time(NULL));
    printf("%d send %s to server\n",  sockfd, message);
    send(sockfd, message, sizeof(message), 0); // send message to server

    recv(sockfd, receiveMessage, sizeof(receiveMessage),0); // receive message from server
    printf("%d receives %s\n", sockfd, receiveMessage);

    printf("==============wait for server===================\n");
    struct sockaddr_in clientaddr;
    unsigned int clientaddrlen = sizeof(clientaddr);
    connfd = accept(listenfd, (struct sockaddr*) &clientaddr, &clientaddrlen);
    recv(connfd, receiveControlMessage, sizeof(receiveControlMessage),0); // receive message from server

    printf("%d receives %s\n", listenfd, receiveControlMessage);
    if(strncmp(receiveControlMessage, message, sizeof(message)) == 0) {
        printf("connect priority socket successfully. \n");
    }else {
        printf("connect priority socket fail. \n");
        close(sockfd);
        close(listenfd);
        return 0;
    }

    char datatype[10];
    while(1) { // transfer normal or priority data
        printf("Please enter the next data type(normal/priority): ");
        scanf("%s", datatype);
        if(strncmp(datatype, "exit", sizeof(datatype)) == 0) {
            close(sockfd);
            close(listenfd);
            return 0;
        }else if(strncmp(datatype, "normal", sizeof(datatype)) == 0) {
            printf("message: ");
            char normalMessage[100];
            scanf("%s", normalMessage);
            send(sockfd, normalMessage, sizeof(normalMessage), 0); // send normal message to server
            recv(sockfd, receiveMessage, sizeof(receiveMessage),0); // receive message from server
            printf("%d receives %s\n", sockfd, receiveMessage);
        }else if(strncmp(datatype, "priority", sizeof(datatype)) == 0) {
            printf("message: ");
            char priorityMessage[100];
            scanf("%s", priorityMessage);
            send(connfd, priorityMessage, sizeof(priorityMessage), 0); // send priority message to server
            recv(connfd, receiveControlMessage, sizeof(receiveControlMessage),0); // receive message from server
            printf("%d receives %s\n", listenfd, receiveControlMessage);
        }
    }

    close(connfd);
    close(sockfd);
    close(listenfd);

	return 0;
}
