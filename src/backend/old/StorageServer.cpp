#include "StorageServer.hpp"

StorageServer *server;

int main(int argc, char *argv[])
{
  int portno = 8000; // will parse later
  StorageServer tmp{portno};
  server = &tmp;

  signal(SIGINT, int_handler_wrapper);

  server->start();
}
