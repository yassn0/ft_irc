#pragma once

#include <string>

// IRC Error codes
#define ERR_NONICKNAMEGIVEN(client) (":server 431 " + std::string(client) + " :No nickname given\r\n")
#define ERR_NICKNAMEINUSE(client, nick) (":server 433 " + std::string(client) + " " + std::string(nick) + " :Nickname is already in use\r\n")
#define ERR_NEEDMOREPARAMS(client, cmd) (":server 461 " + std::string(client) + " " + std::string(cmd) + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTRED(client) (":server 462 " + std::string(client) + " :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH(client) (":server 464 " + std::string(client) + " :Password incorrect\r\n")
#define ERR_NOTREGISTERED(client) (":server 451 " + std::string(client) + " :You have not registered\r\n")
#define ERR_NOSUCHCHANNEL(client, chan) (":server 403 " + std::string(client) + " " + std::string(chan) + " :No such channel\r\n")
#define ERR_BADCHANNELKEY(client, chan) (":server 475 " + std::string(client) + " " + std::string(chan) + " :Cannot join channel (+k)\r\n")
#define ERR_CHANNELISFULL(client, chan) (":server 471 " + std::string(client) + " " + std::string(chan) + " :Cannot join channel (+l)\r\n")
#define ERR_NOTEXTTOSEND(client) (":server 412 " + std::string(client) + " :No text to send\r\n")
#define ERR_NOSUCHNICK(client, nick) (":server 401 " + std::string(client) + " " + std::string(nick) + " :No such nick/channel\r\n")
#define ERR_CANNOTSENDTOCHAN(client, chan) (":server 404 " + std::string(client) + " " + std::string(chan) + " :Cannot send to channel\r\n")
#define ERR_NOTONCHANNEL(client, chan) (":server 442 " + std::string(client) + " " + std::string(chan) + " :You're not on that channel\r\n")
#define ERR_USERNOTINCHANNEL(client, nick, chan) (":server 441 " + std::string(client) + " " + std::string(nick) + " " + std::string(chan) + " :They aren't on that channel\r\n")
#define ERR_USERONCHANNEL(client, nick, chan) (":server 443 " + std::string(client) + " " + std::string(nick) + " " + std::string(chan) + " :is already on channel\r\n")
#define ERR_KEYSET(client, chan) (":server 467 " + std::string(client) + " " + std::string(chan) + " :Channel key already set\r\n")
#define ERR_UNKNOWNMODE(client, mode) (":server 472 " + std::string(client) + " " + std::string(mode) + " :is unknown mode char to me\r\n")
#define ERR_INVITEONLYCHAN(client, chan) (":server 473 " + std::string(client) + " " + std::string(chan) + " :Cannot join channel (+i)\r\n")
#define ERR_CHANOPRIVSNEEDED(client, chan) (":server 482 " + std::string(client) + " " + std::string(chan) + " :You're not channel operator\r\n")

// IRC Reply codes
#define RPL_TOPIC(client, chan, topic) (":server 332 " + std::string(client) + " " + std::string(chan) + " :" + std::string(topic) + "\r\n")
#define RPL_INVITING(client, nick, chan) (":server 341 " + std::string(client) + " " + std::string(nick) + " " + std::string(chan) + "\r\n")
