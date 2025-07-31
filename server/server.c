#include <header.h>
//某个用户10s没有说话，踢出
typedef struct conn_s{
    int client_fd;
    time_t last_time;//用于超时踢出
    int status;//当为0时为TCP，为1时为HTTP;默认为TCP
}conn;


int getparameter(char *key, char *value){

    FILE * file = fopen("config.ini", "r");
    while(1){
        char line[100];
        bzero(line, sizeof(line));
        // 读一行数据
        char *res = fgets(line, sizeof(line), file);
        if(res == NULL){
            char buf[] = "没有要找的内容 \n";
            memcpy(value, buf, strlen(buf));
            return -1;
        }
        // 处理数据
        char *line_key = strtok(line, "=");
        if(strcmp(key, line_key) == 0){
            // 要找的内容
            char *line_value = strtok(NULL, "=");
            memcpy(value, line_value, strlen(line_value));
            return 0;
        }
    }

    return 0;
}

void Current_Time(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_info;
    // 使用localtime_r替代localtime
    localtime_r(&now, &tm_info);
    if (size >= 20)
    {
        strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);
    }
    else
    {
        buffer[0] = '\0';
    }
}
int main() {

    char ip[100];
    bzero(ip, sizeof(ip));
    getparameter("ip", ip);

    char port[100];
    bzero(port, sizeof(port));
    getparameter("port", port);

    int fd_w = open("log.txt", O_RDWR | O_APPEND | O_CREAT,0777);
    int fd_log = open("Log_File.txt",O_WRONLY | O_APPEND | O_CREAT,0777);
    ERROR_CHECK(fd_w, -1, "open_w");
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(socket_fd, -1, "socket");

    struct sockaddr_in sock;
    sock.sin_family = AF_INET;
    sock.sin_port = htons(atoi(port));
    sock.sin_addr.s_addr = inet_addr(ip);

    int resue = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &resue, sizeof(resue));

    bind(socket_fd, (struct sockaddr *)&sock, sizeof(sock));

    listen(socket_fd, 10);
    conn list[100];//用来记录所有用户的连接
    memset(list,0,sizeof(list));
    int size = 0;//记录有多少个用户连接到了服务器
    fd_set base_set;
    FD_ZERO(&base_set);
    FD_SET(socket_fd,&base_set);
    while(1){
        fd_set set;
        FD_ZERO(&set);
        memcpy(&set,&base_set,sizeof(base_set));

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        select(100,&set,NULL,NULL,&tv);

        if(FD_ISSET(socket_fd,&set)){
            int net_fd = accept(socket_fd, NULL, NULL);//每一个net_fd都是文件描述符数组的一个元素，本身是一个int型数据
            printf("---------accept---------\n");
            FD_SET(net_fd,&base_set);
            list[size].client_fd = net_fd;
            //time函数是获取当前时间存到某个时间
            time(&list[size].last_time);
            size++;

        }
        //遍历所有的用户连接连接
        for(int i = 0;i<size;i++)
        {
            int client_fd = list[i].client_fd;
            if(FD_ISSET(client_fd,&set)){
                char buf[2048] = {0};//接收esp01s或者请求报文
                                     //在TCP模式，接收ESP01S透传模式发送过来的数据，在HTTP模式，接收HTTP连接时，手机发送的请求报文
                ssize_t ret = recv(client_fd, buf, sizeof(buf), 0);
                if(buf[0]!='$'&& (buf[0] != 'A' || buf[1] != 'T')&&(buf[0]!= 'G'||buf[1]!='E'||buf[2]!='T')){
                    printf("buf:%s \n",buf);//查看传过来的数据，打印在终ist[i].client端
                }
                if (buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T' ) {
                    list[i].status = 1;//为HTTP
                }

                if(ret <= 0)
                {
                    FD_CLR(list[i].client_fd,&base_set);
                    close(list[i].client_fd);

                    if (i < size - 1) {
                        memmove(&list[i], &list[i+1], (size - i - 1) * sizeof(conn));
                    }
                    size--;
                    i--;
                    continue;
                }
                //更新最新说话时间
                time(&list[i].last_time);
                int len = strlen(buf);
                if(len <= 0)
                {
                    FD_CLR(list[i].client_fd,&base_set);
                    close(list[i].client_fd);
                    continue;
                }

                // STM32传过来的数据写进log.txt&len
                // 每次写都追加写
                char buffer[26] = {0};
                if(list[i].status == 0){//为TCP时才往文件中写数据
                    if(buf[0]!='$'&&len < 16 && (buf[0] != 'A' || buf[1] != 'T')){
                        write(fd_w, &len, sizeof(len));

                        write(fd_w, buf, len);  //将收到的数据写进文件中

                        Current_Time(buffer,26);
                        dprintf(fd_log, "%s:%s:%s:%d:%s\n",buffer,__FILE__, __func__, __LINE__, buf);
                    }               
                }
                //----------------------------数据写进文件结束---------------------------------
                if(list[i].status == 1){//为HTTP才从文件中读数据
                                        //不是自己-->转发
                    char *str[1024] = {0};
                    char str1[2000] = {0};  //用于HTTP响应报文的拼接
                                            //将log.txt中的数据写进*str缓冲区
                                            //每次读都从头读
                    int k = 0;//str数组下标,也是元素的个数
                    lseek(fd_w, 0, SEEK_SET);  //让fd_w指针回到文件开头
                    char buf1[30] = {0};
                    printf("---------------------file_content-----------------------\n");
                    while (1) {
                        int len2 = 0;//str列下标
                        bzero(buf1, sizeof(buf1));
                        read(fd_w, &len2, sizeof(len2));
                        int ret = read(fd_w, buf1, len2);
                        if (ret == 0) {
                            break;
                        }
                        // 修改此行：str[i++] = buf1;可能需要删除\n
                        str[k] = (char *)malloc(len2);  // 动态分配内存
                        memcpy(str[k], buf1, len2);      // 复制数据
                        str[k][len2 - 2] = '\0';        // 添加终止符,因为文件里的数据带/r/n
                        printf("data:%s \n", str[k]);
                        k++;
                    }
                    //--------------------------文件数据写到str字符串数组指针结束--------------------------------------

                    //字符串拼接，并将凭借后的内容发送到手机或者由esp01s到PC端，以is的形式
                    // 第一次发送：HTTP 头
                    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n[", strlen("HTTP/1.1 200 OK\r\n\r\n["), 0);

                    for(int j=0;j<k;j++)
                    {
                        // 中间发送：除最后一个元素的其它所有元素
                        if (j > 0) send(client_fd, ",", 1, 0);
                        sprintf(str1, "\"%s\"", str[j]);  // 注意逗号
                        send(client_fd, str1, strlen(str1), 0);
                    }

                    // 最后一次发送：闭合括号
                    send(client_fd, "]", 1, 0);
                    //                    ////--------------------------------HTTP响应报文结束---------------------------------

                    //关闭HTTP连接
                    FD_CLR(list[i].client_fd,&base_set);
                    close(list[i].client_fd);
                    if (i < size - 1) {
                        memmove(&list[i], &list[i+1], (size - i - 1) * sizeof(conn));
                    }
                    size--;
                    i--;

                    // 在内层循环结束后统一释放
                    for(int m = 0; m < k; m++) { // k为实际分配的数量
                        free(str[m]);
                    }
                }
            }

        }
        //遍历所有用户，查看是否超时
        time_t now_time;
        time(&now_time);

        char release_tag = '$';

        for(int i = 0;i<size;i++){
            if(now_time - list[i].last_time > 60)//思考：需要不要移除HTTP
            {
                //移除监听

                //超时了，发送一个特殊字符给客户端
                //往客户端发送断开连接字符
                if(list[i].status == 0){//只有要移除的是TCP连接才发送
                    send(list[i].client_fd,&release_tag,sizeof(char),0);
                }
                printf("------------server_release_%d-----------\n ",list[i].status);
                FD_CLR(list[i].client_fd,&base_set);
                close(list[i].client_fd);
                printf("------------server_release_-----------\n ");
                //超时给客户端发送特殊字符

                // 修改后代码
                if (i < size - 1) {
                    memmove(&list[i], &list[i+1], (size - i - 1) * sizeof(conn));
                }
                size--;
                i--;
            }
        }

    }

    close(fd_w);
    close(fd_log);

    //服务器不要死，让客户端先挥手
    while (1);


    close(socket_fd);
    return 0;
}
