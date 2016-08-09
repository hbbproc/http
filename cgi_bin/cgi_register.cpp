#include<iostream>
using namespace std;
#include<stdlib.h>
#include<string>
#include"mysql_connect.h"

const string remote_ip = "192,168.9.232";
const string remote_user = "hbb";
const string remote_passwd = "123456";
const string remote_db = "remote_db";

int main()
{
    //string content_length;
    //string method;
    //string query_string;
    //string post_data;

    cout<<"<html>"<<endl;
    cout<<"<head>show student</head>"<<endl;
    cout<<"<body>"<<endl;
    string name;
    string age;
    string school;
    string hobby;
    string insert_data = "qq, 12, wxyz, read";
    const string _host = remote_ip;
    const string _user = remote_user;
    const string _passwd = remote_passwd;
    const string _db = remote_db;
    mysql_connect conn(_host,_user,_passwd,_db);
    conn.begin_connect();
    conn.insert_sql(insert_data);

    cout<<"</body>"<<endl;
    cout<<"</html>"<<endl;
    return 0;
}
