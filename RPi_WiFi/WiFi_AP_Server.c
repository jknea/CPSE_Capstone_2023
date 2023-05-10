//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <time.h>
#include <math.h> // compile with -lm to link math library
#include <sys/ioctl.h> 
#include <signal.h>


#define FALSE 					0
#define TRUE 					1
#define PORT 					8888
#define BUFF_SIZE 			8*3
#define LARGE_BUFF_SIZE 	BUFF_SIZE*10


/* Forward Declarations */
void sigint(int sig);
void initWiFi();
void updateSocketSet();		
void handleMasterSocket();
void handleOtherSockets();		
void handleTime();


// number of exchanges (two-way) we wish to have with each client per second
#define fsamp  1
#define dt_us  1000000/fsamp	

int msgcount = 0;
int base_station = 0;


// for computing elapsed time
struct timeval start, end;
long seconds, micros;
clock_t sample_timer;

// for receiving bytes from eth client
int bytes_avail;
uint8_t eth_buf[LARGE_BUFF_SIZE] = {0};

// WiFi related
int opt = TRUE;
int  addrlen , new_socket;
int activity, i , valread , sd;
int max_sd;
struct sockaddr_in address;
fd_set readfds;	//set of socket descriptors
const int MAX_CLIENTS = 1;
int master_socket;
int client_socket[1];  // for Teensy and Base Station
const char* addr = "192.168.5.1";	// the static IP address determined when making RPi a WiFi AP





int main(int argc , char *argv[])
{
	signal(SIGINT, sigint);


	initWiFi();

	gettimeofday(&start, NULL);
	sample_timer = clock();
	// entering infinite connection loop
	while(TRUE)
	{
		updateSocketSet();		
		handleMasterSocket();
		handleOtherSockets();		
//		handleTime();
	}
	return 0;
}



void handleTime()
{
	gettimeofday(&end, NULL);
	seconds = (end.tv_sec - start.tv_sec);
	micros = ((seconds*1000000) + end.tv_usec) - start.tv_usec;

	if (micros > 1000000) {
		printf("1s elapsed...\n");
		gettimeofday(&start, NULL);
	}
	
	usleep(dt_us);
	sample_timer -= dt_us;
}


/*
Clear the socket set, add master and children sockets to set, and wait for activity on one of the sockets.
*/
void updateSocketSet()
{
	//clear the socket set
	FD_ZERO(&readfds);

	//add master socket to set
	FD_SET(master_socket, &readfds);
	max_sd = master_socket;

	//add child sockets to set
	for ( i = 0 ; i < MAX_CLIENTS; i++)
	{
		sd = client_socket[i];					//socket descriptor
		if(sd > 0) { FD_SET( sd , &readfds); } 	//if valid socket descriptor then add to read list
		if(sd > max_sd) { max_sd = sd; }		//highest file descriptor number, need it for the select function
	}

	//wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
	activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
	
	if ((activity < 0) && (errno!=EINTR)) 
	{ 
		printf("select error"); 
	}
}

/*
Process incoming connections on the master socket
*/
void handleMasterSocket()
{
	//If something happened on the master socket , then its an incoming connection
	if (FD_ISSET(master_socket, &readfds))
	{
		if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		//inform user of socket number - used in send and receive commands
		printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
		// TODO: add a read operation here to determine the type of client (base station or pi zero w)
		//add new socket to array of sockets
		for (i = 0; i < MAX_CLIENTS; i++)
		{
			//if position is empty
			if( client_socket[i] == 0 )
			{
				client_socket[i] = new_socket;
				printf("Adding to list of sockets as %d\n" , i);
				break;
			}
		}
	}
}


/*
Process IO operations on non-master sockets
*/
void handleOtherSockets()
{
	//else its some IO operation on some other socket
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		sd = client_socket[i];
		if (FD_ISSET( sd , &readfds))
		{
			ioctl(sd, FIONREAD, &bytes_avail);
// 			printf("%d bytes available for read\n", bytes_avail);
			if (bytes_avail < BUFF_SIZE) {}
			else {
				if (sd != base_station) {
					int good_bytes_avail = bytes_avail - (bytes_avail % BUFF_SIZE);
//					printf("good avail = %d\n", good_bytes_avail);
					int bread = read(sd, eth_buf, 2560);
					if (bread % BUFF_SIZE == 0) {
						for (int j = bytes_avail - BUFF_SIZE; j < BUFF_SIZE; j++) {
							printf("%4u", eth_buf[j]);
						}
					}
					printf("\n");
					bzero(eth_buf, sizeof(eth_buf));
				}
			}
		}
	}
}


void initWiFi()
{
	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		client_socket[i] = 0;
	}

	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(addr);
	address.sin_port = htons( PORT );

	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener on port %d \n", PORT);

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(master_socket, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(address);
	printf("Waiting for connections ...\n");
}


// close the clients
void sigint(int sig)
{
	printf("caught ctrl-c\n");
	for ( int i = 0 ; i < MAX_CLIENTS; i++)
	{
		int sd = client_socket[i];
		close(sd);
	}
	close(master_socket);
	exit(0);
}

