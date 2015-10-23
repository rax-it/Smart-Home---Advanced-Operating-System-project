//SERVER
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

int numRegDevices = 0, count = 1, numberofsensor = 0;
char *stype, *sipaddress, *sport, *sarea, *status;
pthread_t thread_id;
pthread_mutex_t lock;

struct device {
char ipAddress[20];
char portNo[5];
int areaCode;
int sockFD;
int id;
char type[20];
int status;
int currValue;
struct device *next;
struct device *prev;
} *start, *end, *temp;

void initialize() {
start, end = NULL;
}

char buffer[256];
int *item_id;

/* Register -- store values of device in structure*/
void registerDevice (int socketfd) {
if (numRegDevices <= 0) {
	//printf("registerDevice START\n");
	initialize();

	start = (struct device *) malloc(1*sizeof(struct device));
	strcpy(start->ipAddress, sipaddress);
	strcpy(start->portNo, sport);
	start->areaCode = atoi(sarea);
	start->sockFD = socketfd;
	start->id = count;
	strcpy(start->type, stype);
	if (strcmp(stype, "sensor") == 0) {
		start->status = 1;
	} else {
		start->status = 0;
	}
	start->next = NULL;
	start->prev = NULL;
	end = start;
	count++;
	numRegDevices++;
	} 
else {
	//printf("registerDevice TEMP\n");
	
	temp = (struct device *) malloc(1*sizeof(struct device));
	end->next = temp;
	strcpy(temp->ipAddress, sipaddress);
	strcpy(temp->portNo, sport);
	temp->areaCode = atoi(sarea);
	temp->sockFD = socketfd;
	temp->id = count;
	strcpy(temp->type, stype);
	if (strcmp(stype, "sensor") == 0) {
		temp->status = 1;
	} else {
		temp->status = 0;
	}
	temp->next = NULL;
	temp->prev = end;
	end = temp;
	count++;
	numRegDevices++;
	}
write(socketfd,"Registered",15);
}

/* Remove a device*/
void removeDevice(int ac) {
pthread_mutex_lock(&lock);
struct device *ptr = start;
while(ptr!=NULL){
if (ptr->areaCode == ac) {
 ptr->prev->next = ptr->next;
 ptr->next->prev = ptr->prev;
 free(ptr);
 break;
}
ptr = ptr->next;
}
pthread_mutex_unlock(&lock);
}

void switchSmartDevice(int areacode, int flag) {
	struct device *pointer = start;
	while(pointer != NULL)
	{
		if((strcmp(pointer->type, "smartDevice") == 0) && pointer->areaCode == areacode) 
		{
			if(pointer->status == flag) {return;}
			if (flag == 1) {
			pointer->status = 1;
			write(pointer->sockFD,"Type:Switch;Action:on",22);
			} else if(flag == 0) {
			pointer->status = 0;
			write(pointer->sockFD,"Type:Switch;Action:off",23);
			}
		break;
		}
		pointer = pointer->next;
	}
}

/* Printing current state of all devices*/
void printStatus() {
struct device *ptr = start;
while(ptr!=NULL){
	if(strcmp(ptr->type, "sensor") == 0 ) {
	printf("%d",ptr->id);
	printf("----");
	printf("%s:%s", ptr->ipAddress, ptr->portNo);
	printf("----");
	printf("%s", ptr->type);
	printf("----");
	printf("%d", ptr->areaCode);
	printf("----");
	printf("%d\n", ptr->currValue);
	//printf("numRegDevices: %d\n",numRegDevices);
	ptr = ptr->next;
	}
	else if(strcmp(ptr->type, "smartDevice") == 0 ) {
        printf("%d",ptr->id);
        printf("----");
        printf("%s:%s", ptr->ipAddress, ptr->portNo);
        printf("----");
        printf("%s", ptr->type);
        printf("----");
        printf("%d", ptr->areaCode);
        printf("----");
	if(ptr->status == 1) {
        printf("on\n");
	}
	else {
	printf("off\n");
	}
        //printf("numRegDevices: %d\n",numRegDevices);
        ptr = ptr->next;
        }

}
}

// Tokenize incoming message
void handleMessage(char *message, int sockfd) {
int dsock,n;
if (strstr(message, "register") != NULL) {
	char *str = strtok(message, ";");
	str = strtok(NULL, ";");
	str = strtok(str,":");
	str = strtok(NULL,":");
	stype = strtok(str, "-");//sensor
	//printf("Type : %s\n",stype);
	sipaddress = strtok(NULL, "-");//ipaddress
	//printf("Sipaddress : %s\n",sipaddress);
	sport = strtok(NULL, "-");//port
	//printf("Sport : %s\n",sport);
	sarea = strtok(NULL, "-");//area
	//printf("Area : %s\n",sarea);
	// Registering device
	registerDevice(sockfd);
	//printStatus();
    } 
else if (strstr(message, "currValue") != NULL) {
	//printf("Inside currValue.\n");
	
	char *str = strtok(message, ";");
	str = strtok(NULL, ";");
	str = strtok(str,":");
	str = strtok(NULL, ":");
	//printf("currValue : %s\n",str);
	// Flushing the number
        numberofsensor = 0;
		int areacode;
		int currValue;
	struct device *ptr = start;
	while(ptr != NULL)
	{
		if(ptr->sockFD == sockfd){
			if(ptr->currValue == atoi(str)){
			//printf("Inside Equal\n");
			return;
			//printf("After return\n");
			}
			ptr->currValue = atoi(str);
			areacode = ptr->areaCode;
			currValue = ptr->currValue;
			break;
		}
		ptr = ptr->next;
	}
	struct device *pointer = start;
	if (currValue < 32) {
		switchSmartDevice(areacode, 1);
	} else if (currValue > 34) {
		int flag = 1;
		while(pointer != NULL)
		{
			if(pointer->areaCode == areacode && strcmp(pointer->type, "sensor") == 0 && pointer->status == 1) {
				if (pointer->currValue <=34 ) {
					flag = 0;
					break;
				} 
			}
			pointer = pointer->next;
		}
		if (flag == 1) {
			switchSmartDevice(areacode, 0);
		}
		
	}
	
    }

else if(strstr(message, "currState") != NULL) {
	char *str = strtok(message, ";");
	str = strtok(NULL, ";");
	str = strtok(str,":");
	str = strtok(NULL,":");
	printf("Value : %s\n",str);
    }

   printf("--------------------------------------------------------\n");
   printStatus();
   printf("--------------------------------------------------------\n");

}

void *read_input(void *socket)
{
	int n, read_size;
        int sock = *(int *)socket;

   	bzero(buffer,256);

   while ((n = read(sock,buffer,255 )) > 1)
   {

   //printf("Register N:%d\n",n);

   if (n < 0)
      {
      perror("ERROR reading from socket");
      exit(1);
      }
   // printf("Here is the message: %s\n",buffer);

   // Parsing input string
   pthread_mutex_lock(&lock);
   handleMessage(buffer, sock);
   pthread_mutex_unlock(&lock);
   /*printf("--------------------------------------------------------\n");
   printStatus();
   printf("--------------------------------------------------------\n");
   */
	/* Write a response to the client */
   n = write(sock,"I got your message",20);
   bzero(buffer,256);
   if (n < 0)
      {
      perror("ERROR writing to socket");
      //exit(1);
      }
   }
}


int main( int argc, char *argv[] )
{
	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
   int sockfd, newsockfd, portno, clilen;
   struct sockaddr_in serv_addr, cli_addr;
   FILE *config;
   char config_read[100], file_name[30] = "./GatewayConfigurationFile.txt";
   char *ipaddress, *port, *token;
   int n, processid;
   int sockopt = 1;
  
   /* Dynamically allocating memory to item_id*/
   item_id = (int *) malloc(sizeof(int));
   
   /* Opening gateway configuration file*/
   config = fopen(file_name,"r");
   if(config == NULL)
   {
	perror("ERROR opening configuration file");
	exit(1);
   }

   while ( fgets (config_read , 100 , config) != NULL ) {
        //printf("Config file: %s",config_read);       
   }

   /* Tokening the stream on :*/
        token =  strtok(config_read, ":");
	//for(int i=0;i<2;i++){
        //while (token != NULL){
        //printf("Token1: %s\n", token);
	ipaddress = token;
	token = strtok(NULL, ":");
	//printf("Token2: %s\n", token);
	port = token;
	token = strtok(NULL, ":");
        //}

   portno = atoi(port);
   printf("Port number: %d\n",portno);
   printf("IP address: %s\n",ipaddress);

   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0)
      {
      perror("ERROR opening socket");
      exit(1);
      }
   
   /* To allow immediate reuse of port*/
   setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int));

   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));   
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
      {
      perror("ERROR on binding");
      exit(1);
      }
      
   /* Now start listening for the clients, here process will
   * go in sleep mode and will wait for the incoming connection
   */
   
   listen(sockfd,10);
   clilen = sizeof(cli_addr);

   printf("Server listening on port no: %d\n",portno);
   
   /* Accept actual connection from the client */
   while(1)
   {

   newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
   if (newsockfd < 0)
      {
      perror("ERROR on accept");
      exit(1);
      }
      
   if( pthread_create(&thread_id, NULL, read_input, (void*) &newsockfd) < 0)
	{
		perror("Unable to create thread");
		exit(1);
	}
  
   }
}

