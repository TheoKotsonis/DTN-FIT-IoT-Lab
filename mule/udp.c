#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "thread.h"
#include "xtimer.h"

#define SERVER_MSG_QUEUE_SIZE   (8)
#define SERVER_BUFFER_SIZE      (256)

static int server_socket = -1;
static char server_buffer[SERVER_BUFFER_SIZE];
static char server_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];
static char **news;
static int ccount = -1;
static int rear = - 1;
static int front = - 1;

static void *_server_thread(void *args)
{
    struct sockaddr_in6 server_addr;
    uint16_t port;
    msg_init_queue(server_msg_queue, SERVER_MSG_QUEUE_SIZE);
    server_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    /* parse port */
    port = (uint16_t)atoi((char *)args);
    if (port == 0) {
        puts("Error: invalid port specified");
        return NULL;
    }
    server_addr.sin6_family = AF_INET6;
    memset(&server_addr.sin6_addr, 0, sizeof(server_addr.sin6_addr));
    server_addr.sin6_port = htons(port);
    if (server_socket < 0) {
        puts("error initializing socket");
        server_socket = 0;
        return NULL;
    }
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        server_socket = -1;
        puts("error binding socket");
        return NULL;
    }
    printf("Success: started UDP server on port %" PRIu16 "\n", port);
    
    while (1) {
        int res,rdm,i,result;
        struct sockaddr_in6 src;
        socklen_t src_len = sizeof(struct sockaddr_in6);
        if ((res = recvfrom(server_socket, server_buffer, sizeof(server_buffer), 0,
                            (struct sockaddr *)&src, &src_len)) < 0) {
            puts("Error on receive");
        }
        else if (res == 0) {
            puts("Peer did shut down");
        }
        else {
            printf("Received data: ");
            puts(server_buffer);
        }
	
	result = atoi(server_buffer);
	if (result > 0 && ccount >= 0 ) {
		for(i=0; i< result; i++) {
			if(result > ccount + 1) {
				if(ccount > 99) {
					ccount = 100;
					rdm = rand() % ccount;
				}
				else if(ccount == 0){
					rdm = 0;
				}
				else
					rdm = rand() % ccount;
			}
			else {
				rdm = rand() % ccount;
			}

			if (sendto(server_socket, news[rdm], strlen(news[rdm]), 0, (struct sockaddr *)&src, src_len) < 0) {
		    		printf("could not send\n");
			}
			else {
		    		printf("Success: send message to %s\n", news[rdm]);
			}
		}
	}
	else if(result > 0) {
		continue;
	}
	else {
		if (rear == 100 - 1) {
			printf("O PINAKAS FOULARE....PAME APO THN ARXH\n");
			rear = front;
		}
		if (front == -1)
			front = 0;
		rear = rear + 1;
		ccount = ccount + 1;
		strcpy(news[rear], server_buffer);
		printf("News from queue is : %s\n", news[rear]);
	}
	memset(server_buffer, 0, sizeof(server_buffer));
    }
    return NULL;
}

static void udp_send(char *addr_str, char *port_str, char *data)
{
    struct sockaddr_in6 src, dst;
    size_t data_len = strlen(data);
    uint16_t port;
    int s;
    src.sin6_family = AF_INET6;
    dst.sin6_family = AF_INET6;
    memset(&src.sin6_addr, 0, sizeof(src.sin6_addr));
    /* parse destination address */
    if (inet_pton(AF_INET6, addr_str, &dst.sin6_addr) != 1) {
        puts("Error: unable to parse destination address");
    }
    /* parse port */
    port = (uint16_t)atoi(port_str);
    dst.sin6_port = htons(port);
    src.sin6_port = htons(port);
    s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) {
        puts("error initializing socket");
    }

    if (sendto(s, data, data_len, 0, (struct sockaddr *)&dst, sizeof(dst)) < 0) {
	puts("could not send");
    }
    else {
	printf("Success: send %u byte to %s:%u\n", (unsigned)data_len, addr_str, port);
    }
    usleep(1000000);

    close(s);
}

static int udp_start_server(char *port_str)
{
    /* check if server is already running */
    if (server_socket >= 0) {
        puts("Error: server already running");
        return 1;
    }
    /* start server (which means registering pktdump for the chosen port) */
    if (thread_create(server_stack, sizeof(server_stack), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST, _server_thread, port_str, "UDP server") <= KERNEL_PID_UNDEF) {
        server_socket = -1;
        puts("error initializing thread");
        return 1;
    }
    return 0;
}

int mule_server(int argc, char **argv)
{
	int i;
  	srand(time(NULL));

    	if (argc > 2) {
		printf("Too many arguments.....Try again\n");
        	return 1;
    	}

	news = malloc(100* sizeof(char *) );
	if(news == NULL) { 
		perror("Memory full!"); 
		return 1;
    	}
	for(i=0; i< 100; i++) {
		news[i] = malloc(SERVER_BUFFER_SIZE * sizeof(char));
	}	
    	if (strcmp(argv[0], "mule_server") == 0) {
        	if (argc < 2) {
         		printf("You forget the port...Try again\n");
        		return 1;
        	}
		if(udp_start_server(argv[1]) > 0) {
			return 1;
		}

		time_t start_t, end_t;
    		double diff_t;
    		time(&start_t);
		while(1) {
			time(&end_t);
   			diff_t = difftime(end_t, start_t);
	
			if(diff_t == 1) {
				udp_send("ff02::1", "8810", "Hello...I am server_mule...I have news for you\n");
				time(&start_t);
			}
		}
    	}
	for(int i=0; i< 100; i++) {
		free(news[i]);
    	}
    	free(news);
	return 1;
}

