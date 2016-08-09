//#pragma once
#ifndef _MYSQL__CONNECT_
#define _MYSQL__CONNECT_

#include<iostream>
using namespace std;
#include"mysql.h"
#include<stdlib.h>
#include<string.h>

class mysql_connect
{
 public:
    mysql_connect(const string &_host,const string &_user,const string &_passwd,const string &_db);
    bool begin_connect();
    bool insert_sql(const string &data);
    bool select_sql(string field_name[],string out_str[][5],int &out_row);
    bool close_connect();
     ~mysql_connect();
     void show_info();

 private:
     MYSQL_RES * res;
     MYSQL *mysql_base;
     string host;
     string user;
     string passwd;
     string db;
};

#endif
