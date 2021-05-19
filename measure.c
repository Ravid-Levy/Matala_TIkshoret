#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include<time.h>

#define SERVER_PORT 5067 //The port that the server listens
#define SIZE 1024

void get_file(int sockfd);
double recive_file_5_times(int listeningSocket);

int main()
{

    signal(SIGPIPE, SIG_IGN); // on linux to prevent crash on closing socket

//=============================================================================================
    /* opening TCP socket: */

    // Open the listening (server) socket
    int listeningSocket = -1;
    if ((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("SERVER: Could not create listening socket\n");
        exit(1);
    }
    else
        printf("SERVER: socket created\n");

    // Reuse the address if the server socket on was closed
    // and remains for 45 seconds in TIME-WAIT state till the final removal.
    int enableReuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0)
    {
        printf("SERVER: setsockopt() failed with error code : %d\n", errno);
        exit(1);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT); //network order

    // Bind the socket to the port with any IP at this port
    if (bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        printf("SERVER: Bind failed with error code : %d\n", errno);
        exit(1);
    }
    printf("SERVER: Bind() success\n");

//=============================================================================================
    /* Listenning to incoming connections: */

    // Make the socket listening; actually mother of all client sockets.
    if (listen(listeningSocket, 500) == -1) //500 is a Maximum size of queue connection requests
                                            //number of concurrent connections
    {
        printf("SERVER: listen() failed with error code : %d", errno);
        close(listeningSocket);
        exit(1);
    }
    //Accept and incoming connection
    printf("SERVER: Waiting for incoming TCP-connections...\n");

//=============================================================================================
    /* Reciveing the file 5 times and measuring: */
    double sumTimes = recive_file_5_times(listeningSocket);
    double avgCubic = sumTimes/5;

//=============================================================================================
    /* Reciveing the file 5 times and measuring: */
    sumTimes = recive_file_5_times(listeningSocket);
    double avgReno = sumTimes/5;

//=============================================================================================
    /* Printing the measured times: */
    printf("\nCubic: average time is %f\n",avgCubic);
    printf("Reno: average time is %f\n",avgReno);

//=============================================================================================
    /* Closing the connection: */
    close(listeningSocket);
    return 0;
}


void get_file(int sockfd)
{
    int n = -1;
    char buffer[SIZE];

    while(n = recv(sockfd, buffer, SIZE, 0) != 0)
    {
        bzero(buffer, SIZE);
    }
    return;
}

double recive_file_5_times(int listeningSocket)
{
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);

    double sumTimes = 0;
    for (int i = 0; i < 5; i++)
    {
        memset(&clientAddress, 0, sizeof(clientAddress));

        // updates the length in each iteration.
        clientAddressLen = sizeof(clientAddress);

        // accept() gets the connection and return the socket of this connection.
        int new_sock = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if(new_sock == -1) {
            printf("listen failed with error code : %d",errno);
            close(listeningSocket);
            exit(0);
        }

        struct timespec t1, t2;
        clock_gettime(CLOCK_REALTIME, &t1);
        get_file(new_sock);
        clock_gettime(CLOCK_REALTIME, &t2);
        double diff = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;
        printf("File recived in %lf seconds\n", diff);
        sumTimes += diff;
        sleep(1);
    }
    return sumTimes;
}