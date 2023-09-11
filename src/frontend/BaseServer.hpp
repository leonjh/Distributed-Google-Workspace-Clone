#ifndef BASE_SERVER_HPP
#define BASE_SERVER_HPP

#include <arpa/inet.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>

#include "definitions.hpp"
#include "utils.hpp"

void *worker_wrapper(void *connection);
void signal_wrapper(int sig_num);

class BaseServer
{
protected:
  const std::string GREETING_MESSAGE{"+OK Server ready (Author: Andy Jiang / jianga)\r\n"};

  void parse_html(Request *request)
  {
    std::string message = request->message;
    std::string buffer;

    HTTP_Message *html_message = new HTTP_Message();
    request->http_message = html_message;

    bool found_r = false;
    for (int i = 0; i < message.size(); i++)
    {
      buffer += message[i];
      if (message[i] == '\r')
      {
        found_r = true;
      }
      else if (found_r && message[i] == '\n' && buffer.length() > 2)
      {
        if (request->http_message->type.size() == 0)
        {
          std::vector<std::string> parts = split_string(buffer, ' ', 3);
          request->http_message->type = parts[0];
          request->http_message->path = parts[1];
        }
        else
        {
          std::vector<std::string> parts = split_string(buffer, ' ', 2);
          std::pair<std::string, std::string> property = {parts[0].substr(0, parts[0].length() - 1),
                                                          parts[1].substr(0, parts[1].length() - 2)};
          request->http_message->header.insert({property.first, property.second});
        }
        buffer = "";
      }
      else
      {
        found_r = false;
      }
    }
  }

public:
  struct Connection
  {
    int comm_fd;
    int thread_status;
    pthread_t thread;
  };

  int port = 3000; // default port
  int v_flag = 0;
  int socket_fd;
  std::vector<Connection *> connections;

  BaseServer(int port, int v_flag) : port{port}, v_flag{v_flag} {}

  void start()
  {
    signal(SIGINT, signal_wrapper);
    int listen_fd = connect_to_port(port); // create new socket
    if (listen_fd < 0)
    {
      fprintf(stderr, "Error while connecting to port\n");
      exit(EXIT_FAILURE);
    }

    handle_connection(listen_fd);
  }

  void handle_connection(int listen_fd)
  {
    while (true)
    {
      struct sockaddr_in clientaddr;
      socklen_t clientaddrlen = sizeof(clientaddr);

      int comm_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientaddrlen);
      if (comm_fd < 0)
      {
        perror("accept():");
        exit(EXIT_FAILURE);
      }

      cleanup_threads(); // check and clean up exited threads

      Connection *connection = new Connection();
      connection->comm_fd = comm_fd;
      connection->thread_status = THREAD_ALIVE;
      connections.push_back(connection);

      log(comm_fd, "New connection", v_flag);

      // start new pthread to read commands and process them
      pthread_t thread;
      pthread_create(&thread, NULL, worker_wrapper, (void *)connection);

      connection->thread = thread;
    }
  }

  void *worker(void *connection)
  {
    // create sigmask for each thread so that it blocks SIGINT, only main thread should receive
    // it
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    int s = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (s != 0)
    {
      fprintf(stderr, "Error with sigmask\n");
    }

    char read_buffer[MAX_BUFFER_LENGTH];
    std::vector<char> message_buffer;
    bool found_r = false;
    bool found_cr = false;

    Request *request = new Request();
    request->type = -1;

    int readBytes;
    while (readBytes = read(((Connection *)connection)->comm_fd, read_buffer, MAX_BUFFER_LENGTH))
    {
      if (request->type == -1)
      {
        if (read_buffer[0] == '!')
        {
          request->type = SERVER_REQUEST;
        }
        else
        {
          request->type = CLIENT_REQUEST;
        }
      }

      for (int i = 0; i < readBytes; i++)
      {
        message_buffer.push_back(read_buffer[i]); // command buffer stores the current command being parsed
        if (read_buffer[i] == '\r')
        { // track if <R> has been right before
          found_r = true;
        }
        else if ((read_buffer[i] == '\n' && found_r) &&
                 (request->type == SERVER_REQUEST || (request->type == CLIENT_REQUEST && found_cr)))
        { // <CR> found - end of command, execute it
          std::string command(message_buffer.begin(), message_buffer.end());
          if (request->type == SERVER_REQUEST)
          {
            command = command.substr(1, command.length() - 1);
            request->message = command;
          }
          else
          {
            request->message = command;
            parse_html(request);

            // read body if any
            if (request->http_message->header.find("Content-Length") != request->http_message->header.end())
            {
              int content_length = stoi(request->http_message->header["Content-Length"]);
              if (content_length > 0)
              {
                std::cout << "wtf is the content length: " << content_length << std::endl;
                int body_read_bytes;
                char body_read_buffer[MAX_BUFFER_LENGTH];
                std::vector<char> body_message_buffer;

                for (int j = i + 1; j < readBytes; j++)
                { // get from current read buffer
                  body_message_buffer.push_back(read_buffer[j]);
                  if (body_message_buffer.size() > content_length)
                  {
                    break;
                  }
                }

                if (body_message_buffer.size() < content_length)
                {
                  int bytesLeft = content_length - body_message_buffer.size();
                  bytesLeft = bytesLeft > MAX_BUFFER_LENGTH ? MAX_BUFFER_LENGTH : bytesLeft;
                  while (body_read_bytes = read(((Connection *)connection)->comm_fd, body_read_buffer, bytesLeft))
                  {
                    for (int i = 0; i < body_read_bytes; i++)
                    {
                      body_message_buffer.push_back(body_read_buffer[i]);
                      if (body_message_buffer.size() > content_length)
                      {
                        break;
                      }
                    }
  
                    if (body_message_buffer.size() > content_length)
                    {
                      break;
                    }

                    bytesLeft = content_length - body_message_buffer.size();
                    bytesLeft = bytesLeft > MAX_BUFFER_LENGTH ? MAX_BUFFER_LENGTH : bytesLeft;
                  }
                }

                std::string body_command(body_message_buffer.begin(), body_message_buffer.end());
                request->message += body_command;
                request->http_message->body = body_command;
              }
            }
          }

          if (handle_request((Connection *)connection, request))
          {
            break;
          }
          message_buffer.clear();
          if (request->type == CLIENT_REQUEST && request->http_message->body.length() > 0)
          {
            request = new Request();
            request->type = -1;
            break;
          }
          request = new Request();
          request->type = -1;
        }
        else if (read_buffer[i] == '\n' && found_r)
        {
          found_cr = true;
        }
        else
        { // reset if not <R> or \n
          found_r = false;
          found_cr = false;
        }
      }

      if (((Connection *)connection)->thread_status == THREAD_NEED_CLEANUP)
      {
        close(((Connection *)connection)->comm_fd);
        log(((Connection *)connection)->comm_fd, "Connection closed", v_flag);
        break;
      }
    }

    return 0;
  }

  virtual bool handle_request(Connection *connection, Request *request) = 0;

  /**
   * create socket and listen at port
   * return listen fd, < 0 if error
   */
  int connect_to_port(int port)
  {
    int ret;

    int listen_fd = socket(PF_INET, SOCK_STREAM, 0); // create new socket
    if (listen_fd < 0)
    {
      perror("socket():");
      return ret;
    }

    socket_fd = listen_fd; // store for cleanup

    // fixes "connection refused" message
    int opt = 1;
    ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    if (ret < 0)
    {
      perror("setsockopt():");
      return ret;
    }

    // bind to listen fd
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port);

    ret = bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret < 0)
    {
      perror("bind():");
      return ret;
    }

    ret = listen(listen_fd, 100); // listen on port
    if (ret < 0)
    {
      perror("listen():");
      return ret;
    }

    return listen_fd;
  }

  // check if any thread exited and hasn't been cleaned up
  void cleanup_threads()
  {
    for (int i = 0; i < connections.size(); i++)
    {
      if (connections[i]->thread_status == THREAD_NEED_CLEANUP)
      {
        pthread_join(connections[i]->thread, NULL);
        connections[i]->thread_status = THREAD_CLEANED;
      }
    }
  }

  /**
   * handle control c case in main thread
   * need to send err message to all threads that are alive and clean up threads
   */
  void signal_handler(int sig_num)
  {
    if (sig_num == SIGINT)
    {
      std::cout << "innnnn" << std::endl;
      fprintf(stderr, "\n");
      // go over all connections and send message to alive thread and clean up threads
      for (int i = 0; i < connections.size(); i++)
      {
        if (connections[i]->thread_status == THREAD_ALIVE)
        {
          // send shutdown message
          log(connections[i]->comm_fd, "Server shutdown", v_flag);
          // do_write(curr_connections[i]->comm_fd, server_shutdown_message.c_str(),
          // server_shutdown_message.length());

          close(connections[i]->comm_fd); // close comm fd

          // close thread and clean up resources
          pthread_cancel(connections[i]->thread);
          pthread_join(connections[i]->thread, NULL);
          connections[i]->thread_status = THREAD_CLEANED;
        }
        else if (connections[i]->thread_status == THREAD_NEED_CLEANUP)
        { // thread already exited, just need to clean up
          pthread_join(connections[i]->thread, NULL);
          connections[i]->thread_status = THREAD_CLEANED;
        }
      }
      close(socket_fd); // close server socket
      exit(EXIT_SUCCESS);
    }
  }
};

BaseServer *server;

void *worker_wrapper(void *connection)
{
  server->worker(connection);
  return NULL;
}

void signal_wrapper(int sig_num)
{
  server->signal_handler(sig_num);
}

#endif
