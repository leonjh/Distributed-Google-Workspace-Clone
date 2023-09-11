#ifndef DEFINITIONS_H__
#define DEFINITIONS_H__

#define MAX_BUFFER_LENGTH 1024

// thread states
#define THREAD_ALIVE 0
#define THREAD_NEED_CLEANUP 1
#define THREAD_CLEANED 2

// request types
#define CLIENT_REQUEST 0
#define SERVER_REQUEST 1

class BaseServer;
extern BaseServer *server;

typedef struct http_message
{
  std::string type;
  std::string path;
  std::string body;
  std::unordered_map<std::string, std::string> header;
} HTTP_Message;

typedef struct request
{
  int type;
  std::string message;
  HTTP_Message *http_message;
} Request;

#endif
