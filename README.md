# C++ Voice Chat TUI --- Version 0 Requirements

## 1. Goal

Build a small but complete C++ voice-chat application that can be run
locally or on a trusted private network. This version focuses on
learning the basic application flow and getting practical experience
with SQLite.

A user should be able to: 
1. Log in with a username and password. 
2. Create a room, list rooms, join a room, and leave a room. 
3. Send and receive text messages in a room. 
4. Speak through their microphone and hear other users in the same room. 
5. Use the application through asimple terminal interface.

This is a learning prototype, not a production-ready service.

## 2. Scope

### Required features

-   Separate server and client executables.
-   C++20 and CMake.
-   SQLite for persistent usernames, passwords, and rooms.
-   In-memory state for currently connected clients and current room
    membership.
-   Login and account creation behavior.
-   Room creation, listing, joining, and leaving.
-   Text chat within a room.
-   Microphone capture, voice transmission, and speaker playback.
-   Basic terminal UI.
-   Basic validation and handling of client disconnects.

### Not required in Version 0

-   Password hashing or encryption.
-   TLS or end-to-end encryption.
-   Session-token systems.
-   User roles, permissions, admin commands, mute, kick, and bans.
-   Persistent text-message history.
-   Opus codec, jitter buffer, voice activity detection, noise
    suppression, and echo cancellation.
-   Reconnection, rate limiting, NAT traversal, or production hardening.
-   Video, screen sharing, GUI, or mobile clients.
-   Complex configuration systems or containerization.

**Prototype warning:** passwords are stored and transmitted in plain
text in this version. Use only on localhost or a trusted private LAN. Do
not expose the server to the public internet. Add password hashing and
transport security in a later version.

## 3. Application behavior

### Accounts and login

-   A user supplies a username and password.
-   If the account exists, the server checks that the supplied password
    matches the stored password.
-   If account creation is supported through the login flow, a new
    username can be registered and stored in SQLite.
-   A username must be unique.
-   Account data survives server restarts because it is stored in
    SQLite.
-   The server associates each active TCP connection with the logged-in
    user.
-   Passwords are plain text for this learning version; do not log them.

### Rooms

-   Any logged-in user may create a room.
-   Each room has a unique ID and a name.
-   Users can request a list of rooms, join one room at a time, and
    leave it.
-   Room definitions persist in SQLite across server restarts.
-   Active room membership is held in memory and is cleared when users
    disconnect or the server stops.
-   Rooms do not have roles or special permissions in Version 0.

### Text chat

-   Logged-in users can send text only after joining a room.
-   The server broadcasts each message to the members currently in that
    room.
-   Messages include the sender's username and message text.
-   Text history is not saved in SQLite.
-   A client in a different room must not receive the message.

### Voice

-   Users can send microphone audio only while they are in a room.
-   The server forwards audio only to other members of the same room.
-   The client plays received audio through the selected output device.
-   The server does not decode or mix audio.
-   Use raw PCM first to reduce codec complexity. This uses more
    bandwidth than a codec such as Opus.
-   Test with headphones to reduce feedback.

## 4. SQLite requirements

Use SQLite as a local database file managed by the server. Do not add a
separate database server.

### Tables

A minimal schema can use two tables:

#### `users`

  Column       Type      Rules
  ------------ --------- ----------------------------------------------
  `id`         INTEGER   Primary key
  `username`   TEXT      Required, unique
  `password`   TEXT      Required; plain text only for this prototype

#### `rooms`

  Column            Type      Rules
  ----------------- --------- -----------------------
  `id`              INTEGER   Primary key
  `name`            TEXT      Required, unique
  `owner_user_id`   INTEGER   References `users.id`

Enable SQLite foreign-key enforcement for each database connection. Use
prepared statements with bound parameters for database queries rather
than building SQL by concatenating user input.

### Data persistence

The following must survive a server restart: - Usernames and
passwords. - Room names and their owner IDs.

The following do not need to survive a restart: - Connected sessions. -
Current room membership. - Text chat messages. - Voice packets or audio
history.

### Database responsibilities

Create a small database layer that: - Opens or creates the SQLite
database file. - Creates the required tables if they do not exist. -
Inserts a new user. - Finds a user by username. - Verifies a login
against the stored password. - Creates a room. - Lists rooms. -
Retrieves a room by ID or name. - Handles duplicate usernames, duplicate
room names, and database errors without crashing.

Keep SQL access out of the terminal UI. The client communicates with the
server; only the server accesses the SQLite database.

## 5. Minimal architecture

Use two network channels:

-   **TCP:** login, room operations, and text chat.
-   **UDP:** live voice packets.

### Server responsibilities

-   Accept TCP client connections.
-   Parse and dispatch control messages.
-   Use SQLite for users and rooms.
-   Track logged-in users and room membership in memory.
-   Broadcast room text messages.
-   Receive voice packets and forward them to other members of the same
    room.
-   Clean up connection and membership state on disconnect.

### Client responsibilities

-   Display the login and room interface.
-   Send login, room, and chat requests over TCP.
-   Display incoming chat messages.
-   Capture microphone audio.
-   Send and receive voice packets over UDP.
-   Play received audio.

### Shared code

A small shared library may contain: - Control-message types. - Message
serialization and parsing. - Protocol constants and size limits.

The client must not compile or link server implementation files. Both
may link the shared protocol library.

## 6. Minimal control protocol

Use length-prefixed JSON over TCP. TCP is a byte stream, so the receiver
must handle partial messages and multiple messages arriving together. A
four-byte big-endian message length followed by the JSON body is
sufficient.

### Client-to-server messages

-   `register`: username and password.
-   `login`: username and password.
-   `create_room`: room name.
-   `list_rooms`.
-   `join_room`: room ID.
-   `leave_room`.
-   `chat_send`: text.

### Server-to-client messages

-   `register_ok` or `error`.
-   `login_ok` or `error`.
-   `room_list`.
-   `room_created`.
-   `room_joined`.
-   `chat_message`: sender username and text.
-   `member_joined` and `member_left`, if needed by the UI.

The server must reject malformed messages and unknown message types
without crashing. Keep this protocol small; do not implement the full
Version 1 protocol.

## 7. Dependencies

Keep dependencies limited to the ones required for the working
application.

-   **CMake 3.25+:** build system.
-   **C++20 compiler:** language support.
-   **SQLite3:** persistent users and rooms.
-   **Standalone Asio:** TCP and UDP networking.
-   **miniaudio:** microphone capture and speaker playback.
-   **FTXUI:** terminal UI, if it does not delay the core functionality.

A line-oriented terminal interface is acceptable while building the
first working version. Replace it with FTXUI after the control flow
works.

Do not add password-hashing libraries, TLS libraries, Opus, or a testing
framework just for this version. Add them later when their corresponding
features are introduced.

## 8. Suggested project structure

``` text
voicechat/
├── CMakeLists.txt
├── README.md
├── common/
│   ├── protocol.hpp
│   └── protocol.cpp
├── server/
│   ├── main.cpp
│   ├── server.hpp
│   ├── server.cpp
│   ├── session.hpp
│   ├── session.cpp
│   ├── database.hpp
│   ├── database.cpp
│   ├── user_repo.hpp
│   ├── user_repo.cpp
│   ├── room_repo.hpp
│   ├── room_repo.cpp
│   ├── room_manager.hpp
│   └── room_manager.cpp
└── client/
    ├── main.cpp
    ├── app.hpp
    ├── app.cpp
    ├── control_client.hpp
    ├── control_client.cpp
    ├── voice_client.hpp
    └── voice_client.cpp
```

Keep this structure flexible. It is fine to combine small classes into
fewer files until the code becomes difficult to navigate.

## 9. Implementation stages

### Stage 0 --- Build foundation

-   Create the repository and CMake project.
-   Add separate `server` and `client` targets.
-   Link SQLite3 and confirm a simple query works.
-   Confirm both executables build and launch.

**Done when:** the project builds and the server can open a SQLite
database file.

### Stage 1 --- SQLite and accounts

-   Create the `users` table.
-   Implement account registration and login.
-   Use prepared statements.
-   Handle duplicate usernames and incorrect passwords.
-   Confirm accounts persist after restarting the server.

**Done when:** a user can register, log in, restart the server, and log
in again with the same account.

### Stage 2 --- TCP protocol

-   Implement length-prefixed JSON messages.
-   Accept multiple TCP clients.
-   Associate each connection with its logged-in user.
-   Return clear success and error messages.

**Done when:** two clients can connect and log in independently.

### Stage 3 --- Persistent rooms and text chat

-   Create the `rooms` table.
-   Implement create, list, join, and leave.
-   Keep current room membership in memory.
-   Broadcast messages only to members of the same room.

**Done when:** two clients in the same room can chat, clients in
different rooms cannot see each other's messages, and rooms remain after
a server restart.

### Stage 4 --- Basic client interface

-   Add login/register prompts.
-   Show rooms and allow room creation and joining.
-   Add a chat log and input line.
-   Show basic connection or error status.

**Done when:** a user can complete login, room creation/joining, and
text chat without manually crafting protocol messages.

### Stage 5 --- Audio loopback

-   Use miniaudio to capture microphone input and play it back locally.
-   Keep the audio callback short and avoid network operations inside
    it.

**Done when:** local microphone loopback works on the target machine.

### Stage 6 --- Networked voice

-   Add UDP sockets to client and server.
-   Associate each client with a UDP endpoint.
-   Forward voice frames only to other members of the sender's room.
-   Play incoming audio through the client's output device.

**Done when:** two clients in one room can hear each other on localhost
or a trusted LAN.

### Stage 7 --- Full application test

Run one server and two clients: 1. Register two accounts. 2. Log in from
both clients. 3. Create a room from one client. 4. Join it from the
other. 5. Exchange text messages. 6. Talk in both directions. 7.
Disconnect one client and verify the server continues running. 8.
Restart the server and confirm accounts and rooms remain in SQLite.

**Version 0 is complete when this scenario works repeatedly.**

## 10. Acceptance checklist

-   [x] `server` and `client` build from CMake.
-   [x] Server creates and opens the SQLite database.
-   [x] Usernames are unique.
-   [x] Login succeeds for the correct password and fails for an
    incorrect password.
-   [x] Accounts persist after server restart.
-   [ ] Rooms persist after server restart.
-   [ ] A user can create, list, join, and leave rooms.
-   [ ] Text is broadcast only within the correct room.
-   [ ] Microphone audio can be captured and played locally.
-   [ ] Two clients in one room can talk to each other.
-   [ ] Disconnecting a client does not crash the server.
-   [ ] The README explains how to build, run, and test the app.



## 12. Version 0 versus Version 1

Version 0 establishes a complete working application and provides
hands-on practice with SQLite, TCP/UDP networking, terminal interaction,
and basic audio I/O.

Version 1 can add password hashing, encrypted transport, persistent chat
history if desired, roles and permissions, Opus, jitter buffering,
reconnect logic, rate limits, and production hardening as separate
learning stages.
