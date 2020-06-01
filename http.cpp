#include "pch.h"
#include <cstring>

using namespace std;

Http::HTTP_STATUS_CODE Http::analyse_requestLine(char *http_req)
{
    char *p = http_req;
    // 网站资源位置
    urlpath = "./web";

    // 获取 http 请求方式
    this->request_method = p;
    while (*p != ' ')
        p++;

    p[0] = '\0';
    if (strcmp(request_method, "GET") != 0 && strcmp(request_method, "POST") != 0)
    {
        cout << "bad request : " << request_method << endl;
        return BAD_REQUEST;
    }

    // 获取 http url
    this->url = ++p;
    while (*p != ' ')
        p++;

    p[0] = '\0';
    if (strcmp(url, "/") == 0)
    {
        urlpath = "./web/html/index.html";
    }
    else
    {
        urlpath += url;
        urlpath = URLdecode(urlpath);
    }

    // 文件是否存在
    int fexist = stat(urlpath.c_str(), &stat_buf);
    if (fexist == -1)
    {
        cout << "file Not exist" << endl;
        cout << "path:  " << urlpath << endl;
        return NOT_FOUND;
    }

    // http Version
    this->httpVersion = ++p;
    while (*p != ' ')
        p++;

    p[0] = '\0';

    p = nullptr; 
    return REQUEST_OK;
}

void Http::analysis(int client_sock, char *http_req)
{
    strcpy(this->requestInfo, http_req);
    this->client_sock = client_sock;

    status = analyse_requestLine(this->requestInfo);

    switch (status)
    {
    case REQUEST_OK:
        response_OK();
        break;

    case BAD_REQUEST:
        response_BAD_REQUEST();
        break;

    case FORBIDDEN_REQUEST:
        response_FORBIDDEN();
        break;

    case NOT_FOUND:
        response_NOT_FOUND();
        break;

    default:
        break;
    }
}

void Http::response_OK()
{
    int read_size = 0, send_size = 0, totalSend_size = 0;
    //bzero(this->response_buf, sizeof(this->response_buf));

    // 检查并打开需要发送的文件
    int fp = open(urlpath.c_str(), O_RDONLY);
    if(fp < 0)
    {
        this->response_NOT_FOUND();
        return;
    }
    // 获取文件大小
    this->responseBody_len = stat_buf.st_size;
    // 发送响应头
    this->concat_response_header();
    if((send_size = send(client_sock, this->response_headers, strlen(this->response_headers), MSG_NOSIGNAL)) == -1)
    {
        close(client_sock);
    }
    // 发送响应内容 body
    while ((read_size = read(fp, response_body, 1500)) > 0)
    {
        if((send_size = send(client_sock, response_body, read_size, MSG_NOSIGNAL) == -1))
        {
            close(client_sock);
        }
        //cout << "Server Responsed -- size(Byte): " << send_size << endl;
        totalSend_size += send_size;
    }

    // cout << "file: \n"
    //      << response_body << endl;

    // send_size = send(client_sock, response_body, stat_buf.st_size, 0);

    cout << "Server Responsed -- total Send Byte: " << totalSend_size << endl;
    cout << endl;

    close(fp);
    // shutdown(client_sock, SHUT_RDWR);
    // close(client_sock);
}

void Http::response_BAD_REQUEST()
{
    this->concat_response_header();
    if((send(this->client_sock, this->response_headers, strlen(this->response_headers), MSG_NOSIGNAL)) == -1)
    {
        close(this->client_sock);
    }

    char sendbuf[] = "404 NOT FOUND";
    if((send(this->client_sock, sendbuf, strlen(sendbuf), MSG_NOSIGNAL)) == -1)
    {
        close(this->client_sock);
    }
}

void Http::response_FORBIDDEN()
{
}

void Http::response_NOT_FOUND()
{
    this->concat_response_header();
    if((send(this->client_sock, this->response_headers, strlen(this->response_headers), MSG_NOSIGNAL)) == -1)
    {
        close(this->client_sock);
    }

    char sendbuf[] = "404 NOT FOUND";
    if((send(this->client_sock, sendbuf, strlen(sendbuf), MSG_NOSIGNAL)) == -1)
    {
        close(this->client_sock);
    }
}

void Http::concat_response_header()
{
    memset(response_headers, '\0', 1024);
    switch (this->status)
    {
    case REQUEST_OK:
        strcat(response_headers, "HTTP/1.1 200 ok\r\n");
        strcat(response_headers, "Server: Linux\r\n");
        // strcat(response_headers, "Content-type: text/html; charset=UTF-8\r\n");
        
        strcat(response_headers, "Content-Length: ");
        // 也可用sprintf 将int装str，再用strncat拼接前strlen()个字符获得Content-Length信息
        strcat(response_headers, to_string(responseBody_len).c_str());
        strcat(response_headers, "\r\n");

        // 使用长连接要给出Content-Length值（body的长度）给客户端（浏览器）判断服务端该次数据传输是否结束
        strcat(response_headers, "Connection: keep-alive\r\n");
        // 使用分块传输（chunked）(unused)
        //strcat(response_headers, "Transfer-Encoding: chunked\r\n");

        //结束请求头信息
        strcat(response_headers, "\r\n");
        break;

    case BAD_REQUEST:
        strcat(response_headers, "HTTP/1.1 400 bad\r\n");
        strcat(response_headers, "Server: Linux\r\n");
        strcat(response_headers, "Content-type: text/html; charset=UTF-8\r\n");
        // strcat(response_headers, "Content-Length: 20749\r\n");
        strcat(response_headers, "Connection: close\r\n");

        strcat(response_headers, "\r\n");
        break;

    default:
        break;
    }
}
