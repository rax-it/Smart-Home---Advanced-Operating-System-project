//DEVICE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

/* Current state of device 0 = off, 1 = on */
int currState = 0;
char item_type[15], sipaddress[50], sport[5], sarea[5];
char string[100];

char * prepareMessage(int messageType, char * value) {
	char *str;
        
	switch (messageType) 
	{
		case 0:
		strcpy(string,"Type:register;Action:smartDevice-");
		strcpy(string, strcat(string, sipaddress));
		strcpy(string, strcat(string, "-"));
		strcpy(string, strcat(string, sport));
		strcpy(string, strcat(string, "-"));
		strcpy(string, strcat(string, sarea));
		break;
		case 1:
		strcpy(string, "Type:currState;Action:");
		strcat(string, value);
		break;
	}
	return string;
}

int main(int argc, char *argv[])
{
   int sockfd, portno, n, count = 0;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   FILE *cfile;
   char config[50];
   strcpy(config, argv[1]);
   char gipaddress[50], gport[5];
   char *gtoken, *stoken;
   int  starttime, endtime, value;
   char config_read[250], input_read[250];
   char buffer[256];
   char *reg, *currvalue;
   
   /* Reading configuration file*/
   cfile = fopen (config,"r");
   if (cfile == NULL) {
	perror("ERROR opening config file ");
	exit(1);
   }   
   
   while ( fgets (config_read , 100 , cfile) != NULL ) {
	if(count == 0) {
	   /* Tokening the stream on :*/
	        gtoken =  strtok(config_read, ":");
		strcpy(gipaddress, gtoken);
		gtoken = strtok(NULL, ":");
		strcpy(gport, gtoken);
		gtoken = strtok(NULL, ":");
		count++;
	}
	else if(count ==1) {
		stoken = strtok(config_read, ":");
		strcpy(item_type, stoken);
		stoken = strtok(NULL, ":");
                strcpy(sipaddress, stoken);
                stoken = strtok(NULL, ":");
                strcpy(sport, stoken);
                stoken = strtok(NULL, ":");
                strcpy(sarea, stoken);
                stoken = strtok(NULL, ":");
	}
	
   }
   
   //printf("Gateway IP: %s\n", gipaddress);
   //printf("Gateway port %s\n", gport);
   //printf("Item type %s\n", item_type);
   //printf("Device ip %s\n", sipaddress);
   //printf("Device port %s\n", sport);
   //printf("Area %s\n", sarea);

   fclose(cfile);
  
   /* Prepare register request*/
   reg = prepareMessage(0, NULL);
   //printf("Prepare message: %s\n",reg);
   

   portno = atoi(gport);
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0)
   {
      perror("ERROR opening socket");
      exit(1);
   }
   server = gethostbyname(gipaddress);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
   {
      perror("ERROR connecting");
      exit(1);
   }
   
   /* Now ask for a message from the user, this message
   * will be read by server
   */
   
   bzero(buffer,256);
   //fgets(buffer,255,stdin);
   printf("Message: %s\n", reg);
   strcpy(buffer, reg);

   /* Send message to the server */
   n = write(sockfd, buffer, strlen(buffer));

   if (n < 0)
   {
  	 perror("ERROR writing to socket");
         exit(1);
   }

   
   while(1) {
   	  
	   /* Now read server response */
	   bzero(buffer,256);
	   //printf("Buffer:\n%s", &buffer);
	   n = read(sockfd, buffer, 255);
   	   //printf("N:%d\n",n);
	   if (n < 0)
	   {
	      perror("ERROR reading from socket");
	      exit(1);
	   }
	   else if(n > 0)
	   {
	   // If gateway sends on
	   if(strcmp(buffer, "Type:Switch;Action:on") == 0){
	   	currState = 1;
		printf("Device switched ON\n");
		}
	   else if (strcmp(buffer, "Type:Switch;Action:off") == 0){
                currState = 0;
                printf("Device switched OFF\n");
                }
	   printf("Got message: %s\n",buffer);
		
	   }
	   //sleep(1);
   
   }
   return 0;
}
