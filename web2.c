//Here concurrency is achieved  by creating a new child process which process e each new client while parent continues to accepting new connections.
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>//INADDR_ANY
#include <fcntl.h> 
#define PORT 8005
#define MAXSZ 5000
int i = 0;;
void php_cgi(char* script_path, int fd);
int main()
{
	int sockfd;//to create socket
 	int newsockfd;//to accept connection
  	char ch; 

 	int sz; 
	struct sockaddr_in serverAddress;//server receive on this address
 	struct sockaddr_in clientAddress;//server sends to client on this address
 void process_rec_send(int newsockfd,char  file_name[100]);
 	int n;
	char buffer[MAXSZ];
	int clientAddressLength;
 	int pid;
 	char *ptr;
	char first_line[100];
 	//create socket
 	sockfd=socket(AF_INET,SOCK_STREAM,0);
 	//initialize the socket addresses
 	memset(&serverAddress,0,sizeof(serverAddress));
 	serverAddress.sin_family=AF_INET;
 	serverAddress.sin_addr.s_addr=htonl(INADDR_ANY);
 	serverAddress.sin_port=htons(PORT);

 	//bind the socket with the server address and port
	bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress));

	//listen for connection from client
 	listen(sockfd,5);

 	while(1)
 	{
  		//parent process waiting to accept a new connection
  		printf("\n*****server waiting for new client connection:*****\n");
  		clientAddressLength=sizeof(clientAddress);
  		newsockfd=accept(sockfd,(struct sockaddr*)&clientAddress,&clientAddressLength);
		printf("connected to client: %s\n",inet_ntoa(clientAddress.sin_addr));

		//child process is created for serving each new clients
		//rceive from client
		// read the message from client and copy it in buffer
		bzero(buffer, MAXSZ);
  		read(newsockfd, buffer, sizeof(buffer));
        	// print buffer which contains the client contents
  		printf("From client: %s\t To client : ", buffer);
  		i = 0;
  		while (ch = (first_line[i] = buffer[i]) != '\n')
        		i++;
 		first_line[i] = '\0';
  		ptr = strstr(first_line, " HTTP/");
 		if (ptr == NULL) {
      			printf("NOT HTTP !\n");
    			continue;
  		} else {
     			*ptr = 0;
   			 ptr = NULL;
  		}
  		if (strncmp(first_line, "GET ", 4) == 0) {
     			ptr = first_line + 5;
  		}
  		if (ptr == NULL) {
			printf("Unknown Request ! \n");
     			continue;
  		}
  		else {
     			printf("ptr length %zd\n",strlen(ptr));
     			if (ptr[strlen(ptr) - 1] == '/') {
        			strcat(ptr, "index.html");
     			}
  		} 
  		printf("First Line %s\n",first_line);
  		printf("First ptr %s\n",ptr);
 		strcpy(first_line, ptr); 
  		printf("First Line%s\n",first_line);
  		pid=fork();
  		if(pid==0)//child process rec and send
  		{
     			process_rec_send(newsockfd,first_line);
     			close(newsockfd);//sock is closed BY child 
     			break;
  		}
 		wait(NULL);
 		close(newsockfd);//sock is closed BY PARENT
	 } /* While */
}

void process_rec_send(int newsockfd,char file_name[100]){
	int fd,i,j;
	char msg[MAXSZ];
	memset(msg,'\0',MAXSZ);
	if (strncmp(strchr(file_name,'.')+1,"php",3) == 0) { 
   		php_cgi(file_name, newsockfd);
  		sleep(1);
   		close(newsockfd);
   		exit(1);
	}
	fd = open(file_name,O_RDONLY);
	char line[120];
	printf("fd1%d\n",fd); 
	if (fd == -1) {
  //       perror("r1"); 
       		close(fd);
   		fd = open("err.html",O_RDONLY);
   		printf("fd%d\n",fd); 
	} 
 	int bytesread;
 	i = 0;
 	bytesread = read (fd, &msg[i], 1); 
 	char c = msg[i];
 	while (c != EOF && bytesread > 0) {
   		i++;
   		bytesread = read (fd, &msg[i], 1); 
   		c = msg[i]; 
 	}

 	msg[i]='\0';
 	send(newsockfd, "HTTP/1.1 200 OK\r\n",16,0);
 	send(newsockfd, "Server : Web Server in C\r\n\r\n",25,0);
/*
     send(newsockfd,"HTTP/1.1 200 OK\n",16,0);
     send(newsockfd, "Content-length: 151\n", 19,0); ///here still is a problem mentioned above
     send(newsockfd, "Content-Type: text/html\n\n", 25,0); */
  	i = 0;
  	j = 0;
  	while (msg[i] != '\0' ) {
    		line[j] = msg[i];
   		j++;
    		i++;
      		if ( j == 100 ) {
			send(newsockfd,line,j,0);
           		j = 0;
        	}
  	}
	send(newsockfd,line,j,0);
  	printf("Receive and set:%s\n",msg);
  	close(fd); 
}
/*
 Handles php requests
 */
void php_cgi(char* script_path, int fd) {
 	send(fd, "HTTP/1.1 200 OK\n Server: Web Server in C\n Connection: close\n",63,0);
 	dup2(fd, STDOUT_FILENO);
 	char script[500];
 	strcpy(script, "SCRIPT_FILENAME=");
 	strcat(script, script_path);
 	putenv("GATEWAY_INTERFACE=CGI/1.1");
 	putenv(script);
 	putenv("QUERY_STRING=");
 	putenv("REQUEST_METHOD=GET");
 	putenv("REDIRECT_STATUS=true");
 	putenv("SERVER_PROTOCOL=HTTP/1.1");
 	putenv("REMOTE_HOST=127.0.0.1");
 	execl("/usr/bin/php-cgi", "php-cgi", NULL);
}
