# Documentation ft_irc - Guide Complet

## Table des matières
1. [Vue d'ensemble du projet](#vue-densemble-du-projet)
2. [Fonctions autorisées et leurs utilités](#fonctions-autorisées-et-leurs-utilités)
3. [Architecture recommandée](#architecture-recommandée)
4. [Division des tâches (2 personnes)](#division-des-tâches-2-personnes)
5. [Ressources pour apprendre](#ressources-pour-apprendre)

---

## Vue d'ensemble du projet

### Objectif
Créer un serveur IRC (Internet Relay Chat) en C++98 capable de gérer plusieurs clients simultanément, permettant la communication en temps réel via des canaux (channels) et des messages privés.

### Contraintes principales
- **Norme**: C++98 strict (`-std=c++98`)
- **Compilation**: `-Wall -Wextra -Werror`
- **I/O non-bloquantes**: Toutes les opérations doivent être non-bloquantes
- **UN SEUL poll()**: (ou équivalent: `select()`, `kqueue()`, `epoll()`)
- **Pas de fork**: Interdiction de créer des processus enfants
- **Pas de crash**: Le serveur ne doit jamais crasher
- **Pas de bibliothèques externes**: Pas de Boost

### Lancement
```bash
./ircserv <port> <password>
```

---

## Fonctions autorisées et leurs utilités

### 1. Création et configuration de sockets

#### `socket()`
```cpp
int socket(int domain, int type, int protocol);
```
**Utilité**: Crée un nouveau socket (point de communication réseau)
- `domain`: AF_INET (IPv4) ou AF_INET6 (IPv6)
- `type`: SOCK_STREAM (TCP)
- `protocol`: 0 (automatique)
- **Retour**: File descriptor du socket ou -1 en cas d'erreur

**Exemple**:
```cpp
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
```

---

#### `setsockopt()`
```cpp
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
```
**Utilité**: Configure les options d'un socket
- Permet de réutiliser une adresse (SO_REUSEADDR) - IMPORTANT pour éviter "Address already in use"
- Configure le timeout, la taille des buffers, etc.

**Exemple critique**:
```cpp
int opt = 1;
setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

---

#### `bind()`
```cpp
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
**Utilité**: Lie un socket à une adresse IP et un port
- Associe le socket à l'adresse locale du serveur
- **DOIT être appelé sur le socket serveur avant `listen()`**

**Exemple**:
```cpp
struct sockaddr_in server_addr;
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY; // Accepte toutes les interfaces
server_addr.sin_port = htons(6667); // Port en network byte order
bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
```

---

#### `listen()`
```cpp
int listen(int sockfd, int backlog);
```
**Utilité**: Met le socket en mode écoute pour accepter des connexions
- `backlog`: Nombre maximal de connexions en attente
- Transforme le socket en "socket serveur"

**Exemple**:
```cpp
listen(server_fd, 10); // Max 10 connexions en attente
```

---

#### `accept()`
```cpp
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
**Utilité**: Accepte une connexion client entrante
- Crée un NOUVEAU socket pour communiquer avec ce client
- Récupère l'adresse du client connecté
- **Retour**: Nouveau file descriptor pour le client

**Exemple**:
```cpp
struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);
int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
```

---

#### `connect()`
```cpp
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
**Utilité**: Établit une connexion vers un serveur distant
- **Note**: Normalement utilisé par les clients, pas le serveur IRC
- Peut être utile pour des fonctionnalités bonus (bot)

---

### 2. Résolution de noms et adresses

#### `getaddrinfo()`
```cpp
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints, struct addrinfo **res);
```
**Utilité**: Convertit un nom d'hôte/service en adresse réseau (moderne, recommandé)
- Supporte IPv4 et IPv6
- Résout les noms de domaine
- **DOIT être suivi de `freeaddrinfo()`**

**Exemple**:
```cpp
struct addrinfo hints, *result;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;
hints.ai_socktype = SOCK_STREAM;
getaddrinfo("localhost", "6667", &hints, &result);
// ... utilisation ...
freeaddrinfo(result);
```

---

#### `freeaddrinfo()`
```cpp
void freeaddrinfo(struct addrinfo *res);
```
**Utilité**: Libère la mémoire allouée par `getaddrinfo()`
- **OBLIGATOIRE** pour éviter les memory leaks

---

#### `gethostbyname()` (Legacy)
```cpp
struct hostent *gethostbyname(const char *name);
```
**Utilité**: Résout un nom d'hôte en adresse IP (ancienne méthode)
- IPv4 uniquement
- Préférer `getaddrinfo()` en pratique

---

#### `getprotobyname()`
```cpp
struct protoent *getprotobyname(const char *name);
```
**Utilité**: Récupère les informations sur un protocole (ex: "tcp")
- Rarement nécessaire (on peut mettre 0 dans `socket()`)

---

#### `getsockname()`
```cpp
int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
**Utilité**: Récupère l'adresse locale d'un socket
- Utile pour connaître le port attribué automatiquement

---

### 3. Conversion d'ordre des octets (Byte Order)

Les données réseau utilisent le "Network Byte Order" (Big Endian), mais les CPU peuvent utiliser le Little Endian. Ces fonctions assurent la compatibilité.

#### `htons()` - Host TO Network Short
```cpp
uint16_t htons(uint16_t hostshort);
```
**Utilité**: Convertit un short (16 bits) de l'ordre hôte vers l'ordre réseau
- **Utilisé pour les numéros de PORT**

**Exemple**:
```cpp
server_addr.sin_port = htons(6667);
```

---

#### `htonl()` - Host TO Network Long
```cpp
uint32_t htonl(uint32_t hostlong);
```
**Utilité**: Convertit un long (32 bits) de l'ordre hôte vers l'ordre réseau
- **Utilisé pour les adresses IP**

---

#### `ntohs()` - Network TO Host Short
```cpp
uint16_t ntohs(uint16_t netshort);
```
**Utilité**: Convertit un short de l'ordre réseau vers l'ordre hôte
- **Lecture de ports depuis le réseau**

---

#### `ntohl()` - Network TO Host Long
```cpp
uint32_t ntohl(uint32_t netlong);
```
**Utilité**: Convertit un long de l'ordre réseau vers l'ordre hôte
- **Lecture d'adresses IP depuis le réseau**

---

#### `inet_addr()`
```cpp
in_addr_t inet_addr(const char *cp);
```
**Utilité**: Convertit une adresse IP format string ("192.168.1.1") en format binaire
- Retourne INADDR_NONE en cas d'erreur

**Exemple**:
```cpp
server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
```

---

#### `inet_ntoa()`
```cpp
char *inet_ntoa(struct in_addr in);
```
**Utilité**: Convertit une adresse IP format binaire en string
- **Attention**: Retourne un pointeur vers un buffer statique (pas thread-safe)

**Exemple**:
```cpp
std::cout << "Client IP: " << inet_ntoa(client_addr.sin_addr) << std::endl;
```

---

### 4. Communication (Envoi/Réception)

#### `send()`
```cpp
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```
**Utilité**: Envoie des données sur un socket connecté
- **Retour**: Nombre d'octets envoyés (peut être < len !)
- Retourne -1 en cas d'erreur
- En mode non-bloquant: Peut retourner moins que demandé

**Exemple**:
```cpp
std::string msg = "PRIVMSG #general :Hello\r\n";
send(client_fd, msg.c_str(), msg.length(), 0);
```

---

#### `recv()`
```cpp
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```
**Utilité**: Reçoit des données depuis un socket
- **Retour**: Nombre d'octets reçus
- Retourne 0 si la connexion est fermée
- Retourne -1 en cas d'erreur (errno == EAGAIN en non-bloquant = pas de données)

**Exemple**:
```cpp
char buffer[512];
ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
if (bytes_read > 0) {
    buffer[bytes_read] = '\0';
    // Traiter le message
}
```

---

### 5. Multiplexage I/O (Cœur du projet)

#### `poll()` ⭐ FONCTION CENTRALE
```cpp
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```
**Utilité**: Surveille plusieurs file descriptors simultanément
- **UN SEUL poll() pour tout gérer**: read, write, accept
- Évite le blocage et la surconsommation CPU

**Structure pollfd**:
```cpp
struct pollfd {
    int fd;           // File descriptor à surveiller
    short events;     // Événements à surveiller (POLLIN, POLLOUT)
    short revents;    // Événements détectés (rempli par poll)
};
```

**Événements importants**:
- `POLLIN`: Données disponibles en lecture
- `POLLOUT`: Prêt pour l'écriture
- `POLLERR`: Erreur
- `POLLHUP`: Déconnexion

**Exemple d'utilisation**:
```cpp
std::vector<struct pollfd> fds;

// Socket serveur
struct pollfd server_poll;
server_poll.fd = server_fd;
server_poll.events = POLLIN; // Surveiller les nouvelles connexions
fds.push_back(server_poll);

// Boucle principale
while (true) {
    int ret = poll(&fds[0], fds.size(), -1); // -1 = attente infinie

    if (ret < 0) {
        // Erreur
        continue;
    }

    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i].revents & POLLIN) {
            if (fds[i].fd == server_fd) {
                // Nouvelle connexion
                int client_fd = accept(server_fd, NULL, NULL);
                // Ajouter le client à fds
            } else {
                // Données d'un client
                // recv() et traiter
            }
        }
    }
}
```

---

**Alternatives à poll()**:

#### `select()` (plus portable)
```cpp
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
```
- Limite de 1024 FD sur certains systèmes
- Plus ancien mais très portable

---

#### `epoll()` (Linux uniquement)
```cpp
int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```
- Plus performant pour beaucoup de connexions
- Linux uniquement

---

#### `kqueue()` (BSD/macOS uniquement)
```cpp
int kqueue(void);
int kevent(int kq, const struct kevent *changelist, int nchanges,
           struct kevent *eventlist, int nevents, const struct timespec *timeout);
```
- Équivalent BSD/macOS de epoll

---

### 6. Configuration non-bloquante

#### `fcntl()` ⭐ ESSENTIEL
```cpp
int fcntl(int fd, int cmd, ... /* arg */ );
```
**Utilité**: Configure les propriétés d'un file descriptor
- **MacOS**: OBLIGATOIRE pour le mode non-bloquant
- **Linux**: Recommandé

**Mettre un socket en mode non-bloquant**:
```cpp
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

**Note MacOS**: Le sujet autorise SEULEMENT:
```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

---

### 7. Gestion des signaux

#### `signal()` (Simple mais limité)
```cpp
void (*signal(int signum, void (*handler)(int)))(int);
```
**Utilité**: Définit un gestionnaire de signal
- **Usage**: Capturer Ctrl+C (SIGINT) pour shutdown propre

**Exemple**:
```cpp
void signal_handler(int signum) {
    std::cout << "\nShutting down server..." << std::endl;
    // Nettoyer et fermer les sockets
    exit(0);
}

signal(SIGINT, signal_handler);
```

---

#### `sigaction()` (Recommandé)
```cpp
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```
**Utilité**: Gestion avancée des signaux (plus fiable que signal())
- Comportement plus prévisible
- Plus de contrôle

**Exemple**:
```cpp
struct sigaction sa;
sa.sa_handler = signal_handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = 0;
sigaction(SIGINT, &sa, NULL);
```

---

### 8. Opérations sur fichiers

#### `close()`
```cpp
int close(int fd);
```
**Utilité**: Ferme un file descriptor (socket inclus)
- **OBLIGATOIRE** pour chaque socket ouvert
- Libère les ressources

**Exemple**:
```cpp
close(client_fd);
```

---

#### `lseek()`
```cpp
off_t lseek(int fd, off_t offset, int whence);
```
**Utilité**: Déplace le curseur dans un fichier
- Peu utile pour les sockets
- Peut servir pour un fichier de configuration

---

#### `fstat()`
```cpp
int fstat(int fd, struct stat *buf);
```
**Utilité**: Récupère les informations sur un fichier
- Taille, permissions, type
- Peut servir pour valider un fichier de configuration

---

## Architecture recommandée

### Structure du code

```
ft_irc/
├── Makefile
├── include/
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   ├── Command.hpp
│   └── Utils.hpp
├── src/
│   ├── main.cpp
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   └── commands/
│       ├── JoinCommand.cpp
│       ├── PrivmsgCommand.cpp
│       ├── KickCommand.cpp
│       ├── InviteCommand.cpp
│       ├── TopicCommand.cpp
│       └── ModeCommand.cpp
└── README.md
```

---

### Classes principales

#### 1. **Server**
- Gère le socket serveur
- Boucle `poll()`
- Accepte les nouvelles connexions
- Dispatche les commandes
- Gère la liste des clients et channels

**Attributs**:
```cpp
int _server_fd;
std::string _password;
int _port;
std::vector<Client*> _clients;
std::vector<Channel*> _channels;
std::vector<struct pollfd> _poll_fds;
```

**Méthodes clés**:
```cpp
void start();
void acceptNewClient();
void handleClientMessage(int client_fd);
void removeClient(int client_fd);
```

---

#### 2. **Client**
- Représente un utilisateur connecté
- Stocke nickname, username, socket fd
- Buffer pour messages incomplets
- État d'authentification

**Attributs**:
```cpp
int _fd;
std::string _nickname;
std::string _username;
std::string _buffer; // Messages partiels
bool _authenticated;
bool _registered;
```

---

#### 3. **Channel**
- Représente un canal (#general, #42, etc.)
- Liste des membres
- Liste des operators
- Modes (i, t, k, o, l)
- Topic

**Attributs**:
```cpp
std::string _name;
std::string _topic;
std::string _key; // Password si mode +k
std::vector<Client*> _members;
std::vector<Client*> _operators;
std::map<char, bool> _modes; // i, t, k, l
int _user_limit; // Si mode +l
```

**Méthodes**:
```cpp
void broadcast(const std::string& message, Client* exclude);
bool isOperator(Client* client);
bool isMember(Client* client);
```

---

#### 4. **Command** (Classe abstraite)
- Interface pour toutes les commandes IRC

```cpp
class Command {
public:
    virtual void execute(Client* client, std::vector<std::string>& params) = 0;
};
```

---

### Flux de traitement des messages

```
1. poll() détecte des données sur un socket client
2. recv() lit les données → ajout au buffer du client
3. Recherche de "\r\n" (fin de commande IRC)
4. Parsing de la commande (ex: "JOIN #general")
5. Création de l'objet Command correspondant
6. Exécution de la commande
7. Envoi des réponses via send()
```

---

## Division des tâches (2 personnes)

### 🟦 Personne 1 : Infrastructure & Réseau (Backend)

#### Phase 1 : Foundation (Semaine 1)
- [ ] Setup Makefile
- [ ] Création de la classe **Server**
  - [ ] Création du socket serveur (`socket()`, `bind()`, `listen()`)
  - [ ] Configuration en mode non-bloquant (`fcntl()`)
  - [ ] Gestion de `SO_REUSEADDR` (`setsockopt()`)
- [ ] Implémentation de la boucle `poll()`
  - [ ] Surveiller le socket serveur
  - [ ] Accepter les nouvelles connexions (`accept()`)
  - [ ] Ajouter les clients au tableau `poll_fds`
- [ ] Classe **Client**
  - [ ] Attributs de base (fd, nickname, username, buffer)
  - [ ] Méthodes de base (send_message, etc.)

#### Phase 2 : Communication (Semaine 2)
- [ ] Réception des messages (`recv()`)
  - [ ] Gestion du buffer incomplet
  - [ ] Détection de "\r\n"
  - [ ] Parsing basique des commandes
- [ ] Gestion des déconnexions
  - [ ] Détection (recv retourne 0 ou POLLHUP)
  - [ ] Nettoyage (close, retrait de poll_fds)
- [ ] Gestion des erreurs réseau
  - [ ] EAGAIN / EWOULDBLOCK
  - [ ] EPIPE (broken pipe)

#### Phase 3 : Authentification (Semaine 2-3)
- [ ] Commande **PASS** (vérification du password)
- [ ] Commande **NICK** (définir le nickname)
  - [ ] Vérification unicité
  - [ ] Format valide
- [ ] Commande **USER** (définir le username)
- [ ] Séquence d'enregistrement (PASS → NICK → USER)
- [ ] Envoi des messages de bienvenue (001, 002, etc.)

#### Phase 4 : Tests & Stabilisation (Semaine 3-4)
- [ ] Tests de charge (multiples clients)
- [ ] Tests de données partielles (ctrl+D avec nc)
- [ ] Gestion propre des signaux (SIGINT, SIGPIPE)
- [ ] Détection et correction des memory leaks (valgrind)

---

### 🟩 Personne 2 : Channels & Commandes (Features)

#### Phase 1 : Classe Channel (Semaine 1)
- [ ] Création de la classe **Channel**
  - [ ] Attributs (name, topic, members, operators, modes)
  - [ ] Méthode `broadcast()` (envoyer à tous les membres)
  - [ ] Méthodes `addMember()`, `removeMember()`
  - [ ] Vérifications `isOperator()`, `isMember()`

#### Phase 2 : Commandes de base (Semaine 2)
- [ ] Commande **JOIN**
  - [ ] Créer le channel s'il n'existe pas
  - [ ] Ajouter le client au channel
  - [ ] Envoyer la liste des membres (353, 366)
  - [ ] Notifier les autres membres
- [ ] Commande **PRIVMSG**
  - [ ] Messages privés (user to user)
  - [ ] Messages de channel (broadcast)
  - [ ] Format: `:sender PRIVMSG target :message`
- [ ] Commande **PART** (quitter un channel)

#### Phase 3 : Commandes d'opérateur (Semaine 2-3)
- [ ] Commande **KICK**
  - [ ] Vérifier que l'auteur est operator
  - [ ] Retirer le client du channel
  - [ ] Notifier tous les membres
- [ ] Commande **INVITE**
  - [ ] Vérifier les permissions
  - [ ] Ajouter à la liste d'invitations (si mode +i)
  - [ ] Envoyer l'invitation au client cible
- [ ] Commande **TOPIC**
  - [ ] Afficher le topic actuel (sans paramètre)
  - [ ] Modifier le topic (avec paramètre)
  - [ ] Vérifier mode +t (operator only)

#### Phase 4 : Commande MODE (Semaine 3-4)
- [ ] **MODE +i / -i** (invitation only)
- [ ] **MODE +t / -t** (topic restrictions)
- [ ] **MODE +k / -k** (channel key/password)
  - [ ] Vérifier le password dans JOIN
- [ ] **MODE +o / -o** (give/take operator)
- [ ] **MODE +l / -l** (user limit)
  - [ ] Vérifier la limite dans JOIN

---

### 📅 Timeline suggérée (4 semaines)

| Semaine | Personne 1 (Réseau) | Personne 2 (Features) |
|---------|---------------------|------------------------|
| **1** | Server + poll() + accept() | Classe Channel + JOIN |
| **2** | recv() + parsing + PASS/NICK/USER | PRIVMSG + KICK + INVITE |
| **3** | Tests de stabilité | TOPIC + MODE (i, t, k) |
| **4** | Memory leaks + signaux | MODE (o, l) + Tests |

---

### 🤝 Points de synchronisation

**Réunions quotidiennes recommandées** (15 min):
- Partage des avancées
- Résolution des blocages
- Validation des interfaces entre classes

**Points critiques de collaboration**:
1. **Interface Server ↔ Channel**: Comment Server transmet les commandes ?
2. **Format des messages**: Parsing et construction (utiliser des fonctions communes)
3. **Gestion des erreurs**: Codes de réponse IRC (401, 403, 461, etc.)
4. **Tests croisés**: Chacun teste le code de l'autre

---

### 🔧 Outils de collaboration

1. **Git**: Branchage par feature
   ```bash
   git checkout -b feature/channel-join
   git checkout -b feature/poll-loop
   ```

2. **Pair programming**: Sur les parties complexes (poll, MODE)

3. **Tests communs**: Script de test partagé
   ```bash
   # test.sh
   ./ircserv 6667 password &
   sleep 1
   irssi -c localhost -p 6667 -w password
   ```

---

## Ressources pour apprendre

### 📚 Documentation officielle IRC

1. **RFC 1459** - Internet Relay Chat Protocol (Original)
   - Lien: https://datatracker.ietf.org/doc/html/rfc1459
   - **LE document de référence**
   - Décrit tous les messages IRC (PRIVMSG, JOIN, KICK, etc.)

2. **RFC 2812** - Internet Relay Chat: Client Protocol (Mise à jour)
   - Lien: https://datatracker.ietf.org/doc/html/rfc2812
   - Version plus moderne et précise

3. **modern.ircdocs.horse**
   - Lien: https://modern.ircdocs.horse/
   - Documentation moderne et claire sur IRC
   - **Recommandé pour commencer**

---

### 🌐 Tutoriels Sockets en C/C++

1. **Beej's Guide to Network Programming**
   - Lien: https://beej.us/guide/bgnet/
   - **LE guide ultime** pour les sockets en C
   - Explications claires avec exemples

2. **IBM Developer - Socket Programming**
   - Lien: https://www.ibm.com/docs/en/i/7.3?topic=communications-socket-programming
   - Tutoriel détaillé sur les sockets

3. **GeeksforGeeks - Socket Programming**
   - Lien: https://www.geeksforgeeks.org/socket-programming-cc/
   - Exemples simples client/serveur

---

### 🔍 Tutoriels poll() / select() / epoll()

1. **man poll(2)** (dans votre terminal)
   ```bash
   man poll
   man select
   man epoll
   ```

2. **The Linux Programming Interface** (Livre)
   - Chapitres 63-64 sur I/O multiplexing

3. **Tutoriel poll() en français**
   - Chercher "poll C++ tutorial" sur YouTube

---

### 🛠️ Clients IRC pour tester

1. **irssi** (Terminal, recommandé)
   ```bash
   # Installation
   sudo apt-get install irssi  # Debian/Ubuntu
   brew install irssi          # macOS

   # Connexion
   irssi
   /connect localhost 6667 password
   /join #test
   ```

2. **WeeChat** (Terminal, moderne)
   ```bash
   sudo apt-get install weechat
   ```

3. **HexChat** (GUI, facile à débugger)
   - Lien: https://hexchat.github.io/

4. **netcat (nc)** - Pour tests bas niveau
   ```bash
   nc localhost 6667
   PASS password
   NICK mynick
   USER myuser 0 * :Real Name
   JOIN #test
   PRIVMSG #test :Hello world
   ```

---

### 📖 Ressources C++98

1. **cppreference.com**
   - Lien: https://en.cppreference.com/w/cpp/98
   - Toutes les fonctionnalités C++98

2. **Différences C++98 vs C++11**
   - Pas de `nullptr` → utiliser `NULL`
   - Pas de `auto` → déclarer les types
   - Pas de `std::to_string()` → utiliser `stringstream`

---

### 🎓 Projets similaires (inspiration)

1. **GitHub - ft_irc examples** (⚠️ À consulter APRÈS avoir commencé)
   - Chercher "42 ft_irc" sur GitHub
   - **Attention**: Ne pas copier, juste s'inspirer de l'architecture

2. **miniircd** - Serveur IRC minimaliste en Python
   - Lien: https://github.com/jrosdahl/miniircd
   - Bon pour comprendre la logique métier

---

### 🐛 Debugging

1. **Wireshark** - Analyser les paquets réseau
   - Voir exactement ce qui est envoyé/reçu
   - Filtrer par port: `tcp.port == 6667`

2. **Valgrind** - Memory leaks
   ```bash
   valgrind --leak-check=full ./ircserv 6667 pass
   ```

3. **netstat / lsof** - Voir les connexions
   ```bash
   netstat -an | grep 6667
   lsof -i :6667
   ```

---

### 📝 Codes de réponse IRC importants

```
001 RPL_WELCOME          : Welcome to the IRC network
353 RPL_NAMREPLY         : Liste des membres d'un channel
366 RPL_ENDOFNAMES       : Fin de la liste NAMES
401 ERR_NOSUCHNICK       : Nickname inexistant
403 ERR_NOSUCHCHANNEL    : Channel inexistant
442 ERR_NOTONCHANNEL     : Vous n'êtes pas sur ce channel
461 ERR_NEEDMOREPARAMS   : Pas assez de paramètres
462 ERR_ALREADYREGISTRED : Déjà enregistré
464 ERR_PASSWDMISMATCH   : Mauvais password
471 ERR_CHANNELISFULL    : Channel plein (mode +l)
473 ERR_INVITEONLYCHAN   : Channel en invite-only (mode +i)
475 ERR_BADCHANNELKEY    : Mauvais password channel (mode +k)
482 ERR_CHANOPRIVSNEEDED : Vous n'êtes pas operator
```

---

## Checklist finale avant évaluation

### ✅ Fonctionnalités obligatoires
- [ ] Authentification (PASS + NICK + USER)
- [ ] JOIN un channel
- [ ] PRIVMSG (channel et privé)
- [ ] KICK
- [ ] INVITE
- [ ] TOPIC
- [ ] MODE i, t, k, o, l

### ✅ Contraintes techniques
- [ ] Un seul poll() (ou équivalent)
- [ ] Pas de fork
- [ ] I/O non-bloquantes
- [ ] Pas de crash
- [ ] Gestion de multiples clients simultanés
- [ ] C++98 strict
- [ ] Compilation sans warnings (-Wall -Wextra -Werror)

### ✅ Tests
- [ ] Test avec irssi/hexchat/weechat
- [ ] Test netcat avec ctrl+D (données partielles)
- [ ] Test avec plusieurs clients simultanés
- [ ] Aucun memory leak (valgrind)
- [ ] Test de tous les modes (+i, +t, +k, +o, +l)

---

## Conseils généraux

### ⚠️ Pièges à éviter
1. **Oublier `\r\n`**: Les commandes IRC se terminent par `\r\n`, pas juste `\n`
2. **Ignorer les messages partiels**: `recv()` peut retourner des données incomplètes
3. **Ne pas tester avec un vrai client**: nc ne suffit pas, utilisez irssi
4. **Mauvaise gestion de poll()**: Ne pas oublier de retirer les FD fermés
5. **Format des réponses IRC**: Respecter exactement la RFC (`:prefix COMMAND params :trailing`)

### 💡 Conseils pro
1. **Commencez simple**: D'abord un serveur qui accepte juste les connexions
2. **Loggez tout**: `std::cout << "Received: " << buffer << std::endl;`
3. **Testez tôt, testez souvent**: Ne codez pas 1000 lignes avant de compiler
4. **Pair programming sur poll()**: C'est la partie la plus complexe
5. **Documentez votre code**: Vous relirez ce code dans 1 mois

### 🚀 Pour aller plus loin (bonus)
- Bot IRC qui répond à des commandes
- Transfert de fichiers (DCC)
- Support SSL/TLS
- Logs persistants
- Interface web de monitoring

---

**Bon courage pour ft_irc !** 🎉

N'hésitez pas à consulter cette doc régulièrement et à l'enrichir avec vos découvertes.
