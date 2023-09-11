#ifndef FILESYSTEM_SERVICE_HPP
#define FILESYSTEM_SERVICE_HPP

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <ctime>
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

class FileSystemService
{
private:
	const std::string filesystem_row_suffix = "_files";
	std::string url_prefix = "/api/filesystem/upload";
	std::string content_type_formatted;
	std::string content_disp_formatted;
	std::string parsed_file_body;

public:
	FileSystemService() {}

	std::string handle_request(Request *request)
	{
		std::pair<bool, json> response;
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
				if (request->http_message->path.find("/api/filesystem/upload") != std::string::npos) {
					std::pair<std::string, json> parsed_file_form_body = parse_file_form_body(request->http_message->body);
					json request_body;
					request_body["content"] = parsed_file_form_body.first;
					request_body["path"] = request->http_message->path.substr(request->http_message->path.find(url_prefix) + url_prefix.length());
					std::string content_disposition = parsed_file_form_body.second["content_disposition"].get<std::string>();
					std::string filename_prefix = "filename=\"";
					std::string name = content_disposition.substr(content_disposition.find(filename_prefix) + filename_prefix.length());
					request_body["name"] = name.substr(0, name.length() - 1);
					request_body["content_type"] = parsed_file_form_body.second["content_type"].get<std::string>();
					request_body["content_disposition"] = content_disposition;
					// std::string body = request_body["body"].get<std::string>();
					// std::cout << "heres the json " << request_body.dump(4) << std::endl;
					// std::cout << "here is the body " << request_body["content"].get<std::string>() << std::endl;

					// input: path, name, content, content_type, content_disposition
					// output: null
					response = upload(cookies["username"], request_body);

				} else {
					json request_body = json::parse(request->http_message->body);

					if (request->http_message->path == "/api/filesystem/navigate")
					{
						// input: path
						// output: all files and directories in that path
						response = navigate(cookies["username"], request_body);
					}
					else if (request->http_message->path == "/api/filesystem/download")
					{
						// input: id
						// output: file content, id, metadata, and name
						response = download(cookies["username"], request_body);
						json response_json = response.second;
						std::string content_type = response_json["metadata"]["content_type"].get<std::string>();
						std::string content_disp = response_json["metadata"]["content_disposition"].get<std::string>();
						std::string body = response_json["content"].get<std::string>();
						return "HTTP/1.1 200 OK\r\nContent-type: " + content_type + "\r\nContent_Disposition: " + content_disp +"\r\nContent-Length: " +
                              std::to_string(body.length()) +
                              "\r\nAccess-Control-Allow-Origin: " + request->http_message->header["Origin"] +
                              "\r\nAccess-Control-Allow-Credentials: true\r\n\r\n" + body;
					}
					else if (request->http_message->path == "/api/filesystem/rename")
					{
						// input: id, new name
						// output: the new file object
						response = rename(cookies["username"], request_body);
					}
					else if (request->http_message->path == "/api/filesystem/star")
					{
						// input: id, star boolean
						// output: the new file object
						response = star(cookies["username"], request_body);
					}
					else if (request->http_message->path == "/api/filesystem/folder")
					{
						// input: path, new folder name
						// output: the new folder object
						response = add_folder(cookies["username"], request_body);
					}
					else if (request->http_message->path == "/api/filesystem/delete")
					{
						// input: path, id
						// output: the deleted id
						response = delete_file(cookies["username"], request_body);
					}
					else if (request->http_message->path == "/api/filesystem/move")
					{
						// input: old path, id, new path
						// output: the new path
						response = move_file(cookies["username"], request_body);
					}

				}
				
				
			}
		}

		int status = response.first ? 200 : 400;
		return get_api_response(request, response.second, status, "");
	}

private:
	std::string serialize_directory_ids(const std::vector<std::string> &ids)
	{
		json result(ids);
		return result.dump();
	}

	std::vector<std::string> deserialize_directory_ids(const std::string &content)
	{
		return json::parse(content);
	}

	/**
	 * Convert a path e.g. (/root/dir1/file1) to an id (file1's id)
	 */
	std::string get_id_from_path(const std::string &row_name, const std::string &path)
	{
		std::vector<std::string> split_path = {""};
		for (auto c : path)
		{
			if (c == '/')
			{
				if (!split_path.back().empty())
				{
					split_path.push_back("");
				}
			}
			else
			{
				split_path.back().push_back(c);
			}
		}
		if (split_path.back().empty())
		{
			split_path.pop_back();
		}

		std::string current_id = "root";
		for (auto &dir_name : split_path)
		{
			auto id_response = BackendInterface::send_get_request(row_name, current_id + ":content");
			if (!id_response.first)
			{
				std::cout << "traversing path failed" << std::endl;
				return "";
			}
			std::vector<std::string> child_ids = deserialize_directory_ids(id_response.second);
			bool found = false;
			for (auto &child_id : child_ids)
			{
				auto name_response = BackendInterface::send_get_request(row_name, child_id + ":name");
				if (name_response.first && name_response.second == dir_name)
				{
					current_id = child_id;
					found = true;
					break;
				}
			}
			if (!found)
			{
				return "";
			}
		}
		return current_id;
	}

	/**
	 * If just_remove, remove id from recents. Else, also insert id to the front of the recents folder
	 */
	void update_recents(const std::string &row_name, const std::string &id, bool just_remove)
	{
		auto content_response = BackendInterface::send_get_request(row_name, "recents:content");
		if (!content_response.first)
		{
			return;
		}
		std::vector<std::string> recent_ids = deserialize_directory_ids(content_response.second);
		auto it = std::find(recent_ids.begin(), recent_ids.end(), id);
		if (it != recent_ids.end())
		{
			recent_ids.erase(it);
		}
		if (!just_remove)
		{
			recent_ids.insert(recent_ids.begin(), id);
		}
		BackendInterface::send_put_request(row_name, "recents:content", serialize_directory_ids(recent_ids));
	}

	/**
	 * Upload file to path
	 */
	std::pair<bool, json> upload(const std::string &username, const json &request_body)
	{
		json response_body;
		if (username.length() == 0 || !request_body.contains("path") || !request_body.contains("name") || !request_body.contains("content") || !request_body.contains("content_type") || !request_body.contains("content_disposition"))
		{
			return {false, response_body};
		}

		std::string name = request_body["name"];
		std::string path = request_body["path"];
		std::string content = request_body["content"].get<std::string>();
		std::string content_type = request_body["content_type"];
		std::string content_disposition = request_body["content_disposition"];
		std::string id = get_unique_id();
		std::string row_name = username + filesystem_row_suffix;

		std::string dir_id = get_id_from_path(row_name, path);
		if (dir_id.empty())
		{
			return {false, response_body};
		}

		auto id_response = BackendInterface::send_get_request(row_name, dir_id + ":content");
		if (!id_response.first)
		{
			std::cout << "traversing path failed" << std::endl;
			return {false, response_body};
		}
		std::vector<std::string> child_ids = deserialize_directory_ids(id_response.second);
		for (auto &child_id : child_ids)
		{
			auto name_response = BackendInterface::send_get_request(row_name, child_id + ":name");
			if (name_response.first && name_response.second == name)
			{
				std::cout << "name conflict: " << name_response.second << std::endl;
				return {false, response_body};
			}
		}
		child_ids.push_back(id);

		if (!BackendInterface::send_put_request(row_name, dir_id + ":content", serialize_directory_ids(child_ids)))
		{
			std::cout << "putting new ids failed" << std::endl;
			return {false, response_body};
		}

		if (!BackendInterface::send_put_request(row_name, id + ":name", name))
		{
			std::cout << "putting child name failed" << std::endl;
			return {false, response_body};
		}

		json metadata;
		metadata["date created"] = std::time(0);
		metadata["starred"] = false;
		metadata["owner"] = username;
		metadata["size"] = content.size();
		metadata["content_type"] = content_type;
		metadata["content_disposition"] = content_disposition;

		if (!BackendInterface::send_put_request(row_name, id + ":metadata", metadata.dump()))
		{
			std::cout << "putting child metadata failed" << std::endl;
			return {false, response_body};
		}

		bool put_content_response;
		if (name.find(".txt") == string::npos) {
			put_content_response = BackendInterface::send_put_bytes_request(row_name, id + ":content", std::move(content));
		} else {
			put_content_response = BackendInterface::send_put_request(row_name, id + ":content", std::move(content));
		}

		if (!put_content_response)
		{
			std::cout << "putting child content failed" << std::endl;
			return {false, response_body};
		}

		update_recents(row_name, id, false);

		response_body["id"] = id;
		response_body["name"] = name;
		response_body["metadata"] = metadata;

		return {true, response_body};
	}

	/**
	 * Download file
	 */
	std::pair<bool, json> download(const std::string &username, const json &request_body)
	{
		json response_body;
		if (username.length() == 0 || !request_body.contains("id"))
		{
			return {false, response_body};
		}

		std::string id = request_body["id"];
		std::string row_name = username + filesystem_row_suffix;

		auto name_response = BackendInterface::send_get_request(row_name, id + ":name");
		auto metadata_response = BackendInterface::send_get_request(row_name, id + ":metadata");
		std::pair<bool, std::string> content_response;

		if (name_response.first && name_response.second.find(".txt") == string::npos) {
			content_response = BackendInterface::send_get_bytes_request(row_name, id + ":content");
		} else {
			content_response = BackendInterface::send_get_request(row_name, id + ":content");
		}
		
		
		if (name_response.first && metadata_response.first && content_response.first)
		{
			json metadata = json::parse(metadata_response.second);
			if (metadata_response.first && !metadata.contains("content_type"))
			{
				std::cout << "Unable to download directory" << std::endl;
				return {false, response_body};
			}

			update_recents(row_name, id, false);

			response_body["name"] = name_response.second;
			response_body["metadata"] = metadata;
			response_body["content"] = content_response.second;
			return {true, response_body};
		}
		else
		{
			return {false, response_body};
		}
	}

	/**
	 * List all files and directories of a certain path
	 */
	std::pair<bool, json> navigate(const std::string &username, const json &request_body)
	{
		json response_body;
		if (username.length() == 0 || !request_body.contains("path"))
		{
			return {false, response_body};
		}

		std::string path = request_body["path"];
		std::string row_name = username + filesystem_row_suffix;
		std::string dir_id = get_id_from_path(row_name, path);
		if (dir_id.empty())
		{
			return {false, response_body};
		}

		auto id_response = BackendInterface::send_get_request(row_name, dir_id + ":content");
		if (!id_response.first)
		{
			std::cout << "traversing path failed" << std::endl;
			return {false, response_body};
		}
		std::vector<std::string> child_ids = deserialize_directory_ids(id_response.second);
		std::vector<json> ret;
		for (auto &child_id : child_ids)
		{
			auto name_response = BackendInterface::send_get_request(row_name, child_id + ":name");
			auto metadata_response = BackendInterface::send_get_request(row_name, child_id + ":metadata");
			if (name_response.first && metadata_response.first)
			{
				json tmp;
				tmp["id"] = child_id;
				tmp["name"] = name_response.second;
				tmp["metadata"] = json::parse(metadata_response.second);
				ret.push_back(std::move(tmp));
			}
		}
		response_body = ret;
		return {true, response_body};
	}

	/**
	 * Rename a file or directory
	 */
	std::pair<bool, json> rename(const std::string &username, const json &request_body)
	{
		json response_body;
		if (username.length() == 0 || !request_body.contains("id") || !request_body.contains("name"))
		{
			return {false, response_body};
		}

		std::string child_id = request_body["id"];
		std::string child_new_name = request_body["name"];
		std::string row_name = username + filesystem_row_suffix;

		if (!BackendInterface::send_put_request(row_name, child_id + ":name", child_new_name))
		{
			std::cout << "putting new name failed" << std::endl;
			return {false, response_body};
		}

		auto metadata_response = BackendInterface::send_get_request(row_name, child_id + ":metadata");
		response_body["id"] = child_id;
		response_body["name"] = child_new_name;
		response_body["metadata"] = json::parse(metadata_response.second);
		return {true, response_body};
	}

	/**
	 * Star a file or directory
	 */
	std::pair<bool, json> star(const std::string &username, const json &request_body)
	{
		json response_body;
		if (username.length() == 0 || !request_body.contains("id") || !request_body.contains("star"))
		{
			return {false, response_body};
		}

		std::string child_id = request_body["id"];
		bool starred = request_body["star"];
		std::string row_name = username + filesystem_row_suffix;

		auto metadata_response = BackendInterface::send_get_request(row_name, child_id + ":metadata");
		auto name_response = BackendInterface::send_get_request(row_name, child_id + ":name");
		if (metadata_response.first && name_response.first)
		{
			json metadata = json::parse(metadata_response.second);
			metadata["starred"] = starred;
			if (!BackendInterface::send_put_request(row_name, child_id + ":metadata", metadata.dump()))
			{
				std::cout << "putting new metadata failed" << std::endl;
				return {false, response_body};
			}
			else
			{
				response_body["id"] = child_id;
				response_body["name"] = name_response.second;
				response_body["metadata"] = metadata;
				return {true, response_body};
			}
		}
		else
		{
			std::cout << "getting old metadata failed" << std::endl;
			return {false, response_body};
		}
	}

	std::pair<bool, json> move_helper(const std::string &row_name, const std::string &old_path_id, const std::string &new_path_id, const std::string &child_id)
	{
		json response_body;

		if (old_path_id == new_path_id)
		{
			return {true, response_body};
		}
		auto old_content_response = BackendInterface::send_get_request(row_name, old_path_id + ":content");
		auto new_content_response = BackendInterface::send_get_request(row_name, new_path_id + ":content");
		if (!old_content_response.first || !new_content_response.first)
		{
			std::cout << "getting contents failed" << std::endl;
			return {false, response_body};
		}
		std::vector<std::string> old_contents = deserialize_directory_ids(old_content_response.second);
		std::vector<std::string> new_contents = deserialize_directory_ids(new_content_response.second);

		auto it = std::find(old_contents.begin(), old_contents.end(), child_id);
		if (it == old_contents.end())
		{
			std::cout << "Requested file/directory not in parent directory" << std::endl;
			return {false, response_body};
		}
		old_contents.erase(it);
		new_contents.push_back(child_id);

		if (!BackendInterface::send_put_request(row_name, old_path_id + ":content", serialize_directory_ids(old_contents)) || !BackendInterface::send_put_request(row_name, new_path_id + ":content", serialize_directory_ids(new_contents)))
		{
			std::cout << "putting new contents failed" << std::endl;
			return {false, response_body};
		}
		response_body["new_location"] = new_path_id;
		return {true, response_body};
	}

	/**
	 * Move a file to Trash if not in Trash. If it is, delete permanently.
	 */
	std::pair<bool, json> delete_file(const std::string &username, const json &request_body)
	{
		json response_body;
		if (username.length() == 0 || !request_body.contains("path") || !request_body.contains("id"))
		{
			return {false, response_body};
		}

		std::string path = request_body["path"];
		std::string row_name = username + filesystem_row_suffix;
		std::string child_id = request_body["id"];
		std::string parent_id = get_id_from_path(row_name, path);
		if (parent_id.empty())
		{
			return {false, response_body};
		}

		update_recents(row_name, child_id, true);

		if (path.find("Trash") == string::npos)
		{
			return move_helper(row_name, parent_id, "trash", child_id);
		}
		else
		{
			auto content_response = BackendInterface::send_get_request(row_name, parent_id + ":content");
			if (!content_response.first)
			{
				std::cout << "getting contents failed" << std::endl;
				return {false, response_body};
			}
			std::vector<std::string> contents = deserialize_directory_ids(content_response.second);

			auto it = std::find(contents.begin(), contents.end(), child_id);
			if (it == contents.end())
			{
				std::cout << "Requested file/directory not in parent directory" << std::endl;
				return {false, response_body};
			}
			contents.erase(it);
			if (!BackendInterface::send_put_request(row_name, parent_id + ":content", serialize_directory_ids(contents)))
			{
				std::cout << "putting new contents failed" << std::endl;
				return {false, response_body};
			}
			response_body["deleted"] = child_id;
			return {true, response_body};
		}
	}

	/**
	 * Move file from old_path to new_path
	 */
	std::pair<bool, json> move_file(const std::string &username, const json &request_body)
	{
		json response_body;
		if (username.length() == 0 || !request_body.contains("old_path") || !request_body.contains("new_path") || !request_body.contains("id"))
		{
			return {false, response_body};
		}

		std::string old_path = request_body["old_path"];
		std::string new_path = request_body["new_path"];
		std::string row_name = username + filesystem_row_suffix;
		std::string child_id = request_body["id"];
		std::string old_parent_id = get_id_from_path(row_name, old_path);
		std::string new_parent_id = get_id_from_path(row_name, new_path);
		if (old_parent_id == "recents" || new_parent_id == "recents" || old_parent_id.empty() || new_parent_id.empty())
		{
			return {false, response_body};
		}

		return move_helper(row_name, old_parent_id, new_parent_id, child_id);
	}

	std::pair<bool, json> add_folder(const std::string &username, const json &request_body)
	{
		json response_body;
		if (username.length() == 0 || !request_body.contains("path") || !request_body.contains("name"))
		{
			return {false, response_body};
		}

		std::string path = request_body["path"];
		std::string child_dir_name = request_body["name"];
		std::string child_dir_id = get_unique_id();
		std::string row_name = username + filesystem_row_suffix;
		std::string parent_dir_id = get_id_from_path(row_name, path);
		if (parent_dir_id.empty() || parent_dir_id == "trash" || parent_dir_id == "recents")
		{
			return {false, response_body};
		}

		std::pair<bool, std::string> id_response = BackendInterface::send_get_request(row_name, parent_dir_id + ":content");
		if (!id_response.first)
		{
			std::cout << "traversing path failed" << std::endl;
			return {false, response_body};
		}
		std::vector<std::string> child_ids = deserialize_directory_ids(id_response.second);
		for (auto &child_id : child_ids)
		{
			std::pair<bool, std::string> name_response = BackendInterface::send_get_request(row_name, child_id + ":name");
			if (name_response.first && name_response.second == child_dir_name) // name conflict
			{
				std::cout << "name conflict: " << name_response.second << std::endl;
				return {false, response_body};
			}
		}
		child_ids.push_back(child_dir_id);

		if (!BackendInterface::send_put_request(row_name, parent_dir_id + ":content", serialize_directory_ids(child_ids)))
		{
			std::cout << "putting new ids failed" << std::endl;
			return {false, response_body};
		}

		if (!BackendInterface::send_put_request(row_name, child_dir_id + ":content", serialize_directory_ids({})))
		{
			std::cout << "putting child content failed" << std::endl;
			return {false, response_body};
		}

		if (!BackendInterface::send_put_request(row_name, child_dir_id + ":name", child_dir_name))
		{
			std::cout << "putting child content failed" << std::endl;
			return {false, response_body};
		}

		json metadata;
		metadata["date created"] = std::time(0);
		metadata["starred"] = false;
		metadata["owner"] = username;

		if (!BackendInterface::send_put_request(row_name, child_dir_id + ":metadata", metadata.dump()))
		{
			std::cout << "putting child metadata failed" << std::endl;
			return {false, response_body};
		}

		response_body["id"] = child_dir_id;
		response_body["name"] = child_dir_name;
		response_body["metadata"] = metadata;

		return {true, response_body};
	}
};

#endif