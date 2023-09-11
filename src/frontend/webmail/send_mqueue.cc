#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>

#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "parse_input.hpp"

namespace fs = std::filesystem;

int portno = 2500;
bool debug_output = false;
std::string directory = "";
const std::string filename = "mqueue";
const int SMTP_port = 25;
const int buffer_length = 4096;

/**
 * Data from each email in mqueue
 */
struct email_info
{
    std::string sender;
    std::string recipient;
    std::string message;
    std::string raw_data;
};

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

/**
 * Read emails from mqueue and construct vector of email_infos
 */
std::vector<email_info> read_mqueue(std::string &path)
{
    FILE *file = fopen(path.c_str(), "r");
    if (file == NULL)
    {
        perror("fopen(): ");
        exit(EXIT_FAILURE);
    }
    int fd = fileno(file);
    if (fd < 0)
    {
        perror("fileno(): ");
    }
    if (flock(fd, LOCK_EX) < 0)
    {
        perror("flock(): ");
    }

    char *line = NULL;
    ssize_t read;
    size_t len;
    bool begin = 1;
    struct email_info email;
    std::vector<email_info> emails;
    while ((read = getline(&line, &len, file)) != -1)
    {
        std::string buf;
        for (int i = 0; i < read; ++i)
        {
            buf.push_back(line[i]);
        }
        email.raw_data += buf;
        if (buf == ".\r\n")
        {
            emails.push_back(email);
            email = {};
            begin = 1;
            continue;
        }
        std::vector<std::string> vec = split(buf);
        if (begin)
        {
            if (vec[0] == "To")
            {
                email.recipient = vec[1].substr(0, vec[1].find(">") + 1);
            }
            else
            {
                raise_error("Unexpected token in file");
            }
            begin = 0;
        }
        else
        {
            if (vec[0] == "From")
            {
                email.sender = vec[1].substr(0, vec[1].find(">") + 1);
            }
            else
            {
                email.message += buf;
            }
        }
    }

    free(line);
    if (flock(fd, LOCK_UN) < 0)
    {
        perror("flock(): ");
    }
    fclose(file);
    return emails;
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

    if (debug_output)
    {
        print_info("Calling res_query() on " + std::string(dname));
    }

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
        if (debug_output)
        {
            print_info("Found mx record: " + mx_server);
        }
        struct hostent *host = gethostbyname(mx_server.c_str());
        if (host == NULL)
        {
            continue;
        }
        else
        {
            struct in_addr addr;
            memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
            if (debug_output)
            {
                print_info("With IP address: " + std::string(inet_ntoa(addr)));
            }
            ips.push_back(addr);
        }
    }
    return ips;
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
 * Attempt to send each email to corresponding mail server using SMTP. If unable to
 * resolve domain name or unable to connect to server IP or received unexpected
 * response, just continue to next email. Return a vector of unsent emails.
 */
std::vector<email_info> send_emails(const std::vector<email_info> &emails)
{
    std::vector<email_info> unsent;
    for (auto email : emails)
    {

        if (debug_output)
        {
            print_info("Sending email from " + email.sender + " to " + email.recipient);
        }

        unsent.push_back(email);

        std::string dname = email.recipient.substr(email.recipient.find("@") + 1);
        dname.pop_back();

        std::vector<in_addr> ips = get_ips(dname.c_str());

        if (ips.empty())
        {
            if (debug_output)
            {
                print_info("Unable to resolve " + std::string(dname));
            }
            continue;
        }

        int fd;
        if ((fd = connect_to_server(ips)) < 0)
        {
            if (debug_output)
            {
                print_info("Unable to connect to a server for " + std::string(dname));
            }
            continue;
        }

        std::string hello_domain = email.sender.substr(email.sender.find("@") + 1);
        hello_domain.pop_back();
        std::string helo_message = "HELO " + hello_domain + "\r\n";
        std::string mail_from = "MAIL FROM:" + email.sender + "\r\n";
        std::string rcpt_to = "RCPT TO:" + email.recipient + "\r\n";
        std::string data_cmd = "DATA\r\n";
        std::string message_data = email.message + "\r\n.\r\n";
        std::string quit_message = "QUIT\r\n";

        char buf[buffer_length];
        int len;
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        if (debug_output)
        {
            std::cout << "[S] " << buf;
        }

        write(fd, helo_message.c_str(), sizeof(char) * helo_message.size());
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        if (debug_output)
        {
            std::cout << "[C] " << helo_message;
            std::cout << "[S] " << buf;
        }
        if (std::string(buf).substr(0, 3) != "250")
        {
            if (debug_output)
            {
                print_info("Unexpected response");
            }
            continue;
        }

        write(fd, mail_from.c_str(), sizeof(char) * mail_from.size());
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        if (debug_output)
        {
            std::cout << "[C] " << mail_from;
            std::cout << "[S] " << buf;
        }
        if (std::string(buf).substr(0, 3) != "250")
        {
            if (debug_output)
            {
                print_info("Unexpected response");
            }
            continue;
        }

        write(fd, rcpt_to.c_str(), sizeof(char) * rcpt_to.size());
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        if (debug_output)
        {
            std::cout << "[C] " << rcpt_to;
            std::cout << "[S] " << buf;
        }
        if (std::string(buf).substr(0, 3) != "250")
        {
            if (debug_output)
            {
                print_info("Unexpected response");
            }
            continue;
        }

        write(fd, data_cmd.c_str(), sizeof(char) * data_cmd.size());
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        if (debug_output)
        {
            std::cout << "[C] " << data_cmd;
            std::cout << "[S] " << buf;
        }
        if (std::string(buf).substr(0, 3) != "354")
        {
            if (debug_output)
            {
                print_info("Unexpected response");
            }
            continue;
        }

        write(fd, message_data.c_str(), sizeof(char) * message_data.size());
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        if (debug_output)
        {
            std::cout << "[C] " << message_data;
            std::cout << "[S] " << buf;
        }
        if (std::string(buf).substr(0, 3) != "250")
        {
            if (debug_output)
            {
                print_info("Unexpected response");
            }
            continue;
        }

        // Message sent successfully!

        write(fd, quit_message.c_str(), sizeof(char) * quit_message.size());
        len = read(fd, buf, sizeof(buf));
        buf[len] = '\0';
        if (debug_output)
        {
            std::cout << "[C] " << quit_message;
            std::cout << "[S] " << buf;
        }

        close(fd);
        unsent.pop_back();

        if (debug_output)
        {
            print_info("Email sent successfully!");
        }
    }

    return unsent;
}

/**
 * Fill mqueue file with unsent emails
 */
void fill_mqueue(std::string &path, const std::vector<email_info> &unsent_emails)
{
    FILE *file = fopen(path.c_str(), "w+");
    if (file == NULL)
    {
        perror("fopen(): ");
        exit(EXIT_FAILURE);
    }
    int fd = fileno(file);
    if (fd < 0)
    {
        perror("fileno(): ");
    }
    if (flock(fd, LOCK_EX) < 0)
    {
        perror("flock(): ");
    }

    for (auto &email : unsent_emails)
    {
        if (fprintf(file, email.raw_data.c_str(), sizeof(char) * email.raw_data.size()) < 0)
        {
            perror("fprintf(): ");
        }
    }

    if (flock(fd, LOCK_UN) < 0)
    {
        perror("flock(): ");
    }
    fclose(file);
}

int main(int argc, char **argv)
{
    parse_args(argc, argv);

    if (directory.back() != '/')
    {
        directory.push_back('/');
    }
    std::string path = directory + filename;

    std::vector<email_info> emails = read_mqueue(path);

    std::vector<email_info> unsent_emails = send_emails(emails);

    fill_mqueue(path, unsent_emails);

    struct sockaddr_in servaddr;
}