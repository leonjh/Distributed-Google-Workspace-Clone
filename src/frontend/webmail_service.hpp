#ifndef WEBMAIL_SERVICE_HPP
#define WEBMAIL_SERVICE_HPP

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "../lib/constants.h"
#include "../lib/tokenizer.h"
#include "backend_interface.hpp"
#include "definitions.hpp"
#include "email.hpp"
#include "json.hpp"
#include "utils.hpp"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/storageserver.grpc.pb.h"
#else
#include "storageserver.grpc.pb.h"
#endif

using backend::StorageServer;
using backend::StorageServerRequest;
using backend::StorageServerResponse;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using json = nlohmann::json;

class WebMailService
{
private:
  int stmp_sock;
  const int buffer_length = 4096;
  const int SMTP_port = 25;

  const std::string stmp_sender_command = "MAIL FROM: ";
  const std::string stmp_rcpt_command = "RCPT TO: ";
  const std::string stmp_data_command = "DATA";
  const std::string stmp_quit_command = "QUIT";

  const std::string received_ids_col_name = "received_ids";
  const std::string sent_ids_col_name = "sent_ids";
  const std::string get_not_found_response = "not found";
  const std::string webmail_row_suffix = "_mbox";

public:
  WebMailService() {}

  std::string handle_request(Request *request)
  {
    std::pair<bool, json> response;
    json request_body = json::parse(request->http_message->body);

    std::unordered_map<std::string, std::string> cookies;

    json no_cookie_json;
    no_cookie_json["message"] = "Not authenticated";
    response.second = no_cookie_json;
    response.first = false;

    if (request->http_message->header.find("Cookie") != request->http_message->header.end())
    {
      cookies = parse_cookies(request->http_message->header["Cookie"]);
      if (cookies.find("username") != cookies.end())
      {
        std::cout << request->http_message->path << std::endl;
        if (request->http_message->path == "/api/webmail/inbox")
        {
          response = get_inbox(cookies["username"], request_body);
          std::cout << response.first << std::endl;
        }
        else if (request->http_message->path == "/api/webmail/forward")
        {
          response = forward_email(request_body);
        }
        else if (request->http_message->path == "/api/webmail/send")
        {
          response = send_email(request_body);
        }
        else if (request->http_message->path == "/api/webmail/reply")
        {
          response = reply_email(request_body);
        }
        else if (request->http_message->path == "/api/webmail/delete")
        {
          response = delete_email(cookies["username"], request_body);
        }
      }
    }

    int status = response.first ? 200 : 400;
    return get_api_response(request, response.second, status, "");
  }

  std::pair<bool, json> delete_email(const std::string &username, const json &request_body)
  {
    json response_body;

    if (username.length() == 0 || !request_body.contains("sent_inbox") || !request_body.contains("email_id"))
    {
      return {false, response_body};
    }

    std::string user_mbox_row = username + webmail_row_suffix;
    std::string ids_col_name = request_body["sent_inbox"] == 1 ? sent_ids_col_name : received_ids_col_name;

    std::pair<bool, std::string> ids_response = BackendInterface::send_get_request(user_mbox_row, ids_col_name);
    if (!ids_response.first)
    {
      std::cout << "ids not found" << std::endl;
      return {false, response_body};
    }

    if (request_body["email_id"].is_null())
    {
      std::cout << "null email id" << std::endl;
      return {false, response_body};
    }

    std::string new_ids = remove_id(ids_response.second, request_body["email_id"]);
    ;

    std::cout << "new ids: " << new_ids << std::endl;
    // delete email id from ids
    if (!BackendInterface::send_cput_request(user_mbox_row, ids_col_name, ids_response.second, new_ids))
    {
      std::cout << "editing ids failed" << std::endl;
      return {false, response_body};
    }

    // delete email col
    if (!BackendInterface::send_delete_request(user_mbox_row, request_body["email_id"]))
    {
      std::cout << "couldnt delete email col" << std::endl;
      return {false, response_body};
    }

    response_body["deleted"] = request_body["email_id"];

    return {true, response_body};
  }

  std::pair<bool, json> reply_email(const json &request_body)
  {
    json response_body;

    if (!request_body.contains("sender") || !request_body.contains("recipients") || !request_body.contains("subject") ||
        !request_body.contains("body") || !request_body.contains("reply"))
    {
      std::cout << "Invalid email json" << endl;
      return {false, response_body};
    }

    time_t curr_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string date(std::ctime(&curr_time));
    date = date.substr(0, date.length() - 2);

    Email email;
    email.sender = request_body["sender"];
    email.recipients = request_body["recipients"];
    std::string subject = request_body["subject"];
    email.subject = "Re: " + subject; // add re: to subject
    email.date = date;
    email.forwarded = 0;
    email.body = request_body["body"];

    std::string sender_username = email.sender.substr(0, email.sender.find("@"));
    std::string user_mbox_row = sender_username + webmail_row_suffix;
    std::pair<bool, std::string> reply_email_response =
        BackendInterface::send_get_request(user_mbox_row, request_body["reply"]);

    if (!reply_email_response.first)
    {
      std::cout << "failed to get reply email" << std::endl;
      return {false, response_body};
    }

    std::vector<Email> email_structs = Email::to_email_struct(reply_email_response.second);
    if (email_structs.size() == 0)
    {
      return {false, response_body};
    }
    email.original_sender = email_structs[0].sender;

    std::string email_id = get_unique_id();
    return send_email_action(sender_username, email_id, email, reply_email_response.second); // add reply email
  }

  std::pair<bool, json> forward_email(const json &request_body)
  {
    json response_body;

    if (!request_body.contains("sender") || !request_body.contains("recipients") || !request_body.contains("subject") ||
        !request_body.contains("body") || !request_body.contains("forward"))
    {
      std::cout << "Invalid email json" << endl;
      return {false, response_body};
    }

    time_t curr_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string date(std::ctime(&curr_time));
    date = date.substr(0, date.length() - 2);

    Email email;
    email.sender = request_body["sender"];
    email.recipients = request_body["recipients"];
    std::string subject = request_body["subject"];
    email.subject = "Fwd: " + subject; // add fwd: to subject
    email.date = date;
    email.forwarded = 1;
    email.body = request_body["body"];

    std::string sender_username = email.sender.substr(0, email.sender.find("@"));
    std::string user_mbox_row = sender_username + webmail_row_suffix;
    std::pair<bool, std::string> forward_email_response =
        BackendInterface::send_get_request(user_mbox_row, request_body["forward"]);

    if (!forward_email_response.first)
    {
      std::cout << "failed to get forward email" << std::endl;
      return {false, response_body};
    }

    std::vector<Email> email_structs = Email::to_email_struct(forward_email_response.second);
    if (email_structs.size() == 0)
    {
      return {false, response_body};
    }
    email.original_sender = email_structs[0].sender;

    std::string email_id = get_unique_id();
    return send_email_action(sender_username, email_id, email, forward_email_response.second); // add forwarded email
  }

  std::pair<bool, json> get_inbox(const std::string &username, const json &request_body)
  {
    json response_body;
    response_body["emails"] = {};

    if (username.length() == 0 || !request_body.contains("sent_inbox"))
    {
      return {false, response_body};
    }

    std::string user_mbox_row = username + webmail_row_suffix;
    std::string ids_col_name = request_body["sent_inbox"] == 1 ? sent_ids_col_name : received_ids_col_name;
    std::pair<bool, std::string> received_ids_response =
        BackendInterface::send_get_request(user_mbox_row, ids_col_name);
    if (!received_ids_response.first)
    {
      std::cout << "retrieving received ids failed" << std::endl;
      return {false, response_body};
    }

    std::vector<std::string> received_ids =
        split_string(received_ids_response.second, ',', received_ids_response.second.length());
    for (int i = 0; i < received_ids.size(); i++)
    {
      std::pair<bool, std::string> get_email_response =
          BackendInterface::send_get_request(user_mbox_row, received_ids[i]);
      if (get_email_response.first)
      {
        std::vector<Email> email_structs = Email::to_email_struct(get_email_response.second);
        json thread;
        thread["threads"] = {};
        thread["email_id"] = received_ids[i];
        for (int j = 0; j < email_structs.size(); j++)
        {
          thread["threads"].push_back(email_structs[j].to_json());
        }
        response_body["emails"].push_back(thread);
      }
    }

    return {true, response_body};
  }

  std::pair<bool, json> send_email(const json &request_body)
  { // success/fail, response body
    json response_body;
    response_body["sent_to"] = {};
    response_body["message"] = {};

    // note: localhost domain for users locally
    if (!request_body.contains("sender") || !request_body.contains("recipients") || !request_body.contains("subject") ||
        !request_body.contains("body"))
    {
      std::cout << "Invalid email json" << endl;
      return {false, response_body};
    }

    time_t curr_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string date(std::ctime(&curr_time));
    date = date.substr(0, date.length() - 2);

    Email email;
    email.sender = request_body["sender"];
    email.recipients = request_body["recipients"];
    email.subject = request_body["subject"];
    email.date = date;
    email.forwarded = 0;
    email.original_sender = request_body["sender"];
    email.body = request_body["body"];

    std::string email_id = get_unique_id();
    std::string sender_username = email.sender.substr(0, email.sender.find("@"));

    return send_email_action(sender_username, email_id, email, "");
  }

  std::pair<bool, json> send_email_action(const std::string &sender_username, const std::string &email_id, Email &email,
                                          const std::string &append_content)
  {
    json response_body;
    response_body["sent_to"] = {};
    response_body["message"] = {};

    bool send_success = false; // only one need to succeed
    for (int i = 0; i < email.recipients.size(); i++)
    {
      std::string recipient_username = email.recipients[i].substr(0, email.recipients[i].find("@"));
      std::string recipient_domain = email.recipients[i].substr(email.recipients[i].find("@") + 1);
      if (recipient_domain != "localhost")
      {
        std::pair<bool, std::string> response = send_external_email(email, email.recipients[i]);
        if (response.first)
        {
          send_success = true;
          response_body["sent_to"].push_back(email.recipients[i]);
        }
        response_body["message"] = response.second;
      }
      else
      {
        std::string received_email_id = "r_" + email_id;
        bool store_r_res = store_email(recipient_username, email_id, email.to_string() + append_content,
                                       true); // add to recipient mbox

        if (store_r_res)
        {
          send_success = true;
          response_body["sent_to"].push_back(email.recipients[i]);
        }
      }
    }

    if (send_success)
    {
      bool store_s_res =
          store_email(sender_username, email_id, email.to_string() + append_content, false); // add to sender mbox
      if (!store_s_res)
      {
        return {false, response_body};
      }
    }

    return {send_success, response_body};
  }

  bool store_email(const std::string& to_user, std::string email_id, const std::string& data, bool is_received)
  {
    std::string user_mbox_row = to_user + webmail_row_suffix;
    email_id = (is_received ? "r_" : "s_") + email_id;
    std::string ids_col_name = is_received ? received_ids_col_name : sent_ids_col_name;
    if (!BackendInterface::send_put_request(user_mbox_row, email_id, data))
    {
      std::cout << "putting email failed" << std::endl;
      return false;
    }

    // add to received_ids
    std::pair<bool, std::string> ids_response = BackendInterface::send_get_request(user_mbox_row, ids_col_name);
    if (!ids_response.first && ids_response.second == get_not_found_response)
    {
      std::cout << "user doesnt exist" << std::endl;
      return false;
    }
    else if (ids_response.first)
    { // col found
      std::string new_recevied_ids = ids_response.second + (ids_response.second.length() > 0 ? "," : "") + email_id;

      if (!BackendInterface::send_cput_request(user_mbox_row, ids_col_name, ids_response.second, new_recevied_ids))
      {
        std::cout << "editing received id failed" << std::endl;
        return false;
      }
    }

    return true;
  }

  std::pair<bool, std::string> send_external_email(Email& email, const std::string& recipient)
  {
    email.sender = "leonjh@seas.upenn.edu";
    print_info("Sending external email from " + email.sender + " to " + recipient);
    std::string dname = recipient.substr(recipient.find("@") + 1);

    std::vector<in_addr> ips = get_ips(dname.c_str());

    if (ips.empty())
    {
      print_info("Unable to resolve " + std::string(dname));
      return {false, "Unable to resolve " + std::string(dname)};
    }

    int fd;
    if ((fd = connect_to_server(ips)) < 0)
    {
      print_info("Unable to connect to a server for " + std::string(dname));
      return {false, "Unable to connect to a server for " + std::string(dname)};
    }

    std::string hello_domain = email.sender.substr(email.sender.find("@") + 1);
    std::string helo_message = "HELO " + hello_domain + "\r\n";
    std::string mail_from = "MAIL FROM:" + email.sender + "\r\n";
    std::string rcpt_to = "RCPT TO:" + recipient + "\r\n";
    std::string data_cmd = "DATA\r\n";
    std::string message_data = email.body +
                               "\r\n.\r\n";
    std::string quit_message = "QUIT\r\n";

    char buf[buffer_length];
    int len;
    len = read(fd, buf, sizeof(buf));
    buf[len] = '\0';

    std::cout << "[S] " << buf;

    write(fd, helo_message.c_str(), sizeof(char) * helo_message.size());
    len = read(fd, buf, sizeof(buf));
    buf[len] = '\0';

    std::cout << "[C] " << helo_message;
    std::cout << "[S] " << buf;

    if (std::string(buf).substr(0, 3) != "250")
    {
      print_info("Unexpected response");
      return {false, "Unexpected response"};
    }

    write(fd, mail_from.c_str(), sizeof(char) * mail_from.size());
    len = read(fd, buf, sizeof(buf));
    buf[len] = '\0';

    std::cout << "[C] " << mail_from;
    std::cout << "[S] " << buf;

    if (std::string(buf).substr(0, 3) != "250")
    {
      print_info("Unexpected response");
      return {false, "Unexpected response"};
    }

    write(fd, rcpt_to.c_str(), sizeof(char) * rcpt_to.size());
    len = read(fd, buf, sizeof(buf));
    buf[len] = '\0';

    std::cout << "[C] " << rcpt_to;
    std::cout << "[S] " << buf;

    if (std::string(buf).substr(0, 3) != "250")
    {
      print_info("Unexpected response");
      return {false, "Unexpected response"};
    }

    write(fd, data_cmd.c_str(), sizeof(char) * data_cmd.size());
    len = read(fd, buf, sizeof(buf));
    buf[len] = '\0';

    std::cout << "[C] " << data_cmd;
    std::cout << "[S] " << buf;

    if (std::string(buf).substr(0, 3) != "354")
    {
      print_info("Unexpected response");
      return {false, "Unexpected response"};
    }

    write(fd, message_data.c_str(), sizeof(char) * message_data.size());
    len = read(fd, buf, sizeof(buf));
    buf[len] = '\0';

    std::cout << "[C] " << message_data;
    std::cout << "[S] " << buf;

    if (std::string(buf).substr(0, 3) != "250")
    {
      print_info("Unexpected response");
      return {false, "Unexpected response"};
    }

    // Message sent successfully!

    write(fd, quit_message.c_str(), sizeof(char) * quit_message.size());
    len = read(fd, buf, sizeof(buf));
    buf[len] = '\0';

    std::cout << "[C] " << quit_message;
    std::cout << "[S] " << buf;
    close(fd);

    return {true, "Email sent successfully!"};
  }

  /**
   * Connect to server by trying each IP address until success. Return
   * the socket file descriptor.
   */
  int connect_to_server(const std::vector<in_addr> &ips)
  {
    struct sockaddr_in addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
      raise_error("Unable to create socket");

    const int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
      raise_error("setsockopt(SO_REUSEADDR) failed");

    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SMTP_port);

    for (auto &ip : ips)
    {
      addr.sin_addr = ip;
      if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
      {
        return sockfd;
      }
    }
    return -1;
  }

  /**
   * Helper function to get ip addresses from a domain name. We first look
   * up corresponding mx records and then gets the IP address of each record.
   */
  std::vector<in_addr> get_ips(const char *dname)
  {
    union
    {
      HEADER hdr;
      u_char buf[NS_PACKETSZ];
    } response;
    ns_msg handle;

    print_info("Calling res_query() on " + std::string(dname));

    int response_len = res_query(dname, ns_c_in, ns_t_mx, (u_char *)&response, sizeof(response));

    if (response_len < 0 || ns_initparse(response.buf, response_len, &handle) < 0)
    {
      return {};
    }

    ns_initparse(response.buf, response_len, &handle);
    ns_rr rr;
    std::vector<in_addr> ips;
    char dispbuf[buffer_length];
    for (int i = 0; i < ns_msg_count(handle, ns_s_an); i++)
    {
      ns_parserr(&handle, ns_s_an, i, &rr);
      ns_sprintrr(&handle, &rr, NULL, NULL, dispbuf, sizeof(dispbuf));

      std::string mx_server = split(dispbuf).back();

      print_info("Found mx record: " + mx_server);

      struct hostent *host = gethostbyname(mx_server.c_str());
      if (host == NULL)
      {
        continue;
      }
      else
      {
        struct in_addr addr;
        memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
        print_info("With IP address: " + std::string(inet_ntoa(addr)));
        ips.push_back(addr);
      }
    }
    return ips;
  }

  // HELPERS

  /**
   * Helper function to split string by spaces and convert to vector
   */
  std::vector<std::string> split(const std::string &s)
  {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ' '))
    {
      result.push_back(item);
    }
    return result;
  }

  std::string remove_id(std::string str, const std::string& element)
  {
    // Find the position of the element within the string
    size_t pos = str.find(element);

    if (pos == std::string::npos)
    {
      return str;
    }

    if (pos == 0)
    { // element at beginning
      str.erase(0, element.length() + 1);
    }
    else if (pos == str.length() - element.length() - 1)
    { // element at end
      str.erase(pos - 1, element.length() + 1);
    }
    // If the element is in the middle of the string, remove it and the surrounding commas
    else
    {
      str.erase(pos - 1, element.length() + 1);
    }

    return str;
  }

  void print_info(const std::string &msg) { std::cerr << "[INFO] " << msg << std::endl; }

  void raise_error(const std::string &msg)
  {
    std::cerr << "Error: " << msg << std::endl;
    exit(EXIT_FAILURE);
  }

  // UNUSED

  std::string do_request_stmp(const std::string& buff, int len)
  {
    if (!do_write(stmp_sock, buff, len))
    {
      fprintf(stderr, "Write error\n");
    }

    std::cout << "before read" << std::endl;
    std::string response = do_read(stmp_sock);
    std::cout << "after read" << std::endl;
    return response;
  }

  void start_stmp_connect(int port)
  {
    // create comm
    stmp_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (stmp_sock < 0)
    {
      perror("socket():");
      exit(EXIT_FAILURE);
    }

    // create sockaddr for server to server comm
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    int ret = connect(stmp_sock, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret < 0)
    {
      perror("connect():");
      std::cout << "Cannot connect to smtp server" << std::endl;
      exit(EXIT_FAILURE);
    }

    std::cout << "Server setup: found stmp at 127.0.0.1" + port << std::endl;
    std::string request = "HELO frontend\r\n";
    std::string response = do_request_stmp(request, request.length());
    std::cout << "init response " << response << std::endl;
  }
};

#endif