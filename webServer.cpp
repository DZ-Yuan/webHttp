#include "pch.h"
#include <iostream>

using namespace std;

const int port = 1234;

void webServer()
{
    char sendBuf[255] = "Web Server has been receive request !";
    char recvBuf[1000] = {0};
    char requestInfo[1000] = {0};

    // char response_body[2048];
    int recv_size;
    int read_size, send_size, totalSend_size;

    struct stat stat_buf;
    deque<int> client_sock_deque;

    Http http;
    threadPools Tpools;

    //
    int m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (m_sock < 0)
        std::cout << "Can't open Server connect";

    std::cout << "Open Server Connection..." << std::endl;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(m_sock));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("172.18.184.76");
    server_addr.sin_port = htons(1234);

    bind(m_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(m_sock, 20);

    std::cout << "Listen Client connect..." << std::endl;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    while (1)
    {
        memset(recvBuf, 0, sizeof(recvBuf));
        memset(requestInfo, 0, sizeof(requestInfo));

        totalSend_size = 0;
        int client_sock = accept(m_sock, (struct sockaddr *)&client_addr, &client_addr_size);

        // 设置非阻塞(永久) or recv(,,,MSG_DONTWAIT)
        // int flags = fcntl(client_sock, F_GETFL, 0);
        // fcntl(client_sock, F_SETFL, flags|O_NONBLOCK);

        if (client_sock < 0)
        {
            std::cout << "Error" << std::endl;
        }
        else
        {
            // --------------------------------------------------------------------
            std::cout << "Client connect successful! " << std::endl;

            recv(client_sock, recvBuf, 1000, 0);
            // while (recv_size = recv(client_sock, recvBuf, 1000, MSG_DONTWAIT) > 0)
            // {
            //     cout << recvBuf << endl;
            //     strcat(requestInfo, recvBuf);
            // }

            cout << recvBuf << endl;
            // std::cout << "Client request :  \n"
            //           << requestInfo;
            cout << endl;

            cout << strlen(recvBuf) << endl;

            if (strlen(recvBuf) > 100)
            {
                http.analysis(client_sock, recvBuf);
            }
            else
            {
                cout << "unknow request" << endl;
            }

            // ----------------------------------------------------------------------
            //char buf[] = "HTTP/1.1 200 ok\r\nconnection: close\r\nContent-Type: text/html\r\n\r\n";

            // send(client_sock, buf, sizeof(buf), 0);
            // int fd = open("hello.html", O_RDONLY);
            // sendfile(client_sock, fd, NULL, 4096);

            // close(fd);
            // close(client_sock);
        }
    }
}

// -------------------------  测试  -----------------------------------------
void testWebServer()
{
    char sendBuf[255] = "Web Server has been receive request !";
    char recvBuf[2500] = {0};

    char response_body[2048];
    int recv_size;
    Http http;

    struct stat stat_buf;
    int read_size, send_size, totalSend_size;

    int m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (m_sock < 0)
        std::cout << "Can't open Server connect";

    std::cout << "Open Server connect..." << std::endl;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(m_sock));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("120.79.114.70");
    server_addr.sin_port = htons(port);

    bind(m_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(m_sock, 20);

    std::cout << "Listen Client connect..." << std::endl;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    while (1)
    {
        totalSend_size = 0;
        int client_sock = accept(m_sock, (struct sockaddr *)&client_addr, &client_addr_size);

        if (client_sock < 0)
        {
            std::cout << "Error" << std::endl;
        }
        else
        {
            int fexist = stat("html/index.html", &stat_buf);
            //char buf[] = "HTTP/1.1 200 OK\r\n";

            //send_size = send(client_sock, buf, sizeof(buf), 0);
            // cout << "buf send size: " << send_size << endl;
            http.status = http.REQUEST_OK;
            http.concat_response_header();
            send_size = send(client_sock, http.response_headers, strlen(http.response_headers), 0);
            cout << "header send size: " << send_size << endl;

            cout << "file size: " << stat_buf.st_size << endl;
            int fp = open("html/index.html", O_RDONLY);

            while ((read_size = read(fp, response_body, 2048)) > 0)
            {
                cout << "read_size : " << read_size << endl;
                send_size = send(client_sock, response_body, read_size, 0);
                cout << "Server Responsed -- size(Byte): " << send_size << endl;
                totalSend_size += send_size;
            }

            // int totalSend_size = sendfile(client_sock, fp, NULL, stat_buf.st_size);

            // cout << "file: \n"
            //      << response_body << endl;

            //send_size = send(client_sock, response_body, stat_buf.st_size, 0);

            // cout << "Server Responsed -- total send Byte: " << totalSend_size << endl;

            close(fp);
            shutdown(client_sock, SHUT_WR);
            // close(client_sock);
        }
    }
}
