#include "pch.h"
#include <iostream>

using namespace std;

const int port = 8080;

// 设置socket非阻塞
int setNonBlock(int sock)
{
    if ((fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK)) < 0)
    {
        return -1;
    }
    return 0;
}

// flag == 0 --- ET, flag == 1 --- LT
int epolladd(int sock, int epfd, epoll_event ep_ev, int ETLT_flag = 0)
{
    ep_ev.data.fd = sock;
    if (ETLT_flag == 0)
    {
        ep_ev.events = EPOLLIN | EPOLLET;
    }
    else
    {
        ep_ev.events = EPOLLIN;
    }
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ep_ev);
}

void webServer()
{
    int m_sock, client_sock; // socket
    int epfd, fds;           // epoll
    char sendBuf[255] = "Web Server has been receive request !";
    deque<int> client_sock_deque;
    threadPools Tpools(20);
    mutex mtx_dequeSock;
    struct epoll_event ep_ev, fd_evnts[255];

    Tpools.start();

    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (m_sock < 0)
        std::cout << "Can't open Server connect";

    std::cout << "Open Server Connection..." << std::endl;

    // 非阻塞
    setNonBlock(m_sock);
    // 端口复用
    int flag = 1;
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(m_sock));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("172.18.184.76");
    server_addr.sin_port = htons(port);

    bind(m_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(m_sock, 20);
    std::cout << "Listen Client connect..." << std::endl;

    // epoll
    // 注册监听事件
    // epolladd(m_sock, epfd, ep_ev, 0);
    epfd = epoll_create(20);
    ep_ev.data.fd = m_sock;
    ep_ev.events = EPOLLIN | EPOLLET;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, m_sock, &ep_ev);
    cout << ret << endl;

    // 线程任务管理
    Tpools.doit_auto();

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    while (1)
    {
        // ======================== epoll + 多线程 ==================================
        fds = epoll_wait(epfd, fd_evnts, sizeof(fd_evnts), -1);
        cout << "fds: " << fds << endl;
        for (int i = 0; i < fds; i++)
        {
            if (fd_evnts[i].data.fd == m_sock)
            {
                // cout << "Client Connection" << endl;
                client_sock = accept(m_sock, (struct sockaddr *)&client_addr, &client_addr_size);
                // cout << "client_sock: " << client_sock << endl;
                setNonBlock(client_sock);
                // epolladd(client_sock, epfd, ep_ev, 0);
                ep_ev.data.fd = client_sock;
                ep_ev.events = EPOLLIN | EPOLLET;
                int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, client_sock, &ep_ev);
                // cout << ret << endl;
            }
            else
            {
                cout << "Http Request" << endl;
                mtx_dequeSock.lock();
                client_sock_deque.push_back(fd_evnts[i].data.fd);
                mtx_dequeSock.unlock();

                // 添加任务给工作线程
                Tpools.addTask([&] {
                    mtx_dequeSock.lock();
                    int clientSock = client_sock_deque.front();
                    client_sock_deque.pop_front();
                    mtx_dequeSock.unlock();
                    // cout << "fd: " << clientSock << endl;

                    int recv_size;
                    char recvBuf[1000] = {0};

                    Http http;

                    if (clientSock < 0)
                    {
                        cout << "Error" << endl;
                    }
                    else
                    {
                        // --------------------------------------------------------------------
                        while (1)
                        {
                            recv_size = recv(clientSock, recvBuf, 1000, 0);

                            if (recv_size == 0)
                            {
                                cout << "FIN" << endl;
                                close(clientSock);
                                break;
                            }
                            else if (recv_size == -1)
                            {
                                if (errno == EAGAIN)
                                {
                                    cout << "EAGAIN" << endl;
                                    break;
                                }
                                else
                                {
                                    cout << "errno: " << errno << endl;
                                    close(clientSock);
                                    break;
                                }
                            }
                            // cout << recvBuf << endl;
                            // cout << "recv_size: " << recv_size << endl;

                            if (strlen(recvBuf) > 100)
                            {
                                http.analysis(clientSock, recvBuf);
                            }
                            else
                            {
                                cout << "unknow request" << endl;
                            }

                            memset(recvBuf, 0, sizeof(recvBuf));
                        }
                        // cout << "Client has been closed connection" << endl;
                    }
                });
                cout << "Client Request has been Done" << endl;
            }
            // Tpools.getPoolsInfo();
        }
    }
    //=========================================================================

    // // ========================= 多线程 =====================================
    // cout << "Waiting for connection" << endl;
    // client_sock = accept(m_sock, (struct sockaddr *)&client_addr, &client_addr_size);

    // cout << "Client connect successful! " << endl;
    // client_sock_deque.push_back(client_sock);

    // Tpools.addTask([&] {
    //     int clientSock = client_sock_deque.front();
    //     client_sock_deque.pop_front();

    //     int recv_size;
    //     char recvBuf[1000] = {0};

    //     Http http;

    //     if (clientSock < 0)
    //     {
    //         cout << "Error" << endl;
    //     }
    //     else
    //     {
    //         // --------------------------------------------------------------------
    //         while ((recv_size = recv(clientSock, recvBuf, 1000, 0)) > 0)
    //         {
    //             cout << recvBuf << endl;
    //             cout << endl;

    //             cout << "requestInfo Length: " << strlen(recvBuf) << endl;

    //             if (strlen(recvBuf) > 100)
    //             {
    //                 http.analysis(clientSock, recvBuf);
    //             }
    //             else
    //             {
    //                 cout << "unknow request" << endl;
    //             }

    //             memset(recvBuf, 0, sizeof(recvBuf));
    //         }
    //     }

    //     close(clientSock);
    //     // shutdown(clientSock, SHUT_RDWR);
    //     cout << "Client has been closed connection" << endl;
    // });
    // =================================================================

    // ======================== 单线程 ==================================
    // if (client_sock < 0)
    // {
    //     std::cout << "Error" << std::endl;
    // }
    // else
    // {
    //     std::cout << "Client connect successful! " << std::endl;
    //     recv(client_sock, recvBuf, 1000, 0);

    //     cout << recvBuf << endl;
    //     cout << endl;

    //     cout << "requestInfo Length: " << strlen(recvBuf) << endl;

    //     if (strlen(recvBuf) > 100)
    //     {
    //         http.analysis(client_sock, recvBuf);
    //     }
    //     else
    //     {
    //         cout << "unknow request" << endl;
    //     }
    //  }
    // ===============================================================

    // close(fd);
    // close(client_sock);
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

    // 设置非阻塞(永久) or recv(,,,MSG_DONTWAIT)
    // int flags = fcntl(client_sock, F_GETFL, 0);
    // fcntl(client_sock, F_SETFL, flags|O_NONBLOCK);

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
