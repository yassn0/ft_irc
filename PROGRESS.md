# Progression du projet ft_irc (pour irssi)

## ✅ Étape 1: Infrastructure de base (TERMINÉ)
- Serveur avec socket + bind + listen
- Boucle poll() fonctionnelle
- Accepte les connexions
- Mode non-bloquant avec fcntl()

## ✅ Étape 2: Classe Client (TERMINÉ)
**Fichiers créés:**
- `inc/Client.hpp`
- `src/Client.cpp`

**Attributs:**
- `_fd`: File descriptor du socket
- `_nickname`: Nickname IRC
- `_username`: Username IRC
- `_buffer`: Buffer pour accumuler les messages partiels
- `_authenticated`: A envoyé PASS avec bon password
- `_registered`: A envoyé NICK + USER (prêt à utiliser IRC)

**Méthodes clés:**
- `sendMessage()`: Envoyer un message au client
- `appendToBuffer()`: Ajouter des données au buffer
- Getters/Setters pour tous les attributs

## ✅ Étape 3: Intégration Client dans Server (TERMINÉ)

### Modifications à faire dans Server.cpp:

#### 1. Inclure Client.hpp
```cpp
#include "../inc/Client.hpp"
```

#### 2. Dans acceptNewClient()
**Avant:**
```cpp
_poll_fds.push_back(client_pollfd);
// TODO: Créer un objet Client
```

**Après:**
```cpp
_poll_fds.push_back(client_pollfd);

// Créer l'objet Client
Client* new_client = new Client(client_fd);
_clients.push_back(new_client);
```

#### 3. Dans handleClientMessage()
**Objectif:** Gérer les messages partiels avec le buffer

**Avant:**
```cpp
buffer[bytes_read] = '\0';
// TODO: Parser et traiter
```

**Après:**
```cpp
buffer[bytes_read] = '\0';

// Trouver le client correspondant
Client* client = NULL;
for (size_t i = 0; i < _clients.size(); i++) {
    if (_clients[i]->getFd() == client_fd) {
        client = _clients[i];
        break;
    }
}

if (!client)
    return;

// Ajouter au buffer du client
client->appendToBuffer(std::string(buffer, bytes_read));

// Chercher des commandes complètes (terminées par \r\n)
std::string& buf = client->getBuffer();
size_t pos;
while ((pos = buf.find("\r\n")) != std::string::npos) {
    std::string command = buf.substr(0, pos);
    buf.erase(0, pos + 2);  // Enlever la commande + \r\n

    // Traiter la commande
    handleCommand(client, command);
}
```

#### 4. Dans removeClient()
**Ajouter:** Supprimer l'objet Client de _clients

```cpp
// Supprimer de _clients
for (size_t i = 0; i < _clients.size(); i++) {
    if (_clients[i]->getFd() == client_fd) {
        delete _clients[i];
        _clients.erase(_clients.begin() + i);
        break;
    }
}
```

#### 5. Nouvelle méthode: handleCommand()
```cpp
void Server::handleCommand(Client* client, const std::string& command);
```

Cette méthode va parser et dispatcher les commandes IRC.

**Ce qui a été fait:**
- ✅ Ajout de `#include "../inc/Client.hpp"`
- ✅ Création d'objets Client dans `acceptNewClient()`
- ✅ Implémentation de `getClientByFd()`
- ✅ Modification de `handleClientMessage()` pour utiliser le buffer et détecter `\r\n`
- ✅ Implémentation de `handleCommand()` (version basique avec echo)
- ✅ Modification de `removeClient()` pour delete les objets Client
- ✅ Modification du destructeur pour nettoyer les clients

**Résultat:**
Le serveur gère maintenant correctement les messages partiels et détecte les commandes complètes terminées par `\r\n`.

---

## ✅ Étape 4: Parsing des commandes IRC (TERMINÉ)

Format IRC: `COMMAND param1 param2 :trailing`

Exemples:
- `NICK alice`
- `USER alice 0 * :Alice Wonderland`
- `PRIVMSG #channel :Hello world`
- `PING :irssi`

---

## ✅ Étape 5: Refactoring - Séparation du code (TERMINÉ)

**Problème:** Tout le parsing des commandes IRC était dans Server.cpp → code non maintenable

**Solution:** Création de la classe CommandHandler

### Architecture après refactoring:

```
Server.cpp
├── Gestion réseau (socket, poll, accept, recv)
└── Appelle CommandHandler pour traiter les commandes

CommandHandler.cpp
├── Parsing des commandes IRC
├── Dispatch vers les bons handlers
└── Implémentation de toutes les commandes (PING, PASS, NICK, USER, etc.)
```

### Fichiers créés:
- `inc/CommandHandler.hpp` - Déclaration de la classe
- `src/CommandHandler.cpp` - Implémentation de toutes les commandes

### Modifications:
- `Server.hpp`: Ajout de `CommandHandler*` et getter `getPassword()`
- `Server.cpp`: Déplacé toute la logique de commandes vers CommandHandler
- `Makefile`: Ajout de CommandHandler.cpp

### Avantages:
✅ **Séparation des responsabilités**: Server = réseau, CommandHandler = logique IRC
✅ **Plus facile à maintenir**: Chaque commande est dans sa propre méthode
✅ **Extensible**: Ajouter une nouvelle commande = ajouter une méthode
✅ **Testable**: On peut tester les commandes indépendamment du réseau

---

## ✅ Étape 6: PING/PONG + Authentification (TERMINÉ)

**Pourquoi c'est critique:**
irssi envoie `PING` régulièrement et attend `PONG` en retour. Sans réponse, irssi se déconnecte après **180 secondes**.

**Format:**
```
Client → PING :irssi
Serveur → :servername PONG servername :irssi
```

**À implémenter dans handleCommand():**
```cpp
if (command.find("PING") == 0) {
    // Extraire le paramètre après PING
    std::string param = command.substr(5);  // " :irssi"
    std::string response = ":server PONG server " + param + "\r\n";
    client->sendMessage(response);
}
```

---

## 📋 Étape 6: Commandes IRC pour irssi

### 1. PING/PONG (CRITIQUE!)
```
Client → PING :irssi
Serveur → :servername PONG servername :irssi
```
Sans ça: irssi timeout après 180 secondes.

### 2. Authentification
```
PASS password
NICK alice
USER alice 0 * :Real Name
```

Réponses attendues:
```
:servername 001 alice :Welcome to the IRC Network
:servername 002 alice :Your host is servername
:servername 003 alice :This server was created <date>
:servername 004 alice servername version usermodes channelmodes
```

### 3. JOIN
```
JOIN #channel
```

Réponses:
```
:alice!user@host JOIN :#channel
:servername 332 alice #channel :Topic
:servername 353 alice = #channel :@operator user1 user2
:servername 366 alice #channel :End of /NAMES list
```

### 4. PRIVMSG
```
PRIVMSG #channel :Hello
PRIVMSG alice :Private message
```

### 5. Commandes opérateur
- KICK, INVITE, TOPIC
- MODE +i +t +k +o +l

---

## Notes importantes pour irssi

1. **Tous les messages IRC se terminent par `\r\n`**
2. **Buffer partiel obligatoire** (recv() peut retourner des données incomplètes)
3. **Codes numériques obligatoires** (001, 002, 353, 366, etc.)
4. **PING/PONG critique** (irssi se déconnecte sans ça)
5. **CAP**: irssi peut envoyer `CAP LS` → répondre `CAP * LS :`
