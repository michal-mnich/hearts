# Hearts Game Server and Client

This repository contains the implementation of a server and client for the game of Hearts. The server conducts the game, while the clients represent the players.

## Game Rules

Hearts is played by four players using a standard 52-card deck. Players sit at the table in positions N (North), E (East), S (South), and W (West). The game consists of multiple deals. In each deal, each player receives 13 cards. Players only know their own cards. The game consists of 13 tricks. In the first trick, the chosen player leads by placing one of their cards on the table. The other players then place one of their cards in clockwise order. Players must follow suit if possible. If a player does not have a card of the required suit, they can place a card of any other suit. There is no obligation to play a higher card. The player who played the highest card of the suit led wins the trick and leads the next trick. Standard card rankings apply (from weakest to strongest): 2, 3, 4, ..., 9, 10, Jack, Queen, King, Ace.

The goal of the game is to take as few cards as possible. Points are awarded for taking cards. The player with the fewest points at the end of the game wins. There are seven types of deals:

1. Avoid taking tricks, 1 point per trick taken.
2. Avoid taking hearts, 1 point per heart taken.
3. Avoid taking queens, 5 points per queen taken.
4. Avoid taking kings and jacks, 2 points per king or jack taken.
5. Avoid taking the king of hearts, 18 points for taking it.
6. Avoid taking the seventh and last trick, 10 points for each.
7. Robber, points are awarded for all the above.

A deal does not need to be played to the end – it can be stopped if all points have been distributed.

## Server Parameters

Server parameters can be provided in any order. If a parameter is provided more than once, the first or last occurrence on the list takes precedence.

- `-p <port>`: Specifies the port number the server should listen on. This parameter is optional. If not provided or set to zero, the port number is chosen by the `bind` function.
- `-f <file>`: Specifies the name of the file containing the game definition. This parameter is mandatory.
- `-t <timeout>`: Specifies the maximum server wait time in seconds. This is a positive number. This parameter is optional. If not provided, the default is 5 seconds.

## Client Parameters

Client parameters can be provided in any order. If a parameter is provided more than once or conflicting parameters are provided, the first or last occurrence on the list takes precedence.

- `-h <host>`: Specifies the IP address or hostname of the server. This parameter is mandatory.
- `-p <port>`: Specifies the port number the server is listening on. This parameter is mandatory.
- `-4`: Forces the use of IPv4 for communication with the server. This parameter is optional.
- `-6`: Forces the use of IPv6 for communication with the server. This parameter is optional.

If neither `-4` nor `-6` is provided, the protocol version is chosen by the `getaddrinfo` function with `ai_family = AF_UNSPEC`.

- `-N`, `-E`, `-S`, `-W`: Specifies the position the client wants to take at the table. This parameter is mandatory.
- `-a`: This parameter is optional. If provided, the client is an automatic player. If not provided, the client acts as an intermediary between the server and the user.

## Communication Protocol

The server and client communicate using TCP. Messages are ASCII strings terminated by `\r\n`. There are no other whitespace characters in the messages. Messages do not contain a terminal zero. Table positions are encoded as N, E, S, or W. Deal types are encoded as digits from 1 to 7. Trick numbers are encoded as numbers from 1 to 13 in base 10 without leading zeros. Cards are encoded first by value (2, 3, 4, ..., 10, J, Q, K, A) and then by suit (C for clubs, D for diamonds, H for hearts, S for spades).

### Server to Client Messages

- `IAM<position>\r\n`: Sent by the client to the server after establishing a connection, indicating the desired table position.
- `BUSY<list of occupied positions>\r\n`: Sent by the server if the chosen position is already occupied, listing the occupied positions.
- `DEAL<deal type><starting position><list of cards>\r\n`: Sent by the server to clients after four clients have connected, indicating the start of a deal.
- `TRICK<trick number><list of cards>\r\n`: Sent by the server to the client, requesting a card to be played.
- `WRONG<trick number>\r\n`: Sent by the server to the client if an incorrect message is received in response to a `TRICK` message.
- `TAKEN<trick number><list of cards><position>\r\n`: Sent by the server to clients, indicating which client took the trick.
- `SCORE<position><points><position><points><position><points><position><points>\r\n`: Sent by the server to clients after a deal, indicating the scores.
- `TOTAL<position><points><position><points><position><points><position><points>\r\n`: Sent by the server to clients after a deal, indicating the total scores.

### Client to Server Messages

- `TRICK<trick number><card>\r\n`: Sent by the client to the server, indicating the card played.

## Functional Requirements

Programs should thoroughly check the validity of input parameters and print understandable error messages to standard error.

The server is assumed to be honest, but clients may not be. The server should strictly enforce the game rules.

The client should implement a heuristic strategy for playing the game.

The server and automatic client should print a game report to standard output.

Programs should exit with code 0 if the game completes successfully, and code 1 otherwise.

## Game Definition File Format

The file specified to the server contains a textual description of the game. Each deal is described by five lines:

1. `<deal type><starting position>\n`
2. `<list of cards for N>\n`
3. `<list of cards for E>\n`
4. `<list of cards for S>\n`
5. `<list of cards for W>\n`

The file content is assumed to be correct.

## Game Report Format

The game report includes all sent and received messages (including incorrect ones), each preceded by the sender's and receiver's IP addresses and ports, and the timestamp.

Example:

```
[44.44.44.44:4321,11.22.33.44:1234,2024-04-25T18:21:00.000] IAMN\r\n
[11.22.33.44:1234,44.44.44.44:4321,2024-04-25T18:21:00.010] BUSYNW\r\n
```

## User Interface

The client acting as an intermediary provides a text interface for the user. The client prints information and requests from the server to standard output and reads user decisions and commands from standard input.

### Commands

- `cards`: Display the list of cards in hand.
- `tricks`: Display the list of tricks taken in the last deal.

### TRICK Message

When a `TRICK` message is received, the user selects a card by typing `!<card>` (e.g., `!10C`) and pressing enter.

## Build Instructions

To build the project, run the following command in the `src` directory:

```sh
make
```

This will create the `kierki-serwer` and `kierki-klient` executables. Use the following command to clean the build files:

```sh
make clean
```

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
