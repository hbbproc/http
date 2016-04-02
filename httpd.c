#include"httpd.h"

static void usage(const char* proc)
{
    printf("Usage: %s [PORT]\n",proc);
}

int get_line(int client,char buf[],int len)
{
    if(!buf && len <= 0)
    {
        return -1;
    }
    int i = 0;
    char c = '\0';
    int n = 0;
    while(i < len-1 && c != '\n')
    {
        n = recv(client,&c,1,0);
        if(n>0)
        {
            if(c == '\r')
            {
                n = recv(client,&c,1,MSG_PEEK);
                if(n > 0 && c == '\n')//'\r''\n'
                {
                    n = recv(client,&c,1,0);
                }
                else//'\r'
                {
                    c = '\n';
                }
            }
            buf[i++] = c;
        }
        else
        {
            c = '\n';
        }
        buf[i] = '\0';
        return i;
    }
}

int startup(int port)
{
    int listen_sock = socket(AF_INET,SOCK_STREAM,0);
    if(listen_sock < 0)
    {
        perror("socket");
        exit(2);
    }
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port   = htons(port);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t len = sizeof(local);

    if(bind(listen_sock,(struct sockaddr*)&local,len) < 0)
    {
        perror("bind");
        exit(3);
    }
    if(listen(listen_sock,_BACK_LOG_) < 0)
    {
        perror("listen");
        exit(4);
    }
    return listen_sock;
}
void  not_found(int client)
{
    char buf[1024];
    sprintf(buf,"HTTP/1.0 404 NOT FOUND\r\n");
    send(client,buf,strlen(buf));
    sprintf(buf,"COntent-type:text/html");
}

//for error_hander
void return_errno_client(int client, int status_code)
{
    switch(status_code)
    {
        case 404:
            not_found(client);
            break;
        default:
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
void exe_cgi(int client,const char* path,const char* method,const char* query_string)
{
    int numchar = -1;
    char buf[MAX];
    int content_length = -1;
    int cgi_input[2] = {0,0};
    int cgi_output[2] = {0,0};
    pid_t id;
    memset(buf,'\0',sizeof(buf));
    if(strcasecmp(method,"GET") == 0)//CGI && GET
    {
        clear_header(client);
    }
    else//CGI && POST  //1.get content_length //2.clear_header
    {
        do{
            numchar = get_line(client,buf,sizeof(buf)-1);
            if(strncasecmp(buf,"Content-Length:",15) == 0)
            {
                content_length =atoi(&buf[16]);
            }
        }while(numchar > 0 && strcmp("\n",buf));
        
        if(content_length == -1)
        {
            return_errno_client();
            return;
        }
    }

    sprintf(buf,"HTTP/1.0 200 OK\r\n\r\n");
    send(client,buf,strlen(buf),0);

    if(pipe(cgi_input) < 0)
    {
        return_errno_client();
        return;
    }
    if(pipe(cgi_output) < 0)
    {
        close(cgi_inout[0]);
        close(cgi_output[1]);
        return_errno_client();
        return;
    }
    if((id = fork()) < 0)
    {
        close(cgi_input[0]);
        close(cgi_input[1]);
        close(cgi_output[0]);
        close(cgi_output[1]);
        return_errno_client();
        return;
    }
    else if(id == 0)
    {
        char meth_env[MAX/10];
        char query_env[MAX];
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
    else
    {
        close(cgi_input[0]);
        close(cgi_output[1]);

        char ch;
        int i = 0;
        if(strcasecmp(method,"POST") == 0)
        {
            for(;i < content length;++i)
            {
                recv(client,&ch,1,0);
                printf("%c",ch);
                write(cgi_input[1],&ch,1);
            }
            printf("\n");
        }

        while(read(cgi_output[0],&ch,1) > 0)
        {
            printf(" %c",ch);
            sent(client,&ch,1,0);
        }
        printf("\n");
    }
}
void  echo_html(int client,char* path,size_t size)
{
    clear_header(client);
    int fd = open(path,O_RDONLY,0);
    if(fd < 0)
    {
        return_errno_client();
        return;
    }
    else
    {
        sendfile(client,fd,NULL,size);
    }
    close(fd);
}
void* accept_request(void* arg)
{
    pthread_detach(pthread_self());
    int client = (int)arg;
    int cgi = 0;
    char* query_string;
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
    while(get_line(client,buf,sizeof(buf)) > 0)
    {
        printf("%s");
    }
#endif
    if((get_line(client,buf,sizeof(buf)-1))<0)
    {
        return_errno_client();
        close(client);
        return (void*)-1;
    }
    //GET / HTTP/1.1
    int i = 0;//index->method
    int j = 0;//index->buf
    while(isspace(buf[j]) && i < sizeof(method)-1 && j < sizeof(buf))
    {
        method[i] = buf[j];
        ++i;
        ++j;
    }
    //GET && POST
    if(strcasecmp(method,"GET") && strcasecmp(method,"POST"))
    {
        return_errno_client();
        close(client);
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
        return_errno_client();
        close(client);
        return (void*)-1;
    }
    else
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
        {}
        if(cgi)
        {
            exe_cgi(client,path,method,query_string);
        }
        else
        {
            echo_html(client,path,st.st_size);
        }
    }

    close(client);
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
            perror("accept");
            continue;
        }
        printf("get a new connect....\n");
        pthread_t id;
        pthread_create(&id,NULL,accept_request,(void*)new_sock);
        pthread_detach(id);
    }
    close(listen_sock);
    return 0;
}
