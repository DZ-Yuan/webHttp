#include "pch.h"

using namespace std;

class Http
{
private:
    int client_sock;
    int responseInfo_len;
    char requestInfo[1000];

    char *request_method;
    char *url;
    char *httpVersion;
    char *acceptType;

    char response_line[255];
    char response_buf[1000];

    struct stat stat_buf;

    vector<string> errorInfo;

public:
    Http(){};

    enum HTTP_STATUS_CODE
    {
        REQUEST_OK = 200,
        BAD_REQUEST = 400,
        FORBIDDEN_REQUEST = 401,
        NOT_FOUND = 404
    };

    char response_body[1500];
    char response_headers[1024];

    string urlpath;

    HTTP_STATUS_CODE status;
    
    void analysis(int, char *);
    HTTP_STATUS_CODE analyse_requestLine(char *);
    void request_head_analyse();

    void response_OK();
    void response_BAD_REQUEST();
    void response_FORBIDDEN();
    void response_NOT_FOUND();

    void concat_response_header();

private:
    //HTTP_STATUS_CODE status;
};