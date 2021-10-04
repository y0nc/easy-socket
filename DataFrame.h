#include<string>
#include<string.h>
using namespace std;

#ifndef DATAFRAME_H

#define DATAFRAME_H

class DataFrame
{
//[---len 4Bytes---]-[---type 16Bytes--]-[----------------data lenBytes-----------------]
private:
    /* data */
public:
    DataFrame();
    DataFrame(string type,const char *buf);
    ~DataFrame();
    int len;
    string type;
    char *data;
};

DataFrame::DataFrame(){
    data = NULL;
}

DataFrame::DataFrame(string type,const char *buf)
{
    this->type=type;
    int len = strlen(buf);
    this->len=len;
    this->data=(char*)malloc(len+1);
    memcpy(this->data,buf,len);
    data[len]=0;
}

DataFrame::~DataFrame()
{
    free(data);
}

#endif