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
#include <iostream>
#include <cstdlib>

// Included to get the support library
#include <calcLib.h>

#define DEBUG

using namespace std;


bool timeoutSend(int& socket)
{
  bool success = true;
  int recivedValue = 0;
  char errorMsg[100] = "ERROR TO\n";
  recivedValue = send(socket, &errorMsg, strlen(errorMsg), 0);
  if(recivedValue < 0)
  {
    perror("Problem sending msg of operation value value\n");
    success = false;
  }
  else
  {
    #ifdef DEBUG
    printf("sent msg: %s  with size: %ld ",errorMsg, strlen(errorMsg));
    #endif
    close(socket);
  }

  return success;
}


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
  #ifdef DEBUG
  int port=atoi(Destport);
  printf("Host %s, and port %d.\n",Desthost,port);
  #endif

  initCalcLib();

  int serverSocket;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  int yes=1; 
  int recvedValue;
  int backlog = 5;

  struct timeval timeout; 
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  double fv1,fv2,fresult;
  int iv1 = 0,iv2 = 0,iresult = 0;
  int recivedIntResult;
  double recivedFloatResutl;
  char msg[1450];


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

    if(setsockopt(serverSocket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0)
    {
      fprintf(stderr,"failed to set socketopt");
      exit(2);
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

  #ifdef DEBUG
  printf("server: waiting for connections\n");
  #endif

  sin_size = sizeof(their_addr);

  int klient_socket;
  char supportedProtocol[] = "TEXT TCP 1.0\n\n";
  char minBuffer[1000];
  int dL, sL;
  while(1)
  {
    klient_socket = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
    if( klient_socket == -1)
    {
      if(errno == EAGAIN)
      {
        #ifdef DEBUG
        printf("Timeout on Accept!\n");
        #endif
        continue;
      }
      perror("Accept Error");
      continue;
    }
    #ifdef DEBUG
    printf("klient ansluten..\n");
    #endif
    memset(&minBuffer,0,sizeof(minBuffer));

    sL = send(klient_socket, &supportedProtocol, strlen(supportedProtocol), 0);
      if(sL == -1)
      {
        printf("-1 på send\n");
        continue;
      }
      else if(sL == 0)
      {
        printf("0 på send\n");
        continue;
      }
      else
      {
        #ifdef DEBUG
        printf("sent msg: %s size = %d\n", supportedProtocol, sL);
        #endif
      }
      
    dL = recv(klient_socket,&minBuffer, sizeof(minBuffer), 0);
    if(dL == -1)
      {
        if(errno == EAGAIN)
        {
          timeoutSend(klient_socket);
        }
        perror("Problem when trying to recive from client\n");
        continue;
      }
      else
      {
        #ifdef DEBUG
        printf("recived msg: %s\n", minBuffer);
        #endif
      }
    if(strcmp(minBuffer, "OK\n") == 0)
    {
      #ifdef DEBUG
      printf("OK RECIVED: %s\n", minBuffer);
      #endif

      char *op=randomType();
      memset(msg, 0,sizeof(msg));

      if(op[0]=='f')
      { /* We got a floating op  */
      #ifdef DEBUG
      printf("Float\t");
      #endif
      fv1=randomFloat();
      fv2=randomFloat();

      if(strcmp(op,"fadd")==0)
      {
        fresult=fv1+fv2;
      } 
      else if (strcmp(op, "fsub")==0)
      {
        fresult=fv1-fv2;
      } 
      else if (strcmp(op, "fmul")==0)
      {
        fresult=fv1*fv2;
      } 
      else if (strcmp(op, "fdiv")==0)
      {
        fresult=fv1/fv2;
      }
      printf("Servers result: %8.8g",fresult);
      sprintf(msg, "%s %8.8g %8.8g\n",op,fv1,fv2);

      } 
      else 
      {
        iv1 = randomInt();
        iv2 = randomInt();

        if(strcmp(op,"add")==0)
        {
          iresult=iv1+iv2;
        } else if (strcmp(op, "sub")==0)
        {
          iresult=iv1-iv2;
        } else if (strcmp(op, "mul")==0)
        {
          iresult=iv1*iv2;
        } else if (strcmp(op, "div")==0)
        {
          iresult=iv1/iv2;
        }
        printf("Servers result: %d",iresult);
        sprintf(msg, "%s %d %d\n",op,iv1,iv2);
      }

      sL = send(klient_socket, &msg, strlen(msg), 0);
      if(sL < 0)
      {
        perror("Problem sending msg of operation value value\n");
        continue;
      }
      else
      {
        #ifdef DEBUG
        printf("sent msg: %s  with size: %d ",msg, sL);
        #endif
      }
      
      memset(&minBuffer,0,sizeof(minBuffer));
      dL = recv(klient_socket,&minBuffer, sizeof(minBuffer), 0);
      if(dL == -1)
      {
        if(errno == EAGAIN)
        {
          timeoutSend(klient_socket);
        }
        perror("Problem when trying to recive from client\n");
        continue;
      }
      else
      {
        #ifdef DEBUG
        printf("recived msg: %s\n", minBuffer);
        #endif
      }
      memset(msg,0,sizeof(msg));
      if(op[0]=='f')
      {
        sscanf(minBuffer,"%lg",&recivedFloatResutl);
        double quotient;
        quotient = abs(recivedFloatResutl - fresult);
        if(quotient < 0.0001)
        {
          strcpy(msg,"OK\n");
        }
        else
        {
          strcpy(msg,"ERROR\n");
        }
      }
      else
      {
        sscanf(minBuffer,"%d",&recivedIntResult);
        
        if(recivedIntResult == iresult)
        {
          strcpy(msg,"OK\n");
        }
        else
        {
          strcpy(msg,"ERROR\n");
        }
      }

      sL = send(klient_socket, &msg, strlen(msg), 0);
      if(sL < 0)
      {
        perror("Problem sending OK or Error msg\n");
        continue;
      }
      else
      {
        #ifdef DEBUG
        printf("sent msg: %s with size: %d ",msg, sL);
        #endif
      }
    }
    else
    {
      printf("NOT OK client will disconnect\n");
    }

  }
}