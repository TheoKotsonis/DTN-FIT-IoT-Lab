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
static int flag_inter_queue = 0;
static char server_buffer[SERVER_BUFFER_SIZE];
static char inter_buffer[SERVER_BUFFER_SIZE];
static char server_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];
char str[INET6_ADDRSTRLEN];
char** news_interests;
int sleepNode;

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
        int res;
        struct sockaddr_in6 src;
        socklen_t src_len = sizeof(struct sockaddr_in6);
        if ((res = recvfrom(server_socket, server_buffer, sizeof(server_buffer), 0,
                            (struct sockaddr *)&src, &src_len)) < 0 ) {
            printf("Error on receive\n");
        }
        else if (res == 0) {
            printf("Peer did shut down\n");
        }
	else if ( sleepNode == 1 ) {
		memset(server_buffer, 0, strlen(server_buffer));
	}
	else if ( strlen(str) != 0 ) {
		continue;
	}
        else {
	    printf("got '%s' from %s and port: %d\n", server_buffer,
	   inet_ntop(AF_INET6, &src.sin6_addr, str, INET6_ADDRSTRLEN),ntohs(src.sin6_port));
	   
        }
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
            printf("Success: send %s to %s:%u\n", data, addr_str, port);
	    flag_inter_queue = 1;
        }

        usleep(10000);
    close(s);
}

static void udp_send_interest(char *addr_str, char *port_str, char *data)
{
    struct sockaddr_in6 src, dst;
    size_t data_len = strlen(data);
    uint16_t port;
    int i,s,res,result;
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
            printf("Success: send %s to %s:%u\n", data, addr_str, port);
        }

	result = atoi(data);
	if (result > 0 && flag_inter_queue == 1) {
		for(i=0; i< result; i++) {
			socklen_t src_len = sizeof(struct sockaddr_in6);
			if ((res = recvfrom(s, inter_buffer, sizeof(inter_buffer), 0,
		                    (struct sockaddr *)&src, &src_len)) < 0 ) {
		    		printf("Error on receive\n");
			}
			else if (res == 0) {
		    		printf("Peer did shut down\n");
			}
			else {
			    printf("data\n");
			    if(inter_buffer != '\0') {
			    	printf("got '%s' \n", inter_buffer);
			    	strcpy(news_interests[i], inter_buffer);
				memset(inter_buffer, 0, strlen(inter_buffer));
			    }
        		}
		}
	}

    close(s);
}

static int udp_start_server(char *port_str)
{
    /* start server (which means registering pktdump for the chosen port) */
    if (thread_create(server_stack, sizeof(server_stack), THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST,
                      _server_thread, port_str, "UDP server") <= KERNEL_PID_UNDEF) {
        server_socket = -1;
        puts("error initializing thread");
        return 1;
    }
    return 0;
}

int mule_client(int argc, char **argv)
{
   	int i,coin_toss,fresh_news,interests,num_news=0,num_interests=0,flag_fresh_news,flag_interests;
	uint32_t sec;
	char* categories[] = { "event", "food", "traffic", "Discount", "Parking" };
	char** news_buffer = NULL;
  	srand(time(NULL));

    	if (argc > 2) {
		printf("Too many arguments.....Try again\n");
        	return 1;
    	}

    	if (strcmp(argv[0], "mule_client") == 0) {
        	if (argc < 2) {
         		printf("You forget the port...Try again\n");
        		return 1;
        	}
		if(udp_start_server("8810") > 0) {
			return 1;
		}
		while(1) {
			coin_toss = rand() % 2;
			sec = (uint32_t)(4 + rand() % 29);
			printf("To kerma einai: %d & o xronos anamonhs einai: %d\n",coin_toss,(int)sec);
			if (coin_toss == 0) {
				sleepNode = 1;
				xtimer_sleep(sec);
			}
			else {
				sleepNode = 0;
				time_t start_t, end_t;
   				double diff_t = 0;
   				time(&start_t);
				
				fresh_news = rand() % 2;
				interests = rand() % 2;
				
				flag_fresh_news = 0;
				flag_interests = 0;

				if(fresh_news == 1) {
					num_news = rand() % 3 + 1;
					printf("O arithmos twn news einai: %d\n",num_news);
					news_buffer = malloc(num_news* sizeof(char *) );
					if(news_buffer == NULL) { 
						perror("Memory full!"); 
						return 1;
				    	}
					for(i=0; i< num_news; i++) {
						news_buffer[i] = malloc(SERVER_BUFFER_SIZE * sizeof(char));
					}
					for(i=0;i < num_news;i++) {
						sprintf(news_buffer[i], "%s/new_%d", categories[ rand() % 5 ], i+1);
						printf("To string einai: %s\n",news_buffer[i]);
					}
				}

				if(interests == 1) {
					num_interests = rand() % 2 + 1;
					printf("O arithmos twn interests einai: %d\n",num_interests);
					news_interests = malloc(num_interests* sizeof(char *) );
					if(news_interests == NULL) { 
						perror("Memory full!"); 
						return 1;
				    	}
					for(i=0; i< num_interests; i++) {
						news_interests[i] = malloc(SERVER_BUFFER_SIZE * sizeof(char));
					}
				}

				while(strlen(server_buffer) == 0) {
					time(&end_t);
   					diff_t = difftime(end_t, start_t);
					if(diff_t <= (double)sec) {
						continue;
					}
					else {
						break;
					}
				}
				if(strlen(server_buffer) > 0 && (diff_t < (double)sec) )  {
					while(diff_t <= (double)sec) {
						time(&end_t);
	   					diff_t = difftime(end_t, start_t);
						if(fresh_news == 1) {
							for(i=0; i< num_news; i++) {
								udp_send(str, argv[1], news_buffer[i]);
							}
							fresh_news = 0;
							flag_fresh_news = 1;
						}
						if(interests == 1) {
								char temp[256];
								sprintf(temp, "%d", num_interests);
								udp_send_interest(str, argv[1], temp);
							interests = 0;
							flag_interests = 1;
						}
					}
				}
				printf("Terminate the client...\n" );
				
				if(flag_fresh_news == 1) {
					for(i=0; i< num_news; i++) {
						free(news_buffer[i]);
    					}
    					free(news_buffer);
					flag_fresh_news = 0;
				}
				if(flag_interests == 1) {
					for(i=0; i< num_interests; i++) {
						free(news_interests[i]);
    					}
    					free(news_interests);
					flag_interests = 0;
				}
				memset(server_buffer, 0, sizeof(server_buffer));
				memset(str, 0, sizeof(str));
			}
		}
    	}    
    	return 1;
}

