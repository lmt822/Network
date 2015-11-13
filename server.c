/*
 * Mengtian Li
 * server.c
 * 02/07/2015
 * Acknowledgement: http://www.linuxhowtos.org/C_C++/socket.html
 * port no. 9520+
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*
 * struct to hold the information for each visitor
 */
typedef struct visitor_info{
    char client_IP[255];
    char client_host[255];
    char user_agent[255];
    int vistor_no;
} visitor_info;
/*
 * protect from script injection
 */
char* protect_buffer(char* buffer);
/*
 * get the user agent information from buffer
 */
char* get_user_agent(char* buffer);
/*
 * store the visitors information to the table
 */
void write_to_visitor(visitor_info* visitor_table, char* ip_address, 
		     char* user_agent, char* host_name);
/*
 * send the 10 recent visitors' information as response
 */
void print_table(visitor_info* visitor_table, int newsockfd);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    int n;
    char buffer[1000], cli_dec[1000];
    struct sockaddr_in serv_addr, cli_addr;
    /*
     * initialize the table
     */ 
    visitor_info* visitor_table = malloc(10 * sizeof(visitor_info));
    int i = 0;
    for (i = 0; i < 10; i++){
	strcpy(visitor_table[i].client_IP, "empty");
	strcpy(visitor_table[i].client_host, "empty");
	strcpy(visitor_table[i].user_agent, "empty");
	visitor_table[i].vistor_no = i;	
    }
    if (argc < 2) {
	fprintf(stderr, "ERROR, no port provided\n");
	exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);	
    if (sockfd < 0)
	error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
    listen(sockfd,5);
    while(1) {
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     	if (newsockfd < 0) 
	    error("ERROR on accept");
	bzero(buffer,999);
     	n = read(newsockfd,buffer,999);
     	if (n < 0) error("ERROR reading from socket");
	char* secure_buf;
	secure_buf = protect_buffer(buffer);
	/*
 	 * get client ip address
 	 */
	inet_ntop(AF_INET, (void *)&(cli_addr.sin_addr), cli_dec, 1000);
	/*
 	 * get client host name
 	 */ 
	char host_name[1024];
	char service[20];
	if (getnameinfo((struct sockaddr *)&cli_addr, sizeof(cli_addr), 
			host_name,sizeof(host_name), service, sizeof(service), 
			0) != 0){
	    error("ERROR on hostname or service");
	}  
	char* user_agent = get_user_agent(secure_buf);
	n = write(newsockfd,"HTTP/1.1 200 OK\nContent-type: text/html\n\n", 41);
	/*
 	 * determine GET request
 	 */ 
	if (secure_buf[5] == 'a' || secure_buf[5] == ' ')
  	    write_to_visitor(visitor_table, cli_dec, user_agent, host_name);
    	if (secure_buf[5] == 'a' && secure_buf[6] == 'b' && 
		secure_buf[7] == 'o' && secure_buf[8] == 'u' && 
		secure_buf[9] == 't') {	
	    n = write(newsockfd,"<html><body>Thank you!</body></html>", 36);
	    if(n < 0) {
		error("ERROR on writing to socket");
	    }
	}else if(secure_buf[5] == ' ') {
	    print_table(visitor_table, newsockfd);
	}else {
	    n = write(newsockfd,"<html><body>No page found</body></html>", 39);
	    if(n < 0) {
		error("ERROR on writing to socket");
	    }		
	}
	    close(newsockfd);
    }


}

char *protect_buffer(char* buffer)
{
	char* to_rtn = malloc(sizeof(char*) * 1000);
	int i = 0;
	while(i < 1000) {
	    if(buffer[i] == '>') {
		strcat(to_rtn, "&gt;");
		i = i + 4;
	    }else if(buffer[i] == '<') {
		strcat(to_rtn, "&lt;");
		i = i + 4;
	    }else if(buffer[i] == '"') {
		strcat(to_rtn, "&quot;");
		i = i + 6;
	    }else {
		char temp[2];
		temp[0] = buffer[i];
		temp[1] = '\0';
	    	strcat(to_rtn, temp);
		i = i + 1;
	    }
	}
	return to_rtn;	
}

char* get_user_agent(char* buffer)
{
    char* start = strstr(buffer, "User-Agent: ");
    char* user_agent = malloc(sizeof(char*) * 1000);
    user_agent = strsep(&start, "\n");
    return user_agent;	
}
void write_to_visitor(visitor_info* visitor_table, char* ip_address, 
			char* user_agent, char* host_name)
{
    int i = 0;
    for(i = 0; i < 10; i++) {
	if(strcmp(visitor_table[i].client_IP, "empty") == 0){
	    strcpy(visitor_table[i].client_IP, ip_address);
	    strcpy(visitor_table[i].client_host, host_name);
	    strcpy(visitor_table[i].user_agent,  user_agent);
	    return;
	}
    }
    i = 0;
    for(i = 0; i < 9; i++){
	strcpy(visitor_table[i].client_IP, visitor_table[i+1].client_IP);
	strcpy(visitor_table[i].client_host, visitor_table[i+1].client_host);
	strcpy(visitor_table[i].user_agent, visitor_table[i+1].user_agent);
    }
    strcpy(visitor_table[9].client_IP, ip_address);
    strcpy(visitor_table[9].client_host, host_name);
    strcpy(visitor_table[9].user_agent, user_agent);
    return;
}
void print_table(visitor_info* visitor_table, int newsockfd)
{
    int n, i;
    n = write(newsockfd, "<html><body>", 12);
    if (n < 0) error("ERROR on writing\n");
    char* tag_ip_addr;
    strcpy(tag_ip_addr, "IP Address: ");
    char* tag_host;
    strcpy(tag_host, " Host name: ");
    n = write(newsockfd, "10 Recent visitors<br>", 22);  
    if (n < 0) error("ERROR on writing\n");
    for (i = 0; i < 10; i++) {
	n = write(newsockfd, tag_ip_addr, strlen(tag_ip_addr));
    	if (n < 0) error("ERROR on writing\n");
	n = write(newsockfd, visitor_table[i].client_IP, 
				strlen(visitor_table[i].client_IP));
    	if (n < 0) error("ERROR on writing\n");
	n = write(newsockfd, tag_host, strlen(tag_host));
	if (n < 0) error("ERROR on writing\n");
	 n = write(newsockfd, visitor_table[i].client_host, 
				strlen(visitor_table[i].client_host));
	if (n < 0) error("ERROR on writing\n");
	n = write(newsockfd, " ", 1);
	if (n < 0) error("ERROR on writing\n");
	n = write(newsockfd, visitor_table[i].user_agent, 
				strlen(visitor_table[i].user_agent));
	if (n < 0) error("ERROR on writing\n");
	n = write(newsockfd,"<br>",4);
	if (n < 0) error("ERROR on writing\n");
    }
    n = write(newsockfd, "</body></html>", 14);
    if (n < 0) error("ERROR on writing\n");
}
