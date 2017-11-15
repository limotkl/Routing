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

void UDP_send(char *idp,char *idto,int index)
{
    printf("==========kaishi send==========\n"); 
    int id,id2;
    int i;

    id  = atoi(idp);
    id2 = atoi(idto);

    int socket_id,socket_id1;
    char buf[10];
    struct sockaddr_in address;

    if(index == 1)//send packet
    {
        strcpy (buf,"AAAA");
        strcat(buf, idto);
        printf("%s\n",buf);
        socket_id=socket(AF_INET,SOCK_DGRAM,0);
        address = get_address(node[id].controlPort,node[id].hostName);
        sendto(socket_id,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
        for(i=0;i<10;i++)
        {
            buf[i]=0;
        }
        
    }
    if(index == 2)//remove link
    {
        strcpy (buf,"BBBB");
        strcat(buf, idto);
        printf("%s\n",buf);
        socket_id=socket(AF_INET,SOCK_DGRAM,0);
        address = get_address(node[id].controlPort,node[id].hostName);
        sendto(socket_id,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
        for(i=0;i<10;i++)
        {
            buf[i]=0;
        }

        strcpy (buf,"BBBB");
        strcat(buf, idp);
        printf("%s\n",buf);
        socket_id1=socket(AF_INET,SOCK_DGRAM,0);
        address = get_address(node[id2].controlPort,node[id2].hostName);
        sendto(socket_id1,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
        close(socket_id1); 
        for(i=0;i<10;i++)
        {
            buf[i]=0;
        }

    }
    if(index == 3)//add link
    {
        strcpy (buf,"CCCC");
        strcat(buf, idto);
        printf("%s\n",buf);
        socket_id=socket(AF_INET,SOCK_DGRAM,0);
        address = get_address(node[id].controlPort,node[id].hostName);
        sendto(socket_id,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
        for(i=0;i<10;i++)
        {
            buf[i]=0;
        }

        strcpy (buf,"CCCC");
        strcat(buf, idp);
        printf("%s\n",buf);
        socket_id1=socket(AF_INET,SOCK_DGRAM,0);
        address = get_address(node[id2].controlPort,node[id2].hostName);
        sendto(socket_id1,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
        close(socket_id1); 
        for(i=0;i<10;i++)
        {
            buf[i]=0;
        } 
    }
    close(socket_id);
    
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
    readfile();
    if(strncmp(argv[1],"generate-packet",strlen("generate-packet")) == 0)
    {
        UDP_send(argv[2],argv[3],1);
    }

    if(strncmp(argv[1],"create-link",strlen("create-link")) == 0)
    {
        UDP_send(argv[2],argv[3],3);
    }

    if(strncmp(argv[1],"remove-link",strlen("remove-link")) == 0)
    {
        UDP_send(argv[2],argv[3],2);
    }
    

    return 0;
}  