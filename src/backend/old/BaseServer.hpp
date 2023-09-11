#ifndef BASE_SERVER_HPP
#define BASE_SERVER_HPP

#include <atomic>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/**
 * Base server abstract class. Starts server by creating socket and binding and listening to
 * specified port
 */
class BaseServer
{
public:
  const int max_num_fds = 5000;
  const int backlog = 256;
  const int portno;
  int sockfd = -1;
  std::vector<pthread_t> threads;
  const std::string shutting_down_message = "-ERR Server shutting down\r\n";

  /**
   * Sigint handler. Iterates through existing threads and sends a shutting down message to
   * each client and closes and frees file descriptor.
   */
  void int_handler(int sig)
  {
    for (int i = 0; i < threads.size(); ++i)
    {
      if (threads[i] == -1)
      {
        continue;
      }
      pthread_cancel(threads[i]);
      pthread_join(threads[i], NULL);
      write(i, shutting_down_message.c_str(), strlen(shutting_down_message.c_str()));
      close(i);
    }
    if (sockfd != -1)
    {
      close(sockfd);
    }
    exit(EXIT_SUCCESS);
  }

  virtual void server_loop() = 0;

  BaseServer(int portno) : threads(max_num_fds, -1), portno{portno} {}

  void start()
  {
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
      perror("socket():");
      exit(EXIT_FAILURE);
    }

    const int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
      perror("setsockopt(SO_REUSEADDR):");
      exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portno);

    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
      perror("bind():");
      exit(EXIT_FAILURE);
    }

    if ((listen(sockfd, backlog)) != 0)
    {
      perror("listen():");
    }

    std::cout << "Server listening" << std::endl;

    server_loop();
  }
};

#endif
