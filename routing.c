#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#define MAXNAMELEN 256
#define LEN 5
#define PNUM 5

struct Packet_struct
{
    int source;
    int to;
    int pack_id;
    int ttl;
    char data[50];
};

struct Node{

    char hostName[30];
    int controlPort;
    int dataPort;
    int neighbor1;
    int neighbor2;
    int neighbor3;
};

struct Node node1[10];
struct Node node[10];

struct vector
{
    int Destination;
    int Next;
    int Distance;
};
struct vector entry[LEN];
int ID;
int neighbor_num;
int lock;

struct sockaddr_in get_address(ushort port,char * servhost)
{
    struct sockaddr_in address;

    bzero(&address,sizeof(address));  
    address.sin_family=AF_INET;  
    address.sin_port=htons(port);  
    struct hostent *hp;
    if( (hp = gethostbyname(servhost)) < 0 )
        printf("gethostbyname() error\n");
    else
    {
        memcpy(&address.sin_addr,hp->h_addr,hp->h_length);
    }
    return address;
}


void packet_send(char* nodeid)
{
    
    printf("==========packet start send==========\n"); 
    int iter,i;
    int socket_id1;
    struct sockaddr_in address1;
    struct Packet_struct packet[PNUM];
    socket_id1=socket(AF_INET,SOCK_DGRAM,0);

    for(iter=0;iter<PNUM;iter++)
    {
            packet[iter].source = ID;
            packet[iter].to = atoi(nodeid);
            packet[iter].pack_id = iter;
            packet[iter].ttl = 15;
            strcpy(packet[iter].data, "from client");
            for(i=0;i<LEN;i++)
            {
                if(entry[i].Destination == packet[iter].to && packet[iter].to != ID && entry[i].Distance != -2)
                {
                    address1 = get_address(node[entry[i].Next].dataPort,node[entry[i].Next].hostName);
                    sendto(socket_id1,(char *)&packet[iter],sizeof(packet[iter])+1,0,(struct sockaddr *)&address1,sizeof(address1));
                }
            }
        sleep(3);
    }
    
    close(socket_id1); 
    printf("packet Sent,terminating\n");  
}

void *packet_listen(void)
{
    ushort port=node[ID].dataPort;
    int i,j;
    i = 0;
    int sin_len;  
    //struct Packet_struct rec[100];
    char message[MAXNAMELEN] = {0};  
    int socket_descriptor;
    int  socketsent;
    struct sockaddr_in sin;  
    printf("Waiting for data form sender -----dataport\n");  

    bzero(&sin,sizeof(sin));  
    sin.sin_family=AF_INET;  
    sin.sin_addr.s_addr=htonl(INADDR_ANY);  
    sin.sin_port=htons(port);  
    sin_len=sizeof(sin);  
  
    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);
    socketsent=socket(AF_INET,SOCK_DGRAM,0);

    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));  
    struct sockaddr_in address1;
  
    while(1)  
    {  
        recvfrom(socket_descriptor,message,sizeof(message),0,(struct sockaddr *)&sin,&sin_len);  
        //printf("Response from server:\n");  
        if(strncmp(message,"stop",4) == 0)
        {
            printf("client has told me to end the connection\n");  
            break;  
        }

        struct Packet_struct temp;
        memcpy(&temp ,message,sizeof(temp)+1);
        if(temp.to == ID)
        {
            //memcpy(&rec[i],message,sizeof(rec[i])+1);
            temp.ttl = temp.ttl -1;
            printf("from dataport -----received packet:%d %d %d %d %s\n",temp.source,temp.to,temp.pack_id,temp.ttl,temp.data);
            i++;
        }
        if(temp.to != ID)
        {
            for(j =0 ;j<LEN;j++)
            {
                if(temp.to == entry[j].Destination && entry[j].Distance > -2 && temp.ttl > 0)
                {
                    temp.ttl = temp.ttl -1;
                    //printf("zhuanfale hahahahahah\n");
                    printf("from dataport -----transfer packet:%d %d %d %d %s\n",temp.source,temp.to,temp.pack_id,temp.ttl,temp.data);
                    address1 = get_address(node[entry[j].Next].dataPort,node[entry[j].Next].hostName);
                    sendto(socketsent,(char *)&temp,sizeof(temp)+1,0,(struct sockaddr *)&address1,sizeof(address1));
                }
            }
        }
    }
    close(socket_descriptor);
    close(socketsent);
    return 0;
}



void *UDP_listen(void)
{
    ushort port=node[ID].controlPort;

    int i,j,tempID,flag,startpoint;

    int sin_len;  
    char message[MAXNAMELEN] = {0};  
    int socket_descriptor;  
    struct sockaddr_in sin;  
    printf("Waiting for data form sender ----controlport\n");  
  
    //get host name(useless)
    char * servhost;
    servhost = (char*)malloc(MAXNAMELEN *sizeof(char));
    if( gethostname(servhost, MAXNAMELEN) < 0 )
        printf("gethostname() error\n");
    struct hostent *hp;
    if( (hp = gethostbyname(servhost)) < 0 )
        printf("gethostbyname() error\n");
    strcpy(servhost, hp->h_name);
    printf("host name %s\n", servhost);

    bzero(&sin,sizeof(sin));  
    sin.sin_family=AF_INET;  
    sin.sin_addr.s_addr=htonl(INADDR_ANY);  
    sin.sin_port=htons(port);  
    sin_len=sizeof(sin);  
  
    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);  
    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));  
  
    while(1)  
    {  
        tempID = -2;
        flag = 0;
        startpoint = -1;
        recvfrom(socket_descriptor,message,sizeof(message),0,(struct sockaddr *)&sin,&sin_len);  
        //printf("Response from server:\n");  
        if(strncmp(message,"stop",4) == 0)
        {
            printf("client has told me to end the connection\n");  
            break;  
        }

        if(strncmp(message,"AAAA",4) == 0)//generate packet
        {
            printf("%s\n",message);
            printf("%c\n",message[4]);
            char str[2];
            str[0] = message[4];
            str[1] = '\0';
            packet_send(str);  
        }
        if(strncmp(message,"BBBB",4) == 0)//remove link
        {
            printf("%s\n",message);
            printf("%c\n",message[4]);
            char str[2];
            str[0] = message[4];
            str[1] = '\0';
            int gg = atoi(str);
            if(node[ID].neighbor1 == gg)
                node[ID].neighbor1 =0;
            if(node[ID].neighbor2 == gg)
                node[ID].neighbor2 =0;
            if(node[ID].neighbor3 == gg)
                node[ID].neighbor3 =0;
                        
            for(i =0 ;i <LEN ;i++)
            { 
                //printf("%d,%d,%d\n",entry[i].Destination,entry[i].Next,entry[i].Distance);
                if(entry[i].Next == gg)
                {
                    entry[i].Distance = -2;
                    // entry[i].Next == -2;
                    // entry[i].Destination == -2;                   
                }
            }
        }
        if(strncmp(message,"CCCC",4) == 0)//add link
        {
            printf("%s\n",message);
            printf("%c\n",message[4]);
            char str[2];
            str[0] = message[4];
            str[1] = '\0';
            int nogg = atoi(str);
            if(node[ID].neighbor1 == 0)
                node[ID].neighbor1 = nogg;
            if(node[ID].neighbor2 == 0)
                node[ID].neighbor2 = nogg;
            if(node[ID].neighbor3 == 0)
                node[ID].neighbor3 = nogg;
                        
            for(i =0 ;i <LEN ;i++)
            { 
                //printf("%d,%d,%d\n",entry[i].Destination,entry[i].Next,entry[i].Distance);
                if(entry[i].Destination == nogg)
                {
                    entry[i].Next = nogg;
                    entry[i].Distance = 1;
                    // entry[i].Next == -2;
                    // entry[i].Destination == -2;                   
                }
            }

        }

        if(strncmp(message,"AAAA",4) != 0 && strncmp(message,"BBBB",4) != 0 && strncmp(message,"CCCC",4) != 0)
        {
            struct vector temp[LEN];
            memcpy(&temp ,message,sizeof(temp)+1);
            for(i=0;i<LEN;i++)
            {
                if(temp[i].Distance == 0 && temp[i].Next == -1)
                {
                    tempID = temp[i].Destination;
                }
                if(entry[i].Destination == -2 )
                {
                    startpoint = i;
                    break;
                }
            }
            printf("from controlport ---Response from server:%d \n",tempID);

            for(i =0 ;i <LEN ;i++)
            { 
                printf("%d,%d,%d\n",entry[i].Destination,entry[i].Next,entry[i].Distance);
            }
            //update routing table

            for(i=0;i<LEN;i++)
            {
                //printf("%d,%d,%d\n",entry[i].Destination,entry[i].Next,entry[i].Distance);
                flag = 0;
                if(temp[i].Destination!= ID && temp[i].Destination!= -2)
                {
                    for(j=0;j<LEN;j++)
                    {
                        lock = 1;

                        if(temp[i].Destination == entry[j].Destination)//in routing table
                        {
                            flag = 1;
                            if(entry[j].Next == tempID)//case 2 :always update
                            { 
                                //entry[j].Next = tempID;
                                entry[j].Distance = temp[i].Distance + 1;
                            }
                            if(entry[j].Next != tempID)//case 3 :compair the cost then update
                            {
                                if(entry[j].Distance > temp[i].Distance + 1)
                                {
                                    entry[j].Next = tempID;
                                    entry[j].Distance = temp[i].Distance + 1;
                                }
                                if(entry[j].Distance == -2)
                                {
                                    entry[j].Next = tempID;
                                    entry[j].Distance = temp[i].Distance + 1;
                                }
                            }
                        }
                    }
                }
                //case 1 :not in routing table
                // if(flag == 0 && temp[i].Destination!= -2 && temp[i].Destination!= ID)
                //     {
                //         for(j =0 ;j <LEN ;j++)
                //         { 
                //             if(entry[j].Destination == -2 && entry[startpoint].Next == -2 && entry[startpoint].Distance == -2)
                //             {
                //                 entry[j].Destination = temp[i].Destination;
                //                 entry[j].Next = tempID;
                //                 entry[j].Distance = temp[i].Distance + 1;
                //             }
                //         }

                //     }

                if(flag == 0 && temp[i].Destination!= -2 && temp[i].Destination!= ID)
                {
                    entry[startpoint].Destination = temp[i].Destination;
                    entry[startpoint].Next = tempID;
                    entry[startpoint].Distance = temp[i].Distance + 1;
                    startpoint = startpoint +1;
                }
                lock = 0;
            }
            
            //     for(i =0 ;i <LEN ;i++)
            // { 
            //     printf("%d,%d,%d\n",entry[i].Destination,entry[i].Next,entry[i].Distance);
            // }
        }
 
    }  
  
    close(socket_descriptor);
    return 0;
}


void *UDP_send(void)
{
    printf("==========kaishi send==========\n"); 
   //ushort port_other = 5000;
    int iter;
    int socket_id1,socket_id2,socket_id3;
    //char buf[MAXNAMELEN];

    struct sockaddr_in address1;
    //for(iter=0;iter<=10;iter++)
    while(1)
    {
        if(node[ID].neighbor1 != 0)
        {
            address1 = get_address(node[node[ID].neighbor1].controlPort,node[node[ID].neighbor1].hostName);
            socket_id1=socket(AF_INET,SOCK_DGRAM,0);
            if(lock==0)
            {
                sendto(socket_id1,(char *)&entry,sizeof(entry)+1,0,(struct sockaddr *)&address1,sizeof(address1));
            }
        }
        if(node[ID].neighbor2 != 0)
        {
            address1 = get_address(node[node[ID].neighbor2].controlPort,node[node[ID].neighbor2].hostName);
            socket_id2=socket(AF_INET,SOCK_DGRAM,0);
            if(lock==0)
            {
                sendto(socket_id2,(char *)&entry,sizeof(entry)+1,0,(struct sockaddr *)&address1,sizeof(address1));
            }
        }
        if(node[ID].neighbor3 != 0)
        {
            address1 = get_address(node[node[ID].neighbor3].controlPort,node[node[ID].neighbor3].hostName);
            socket_id3=socket(AF_INET,SOCK_DGRAM,0);
            if(lock==0)
            {
                sendto(socket_id3,(char *)&entry,sizeof(entry)+1,0,(struct sockaddr *)&address1,sizeof(address1));
            }
        }
        sleep(4);
    }
    
    if(node[ID].neighbor1 != 0)
        close(socket_id1); 
    if(node[ID].neighbor2 != 0)
        close(socket_id2);
    if(node[ID].neighbor3 != 0)
        close(socket_id3);

    printf("Sent,terminating\n");  
    return 0;
}

void readfile()
{
    //printf("unable to open file\n");
    int i,n;
    i = 0;
    n = 0;
    int index;
    int box[100];
    FILE *fp;
    if((fp=fopen("config.txt","r"))==NULL){
        printf("unable to open file\n");
    }

    char s[1024];  
    int  num=0;  
    while((fgets(s,1024,fp))!=NULL)  
        n++;  
    printf("\n%d\n",n );
    fclose(fp);

    if((fp=fopen("config.txt","r"))==NULL){
        printf("unable to open file\n");
    }
    for(i = 0;i < n;i++){
        fscanf(fp,"%d %s %d %d %d %d %d\n",&index, node1[i].hostName, &node1[i].controlPort, &node1[i].dataPort, 
            &node1[i].neighbor1, &node1[i].neighbor2, &node1[i].neighbor3);
        box[i] = index;
    }
    fclose(fp);

    if((fp=fopen("config.txt","r"))==NULL){
        printf("unable to open file\n");
    }
    for(i =0 ;i< n;i++)
    {
        //printf("%d\n",box[i]);
        fscanf(fp,"%d %s %d %d %d %d %d\n",&index, node[box[i]].hostName, &node[box[i]].controlPort, &node[box[i]].dataPort, 
            &node[box[i]].neighbor1, &node[box[i]].neighbor2, &node[box[i]].neighbor3);
    }
    for(i =0 ;i< n;i++)
    {
        printf("%d %s %d %d %d %d %d\n",box[i], node[box[i]].hostName, node[box[i]].controlPort, node[box[i]].dataPort, 
            node[box[i]].neighbor1, node[box[i]].neighbor2, node[box[i]].neighbor3);
    }
    fclose(fp);
}

int main(int argc, char** argv) {  

    lock = 0;
    ID = atoi(argv[1]);
    int i;
    readfile();
    for(i=0;i<LEN;i++)
    {
        entry[i].Destination = -2;
        entry[i].Next = -2;
        entry[i].Distance = -2;
    }//-2 means
    entry[0].Destination = ID;
    entry[0].Next = -1;
    entry[0].Distance = 0;

    int ret=0;
    pthread_t thread_listen,thread_send,thread_listen_p,thread_send_p;
    ret=pthread_create(&thread_listen,NULL,(void * (*)(void *))UDP_listen, NULL);
    if(ret)
    {
        printf("create listen pthread error!\n");
    }
    ret =pthread_create(&thread_send,NULL,(void * (*)(void*))UDP_send, NULL);
    if(ret)
    {
        printf("create send pthread error!\n"); 
    }
    ret=pthread_create(&thread_listen_p,NULL,(void * (*)(void *))packet_listen, NULL);
    if(ret)
    {
        printf("create listen pthread error!\n");
    }
    // ret=pthread_create(&thread_send_p,NULL,(void * (*)(void *))packet_send, NULL);
    // if(ret)
    // {
    //     printf("create listen pthread error!\n");
    // }
    pthread_join(thread_listen,NULL);
    pthread_join(thread_send,NULL);
    pthread_join(thread_listen_p,NULL);
    //pthread_join(thread_send_p,NULL);

    return 0;
}  