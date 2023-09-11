#ifndef PARSE_INPUT_H
#define PARSE_INPUT_H

#include <iostream>
#include <algorithm>
#include <string>

extern int portno;
extern bool debug_output;
extern std::string directory;

/**
 * Get index of argument
 */
int get_cmd_option(char **begin, char **end, const std::string &option)
{
  char **itr = std::find(begin, end, option);
  if (itr != end && ++itr != end)
  {
    return itr - begin;
  }
  return -1;
}

/**
 * Check if flag exists
 */
bool cmd_option_exists(char **begin, char **end, const std::string &option)
{
  return std::find(begin, end, option) != end;
}

void raise_error(const std::string &msg)
{
  std::cerr << "Error: " << msg << std::endl;
  exit(EXIT_FAILURE);
}

void print_info(const std::string &msg)
{
  std::cerr << "[INFO] " << msg << std::endl;
}

void output_author_details()
{
  std::cerr << "Joseph Zhang (jzhang25)" << std::endl;
  exit(EXIT_SUCCESS);
}

void debug_new_conn(int fd)
{
  std::cerr << "[" << fd << "] New connection" << std::endl;
}

void debug_received(int fd, const std::string &msg)
{
  std::cerr << "[" << fd << "] C: " << msg;
}

void debug_sent(int fd, const std::string &msg)
{
  std::cerr << "[" << fd << "] S: " << msg;
}

void debug_closed_conn(int fd)
{
  std::cerr << "[" << fd << "] S: Connection closed" << std::endl;
}

/**
 * Parse command line arguments
 */
void parse_args(int argc, char *argv[])
{
  if (cmd_option_exists(argv, argv + argc, "-a"))
  {
    output_author_details();
  }

  if (cmd_option_exists(argv, argv + argc, "-v"))
  {
    debug_output = true;
  }

  if (cmd_option_exists(argv, argv + argc, "-p"))
  {
    int pos = get_cmd_option(argv, argv + argc, "-p");
    portno = atoi(argv[pos]);
  }

  if (std::string(argv[0]) == "./smtp" || std::string(argv[0]) == "./pop3" || std::string(argv[0]) == "./send_mqueue")
  {
    directory = argv[argc - 1]; // given that last arg will be directory name
  }
}

#endif