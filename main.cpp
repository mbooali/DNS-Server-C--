#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<fstream>
#include<errno.h>
#include<sstream>
#include<time.h>
#include <pthread.h>
#include <vector>
#include <signal.h>
#include <stdlib.h>
#include "Cache.h"
using namespace std;
//List of DNS Servers registered on the system


#define TPOOL_SIZE 4

//Function Prototypes
bool ngethostbyname(unsigned char*,string);
void ChangetoDnsNameFormat(unsigned char*, unsigned char*);
unsigned char* ReadName(unsigned char*, unsigned char*, int*);
void get_dns_servers();
void* handler(void* entry);
int response_size(unsigned char* buf,unsigned char* qname);
string char2string(unsigned char*);

//DNS header structure
#pragma pack(push, 4)
struct DNS_HEADER {
    unsigned short id; // identification number

    unsigned char rd : 1; // recursion desired
    unsigned char tc : 1; // truncated message
    unsigned char aa : 1; // authoritive answer
    unsigned char opcode : 4; // purpose of message
    unsigned char qr : 1; // query/response flag

    unsigned char rcode : 4; // response code
    unsigned char cd : 1; // checking disabled
    unsigned char ad : 1; // authenticated data
    unsigned char z : 1; // its z! reserved
    unsigned char ra : 1; // recursion available

    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};

//Constant sized fields of query structure

struct QUESTION {
    unsigned short qtype;
    unsigned short qclass;
};

struct Entry
{
    int lent;
    int csocket;
    struct sockaddr_in cli_getter;
    unsigned char* buf;
    socklen_t frmln_getter;
};

#pragma pack(pop)
pthread_mutex_t adding_lock0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t adding_lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t free_que = PTHREAD_MUTEX_INITIALIZER;

Cache DNS_CACHE[2];
vector<int> frees;
pthread_t ethread;
void force_exit(int)
{
    int x = 10;
    cout << endl << endl << endl << endl << endl << "Server exiting in 10 seconds";

    
    while(x--)
    {
        sleep(1);
        cout << ".";
        fflush(stdout);
    }
    pthread_cancel(ethread);
    exit(-1);
}
int main() {

    signal(SIGINT,force_exit);

    int i;
    unsigned char buf[65536];
    struct sockaddr_in sa_getter;
    memset(&sa_getter, 0, sizeof(sa_getter));


    sa_getter.sin_family = AF_INET;
    sa_getter.sin_addr.s_addr = INADDR_ANY;
    sa_getter.sin_port = htons(53);

    int client_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (-1 == bind(client_sock,(struct sockaddr *)&sa_getter, sizeof(sa_getter)))
    {
        perror("error bind failed");
        close(client_sock);
        return 0;
    }
    for(i = 0; i < TPOOL_SIZE; i++)
        frees.push_back(i);

    Entry* e = NULL;


    pthread_t thread[TPOOL_SIZE];
    
    while(true)
    {
        
        
        socklen_t frmln_getter = sizeof(sa_getter);
        struct sockaddr_in ca_getter;

        int recsize = recvfrom(client_sock, (void *)buf, 65536, 0, (struct sockaddr *)&ca_getter, &frmln_getter);
        if (recsize < 0)
        {
            cerr << strerror(errno) << endl;
            return 0;
        }
        e = new Entry[1];
        e->lent = recsize;
        e->frmln_getter = frmln_getter;
        e->csocket = client_sock;
        e->cli_getter = ca_getter;
        e->buf = new unsigned char[recsize];
        memcpy(e->buf,buf,recsize);

        pthread_create( &ethread, NULL, handler, (void*) e);
        pthread_join(ethread,NULL);
        
    }

    return 0;
}

void* handler(void* entry)
{
    unsigned char buf[65536], *qname;
    struct DNS_HEADER *dns_rec = NULL;
    struct QUESTION *qinfo = NULL;


    //prepare for get request
    
    struct sockaddr_in ca_getter;
    struct sockaddr_in dest;

    Entry* e;
    e = (Entry*) entry;
    int client_sock = e->csocket;
    socklen_t frmln_getter = e->frmln_getter;
    memcpy(buf,e->buf,e->lent);
    ca_getter = e->cli_getter;

    int server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //UDP packet for DNS queries

    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr("4.2.2.4"); //dns servers

    dns_rec = (struct DNS_HEADER *) &buf;

    
    qname = (unsigned char*) &buf[sizeof (struct DNS_HEADER) ];
    
    
    qinfo = (struct QUESTION*) &buf[sizeof (struct DNS_HEADER) + (strlen((const char*) qname) + 1)]; //fill it

    cout << "**** " << qname << " ****" << qinfo->qtype  << " **** " << qinfo->qclass << endl;

    int u;
    unsigned char* h;
    h = qname;
    cout << "nSending Packet..." << endl;
    Block* b = NULL;
    unsigned char* keyname = ReadName(h, qname, &u);
    if(qinfo->qtype == 256)
        b = DNS_CACHE[0].hit(keyname);
    else if(qinfo->qtype == 7168)
        b = DNS_CACHE[1].hit(keyname);

    if(b == NULL)
    {
        if (sendto(server_sock, (char*) buf, sizeof (struct DNS_HEADER) + (strlen((const char*) qname) + 1) + sizeof (struct QUESTION), 0, (struct sockaddr*) &dest, sizeof (dest)) == 0) {
            cout << "Error sending socket" << endl;
            return 0;
        }
        int i = sizeof(dest);
        cout << "nReceiving answer..." << endl;
        int tool_e_response = 0;
        tool_e_response = recvfrom(server_sock, (char*) buf, 65536, 0, (struct sockaddr*) &dest, (socklen_t*)&i);
        if (tool_e_response == 0) {
            return 0;
            cout << "Failed. Error Code " << endl;
        }

        
        if(qinfo->qtype == 256)
        {
            pthread_mutex_lock(&adding_lock0);
            DNS_CACHE[0].add(keyname,buf,tool_e_response);
            pthread_mutex_unlock(&adding_lock0);
        }
        else if(qinfo->qtype == 7168)
        {
            pthread_mutex_lock(&adding_lock1);
            DNS_CACHE[1].add(keyname,buf,tool_e_response);
            pthread_mutex_unlock(&adding_lock1);
        }


        cout << "Received." << endl;
        cout << sendto(client_sock,buf,tool_e_response,0,(struct sockaddr*) &ca_getter,frmln_getter) << endl;
        cout << tool_e_response << endl;

    }
    else
    {
        cout << "from cache" << endl;
        cout << sendto(client_sock,b->content,b->lent,0,(struct sockaddr*) &ca_getter,frmln_getter) << endl;
    }

    cout << "**** " << qname << " ****" << qinfo->qtype  << " **** " << qinfo->qclass << endl;
    close(server_sock);
    //close(client_sock);

}

unsigned char* ReadName(unsigned char* reader, unsigned char* buffer, int* count) {
    unsigned char *name;
    unsigned int p = 0, jumped = 0, offset;
    int i, j;

    *count = 1;
    name = new unsigned char[256];

    name[0] = '\0';

    //read the names in 3www6google3com format
    while (*reader != 0) {
        if (*reader >= 192) {
            offset = (*reader)*256 + *(reader + 1) - 49152;
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        } else
            name[p++] = *reader;

        reader = reader + 1;

        if (jumped == 0)
            *count = *count + 1; //if we havent jumped to another location then we can count up
    }

    name[p] = '\0'; //string complete
    if (jumped == 1)
        *count = *count + 1; //number of steps we actually moved forward in the packet

    //now convert 3www6google3com0 to www.google.com
    for (i = 0; i < (int) strlen((const char*) name); i++) {
        p = name[i];
        for (j = 0; j < (int) p; j++) {
            name[i] = name[i + 1];
            i = i + 1;
        }
        name[i] = '.';
    }
    name[i - 1] = '\0'; //remove the last dot
    return name;
}
