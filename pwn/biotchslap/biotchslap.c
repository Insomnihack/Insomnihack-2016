#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT 1070 // the port client will be connecting to 

char name[32];

void handle_biotch(int sockfd)
{
    int i,n = 0;
    char buf[32];

    printf("Alright, %s how much slaps ? ", name);
    n = read(1, buf, 4);
    buf[n-1] = 0;
    n = atoi(buf);
    printf("Sending %d SLAPs\n", n);
    for (i=0; i<n; i++)
    {
    	send(sockfd, "SLAP", 4, 0);
    }
    printf("Checking status\n");
    if ((n = recv(sockfd, buf, 4, 0)) == -1) {
        perror("Malformed message!\n");
        exit(1);
    }
    buf[n] = 0;
    n = atoi(buf);
    if ((n = recv(sockfd, buf, n, 0)) == -1) {
        perror("Bad message!\n");
        exit(1);
    }
    return;
}

int main(void)
{
    int sockfd, portno, n;
    struct sockaddr_in remoteaddr;
    char buf[32];

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Enter your name :");
    n = read(1,name,31);
    name[n-1] = 0;
    printf("Hello %s! Who shall we biotch-slap today ?\n", name);
    printf("Enter IP address : ");
    n = read(1,buf,16);
    buf[n-1] = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        printf("ERROR opening socket");
        exit(-2);
    }

    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_addr.s_addr = inet_addr(buf);
    remoteaddr.sin_port = htons(PORT);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr)) < 0)
    {
      printf("ERROR connecting to remote biotch %s\n", buf);
      exit(-1);
    }

    handle_biotch(sockfd);

    printf("Hope you liked it! Next version will feature a webcam to capture those treasure moments!\n");
    close(sockfd);
    return 0;
}
