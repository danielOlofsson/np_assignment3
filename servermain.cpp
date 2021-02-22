#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h> 


// Included to get the support library
#include <calcLib.h>


#define DEBUG


void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

using namespace std;


int main(int argc, char *argv[]){
  
  if(argc!=2)
  {
    printf("(ip argument missing) Usage; %s <ip>:<port> \n", argv[0]);
    exit(1);
  }

  /*
    Read first input, assumes <ip>:<port> syntax, convert into one string (Desthost) and one integer (port). 
    Atm, works only on dotted notation, i.e. IPv4 and DNS. IPv6 does not work if its using ':'. 
  */
  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);
  // *Desthost now points to a sting holding whatever came before the delimiter, ':'.
  // *Dstport points to whatever string came after the delimiter. 

  /* Do magic */
  int port=atoi(Destport);
#ifdef DEBUG  
  printf("Host %s, and port %d.\n",Desthost,port);
#endif


  int serverSocket;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  int yes=1; 
  int recvedValue;
  int backlog = 5;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((recvedValue = getaddrinfo(Desthost, Destport, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(recvedValue));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((serverSocket = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) 
    {
      perror("server: socket");
      continue;
    }

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1) {
      close(serverSocket);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(serverSocket, backlog) == -1) {
    perror("listen");
    exit(1);
  }

  printf("server: waiting for connections BINJER...\n");

  sin_size = sizeof(sin_size);

  int klient_socket;
  char minBuffer[1000];
  int dL, sL;
  while(1)
  {
  
    klient_socket = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
    if( klient_socket == -1)
    {
      perror("Accept");
      continue;
    }

    printf("klient ansluten!!!\n");
    memset(&minBuffer,0,1000);
    dL = recv(klient_socket,&minBuffer, sizeof(minBuffer), 0);
    while(1)
    {

  
      if(dL == -1)
      {
        perror("problem med klient.\n");
        break;

      }
      else if( dL == 0)
      {
        printf(" fick 0 bytes. \n");
        break;
      }
      else
      {
        printf("%s [%d]\n", minBuffer, dL);
      }
      sL = send(klient_socket, &minBuffer, strlen(minBuffer), 0);
      if(sL == -1)
      {
        printf("-1 på send\n");
        break;
      }
      else if(sL == 0)
      {
        printf("0 på send\n");
      }
      else
      {
        printf("allt normalt\n");
      }
    } 
  }
}