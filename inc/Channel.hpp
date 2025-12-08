#pragma once

#include <string>
#include <vector>
#include "Client.hpp"

class Channel
{
public:
	Channel(const std::string &name, const std::string &key, Client *admin);
	~Channel();

	// Getters
	std::string get_name() const;
	Client *get_admin() const;
	std::string get_key() const;
	size_t get_limit() const;
	bool ext_msg() const;
	size_t get_size() const;
	std::vector<std::string> get_nicknames();

	// Setters
	void set_key(std::string key);
	void set_limit(size_t limit);
	void set_ext_msg(bool flag);

	// Actions
	void add_client(Client *client);
	void remove_client(Client *client);
	void broadcast(const std::string& message, Client* exclude);
	bool has_client(Client* client) const;

private:
	Channel(const Channel &other);
	Channel &operator=(const Channel &other);

	std::string _name;
	Client *_admin;
	std::vector<Client *> _clients;
	std::string _k; // channel password
	size_t _limit;	// limit de membre
	bool _n;		// yes/no messages externes
};
