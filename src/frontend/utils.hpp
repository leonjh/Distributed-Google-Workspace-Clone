#ifndef UTILS_H__
#define UTILS_H__

#include "definitions.hpp"
#include "json.hpp"
#include <cassert>
#include <fcntl.h>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>

using json = nlohmann::json;

bool replace(std::string &str, const std::string &from, const std::string &to)
{
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

std::pair<std::string, json> parse_file_form_body(std::string str)
{
  json parsed_file_form_body;

  // Find the position of the first occurrence of '\r\n'
  size_t delimiterPos = str.find("\r\n");

  // Extract the content-disposition line
  std::string delimiter = str.substr(0, delimiterPos);

  // Find the position of the second occurrence of '\r\n'
  delimiterPos = str.find("\r\n", delimiterPos + 1);

  // Extract the content type line
  std::string content_disp = str.substr(delimiter.length() + 2, delimiterPos - delimiter.length() - 2);
  delimiterPos = str.find("\r\n", delimiterPos + 1);
  std::string content_type = str.substr(delimiter.length() + 2 + content_disp.length() + 2,
                                        delimiterPos - content_disp.length() - 2 - delimiter.length() - 2);

  // Find the position of the delimiter that separates body text
  size_t startBodyPos = str.find("\r\n\r\n") + 4;
  size_t lastDelimiterPos = str.find(delimiter + "--") - 2;

  // Extract the body text
  std::string body = str.substr(startBodyPos, lastDelimiterPos - startBodyPos);

  parsed_file_form_body["content_disposition"] = content_disp;
  parsed_file_form_body["content_type"] = content_type;

  return {body, parsed_file_form_body};
}

std::string get_api_response(Request *request, nlohmann::json response_body, int status, std::string additional_headers)
{
  std::string status_text = "200 OK";
  switch (status)
  {
    case 400:
      status_text = "400 Bad Response";
      break;
  }
  std::string response_body_string = response_body.dump();
  std::string response_text = "HTTP/1.1 " + status_text + "\r\nContent-type: application/json\r\nContent-Length: " +
                              std::to_string(response_body_string.length()) +
                              "\r\nAccess-Control-Allow-Origin: " + request->http_message->header["Origin"] +
                              "\r\nAccess-Control-Allow-Credentials: true";

  response_text += additional_headers;
  response_text += "\r\n\r\n" + response_body_string;

  return response_text;
}

/**
 * @brief Repairs the string after its been tokenized
 *
 * @param cmd - the char pointer array
 * @param cmd_length - the length of the string
 */
void repair_string(char cmd[], int cmd_length)
{
  for (int i = 0; i < cmd_length; i++)
  {
    if (cmd[i] == '\0')
    {
      cmd[i] = ' ';
    }
  }
}

/** @brief Populates a vector with the tokens in a string, tokenizing
 * based off the specified pattern character
 *
 */
void split_tokens_given_token(std::vector<std::string> &strings, char cmd[], int cmd_length, const char *pattern)
{
  char *token;
  char *rest = &cmd[0];
  while ((token = strtok_r(rest, pattern, &rest)))
  {
    for (int i = 0; i < strlen(token); i++)
    {
      if (token[i] == '\r' || token[i] == '\n')
      {
        token[i] = 0;
      }
    }

    strings.push_back(std::string(token));
  }

  repair_string(cmd, cmd_length);
}

std::unordered_map<std::string, std::string> parse_cookies(std::string cookies)
{
  std::unordered_map<std::string, std::string> cookie_map;
  std::string cookies_copy(cookies);

  // Split the string by ';'
  std::vector<std::string> colon_split;
  split_tokens_given_token(colon_split, (char *)cookies_copy.c_str(), cookies_copy.size(), ";");

  for (std::string cur_cookie : colon_split)
  {
    if (cur_cookie[0] == ' ')
    {
      cur_cookie.erase(0, 1);
    }

    std::vector<std::string> equals_split;
    split_tokens_given_token(equals_split, (char *)cur_cookie.c_str(), cur_cookie.size(), "=");

    if (equals_split.size() < 2)
    {
      std::cout << "Error while parsing equals in cookies" << std::endl;
    }

    cookie_map[equals_split[0]] = equals_split[1];
  }

  return cookie_map;
}

std::string get_unique_id()
{
  static std::random_device dev;
  static std::mt19937 rng(dev());

  std::uniform_int_distribution<int> dist(0, 15);

  const char *v = "0123456789abcdef";
  const bool dash[] = {0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};

  std::string res;
  for (int i = 0; i < 16; i++)
  {
    if (dash[i])
      res += "-";
    res += v[dist(rng)];
    res += v[dist(rng)];
  }
  return res;
}

bool do_write(int fd, std::string buff, int len)
{
  std::cout << "doing write" << std::endl;
  // keep calling write until entire buff is written
  int sent = 0;
  while (sent < len)
  {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
    {
      std::cout << "invalid fd" << std::endl;
      return false;
    }

    int n = write(fd, buff.substr(sent).c_str(), len - sent);
    std::cout << "after write" << std::endl;
    if (n < 0)
    {
      std::cout << "-1 write" << std::endl;
      return false;
    }

    sent += n;
  }

  return true;
}

std::string do_read(int fd)
{
  char read_buffer[MAX_BUFFER_LENGTH];
  std::vector<char> message_buffer;
  bool found_r = false;

  int readBytes;
  while (readBytes = read(fd, read_buffer, MAX_BUFFER_LENGTH))
  {
    for (int i = 0; i < readBytes; i++)
    {
      message_buffer.push_back(read_buffer[i]); // command buffer stores the current command being parsed
      if (read_buffer[i] == '\r')
      { // track if <R> has been right before
        found_r = true;
      }
      else if ((read_buffer[i] == '\n' && found_r))
      {
        std::string command(message_buffer.begin(), message_buffer.end());

        return command;
      }
      else
      { // reset if not <R> or \n
        found_r = false;
      }
    }
  }

  return "";
}

std::pair<int, int> parse_command(int argc, char *argv[])
{
  // parse command line, make sure number comes after p
  int p_val = 3000;
  int v_flag = 0;

  int c;
  while ((c = getopt(argc, argv, "p:v")) != -1)
  {
    switch (c)
    {
      case 'p':
        try
        {
          // make sure number is specified after -p, error otherwise
          if (!isdigit(*optarg))
          {
            fprintf(stderr, "Command Parsing Error (-p)\n");
            exit(EXIT_FAILURE);
          }

          p_val = atoi(optarg);
        }
        catch (std::exception e)
        {
          fprintf(stderr, "Command Parsing Error (-p)\n");
          exit(EXIT_FAILURE);
        }
        break;
      case 'v':
        v_flag = 1;
        break;
      case '?':
        if (optopt == 'p')
        {
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        }
        else if (isprint(optopt))
        {
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        }
        else
        {
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        }

        exit(EXIT_FAILURE);
      default:
        abort();
    }
  }

  return {p_val, v_flag};
}

std::pair<std::string, std::string> parse_full_addr(std::string full_addr)
{
  std::string addr = full_addr.substr(0, full_addr.find(":"));
  std::string port = full_addr.substr(full_addr.find(":") + 1, full_addr.find(":") - full_addr.length());

  return {addr, port};
}

// check if command with args is valid with folllowing format: <command><sp> args..
bool command_check(std::string command, std::string valid_command)
{
  int end = command.find(" ") == std::string::npos ? command.length() : command.find(" ");
  return command.substr(0, end) == valid_command;
}

void log(int fd, std::string message, int v_flag)
{
  if (v_flag)
  {
    std::cout << "[" << std::to_string(fd) << "] " << message << std::endl;
  }
}

// split string based on 'separator' delimiter into 'num_parts' parts
std::vector<std::string> split_string(std::string s, char separator, int num_parts)
{
  std::vector<std::string> parts;
  int start = 0;
  int end = 0;

  for (int i = 0; i <= s.size(); i++)
  { // iterate over all chars
    if (parts.size() + 1 == num_parts)
    { // max parts reached
      end = s.length();
      std::string temp;
      temp.append(s, start, end - start); // find part
      parts.push_back(temp);
      break;
    }
    if (s[i] == separator || i == s.size())
    { // delimiter found
      end = i;
      std::string temp;
      temp.append(s, start, end - start); // find part
      parts.push_back(temp);
      start = end + 1;
    }
  }

  return parts;
}

#endif
