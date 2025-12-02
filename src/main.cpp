#include <iostream>
#include <exception>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <cstring>
#include "../inc/Server.hpp"
// #include "../inc/Client.hpp"  // Pas encore utilisé
// #include "../inc/Channel.hpp" // Pas encore utilisé

static bool is_valid_number(const char *str)
{
	if (!str || !*str)
		return false;
	for (int i = 0; str[i]; i++)
	{
		if (str[i] < '0' || str[i] > '9')
			return false;
	}
	return true;
}

static void check_arg(char **av)
{
	if (!is_valid_number(av[1]))
		throw std::runtime_error("Error: Port must be a valid number");
	errno = 0;
	char *endptr;
	long port = std::strtol(av[1], &endptr, 10);

	if (*endptr != '\0')
		throw std::runtime_error("Error: Port contains invalid characters");
	if (port < 1024 || port > 65535)
		throw std::runtime_error("Error: Port must be between 1024 and 65535");
	if (!av[2] || strlen(av[2]) == 0)
		throw std::runtime_error("Error: Password cannot be empty");
}

int main(int ac, char **av)
{
	try
	{
		if (ac != 3)
			throw std::runtime_error("Error: Two arguments needed, <port> and <password>");

		check_arg(av); // check le port et le password

		// Créer et lancer le serveur
		Server server(av[1], av[2]);
		server.start(); // Boucle infinie avec poll()
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
