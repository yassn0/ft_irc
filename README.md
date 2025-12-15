# ft_irc

An IRC (Internet Relay Chat) server implementation in C++98.

## Description

This project is a fully functional IRC server that handles multiple clients simultaneously using non-blocking I/O and the `poll()` function. It implements the core IRC protocol commands and channel management features.

## Features

### Authentication & User Management
- Password authentication (PASS)
- Nickname management (NICK)
- User registration (USER)

### Channel Operations
- Create and join channels (JOIN)
- Send messages to channels and users (PRIVMSG)
- Leave channels (PART)
- Disconnect from server (QUIT)

### Channel Modes
- `+i` : Invite-only channel
- `+t` : Topic restricted to operators
- `+k` : Channel password
- `+o` : Operator privileges
- `+l` : User limit

### Operator Commands
- KICK: Remove users from channels
- INVITE: Invite users to invite-only channels
- TOPIC: View or modify channel topic

## Compilation

```bash
make
```

## Usage

```bash
./ircserv <port> <password>
```

**Example:**
```bash
./ircserv 6667 mypassword
```

## Connect with a Client

Using **irssi** (recommended):
```bash
irssi
/connect localhost 6667 mypassword
/nick YourNickname
/join #general
```

Or use **netcat** for testing:
```bash
nc localhost 6667
PASS mypassword
NICK mynick
USER myuser 0 * :Real Name
JOIN #general
PRIVMSG #general :Hello World!
```

## Technical Details

- **Language**: C++98
- **I/O Multiplexing**: `poll()`
- **Non-blocking I/O**: All sockets use `fcntl()` with `O_NONBLOCK`
- **RFC Compliance**: Implements IRC protocol (RFC 1459/2812)
- **Error Handling**: Proper IRC numeric replies
- **Memory Management**: No leaks (tested with valgrind)

## Project Structure

```
.
├── inc/              # Header files
├── src/              # Source files
│   └── commands/     # IRC command implementations
└── Makefile          # Build configuration
```

## 42 School Project

This project is part of the 42 Network curriculum. It focuses on:
- Network programming
- Socket management
- Concurrent client handling
- Protocol implementation
- C++98 standard compliance

---

**Grade**: Pending evaluation
