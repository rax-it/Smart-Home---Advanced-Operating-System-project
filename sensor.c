#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

int currState;
int interval = 5;
int timeCounter = 1;
char item_type[15], sipaddress[50], sport[5], sarea[5];
char string[100];
FILE *cfile, *ifile;
char config_read[250], input_read[250];
int state = 1;
char value[10];

char * fetchValue() {
	//char value[10];
	while ( fgets (config_read , 100 , ifile) != NULL ) { 
		char *itoken;
		int startTime, endTime;
		itoken = strtok(config_read, ";");
		startTime = atoi(itoken);
		itoken = strtok(NULL, ";");	
		endTime = atoi(itoken);
		itoken = strtok(NULL, ";");
		strcpy(value,itoken);

		if (timeCounter > startTime && timeCounter <= endTime) {
			rewind(ifile);			
			return value;
		}
	} 
	rewind(ifile);
	timeCounter = 1;
	char *value1 = fetchValue();
	return value1;
}

char * extractValue(char *message) {
	char *str = strtok(message, ";");
	str = strtok(NULL, ";");
	str = strtok(str, ":");
	str = strtok(NULL, ":");
	return str;
}

void handleServerMessage(char * message) {
	char * str;
	if (strstr(message, "ERROR") != NULL) {
		state = 0;
		printf("Error while Registering");
	} else if (strstr(message, "Registered") != NULL) {
		state = 1;
		printf("Registered Successfully");
	} else if (strstr(message, "Switch") != NULL) {
		str = extractValue(message);
		if (strcmp(str,"ON") == 0) {
			state = 1; 
		} else {
			state = 0;
		}
	} else if (strstr(message, "setInterval") != NULL) {
		str = extractValue(message);
		interval = atoi(str);
	}
		
}

char * prepareMessage(int messageType, char * value) {
	//printf("prepare message Value: %s\n", value);
	char *str;
        
	switch (messageType) 
	{
		case 0:
  		//printf("22\n");
		strcpy(string,"Type:register;Action:sensor-");
		//printf("33\n");
		strcpy(string, strcat(string, sipaddress));
                //printf("44\n");  
		strcpy(string, strcat(string, "-"));
		strcpy(string, strcat(string, sport));
		strcpy(string, strcat(string, "-"));
		strcpy(string, strcat(string, sarea));
                //printf("Concat string: %s\n",string);
		break;
		case 1:
		str = strcat(str, "Type:currState;Action:");
		char *state = "";
		sprintf(state,"%d",currState);
		str = strcat(str, state);
		break;
		case 2:
		//printf("In case 2\n");
		//*string = NULL;
		strcpy(string, "Type:currValue;Action:");
		strcat(string, value);
		//printf("Case 2 ends, string: %s\n",string);
		break;
	}
	return string;
}

int main(int argc, char *argv[])
{
   int sockfd, portno, n, count = 0;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   char config[50];
   char gipaddress[50], gport[5], *gtoken;
   //char item_type[15], sipaddress[50], sport[5], sarea[5];
   //char input[50] = "./SensorInputFile.txt";
   char *itoken, *stoken;
   int  starttime, endtime, value;
   char buffer[256];
   char *reg, *currvalue;
   //char inputfilename[] = "./";
   char input[50];

   strcpy(config, argv[1]);
   strcpy(input, argv[2]);
   //strcpy(input, strcat(inputfilename, input));
   
   /* Reading configuration file*/
   cfile = fopen (config,"r");
   if (cfile == NULL) {
	perror("ERROR opening config file ");
	exit(1);
   }   
   
   while ( fgets (config_read , 100 , cfile) != NULL ) {
       	//printf("Config file: %s\n", config_read);
	if(count == 0) {
	   /* Tokening the stream on :*/
	        gtoken =  strtok(config_read, ":");
		//for(int i=0;i<2;i++){
	       	//while (token != NULL){
	        //printf("Token1: %s\n", gtoken);
		strcpy(gipaddress, gtoken);
		gtoken = strtok(NULL, ":");
		//printf("Token2: %s\n", gtoken);
		strcpy(gport, gtoken);
		gtoken = strtok(NULL, ":");
		count++;
	}
	else if(count ==1) {
		stoken = strtok(config_read, ":");
		//printf("Token3: %s\n", stoken);
		strcpy(item_type, stoken);
		stoken = strtok(NULL, ":");
		//printf("Token4: %s\n", stoken);
                strcpy(sipaddress, stoken);
                stoken = strtok(NULL, ":");
		//printf("Token5: %s\n", stoken);
                strcpy(sport, stoken);
                stoken = strtok(NULL, ":");
		//printf("Token6: %s\n", stoken);
                strcpy(sarea, stoken);
                stoken = strtok(NULL, ":");
	}
	
   }
   
   //printf("Gateway IP: %s\n", gipaddress);
   //printf("Gateway port %s\n", gport);
   //printf("Token3 %s\n", item_type);
   //printf("Token4 %s\n", sipaddress);
   //printf("Token5 %s\n", sport);
   //printf("Token6 %s\n", sarea);

   fclose(cfile);
  
   /* Reading configuration file*/
   ifile = fopen (input,"r");
   if (ifile == NULL) {
        perror("ERROR opening config file ");
        exit(1);
   }

   /*while ( fgets (input_read , 100 , ifile) != NULL ) {
        printf("Input file: %s", input_read);
   }
   
   
   if (argc <3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
   */

   //fclose(ifile);   

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
   while(1) {
   	//printf("Please enter the message: ");
	  bzero(buffer,256);
	   //fgets(buffer,255,stdin);
	   //printf("Message: %s\n", reg);
	   strcpy(buffer, reg);
   
	   /* Send message to the server */
	   if (state == 1) {
		   n = write(sockfd, buffer, strlen(buffer));
	   }
   
	   if (n < 0)
	   {
	      perror("ERROR writing to socket");
	      exit(1);
	   }
   
	   /* Now read server response */
	   bzero(buffer,256);
	   n = read(sockfd, buffer, 255);
   
	   if (n < 0)
	   {
	      perror("ERROR reading from socket");
	      exit(1);
	   } else {
		   handleServerMessage(buffer);
	   }
	   //printf("printing buffer: %s\n",buffer);
	   sleep(interval);

	   // Prepare Message
	   reg = NULL;
	   char temp[10];
	   strcpy(temp,fetchValue());
	   //printf("Temp: %s\n", temp);
	   reg = prepareMessage(2, temp);
	   timeCounter = timeCounter + interval;
	   printf("CurrValue Message: %s\n",reg);
	   }
  
	   bzero(buffer,256); 
   
   return 0;
}
