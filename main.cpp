#include "Socket.h"
#include "DataFrame.h"
using namespace std;
int main(){
    string s;
    char buf[500]={};
    startWSA();
    Socket server;
    server.setAddr("server.yonc.xyz");
    server.setPort(21923);
    int err = server.Connect();
    if(err){
        cout<<"Failed."<<endl;
        return 0;
    }
    for(;;){
        cin.getline(buf,sizeof(buf)-1);
        for(int i=sizeof(buf)-1;i>=0;i--){
            if(isspace(buf[i])) 
                buf[i]=0;
            else 
                break;
        }
        if(strlen(buf)){
            s=buf;
        }else{
            continue;
        }
        if(s=="#shutdown") break;
        DataFrame frame = DataFrame("message",s.c_str());
        frame.type="message";
        int err = (server<<frame);
    }
    server.Close();
    stopWAS();
}