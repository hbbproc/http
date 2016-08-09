#include"httpd.h"

static void usage(const char* proc)
{
    printf("Usage: %s [PORT]\n",proc);
}

int get_line(int client_sock,char buf[],int len)
{
    if(!buf && len <= 0)
    {
        return -1;
    }
    int i = 0;
    int n = 0;
    char c = '\0';
    while(i < len-1 && c != '\n')
    {
        n = recv(client_sock,&c,1,0);
        if(n>0)//success
        {
            if(c == '\r')
            {
                n = recv(client_sock,&c,1,MSG_PEEK);
                if(n > 0 && c == '\n')//'\r''\n' windows
                {
                    n = recv(client_sock,&c,1,0);//delete
                }
                else//'\r'
                {
                    c = '\n';
                }
            }
            buf[i++] = c;
        }
        else//failed
        {
            c = '\n';
        }
    }
        buf[i] = '\0';
        return i;
}

//打印日志
void print_log(const char* fun,int line,int _errno_,const char* _errstr_)
{
    printf("[%s: %d] [%d] [%s]\n",fun,line,_errno_,_errstr_);
}

int startup(int port)
{
    int listen_sock = socket(AF_INET,SOCK_STREAM,0);
    if(listen_sock < 0)
    {
       //perror("socket");
       print_log(__FUNCTION__,__LINE__,errno,strerror(errno));
        exit(2);
    }

    int flag = 1;
    setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port   = htons(port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t len = sizeof(local);

    if(bind(listen_sock,(struct sockaddr*)&local,len) < 0)
    {
        //perror("bind");
        print_log(__FUNCTION__,__LINE__,errno,strerror(errno));
        exit(3);
    }
    if(listen(listen_sock,_BACK_LOG_) < 0)
    {
        //perror("listen");
        print_log(__FUNCTION__,__LINE__,errno,strerror(errno));
        exit(4);
    }
    return listen_sock;
}

static void  not_found(int client)
{
    char buf[1024];
    sprintf(buf,"HTTP/1.0 404 NOT FOUND\r\n");
    send(client,buf,strlen(buf),0);
    sprintf(buf,"Content-type: text/html\r\n");
    send(client,buf,strlen(buf),0);
    sprintf(buf,"\r\n");
    send(client,buf,strlen(buf),0);
    sprintf(buf,"<html><title>NOT FOUND</title>\r\n");
    send(client,buf,strlen(buf),0);
    sprintf(buf,"<body><p>file  is not exist</p></body>\r\n");
    send(client,buf,strlen(buf),0);
    sprintf(buf,"</html>\r\n");
    send(client,buf,strlen(buf),0);
}

static void bad_request(int client)
{
    char buf[1024];
    sprintf(buf,"HTTP/1.0 400 bad request\r\n");
    send(client,buf,strlen(buf),0);
    sprintf(buf,"Content-type:text/html\r\n");
    send(client,buf,strlen(buf),0);
    sprintf(buf,"\r\n");
    send(client,buf,strlen(buf),0);
    sprintf(buf,"<html></br><p>your enter message is a bad request</p></br></html>\r\n");
    send(client,buf,strlen(buf),0);
}

//for error_hander
void return_errno_client(int client, int status_code)
{
    switch(status_code)
    {
        case 404:
            not_found(client);
            break;
        case 405:
            break;
        case 400:
            bad_request(client);
            break;
        case 500:
           //server_error(client);
            break;
        case 503:
           //server_unavaillable(client);
            break;
        default:
           //default_error(client);
            break;
    }
}

void clear_header(int client)
{
    char buf[MAX];
    int ret = -1;
    do{
        ret = get_line(client,buf,sizeof(buf));
    }while(ret>0 && strcmp(buf,"\n") != 0);
}

void exe_cgi(int client_sock,const char* path,const char* method,const char* query_string)
{
    int numchar = -1;
    char buf[MAX];
    int content_length = -1;
    int cgi_input[2] = {0,0};
    int cgi_output[2] = {0,0};
    pid_t id;
    if(strcasecmp(method,"GET") == 0)//CGI && GET
    {
        clear_header(client_sock);
    }
    else//CGI && POST  //1.get content_length //2.clear_header
    {
        do{
            memset(buf,'\0',sizeof(buf));
            numchar = get_line(client_sock,buf,sizeof(buf)-1);
            if(strncasecmp(buf,"Content-Length:",strlen("Content-Length:")) == 0)
            {
                content_length =atoi(&buf[16]);
            }
        }while(numchar > 0 && strcmp("\n",buf) != 0);
        
        if(content_length == -1)
        {
            return_errno_client(client_sock,404);
            return;
        }
    }
    
    memset(buf,'\0',sizeof(buf));
    sprintf(buf,"HTTP/1.0 200 OK\r\n\r\n");
    send(client_sock,buf,strlen(buf),0);

    if(pipe(cgi_input) < 0)
    {
        return_errno_client(client_sock,404);
        return;
    }
    if(pipe(cgi_output) < 0)
    {
        close(cgi_input[0]);
        close(cgi_output[1]);
        return_errno_client(client_sock,404);
        return;
    }
    if((id = fork()) < 0)
    {
        close(cgi_input[0]);
        close(cgi_input[1]);
        close(cgi_output[0]);
        close(cgi_output[1]);
        return_errno_client(client_sock,404);
        return;
    }
    else if(id == 0)//child
    {
        char meth_env[MAX];
        char query_env[MAX/10];
        char content_len_env[MAX];
        memset(meth_env,'\0',sizeof(meth_env));
        memset(query_env,'\0',sizeof(query_env));
        memset(content_len_env,'\0',sizeof(content_len_env));

        close(cgi_input[1]);
        close(cgi_output[0]);
        //0->input[0]
        dup2(cgi_input[0],0);
        //1->output[1]
        dup2(cgi_output[1],1);
        sprintf(meth_env,"REQUEST_METHOD=%s",method);
        putenv(meth_env);

        if(strcasecmp(method,"GET") == 0)//GET
        {
            sprintf(query_env,"QUERY_STRING=%s",query_string);
            putenv(query_env);
        }
        else//POST
        {
            sprintf(content_len_env,"CONTENT_LENGTH=%d",content_length);
            putenv(content_len_env);
        }
        execl(path,path,NULL);
        exit(1);
    }
    else//father
    {
        close(cgi_input[0]);
        close(cgi_output[1]);

        char ch = '\0';
        int i = 0;
        if(strcasecmp(method,"POST") == 0)
        {
            for(;i < content_length;++i)
            {
                recv(client_sock,&ch,1,0);
                printf("%c",ch);
                write(cgi_input[1],&ch,1);
            }
            printf("\n");
        }

        while(read(cgi_output[0],&ch,1) > 0)
        {
            printf(" %c",ch);
            send(client_sock,&ch,1,0);
        }
        printf("\n");
        close (cgi_input[1]);
        close(cgi_output[0]);

        waitpid(id,NULL,0);
    }
}

void  echo_html(int client,const char* path,size_t size)
{
    clear_header(client);
    if(!path)
      return;
    int fd = open(path,O_RDONLY,0);
    if(fd < 0)
    {
        return_errno_client(client,404);
        return;
    }
    char echo_line[1024];
    memset(echo_line,'\0',sizeof(echo_line));
    strncpy(echo_line,HTTP_VERSION,strlen(echo_line)+1);
    strcat(echo_line," 200 OK");
    strcat(echo_line,"\r\n\r\n");
    send(client,echo_line,strlen(echo_line),0);
    if(sendfile(client,fd,NULL,size)<0){
        printf("send_file error\n");
        close(fd);
        return;
    }
    printf("sendfile success\n");
    close(fd);
}
void* accept_request(void* arg)
{
    pthread_detach(pthread_self());
    int client_sock = (int)arg;
    int cgi = 0;
    char* query_string = NULL;
    char path[MAX];
    char method[MAX/10];
    char url[MAX];
    char buf[MAX];
    memset(path,'\0',sizeof(path));
    memset(method,'\0',sizeof(method));
    memset(url,'\0',sizeof(url));
    memset(buf,'\0',sizeof(buf));

#ifdef _DEBUG_
    printf("in debug mode\n");
    while(get_line(client_sock,buf,sizeof(buf)) > 0)
    {
        printf("%s",buf);
        fflush(stdout);
    }
#endif

    if((get_line(client_sock,buf,sizeof(buf)))<0)
    {
        return_errno_client(client_sock,404);
        close(client_sock);
        return (void*)-1;
    }
    //GET / HTTP/1.1
    int i = 0;//index->method
    int j = 0;//index->buf
    while(!isspace(buf[j]) && i < sizeof(method)-1 && j < sizeof(buf))
    {
        method[i] = buf[j];
        ++i;
        ++j;
    }
    //GET && POST
    if(strcasecmp(method,"GET") && strcasecmp(method,"POST"))
    {
        return_errno_client(client_sock,404);
        close(client_sock);
        return (void*)-1;
    }
    if(strcasecmp(method,"POST") == 0)
    {
        cgi = 1;   
    }

    while(isspace(buf[j]) && j < sizeof(buf))
    {
        ++j;
    }

    i = 0;
    while(!isspace(buf[j]) && i < sizeof(url)-1 && j < sizeof(buf))
    {
        url[i] = buf[j];
        ++i;
        ++j;
    }
    
    if(strcasecmp(method,"GET") == 0)
    {
        query_string = url;
        while(*query_string != '?' && *query_string != '\0')
        {
            query_string++;
        }
        if(*query_string == '?')
        {
            *query_string = '\0';
            ++query_string;
            cgi = 1;
        }
    }
    sprintf(path,"htdocs%s",url);
    if(path[strlen(path)-1] == '/')//dir
    {
         strcat(path,MAIN_PAGE);
    }
    //判断path当中文件的合法性:请求的文件可能不存在，也有可能没有权限去访问他 
    struct stat st;
    if(stat(path,&st) < 0)//文件不存在
    {
        return_errno_client(client_sock,404);
        close(client_sock);
        return (void*)-1;
    }
    else//文件存在
    {
        if(S_ISDIR(st.st_mode))// 目录
        {
            strcat(path,"/");
            strcat(path,MAIN_PAGE);
        }
        else if((st.st_mode &  S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))//可执行文件
        {
            cgi = 1;
        }
        else
        {
            //do nothing
        }
        if(cgi)
        {
            exe_cgi(client_sock,path,method,query_string);
        }
        else
        {
            clear_header(client_sock);
            echo_html(client_sock,path,st.st_size);
        }
    }

    close(client_sock);
    return (void*)0;
}

int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        usage(argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);
    int listen_sock = startup(port);

    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    while(1)
    {
        int new_sock = accept(listen_sock,(struct sockaddr*)&client,&len);
        if(new_sock < 0)
        {
            //perror("accept");
            print_log(__FUNCTION__,__LINE__,errno,strerror(errno));
            continue;
        }
        printf("get a new connect....\n");
        pthread_t new_threadId;
        pthread_create(&new_threadId,NULL,accept_request,(void*)new_sock);
        pthread_detach(new_threadId);
    }
    close(listen_sock);
    return 0;
}
