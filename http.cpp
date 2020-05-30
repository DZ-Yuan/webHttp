#include "pch.h"
#include <cstring>

using namespace std;

Http::HTTP_STATUS_CODE Http::analyse_requestLine(char *http_req)
{
    char *p = http_req;
    // 网站资源位置
    urlpath = "./web";

    // Get http request Mode
    this->request_method = p;
    while (*p != ' ')
        p++;

    p[0] = '\0';
    if (strcmp(request_method, "GET") != 0 && strcmp(request_method, "POST") != 0)
    {
        cout << "bad request : " << request_method << endl;
        return BAD_REQUEST;
    }

    // Get request url
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

    // Get http Version
    this->httpVersion = ++p;
    while (*p != ' ')
        p++;

    p[0] = '\0';

    p = nullptr; //?
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

    // 发送响应头
    this->concat_response_header();
    send_size = send(client_sock, this->response_headers, strlen(this->response_headers), 0);
    // 发送响应内容 body
    while ((read_size = read(fp, response_body, 1500)) > 0)
    {
        send_size = send(client_sock, response_body, read_size, 0);
        cout << "Server Responsed -- size(Byte): " << send_size << endl;
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
    send(this->client_sock, this->response_headers, strlen(this->response_headers), 0);

    char sendbuf[] = "404 NOT FOUND";
    send(this->client_sock, sendbuf, strlen(sendbuf), 0);
}

void Http::response_FORBIDDEN()
{
}

void Http::response_NOT_FOUND()
{
    this->concat_response_header();
    send(this->client_sock, this->response_headers, strlen(this->response_headers), 0);

    char sendbuf[] = "404 NOT FOUND";
    send(this->client_sock, sendbuf, strlen(sendbuf), 0);
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
        // strcat(response_headers, "Content-Length: 20749\r\n");
        strcat(response_headers, "Connection: close\r\n");

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
