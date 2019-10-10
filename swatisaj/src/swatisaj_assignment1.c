/**
 * @swatisaj_assignment1
 * @author  Swati Sajee Kumar <swatisaj@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //write,read,close
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <arpa/inet.h>

#define MAX_CLIENT_SUPPORTED 6  /* We consider the console as client as well (for server) */
#define YOUR_UBIT_NAME "swatisaj"

char data_buffer_for_console [255];

int monitored_fd_set[MAX_CLIENT_SUPPORTED];



/*Each connected client's intermediate result is 
 * maintained in this client array.*/
int client_result[MAX_CLIENT_SUPPORTED] = {0};

static void
intitiaze_monitor_fd_set(){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++)
        monitored_fd_set[i] = -1;
}


/*Add a new FD to the monitored_fd_set array*/
static void
add_to_monitored_fd_set(int skt_fd){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){

        if(monitored_fd_set[i] != -1)
            continue;
        monitored_fd_set[i] = skt_fd;
        break;
    }
}

/*Remove the FD from monitored_fd_set array*/
static void
remove_from_monitored_fd_set(int skt_fd){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){

        if(monitored_fd_set[i] != skt_fd)
            continue;

        monitored_fd_set[i] = -1;
        break;
    }
}

/* Clone all the FDs in monitored_fd_set array into 
 * fd_set Data structure*/
static void
re_init_readfds(fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);
    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != -1){
            FD_SET(monitored_fd_set[i], fd_set_ptr);
        }
    }
}


/*Get the numerical max value among all FDs which server
 * is monitoring*/

static int
get_max_fd(){

    int i = 0;
    int max = -1;

    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] > max)
            max = monitored_fd_set[i];
    }

    return max;
}

/*int p=0;
void int client_info(int client_port_number, char client_ip_addr){
	char CLIENT_IP[5];
	int CLIENT_PORT[5];
	for(;p< MAX_CLIENT_SUPPORTED;p++){
		CLIENT_IP[p]= client_ip_addr;
		CLIENT_PORT[p]= client_port_number;
		break;
	}
	//return CLIENT_IP, CLIENT_PORT;
} */

/////////////////////////////SERVER//////////////////////////////////////

void SERVER(int user_assigned_port)
{
	int master_sock_tcp_fd =0, addr_len =0, opt=1;
	int comm_socket_fd =0; 
	fd_set readfds;
	int COUNT =0 ;
	
	/*variables to hold server information */
	struct sockaddr_in server_addr, client_addr;
	
    /* Just drain the array of manitored file descriptor socket */
	intitiaze_monitor_fd_set();
	
	int console_stdin_fd =0; /* The input from user */
	int console_stdout_fd = 1;
	/* Step 2 : TCP socket creation */
	
	if ((master_sock_tcp_fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1)
	{
		printf("Socket Connection for Network Failed : ERROR\n");
		exit(1);
	}
	
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port= user_assigned_port;
	server_addr.sin_addr.s_addr= INADDR_ANY;
	
	addr_len = sizeof(struct sockaddr);
	

	if (bind (master_sock_tcp_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))==-1)
	{
		printf("Socket bind failed for Network Input :ERROR\n");
		return;
		
	}
	
	add_to_monitored_fd_set(console_stdin_fd);
	
	
	if(listen(master_sock_tcp_fd,MAX_CLIENT_SUPPORTED)<0)
	{
		printf("Listen to Network Socket Failded\n: ERROR");
		return;
		
	}
	
	add_to_monitored_fd_set(master_sock_tcp_fd);
	
	while(1)
	{
		
		re_init_readfds(&readfds); /*Copy the entire monitored FDs to readfds */
		
		//select system call to check input at all handlers
		select(get_max_fd()+1,&readfds,NULL,NULL,NULL);
		
		if (FD_ISSET(console_stdin_fd,&readfds))
		{
		    char recv_buffer[255];
			gets(recv_buffer);
			fflush(stdin); 
			if(strcmp("AUTHOR",recv_buffer)==0)
				{
				 cse4589_print_and_log("[%s:SUCCESS]\n",recv_buffer);
     			 cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",YOUR_UBIT_NAME );
   				 cse4589_print_and_log("[%s:END]\n",recv_buffer);	
				 }
				 
			else if (strcmp("PORT",recv_buffer)==0)
				{
				cse4589_print_and_log("[PORT:SUCCESS]\n");
				cse4589_print_and_log("PORT:%d\n",user_assigned_port);
				cse4589_print_and_log("[PORT:END]\n");
				}
			else if (strcmp("IP",recv_buffer)==0)
			{
			
				cse4589_print_and_log("[IP:SUCCESS]\n");
					char hostbuffer[256]; 
					char *IPbuffer; 
					struct hostent *host_entry; 
					int hostname; 
										  
					// To retrieve hostname 
					hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
										    										  
					// To retrieve host information 
					host_entry = gethostbyname(hostbuffer); 
																  
					// To convert an Internet network 
				    // address into ASCII string 
					IPbuffer = inet_ntoa(*((struct in_addr*) 
			         host_entry->h_addr_list[0])); 
										    
										    
				printf("IP:%s\n",IPbuffer);				 
				cse4589_print_and_log("[IP:END]\n");
				
			}
			    
			else
			{
				printf("[%s:ERROR]\n",recv_buffer);
			}
		}
		
					/*If data arrives on master socket fd */
		else if (FD_ISSET(master_sock_tcp_fd,&readfds))
		{
			
			comm_socket_fd =accept(master_sock_tcp_fd,(struct sockaddr*)&client_addr,&addr_len);
			
			if(comm_socket_fd <0){
				printf("[New Client Login Failed:ERROR]\n"); 
				exit(0);
			}
			else
			{
				printf("[New Client Logged in :SUCCESS %s,%d]\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port)); // do not change this printf statement to the library one
				char *client_ip_set;
	     		client_ip_set = inet_ntoa(client_addr.sin_addr); // return the IP
								
			}
			
			add_to_monitored_fd_set(comm_socket_fd);
			
			int client_port_number = ntohs(client_addr.sin_port);
		//	char client_ip_addr = inet_ntoa(client_addr.sin_addr); 
		//	client_info(int client_port_number, char client_ip_addr);
			
			
		}
		
		else 
			
		{
			int i=0, comm_socket_fd=-1,n,p; /* monitored_fd_set[0] and [1] is already set by console and master socket since those are compulsory initiation arguments */
			
			
			///////////////////CLIENT 1 ///////////////
			
			
			if(FD_ISSET(monitored_fd_set[2],&readfds))  
			{
					comm_socket_fd = monitored_fd_set[2];
					char data_buffer [1024];
	   		
					
					memset(data_buffer,0,sizeof(data_buffer));
					
					n=read(comm_socket_fd,data_buffer,1024);
    	            if(n<0)
    		              printf("Error in reading from Client 1\n");
    		       else 
    	                  
						  {
						  /*	if (strncmp("SEND",data_buffer, 5)==0){
											char *ptr;
											ptr= &buffer_for_console[0];
								    		
											for(*ptr=buffer_for_console[0];*ptr!=' ' ;ptr++){
												
											}
											ptr++;
											char TO_CLIENT_IP_ADDR[13];
											
											int i=0;
											for(;*ptr!= ' ' ;ptr++){
											   TO_CLIENT_IP_ADDR[i]= *ptr;
												i++;
											}
										//	printf("%s is the IP_address\n",SERVER_IP_ADDRESS); // remove this later
											ptr++;
											int j=0;
											char message[256];
											for(;*ptr!='\0';ptr++){
												message[j]=*ptr;
												j++;   		
											
							                } */
						  	 if(strcmp("LOGOUT",data_buffer)==0)
						  	    { 
						  	    bzero(data_buffer,1024);
						  	    //data_buffer= {[};
						  	    p=write(comm_socket_fd,"[LOGOUT:SUCCESS]\n",strlen("[LOGOUT:SUCCESS]\n"));
						  	    remove_from_monitored_fd_set (comm_socket_fd);
						  	    
						  	    
							    }
						  	else 
						     	{
						  		
						  	    printf("Client 1 : %s \n",data_buffer);
						  
				                memset(data_buffer,0,sizeof(data_buffer));
				                printf("SERVER: ");
				    	        fgets(data_buffer,1024,stdin);
				    	        //printf("SERVER:");
				                p=write(comm_socket_fd,data_buffer,strlen(data_buffer));
				                if(p<0)
    	                	    printf("Error in writing to Client 1 \n");
    	                        }
    	                 }
						  
			}
			 ///////////////// CLIENT 2 ///////////////////////////////
			
			else if (FD_ISSET(monitored_fd_set[3],&readfds))
			{
					comm_socket_fd = monitored_fd_set[3];
					char data_buffer [1024];
	   		
					
					memset(data_buffer,0,sizeof(data_buffer));
					
					n=read(comm_socket_fd,data_buffer,1024);
    	            if(n<0)
    		              printf("Error in reading from Client 2\n");
    		        else 
    	                  
						  {
						  	/*if(strcmp("EXIT",data_buffer)==0)
						  	  { 
						  	    
						  	    remove_from_monitored_fd_set (comm_socket_fd);
						  	    close(comm_socket_fd);
						  	   /* printf("Client %s   %u: EXITED", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port)); //prints on server side so no need to change output function */
						  	  /* break;
							  }*/
							  
						  	//else 
						  	if(strcmp("LOGOUT",data_buffer)==0)
							   {
							  	bzero(data_buffer,1024);
							     
							  	p=write(comm_socket_fd,"[LOGOUT:SUCCESS]\n",strlen("[LOGOUT:SUCCESS]\n"));
							  	remove_from_monitored_fd_set (comm_socket_fd);
							  }
							else{
								printf("Client 2 : %s \n",data_buffer);						  
			                    memset(data_buffer,0,sizeof(data_buffer));
			                    printf("SERVER:");
			    	            fgets(data_buffer,1024,stdin);
			    	           	p=write(comm_socket_fd,data_buffer,strlen(data_buffer));
			    	           	printf("\n");
			                 	if(p<0)
			    	            printf("Error in writing to Client 2 \n");
    	            		
			}
			
			////////////// CLIENT 3 //////////////////////// 
			else if (FD_ISSET(monitored_fd_set[4],&readfds)) 
			{
					comm_socket_fd = monitored_fd_set[4];
					char data_buffer [1024];
	   		
					
					memset(data_buffer,0,sizeof(data_buffer));
					
					n=read(comm_socket_fd,data_buffer,1024);
    	            if(n<0)
    		              printf("Error in reading from Client 3\n");
    		        else 
    	                  
						  {
						  	/*if(strcmp("EXIT",data_buffer)==0)
						  	  { 
						  	    
						  	    remove_from_monitored_fd_set (comm_socket_fd);
						  	    close(comm_socket_fd);
						  	   /* printf("Client %s   %u: EXITED", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port)); //prints on server side so no need to change output function */
						  	  /* break;
							  }
						  	else */
						  if(strcmp("LOGOUT",data_buffer)==0)
							  {
						  		bzero(data_buffer,1024);
							     
							  	p=write(comm_socket_fd,"[LOGOUT:SUCCESS]\n",strlen("[LOGOUT:SUCCESS]\n"));
							  	remove_from_monitored_fd_set (comm_socket_fd);
							  }
						  	else{
						  		printf("Client 3: %s \n",data_buffer);
						  	memset(data_buffer,0,sizeof(data_buffer));
		                    printf("SERVER:");
		    	            fgets(data_buffer,1024,stdin);
		                   	p=write(comm_socket_fd,data_buffer,strlen(data_buffer));
		                   	printf("\n");
		                 	if(p<0)
    	                	printf("Error in writing to Client 3 \n");
							  }
						  	
    	           		
			}
			/////////////// CLIENT 4 /////////////////
			else if (FD_ISSET(monitored_fd_set[5],&readfds)) 
			{
					comm_socket_fd = monitored_fd_set[5];
					char data_buffer [1024];
	   		
					
					memset(data_buffer,0,sizeof(data_buffer));
					
					n=read(comm_socket_fd,data_buffer,1024);
    	            if(n<0)
    		              printf("Error in reading from Client 4\n");
    		        else 
    	                  
						  {
						  	/*if(strcmp("EXIT",data_buffer)==0)
						  	  { 
						  	    
						  	    remove_from_monitored_fd_set (comm_socket_fd);
						  	    close(comm_socket_fd);
						  	   /* printf("Client %s   %u: EXITED", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port)); //prints on server side so no need to change output function */
						  	  /* break;
							  }
						  	else */
						  if(strcmp("LOGOUT",data_buffer)==0)
							  {
						  		bzero(data_buffer,1024);
							     
							  	p=write(comm_socket_fd,"[LOGOUT:SUCCESS]\n",strlen("[LOGOUT:SUCCESS]\n"));
							  	remove_from_monitored_fd_set (comm_socket_fd);
							  }
						  	else{
						  		printf("Client 4 : %s \n",data_buffer);
						     	memset(data_buffer,0,sizeof(data_buffer));
		                        printf("SERVER:");
		    	                fgets(data_buffer,1024,stdin);
    	           
		                		p=write(comm_socket_fd,data_buffer,strlen(data_buffer));
		                		printf("\n");
		                 		if(p<0)
    	                    	printf("Error in writing to Client 4 \n");
			} /* Client 4 end */
			
			
			
		} /*else loop closed for comm socket */
	} /* WHILE LOOP CLOSED HERE */
		

    
   	close(comm_socket_fd);
    remove_from_monitored_fd_set(comm_socket_fd);
	close(master_sock_tcp_fd);
	remove_from_monitored_fd_set(master_sock_tcp_fd);
	close(console_stdin_fd);
	remove_from_monitored_fd_set(console_stdin_fd);
	
		
} /* SERVER FUNCTION CLOSED HERE */


  
  /* CLIENT FUNCTION STARTS FROM HERE */
  
/* struct destination_info {
 	int SERVER_PORT;
 	char SERVER_IP_ADDRESS[13];
 }destination; */


/*void destination_info_store (){
	                    struct sockaddr_in dest,client_addr; //starndard API	
						struct destination_info destination;
	                    dest.sin_family = AF_INET;
						dest.sin_port= destination.SERVER_PORT;
						struct hostent *host=(struct hostent *)gethostbyname(destination.SERVER_IP_ADDRESS);
						dest.sin_addr = *((struct in_addr *)host->h_addr);
						
} */

void CLIENT(int user_assigned_port)
	{
		int client_sock_fd = 0;   	
		int addr_len =0 ;
		addr_len = sizeof(struct sockaddr);
		
		struct sockaddr_in dest,client_addr;
		 //starndard API	
		//struct destination_info destination;
			
		
			/* client_addr.sin_family = AF_INET;
			client_addr.sin_port= user_assigned_port;
			client_addr.sin_addr.s_addr= INADDR_ANY;
			addr_len = sizeof(struct sockaddr);  
		
		/*SOCKET creation for Network and console*/
		client_sock_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		int client_stdin_fd =0; // standard descriptor value for a stdin file
		
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(client_sock_fd,&readfds);
		FD_SET(client_stdin_fd, &readfds);
		
		
		
				while(1)
				{
                      
					   select(client_sock_fd+1,&readfds,NULL,NULL,NULL); // since file descriptor vale cannot be less than zero and we have only two fds thus we know that the max value is of this fd
					    
					    
						
					    
					  	if (FD_ISSET(client_stdin_fd,&readfds))
					       {    
					            char buffer_for_console [255];
								bzero(buffer_for_console,255);
								gets(buffer_for_console);
								fflush(stdin);
									    
							    if( strncmp ( "LOGIN",buffer_for_console, 5 )==0)
						  			  {     											
										char *ptr;
										ptr= &buffer_for_console[0];
								    		
										for(*ptr=buffer_for_console[0];*ptr!=' ' ;ptr++){
												
										}
										ptr++;
										char SERVER_IP_ADDRESS[16];											
										int i=0;
										for(;*ptr!= ' ' ;ptr++){
									    SERVER_IP_ADDRESS[i]= *ptr;
										i++;
										}
										ptr++;
										int j=0;
										int SERVER_PORT =0;
										char array_of_port[10];
										for(;*ptr!='\0';ptr++){
										array_of_port[j]=*ptr;
										j++;
										}
										SERVER_PORT= atoi(array_of_port);
										
										/*SERVER INFORMATION */
										dest.sin_family = AF_INET;
										dest.sin_port= SERVER_PORT;
										struct hostent *host=(struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
										dest.sin_addr = *((struct in_addr *)host->h_addr);
										connect(client_sock_fd,(struct sockaddr *)&dest,sizeof(struct sockaddr));		
																										
										cse4589_print_and_log("[LOGIN:SUCESS]\n");
									        cse4589_print_and_log("[LOGIN:END]\n);
									    										
											
					    				}
													
					  		    else if(strcmp ("AUTHOR",buffer_for_console)==0)
								     	{
										cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
										cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n",YOUR_UBIT_NAME );
					   					cse4589_print_and_log("[AUTHOR: END]\n");
					   					
								     	}
									
								else if (strcmp("PORT",buffer_for_console)==0)
									{
									cse4589_print_and_log("[PORT:SUCCESS]\n");
									cse4589_print_and_log("PORT:%d\n",user_assigned_port);
									cse4589_print_and_log("[PORT:END]\n");
									
									}
								
								else if (strcmp("IP",buffer_for_console)==0)
									{
									cse4589_print_and_log("[IP:SUCCESS]\n");
											char hostbuffer[256]; 
										    char *IPbuffer; 
										    struct hostent *host_entry; 
										    int hostname; 
										  
										    // To retrieve hostname 
										    hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
										    										  
										    // To retrieve host information 
										    host_entry = gethostbyname(hostbuffer); 
																  
										    // To convert an Internet network 
										    // address into ASCII string 
										    IPbuffer = inet_ntoa(*((struct in_addr*) 
										                           host_entry->h_addr_list[0])); 
										    
										   
									printf("IP:%s\n",IPbuffer);				 
									cse4589_print_and_log("[IP:END]\n");
									
							    	}
							    							   					    
								else 
								{
									int n=write(client_sock_fd,buffer_for_console,strlen(buffer_for_console)); // write to server using socket file descriptor
									if(n<0)
										printf("Error in writing\n");	
									FD_SET(client_sock_fd,&readfds);
									char data_buffer_client [255];
									memset(data_buffer_client,0,sizeof(data_buffer_client));
									//fgets(data_buffer_client,255,stdin); //to pass string to server
									int p=read(client_sock_fd,data_buffer_client,255);
									if(p<0)
										printf("reading from socket failed");
									else
										{
											if (strcmp ("[LOGOUT:SUCCESS]\n",data_buffer_client)==0)
											{
												printf("%s[LOGOUT:END]\n",data_buffer_client);
											
											}
									    
											else								
										        printf("SERVER: %s",data_buffer_client);
								    	}
								    	
								    
							    }
							    	
							                        
  							}
	                  
					                        
                       	
			   	}
							  
			   	 
	                     
 close(client_sock_fd);
 close(client_stdin_fd);

  	
   }

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
int user_assigned_port = atoi(argv[2]);
	if (argc <3)
     	{
			printf("ERROR: Argument missing !\n"); 
			exit(1);
    	}
	// check if it is server or client

	else 
    	{
  		   if (strcmp(argv[1],"c")==0)
	    		 CLIENT(user_assigned_port ); 

			else if (strcmp(argv[1],"s")==0)
   			  	SERVER( user_assigned_port);
     	
			else
     			{
					printf("ERROR: Invalid mode %c \n",argv[1]);
					exit(1);
	  		    }
        }

	return 0;
}
