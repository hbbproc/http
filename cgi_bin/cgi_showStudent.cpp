#include<iostream>
using namespace std;
#include<string>
#include<stdlib.h>
#include"mysql_connect.h"

const string remote_ip = "192.168.9.232";
const string remote_user = "hbb";
const string remote_passwd = "123456";
const string remote_db = "remote_db";

int main()
{
   // string content_length;
   // string method;
   // string qurry_string;
   // string post_data;
    cout<<"<html>"<<endl;
    cout<<"<head>show student</head>"<<endl;
    cout<<"<body>"<<endl;
    const string _host = remote_ip;
    const string _user = remote_user;
    const string _passwd = remote_passwd;
    const string _db = remote_db;

    string mysql_data[1024][5];
    string header[5];
    int cur_row = -1;
    mysql_connect conn(_host,_user,_passwd,_db);
    conn.begin_connect();
    conn.select_sql(header,_mysql_data,cur_row);
    cout<<"<table boder=\"1\">"<<endl;
    cout<<"<tr>"<<endl;
    for(int i = 0;i < 5;++i){
        cout<<"</th>"<<header[i]<<"</th>"<<endl;
    }
    cout<<"</tr>"<<endl;

    for(int i = 0;i < cur_row;++i){
        cout<<"<tr>"<<endl;
        for(int j = 0;j < 5;++j){
            cout<<"<td>"<<mysql_data[i][j]<<"</td>"<<endl;
        }
        cout<<"</tr>"<<endl;
    }
    cout<<"</table>"<<endl;
    cout<<"</body>"<<endl;
    cout<<"</html>"<<endl;
    return 0;
}
