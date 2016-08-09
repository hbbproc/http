#include"mysql_connect.h"

mysql_connect::mysql_connect(const string &_host,const string &_user,const string &_passwd,const string &_db)
{
    mysql_base = mysql_init(NULL);
    res = NULL;
    host = _host;
    user = _user;
    passwd = _passwd;
    db = _db;
}

bool mysql_connect::begin_connect()
{
    if(mysql_real_connect(mysql_base,host.c_str(),user.c_str(),passwd.c_str(),db.c_str(),3306,NULL,0))
    {
        return false;
    }
    else
    {}
    return true;
}

bool mysql_connect::insert_sql(const string &data)
{
    string sql = "INSET INTO student_six_class(name, age, school, hobby) values ";
    sql += "(";
    sql += data;
    sql += ")";
    if(mysql_query(mysql_base,sql.c_str()) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool mysql_connect::select_sql(string field_name[],string out_str[][5],int &out_row) 
{
    string sql = "SELECT * FROM student_six_class";
    if(mysql_query(mysql_base,sql.c_str()) == 0)
    {}
    else
    {}
    res = mysql_store_result(mysql_base);
    int row_num = mysql_num_rows(res);
    int field_num = mysql_num_fields(res);
    out_row = row_num;

    MYSQL_FIELD *fd = NULL;
    int i = 0;
    for(;fd= mysql_fetch_field(res);){
        field_name[i++] = fd->name;
    }

    for(int index = 0;index < row_num;index++){
        MYSQL_ROW _row  = mysql_fetch_row(res);
        if(_row){
            int start = 0;
            for(;start < field_num;++start){
                out_str[index][start] = _row[start];
            }
        }
    }
    return true;
}

bool mysql_connect::close_connect()
{
    mysql_close(mysql_base);
}

mysql_connect::~mysql_connect()
{
    close_connect();
    if(res != NULL)
    {
        free(res);
    }
}

void mysql_connect::show_info()
{
    cout<<mysql_get_client_info()<<endl;
}

#ifdef _DEBUG_
int main()
{
    string sql_data[1024][5];
    string header[5];
    int cur_row = -1;
    const string _host = "192.168.9.232";
    const string _user = "hbb";
    const string _passwd = "123456";
    const string _db = "remote_db";
    const string data = "'xiaohua', 22, 'xgd', 'sleep'";
    mysql_connect _con(_host,_user,_passwd,_db);
    _con.begin_connect();
    _con.select_sql(header,sql_data,cur_row);
    sleep(1);
    for(int i = 0;i < 5;++i)
    {
        cout<<header[i] <<"\t";
    }
    cout<<endl;

    for(int i = 0;i < cur_row;++i)
    {
        for(int j = 0;j < 5;++j)
        {
            cout<<sql_data[i][j]<<"\t";
        }
        cout<<endl;
    }
    _con.show_info();
    return 0;
}
#endif
