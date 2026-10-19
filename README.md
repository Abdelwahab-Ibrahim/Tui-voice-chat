# C++ Voice Chat TUI --- Version 0

## 1. Goal

Build a small C++ voice-chat application that runs locally or on a
trusted private network. Version 0 focuses on the core experience:
create a room, share its key privately, and talk to whoever joins.

The application should let a user:

1.  Register or log in with a username and password.
2.  Create a voice room and receive its room key.
3.  Join a room by entering its key.
4.  Talk to other users currently in the same room.
5.  Leave a room or disconnect.
6.  Use the application through a simple terminal interface.

A one-to-one call uses the same room mechanism as a group call: the
caller creates a room and shares its key with the intended friend. The
application does not manage friends or send invitations in Version 0;
users share keys themselves.

This is a learning prototype, not a production-ready service.

## 2. Scope

### Required features

-   Separate `server` and `client` executables.
-   C++20 and CMake.
-   SQLite for persistent user accounts.
-   In-memory room state, room keys, connected clients, and current room
    membership.
-   Account registration and login.
-   Room creation that returns a room key.
-   Joining a room by room key.
-   Voice transmission and playback between members of the same room.
-   Basic terminal UI.
-   Basic validation and cleanup when clients disconnect.

### Not required in Version 0

-   Persistent rooms or room history.
-   A public room directory or room listing.
-   Friends, contacts, invitations, or contact synchronization.
-   Persistent saved rooms on the server. A client may optionally save
    room keys locally for convenience.
-   Text chat or persistent message history.
-   Room owners, roles, access-control lists, or membership permissions.
-   Password hashing, TLS, transport encryption, or end-to-end
    encryption.
-   Session-token systems.
-   Opus codec, jitter buffer, voice activity detection, noise
    suppression, and echo cancellation.
-   Reconnection, rate limiting, NAT traversal, or production hardening.
-   Video, screen sharing, GUI, or mobile clients.
-   Complex configuration systems or containerization.

**Prototype warning:** passwords are stored and transmitted in plain
text in this version. A room key acts as a bearer credential: anyone who
obtains it can join. Use only on localhost or a trusted private LAN. Do
not expose the server to the public internet.

## 3. Application behavior

### Accounts and login

-   A user supplies a username and password.
-   The server checks credentials against the users stored in SQLite.
-   If registration is supported, usernames must be unique.
-   Account data survives server restarts because it is stored in
    SQLite.
-   The server associates each active TCP connection with the logged-in
    user.
-   Passwords are plain text for this learning version; do not log them.

### Rooms and room keys

-   Any logged-in user may create a room.
-   The server generates a unique, hard-to-guess room key and returns it
    to the client.
-   The creator joins the newly created room automatically.
-   Another logged-in user can join by entering that key.
-   The key is shared manually, outside the application. There is no
    friends list or invitation workflow.
-   A private one-to-one call is simply a room whose key the two people
    choose to share only with each other. The server does not enforce
    that social expectation: anyone with the key can join.
-   Room keys are not sequential database IDs. Generate them with a
    secure random source and enough entropy to make guessing
    impractical.
-   Rooms and their keys exist only in server memory. They do not need
    to survive a server restart.
-   The server tracks active membership in memory and removes members
    when they leave or disconnect.
-   An empty room can be removed from memory. If removed, its old key no
    longer works.
-   A client may save room keys locally as shortcuts, but saved keys are
    not authoritative server state.

### Voice

-   A client can send microphone audio only while joined to a room.
-   The server forwards voice packets only to the other active members
    of the same room.
-   The client plays received audio through its selected output device.
-   The server does not decode or mix audio.
-   Use raw PCM first to reduce codec complexity. This uses more
    bandwidth than a codec such as Opus.
-   Test with headphones to reduce feedback.

## 4. SQLite requirements

Use SQLite as a local database file managed by the server. Do not add a
separate database server.

### Table: `users`

  Column       Type      Rules
  ------------ --------- ----------------------------------------------
  `id`         INTEGER   Primary key
  `username`   TEXT      Required, unique
  `password`   TEXT      Required; plain text only for this prototype

Rooms are not stored in SQLite in Version 0. There is no `rooms` table
or room-membership table in this version because rooms are temporary
runtime sessions.

Enable SQLite foreign-key enforcement for each database connection if
foreign keys are introduced later. Use prepared statements with bound
parameters for database queries rather than building SQL by
concatenating user input.

### Data persistence

The following survive a server restart:

-   Usernames and passwords.

The following do not need to survive a restart:

-   Connected sessions.
-   Room keys and room state.
-   Current room membership.
-   Voice packets or audio history.
-   Client-side saved room keys only survive if the client deliberately
    saves them locally; the corresponding room may no longer exist after
    a server restart.

### Database responsibilities

The server's database layer should:

-   Open or create the SQLite database file.
-   Create the `users` table if it does not exist.
-   Insert a new user.
-   Find a user by username.
-   Verify a login against the stored password.
-   Handle duplicate usernames and database errors without crashing.

Keep SQL access out of the terminal UI. The client communicates with the
server; only the server accesses the SQLite database.

## 5. Minimal architecture

Use two network channels:

-   **TCP:** registration, login, room creation, room joining, room
    leaving, and other control messages.
-   **UDP:** live voice packets.

### Server responsibilities

-   Accept TCP client connections.
-   Parse and dispatch control messages.
-   Use SQLite for user accounts.
-   Generate room keys and maintain active rooms in memory.
-   Track logged-in users, connected clients, and current room
    membership in memory.
-   Receive voice packets and forward them only to other members of the
    sender's room.
-   Clean up connection, room membership, and empty-room state on
    disconnect.

### Client responsibilities

-   Display login/register prompts and the main terminal interface.
-   Send account and room requests over TCP.
-   Display the room key after room creation and allow a user to enter a
    key to join.
-   Optionally save room keys locally for convenient reuse.
-   Capture microphone audio.
-   Send and receive voice packets over UDP.
-   Play received audio.

### Shared code

A small shared library may contain:

-   Control-message types.
-   Message serialization and parsing.
-   Protocol constants and size limits.

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
-   `create_room`: request a new temporary room.
-   `join_room`: room key.
-   `leave_room`.

### Server-to-client messages

-   `register_ok` or `error`.
-   `login_ok` or `error`.
-   `room_created`: generated room key.
-   `room_joined`: confirmation and any basic room/session details
    needed by the client.
-   `room_left`.
-   `error`.

The server must reject malformed messages, invalid room keys,
unauthenticated room requests, and unknown message types without
crashing. Keep the protocol small; do not implement a full invitation or
contact system.

### Voice packet routing

The client and server use UDP for audio. The server must associate the
authenticated client/session with its UDP endpoint and current room. A
received voice packet is forwarded only to other active members of that
room, never back to the sender unless local echo is deliberately
implemented by the client.

## 7. Dependencies

Keep dependencies limited to those required for the working application.

-   **CMake 3.25+:** build system.
-   **C++20 compiler:** language support.
-   **SQLite3:** persistent user accounts.
-   **Standalone Asio:** TCP and UDP networking.
-   **miniaudio:** microphone capture and speaker playback.
-   **FTXUI:** terminal UI, if it does not delay core functionality.

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

**Done when:** a user can register, restart the server, and log in again
with the same account.

### Stage 2 --- TCP protocol and sessions

-   Implement length-prefixed JSON messages.
-   Accept multiple TCP clients.
-   Associate each connection with its logged-in user.
-   Return clear success and error messages.

**Done when:** two clients can connect and log in independently.

### Stage 3 --- Temporary rooms and room keys

-   Implement room creation and generate a hard-to-guess key.
-   Automatically join the creator to the new room.
-   Implement joining by room key and leaving.
-   Track active room membership in memory.
-   Remove membership on disconnect and clean up empty rooms.

**Done when:** one client creates a room, a second client joins using
the returned key, and both can leave without crashing the server.
Restarting the server invalidates all previous rooms and keys.

### Stage 4 --- Basic client interface

-   Add login/register prompts.
-   Add a create-room action and display the generated key.
-   Add a join-room-by-key prompt and a leave-room action.
-   Show basic connection and error status.
-   Optionally allow room keys to be saved locally.

**Done when:** users can create a room and join it by key without
manually crafting protocol messages.

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

Run one server and two clients:

1.  Register two accounts.
2.  Log in from both clients.
3.  Create a room from one client and copy its key.
4.  Join the room from the other client using that key.
5.  Talk in both directions.
6.  Leave the room and verify membership is cleaned up.
7.  Disconnect one client and verify the server continues running.
8.  Restart the server, confirm accounts remain, and confirm old room
    keys no longer work.

**Version 0 is complete when this scenario works repeatedly.**

## 10. Acceptance checklist

-   [ ] `server` and `client` build from CMake.
-   [ ] Server creates and opens the SQLite database.
-   [ ] Usernames are unique.
-   [ ] Login succeeds with the correct password and fails with an
    incorrect password.
-   [ ] Accounts persist after server restart.
-   [ ] A user can create a room and receive a room key.
-   [ ] A second user can join using the room key.
-   [ ] Room membership is removed when a user leaves or disconnects.
-   [ ] Empty rooms are cleaned up.
-   [ ] Old room keys stop working after server restart.
-   [ ] Microphone audio can be captured and played locally.
-   [ ] Two clients in one room can talk to each other.
-   [ ] Voice packets are not forwarded across different rooms.
-   [ ] Disconnecting a client does not crash the server.
-   [ ] The README explains how to build, run, and test the app.

## 11. Version 0 versus later versions

Version 0 establishes the core application flow and gives hands-on
practice with SQLite, TCP/UDP networking, terminal interaction, and
basic audio I/O. Room keys are shared manually, and anyone who has a key
can join the associated active room.

Later versions may add password hashing, encrypted transport, persistent
rooms, explicit invitations, friends and contacts, revocable invite
codes, call history, Opus, jitter buffering, reconnect logic, rate
limits, and production hardening as separate learning stages.
