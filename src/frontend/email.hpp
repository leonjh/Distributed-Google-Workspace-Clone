#ifndef EMAIL_HPP
#define EMAIL_HPP

#include <fstream>
#include <memory>
#include <random>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <unordered_map>
#include <vector>
#include <sstream>

#include "json.hpp"
#include "utils.hpp"

using json = nlohmann::json;

struct Email
{
	std::string sender;
	std::vector<std::string> recipients;
	std::string subject;
	std::string date;
	int forwarded;
	std::string original_sender;
	std::string body;

	Email() {}

	Email(std::string sender, std::vector<std::string> recipients, std::string subject, std::string date, int forwarded, std::string original_sender, std::string body) : sender{sender},
																																										  recipients{recipients},
																																										  subject{subject},
																																										  date{date},
																																										  forwarded{forwarded},
																																										  original_sender{original_sender},
																																										  body{body} {}

	std::string to_string()
	{
		std::ostringstream os;
		os << sender << ";" << join(recipients, ",") << ";" << subject << ";" << date << ";" << forwarded << ";" << original_sender << ";" << body.size() << ";" << body;
		return os.str();
	}

	json to_json()
	{
		json email_json;
		email_json["sender"] = sender;
		email_json["recipients"] = recipients;
		email_json["subject"] = subject;
		email_json["date"] = date;
		email_json["forwarded"] = forwarded;
		email_json["original_sender"] = original_sender;
		email_json["body"] = body;

		return email_json;
	}

	static Email from_string(const std::string &str)
	{
		std::vector<std::string> email_parts = split_string(str, ';', 8);
		std::vector<std::string> email_recipients = split_string(email_parts[1], ',', email_parts[1].length());
		Email email;
		email.sender = email_parts[0];
		email.recipients = email_recipients;
		email.subject = email_parts[2];
		email.date = email_parts[3];
		email.forwarded = std::stoi(email_parts[4]);
		email.original_sender = email_parts[5];
		email.body = email_parts[7];

		return email;
	}

	static std::vector<Email> to_email_struct(std::string str)
	{
		std::vector<Email> result;
		while (str.length() > 0)
		{
			std::vector<std::string> parts = split_string(str, ';', 8);
			std::string original_last_part = parts[7];
			parts[7] = parts[7].substr(0, std::stoi(parts[6]));
			std::string email_string = join(parts, ";");
			result.push_back(from_string(email_string));

			int length = original_last_part.length() - std::stoi(parts[6]);
			str = original_last_part.substr(std::stoi(parts[6]), length);
		}

		return result;
	}

	void test_email_serializer()
	{
		std::string emails_str = "andy@example.com;andy@example.com,test@example.com;Hello world;2022-05-01;0;andy@localhost;11;Hello Jane!andy@example.com;andy@example.com,test@example.com;Hello test;2023-05-01;0;andy@localhost;11;Hello test!";

		// Convert the combined string representation of the email structs back to email structs
		std::vector<Email> emails = Email::to_email_struct(emails_str);

		// Print the contents of the email structs
		for (const auto &email : emails)
		{
			std::cout << "Sender: " << email.sender << std::endl;
			std::cout << "Recipients: ";
			for (const auto &recipient : email.recipients)
			{
				std::cout << recipient << ", ";
			}
			std::cout << std::endl;
			std::cout << "Subject: " << email.subject << std::endl;
			std::cout << "Date: " << email.date << std::endl;
			std::cout << "Forwarded: " << email.forwarded << std::endl;
			std::cout << "Original Sender: " << email.original_sender << std::endl;
			std::cout << "Body: " << email.body << std::endl;
			std::cout << std::endl;
		}
	}

private:
	static std::string join(const std::vector<std::string> &strs, const std::string &delim)
	{
		std::ostringstream os;
		for (size_t i = 0; i < strs.size(); ++i)
		{
			if (i > 0)
				os << delim;
			os << strs[i];
		}
		return os.str();
	}
};

#endif