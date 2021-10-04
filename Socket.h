#include <string>
#include "DataFrame.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

#ifndef SOCKET_H

#define SOCKET_H

class Socket
{
private:
    int recvInt(int &num);
    int recvType(string &s);
    void init();
    string addr;
    int port;
    int maxn;
    SOCKET fd;
    SOCKADDR_IN sock_in;
    int AF,type,protocol;
    bool connFlag;
    bool bindFlag;
    bool listenFlag;
public:
    Socket();
    Socket(string addr,int port,int AF, int type, int protocol,int maxconn);
    ~Socket();
    static bool isDomain(string &str);
    static vector<string> resolveDomain(string &addr);
    void setAddr(const string &addr);
    void setPort(int port);
    bool isConnected();
    bool isBinded();
    bool isListening();
    int Bind();
    int Listen();
    int Connect();
    int Close();
    int operator>>(DataFrame &f);
    int operator>>(Socket &sock);
    int operator<<(DataFrame &f);
};

void Socket::init(){
    connFlag=false;
    bindFlag=false;
    listenFlag=false;
    cout<<"init"<<endl;
}

Socket::Socket(){
    this->AF=AF_INET;
    this->type=SOCK_STREAM;
    this->protocol=0;
    this->maxn=10;
    sock_in.sin_family=this->AF;
    init();
}

Socket::Socket(string str,int port,int AF = AF_INET, int type = SOCK_STREAM, int protocol = 0,int maxconn = 10)
{
    this->addr=str;
    this->port=port;
    this->AF=AF;
    this->type=type;
    this->protocol=protocol;
    this->maxn=maxconn;
    sock_in.sin_family=AF;
    init();
}

Socket::~Socket()
{
    closesocket(fd);
}

void Socket::setAddr(const string &addr){
    this->addr=addr;
}

void Socket::setPort(int port){
    this->port=port;
}

int Socket::Bind(){
    if(isBinded()) return -9;
    this->sock_in.sin_family=AF_INET;
    this->sock_in.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
    this->sock_in.sin_port=htons(this->port);
    int err = bind(this->fd,(sockaddr*)&sock_in,sizeof(sock_in));
    if(err){
        return err;
    }else{
        bindFlag=false;
        return err;
    }
}

int Socket::Listen(){
    if(isListening()) return -9;
    int err = listen(this->fd,this->maxn);
    if(err){
        return err;
    }else{
        listenFlag=false;
        return err;
    }
}

int Socket::recvInt(int &num){
    if(!isConnected()) return -9;
    char buf[5]={};
    for(int p=0;p<sizeof(int);){
        int d = recv(this->fd,buf+p,sizeof(int)-p,0);
        if(d>0) 
            p+=d;
        else 
            return -1;
    }
    num = atoi(buf);
    return 0;
}

int Socket::recvType(string &s){
    if(!isConnected()) return -9;
    char buf[17]={};
    for(int p=0;p<sizeof(int);){
        int d = recv(this->fd,buf+p,sizeof(int)-p,0);
        if(d>0) 
            p+=d;
        else 
            return -1;
    }
    s = buf;
    return 0;
}

bool Socket::isDomain(string &str){
    for(char ch : str){
        if(!(ch=='.'||isdigit(ch))) return true;
    }
    return false;
}

vector<string> Socket::resolveDomain(string &addr){
    hostent *host = gethostbyname(addr.c_str());
    vector<string> res;
    for(int i=0;host && host->h_addr_list[i];i++){
        res.push_back(inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
    }
    return res;
}

int Socket::Connect(){
    if(isConnected()) return -9;
    this->fd = socket(AF,type,protocol);
    sock_in.sin_port = htons(port);
    std::vector<std::string> ips;
    if(Socket::isDomain(addr)){
        ips = Socket::resolveDomain(addr);
    }else{
        ips.push_back(addr);
    }
    bool flag=false;
    for(std::string ip : ips){
        sock_in.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
        if(!connect(this->fd,(SOCKADDR*)&sock_in,sizeof(SOCKADDR))){
            flag=true;
            break;
        }
    }
    if(flag){
        connFlag=true;
        return 0;
    }else{
        return -1;
    }    
}

int Socket::Close(){
    if(!isConnected()) return -9;
    int err = closesocket(this->fd);
    if(err){
        return err;
    }else{
        connFlag=false;
        return err;
    }
}

bool Socket::isConnected(){
    return connFlag;
}

bool Socket::isBinded(){
    return bindFlag;
}

bool Socket::isListening(){
    return listenFlag;
}

int Socket::operator<<(DataFrame &f){
    if(!isConnected()) return -9;
    if(f.data==NULL) return -8;
    if(f.len<=0) return -1;
    char lenbuf[5]={},typebuf[17]={};
    itoa(f.len,lenbuf,10);
    strcpy(typebuf,f.type.c_str());
    int len1=send(fd,lenbuf,4,0);
    int len2=send(fd,typebuf,16,0);
    int len3=send(fd,f.data,f.len,0);
    if(len1==4&&len2==16&&len3==f.len)
        return 0;
    else 
        return -1;
}

int Socket::operator>>(DataFrame &f){
    if(!isConnected()) return -9;
    int err1 = recvInt(f.len);
    int err2 = recvType(f.type);
    f.data = (char*)malloc(f.len);
    bool flag = true;
    for(int p=0;p<f.len&&flag;){
        int d = recv(fd,f.data,f.len-p,0);
        if(d>0)
            p+=d;
        else{
            flag=false;
        }
    }
    if(err1||err2||!flag)
        return -1;
    else 
        return 0;
}

int Socket::operator>>(Socket &sock){
    if(!isBinded()||!isListening()) return -9;
    if(sock.isConnected()) return -8;
    SOCKADDR_IN sock_in;
    int addr_len=0;
    int client_fd = accept(fd,(sockaddr*)&sock_in,&addr_len);
    if(client_fd){
        sock.fd = client_fd;
        sock.connFlag = true;
        return 0;
    }else{
        return -1;
    }
}


int startWSA(){
    WORD versionRequired = MAKEWORD(1,1);
    WSADATA wsaData;
    return WSAStartup(versionRequired,&wsaData);
}

int stopWAS(){
    return WSACleanup();
}

#endif