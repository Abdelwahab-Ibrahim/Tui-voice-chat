# C++ Voice Chat TUI — Project Roadmap

Oct 15, 2026 · @A-bood

## What we're building

A terminal voice-chat app in C++: one server, many TUI clients, with account login, roles, and group rooms that carry live voice and text. You build it in 12 small stages (0–11), and every stage ends with something you can run and test before moving on.

**Version 1 includes**

- Register and log in with username + password
- Rooms: create, list, join, leave
- Roles with permissions: owner, admin, moderator, member, plus a muted state
- Text chat inside a room
- Live group voice inside a room
- A terminal UI (TUI) client

**Version 1 leaves out:** video, screen share, GUI or mobile clients, end-to-end encryption, and NAT traversal (use port forwarding or a VPN when testing across networks).

**One assumption to confirm:** I read "rules" as *roles* (who may kick, mute, or create rooms). If you meant room rules shown on join, that is a small add-on in Stage 4.

**How to use this roadmap**

1. Build strictly in stage order; a stage is finished only when its "done when" check passes.
2. Text chat comes before voice because it is far easier to debug.
3. Audio is built and tested on your own machine before any networking touches it.
4. Target Linux with C++20 first. The chosen libraries also run on macOS and Windows, but Linux has the best tooling for a learner.

## Architecture

The client and server are joined by two channels, each suited to its job: TCP for login, rooms, and text, and UDP for voice.

&#91;embedded content: architecture · client and server, 2 channels\]

Stages 2–5 build the TCP side and the UI; Stages 6–9 add the voice side. Because the server never decodes audio, it stays cheap and simple.

## Libraries and tools

Every library below is free, widely used, and has good beginner documentation. The stage column says when you first need it, so you install things gradually.

| Job | Library | Why this one | First used |
| --- | --- | --- | --- |
| Build system | CMake 3.25+ with Ninja | The standard for C++ projects; one build for server, client, tests | Stage 0 |
| Dependencies | vcpkg (manifest mode) | A single `vcpkg.json` lists every library; no manual installs | Stage 0 |
| Logging | spdlog | Fast, simple, readable logs for debugging network code | Stage 0 |
| Testing | Catch2 | Readable test syntax; works with CTest | Stage 1 |
| Control messages | nlohmann/json | Human-readable messages you can print and inspect | Stage 1 |
| Networking | Asio (standalone) | Async TCP and UDP; the model behind C++ networking | Stage 2 |
| Password hashing | libsodium | Argon2id hashing; later also encrypts voice packets | Stage 3 |
| Database | SQLite3 | One file, no server to run; enough for users and rooms | Stage 3 |
| Terminal UI | FTXUI | Modern C++ components and layouts; no ncurses knowledge needed | Stage 5 |
| Audio capture and playback | miniaudio | One header; microphone and speakers on all platforms | Stage 6 |
| Voice codec | libopus | The standard voice codec: 20 ms frames, low bitrate | Stage 7 |
| Encrypted control channel | OpenSSL (via Asio SSL) | TLS so passwords never cross the network in plain text | Stage 10 |
| Config files | toml++ | Simple human-editable config | Stage 10 |

**Optional, Stage 11:** RNNoise (noise suppression) and SpeexDSP (echo cancellation).

**Three choices made for you**

- JSON for control messages, a tiny binary header for voice. JSON is easy to debug; voice needs to be small and fast.
- miniaudio over PortAudio. Both work; miniaudio has no install step.
- FTXUI over ncurses. ncurses is C-style and harder to structure; FTXUI fits modern C++.

**Debugging tools worth installing now:** gdb or lldb, AddressSanitizer (a compiler flag), Wireshark (watch your packets), and netcat (`nc`) for poking at your server by hand.

## Roles and protocol

Design these two things on paper before writing code: who may do what, and what the messages look like. Everything else depends on them.

### Room roles and permissions

Each user has one role per room. A server-wide **admin** account overrides all of this (delete any room, ban accounts).

| Action | Owner | Moderator | Member | Muted |
| --- | --- | --- | --- | --- |
| Speak in voice | Yes | Yes | Yes | No |
| Send text | Yes | Yes | Yes | No |
| Mute a member | Yes | Yes | No | No |
| Kick a member | Yes | Yes (members only) | No | No |
| Promote or demote moderators | Yes | No | No | No |
| Delete the room | Yes | No | No | No |

Keep every permission check in one function, `can(actorRole, action, targetRole) -> bool`. It is pure logic with no networking, so it is easy to unit test.

### Control channel (TCP)

Every message is a 4-byte length prefix (big-endian) followed by a JSON body, for example `{"type":"login","username":"sara","password":"..."}`. The length prefix matters: TCP is a stream, not messages, so you must mark where each one ends.

| Direction | Message types |
| --- | --- |
| Client to server | `register`, `login`, `create_room`, `join_room`, `leave_room`, `chat_send`, `mute_user`, `kick_user`, `set_role`, `ping` |
| Server to client | `login_ok` (carries a voice token), `error` (code + text), `room_list`, `room_joined`, `member_joined`, `member_left`, `chat_message`, `role_changed`, `kicked`, `pong` |

### Voice channel (UDP)

Voice uses UDP because a late packet is worthless; resending it would only add delay. After login the client sends one UDP hello carrying its voice token, so the server learns which address belongs to which user. Each voice packet then has a 12-byte header followed by one 20 ms Opus frame.

| Field | Bytes | Purpose |
| --- | --- | --- |
| Version | 1 | Reject junk packets |
| Flags | 1 | Bit 0 = speaker stopped talking |
| Token / sender id | 4 | Client sends its token; server rewrites it to the sender's id when forwarding |
| Sequence | 2 | Detect loss and reordering |
| Timestamp | 4 | Lets the jitter buffer play frames at the right time |
| Opus payload | rest | One 20 ms voice frame |

The server never decodes audio. It checks the token, finds the sender's room, and forwards the packet to everyone else in that room. Each client decodes and mixes what it receives.

## The 12 stages

Each stage adds one layer on top of the last, and each has a check you can run to prove it works. Do not start a stage until the previous check passes. Stages 0–5 give you a working text-chat app; stages 6–9 add voice; 10–11 make it solid.

| Stage | Layer | What you build | Done when |
| --- | --- | --- | --- |
| 0 | Foundation | Repo, CMake, vcpkg, logging, test setup, empty server and client | Both executables build; one dummy test passes |
| 1 | Protocol | Message framing, JSON messages, voice packet format, permission logic (no sockets yet) | Unit tests pass for encode/decode round trips and for split or merged frames |
| 2 | Networking | Async TCP server and client with Asio; ping/pong; many clients | Several clients ping at once; disconnects are handled cleanly |
| 3 | Accounts | SQLite database, register, login, Argon2id hashing, sessions | Wrong password is rejected; accounts survive a server restart |
| 4 | Rooms and text | Room manager, roles, text broadcast, kick and mute | Three plain command-line clients chat in rooms; a moderator can kick; a muted user cannot send |
| 5 | TUI client | FTXUI screens: login, room list, chat room | The whole text-chat flow works inside the terminal UI |
| 6 | Audio I/O | Mic capture and speaker playback with miniaudio; ring buffer | `audio_loopback` plays your own mic through your speakers without crackling |
| 7 | Codec | Opus encode and decode wrappers | `opus_roundtrip` outputs clear voice and prints the bitrate |
| 8 | Voice transport | UDP voice server, room-based forwarding, client sender and receiver | Two clients in a room hear each other on a LAN |
| 9 | Voice quality | Jitter buffer, mixer, push-to-talk, voice activity detection, speaking indicators | Three people talk at once; with 5% simulated packet loss it is still understandable |
| 10 | Hardening | TLS, rate limits, reconnect, config files, graceful shutdown | Passwords are unreadable in Wireshark; server survives garbage packets |
| 11 | Stretch | Noise suppression, echo cancellation, voice encryption, Docker, CI | Pick any; none are required |

## Stage-by-stage guide

For each stage: what to build, the files you touch (full paths are in the file tree below), and how to test it.

### Stage 0 — Foundation

- Create the git repo, root `CMakeLists.txt`, `vcpkg.json`, and `CMakePresets.json` with a debug preset (AddressSanitizer on) and a release preset.
- Define the empty library targets (`vc_common`, `vc_protocol`, `vc_net`, `vc_audio`) and make `server` and `client` each print one log line.
- Test: `cmake --build` succeeds and `ctest` runs one dummy test.

### Stage 1 — Protocol (no sockets)

- Write a `FrameDecoder` class: you feed it raw bytes, it hands back complete messages. It must handle half a message and two messages arriving in one chunk.
- Write message structs with `to_json` and `from_json`, the `VoicePacket` serialize and parse functions (careful with byte order), and the pure `can()` permission function.
- Test: round trips, split and merged frames, oversized length rejected, every permission-table cell.

### Stage 2 — Networking

- First build `tools/tcp_echo` to learn `async_read` and `async_write`. Then replace echo with the frame decoder and answer `ping` with `pong`.
- A `Session` object owns one socket and lives through `shared_from_this`. Keep a write queue so only one `async_write` is in flight at a time.
- Test: three clients connected at once; kill one with Ctrl+C and watch the server log it and carry on.

### Stage 3 — Accounts

- Create `001_init.sql` with a `users` table (id, unique username, password hash, server role, created time) and a `UserRepo` that wraps SQLite.
- Hash with libsodium's `crypto_pwhash_str` (Argon2id). Never store or log a password. Return the same error for "no such user" and "wrong password".
- On login, generate a random session token and a random 32-bit voice token.
- Test with an in-memory SQLite database (`":memory:"`) so tests need no files.

### Stage 4 — Rooms, roles, text chat

- `RoomManager` keeps live membership in memory; room name and owner are saved in the database.
- A dispatcher maps each message `type` to a handler function. Every handler checks `can()` before acting.
- Build a throwaway command-line client (`/login`, `/join`, `/kick`) just to exercise the server before the TUI exists.
- Test: permission and room-manager unit tests, plus an integration test where three clients chat and one gets muted.

### Stage 5 — TUI client

- Build the FTXUI screen loop with screens for login, register, room list, and room view (chat log, member list with roles, input line, status bar).
- The network runs on its own thread and hands events to the UI thread; touch `AppState` only from the UI thread, or guard it with a mutex.
- Test: complete a full session by hand, including a kick and a reconnect.

### Stage 6 — Audio I/O

- Open a miniaudio capture and playback device at 48 kHz, mono. A 20 ms frame is 960 samples.
- The audio callback runs on a real-time thread: no locks, no memory allocation, no logging inside it. Pass samples through a lock-free single-producer single-consumer ring buffer.
- Test: `test_ring_buffer` (wraparound, full, empty), then `tools/audio_loopback` by ear.

### Stage 7 — Opus

- Wrap the encoder (VoIP mode, 48 kHz mono, about 24 kbps, 960 samples in, at most a few hundred bytes out) and the decoder. Note that `opus_decode` with no data produces concealment audio for a lost packet; you use that in Stage 9.
- Test: encode and decode a generated sine wave, then your own recorded voice, with `tools/opus_roundtrip`.

### Stage 8 — Voice over UDP

- Add `UdpSocket`, then `VoiceServer` (handles the token hello and records the sender's address) and `VoiceRouter` (forwards a packet to everyone else in the sender's room).
- On the client, an `AudioEngine` glues the pieces: mic, encode, send; receive, decode, playback. Play frames immediately for now; it will sound choppy, and that is expected.
- Test: `tools/fake_client` streams a sine wave; use `tools/packet_dump` to inspect headers; then two real clients on a LAN.

### Stage 9 — Voice quality

- Give each remote speaker their own `JitterBuffer`: frames are ordered by sequence number and played after a 40–60 ms delay; a missing frame triggers Opus concealment.
- `Mixer` sums all speakers with clipping protection. `Vad` is a simple energy threshold with a 200 ms hangover so words are not cut off. Add push-to-talk and a speaking indicator in the UI.
- Test: jitter buffer with reordered, duplicated and lost packets; make `fake_client` drop 5% and shuffle packets.

### Stage 10 — Hardening

- Add TLS to the TCP channel with a self-signed certificate (`scripts/gen_certs.sh`) and a client that trusts only that certificate.
- Add rate limits (login attempts per address, chat messages per second), a maximum message size, idle timeouts via ping, client reconnect with backoff, and clean shutdown on Ctrl+C.
- Test: send random bytes to both ports; the server must log and survive.

### Stage 11 — Stretch (pick any)

RNNoise noise suppression, SpeexDSP echo cancellation, encrypting voice packets with libsodium, a Dockerfile for the server, GitHub Actions CI, a stats panel (ping, loss, bitrate).

## Full file tree

The tree below holds every file the finished project needs; the tag on each line (S0–S11) is the stage that first fills it in. Create all the files at Stage 0 as stubs holding one comment line, such as `// S8: forwards voice packets to a room`, so the shape of the project is visible from day one. Add each file to CMake only when its stage arrives.

```text
voicechat/
├── CMakeLists.txt                  S0   root build
├── CMakePresets.json               S0   debug (sanitizers) and release
├── vcpkg.json                      S0   dependency list
├── README.md                       S0
├── .gitignore                      S0
├── .clang-format                   S0
├── .clang-tidy                     S0   optional
│
├── cmake/
│   ├── CompilerWarnings.cmake      S0
│   └── Sanitizers.cmake            S0
│
├── docs/
│   ├── architecture.md             S0   diagram and decisions
│   ├── protocol.md                 S1   messages and packet layout
│   ├── permissions.md              S1   role table
│   └── learning-log.md             S0   what you learned, bugs you hit
│
├── config/
│   ├── server.example.toml         S10
│   └── client.example.toml         S10
│
├── scripts/
│   ├── run_server.sh               S2
│   ├── run_client.sh               S5
│   └── gen_certs.sh                S10  self-signed TLS certificate
│
├── common/                         library vc_common
│   ├── CMakeLists.txt              S0
│   ├── include/vc/common/
│   │   ├── log.hpp                 S0   spdlog wrapper
│   │   ├── types.hpp               S1   UserId, RoomId aliases
│   │   ├── error.hpp               S1   error codes
│   │   ├── result.hpp              S1   Result<T> (std::expected)
│   │   ├── time.hpp                S2   clock helpers
│   │   ├── ring_buffer.hpp         S6   lock-free SPSC queue
│   │   └── config.hpp              S10
│   └── src/
│       ├── log.cpp                 S0
│       └── config.cpp              S10
│
├── protocol/                       library vc_protocol (no sockets)
│   ├── CMakeLists.txt              S0
│   ├── include/vc/protocol/
│   │   ├── limits.hpp              S1   max message and name sizes
│   │   ├── framing.hpp             S1   encode_frame, FrameDecoder
│   │   ├── messages.hpp            S1   message structs
│   │   ├── json_codec.hpp          S1   to_json / from_json
│   │   ├── roles.hpp               S1   Role and Action enums
│   │   ├── permissions.hpp         S1   can(actor, action, target)
│   │   └── voice_packet.hpp        S1   header build and parse
│   └── src/
│       ├── framing.cpp             S1
│       ├── json_codec.cpp          S1
│       ├── permissions.cpp         S1
│       └── voice_packet.cpp        S1
│
├── net/                            library vc_net (Asio helpers)
│   ├── CMakeLists.txt              S0
│   ├── include/vc/net/
│   │   ├── io_runner.hpp           S2   threads that run io_context
│   │   ├── tcp_connection.hpp      S2   framed read and write queue
│   │   ├── udp_socket.hpp          S8
│   │   └── tls_context.hpp         S10
│   └── src/
│       ├── io_runner.cpp           S2
│       ├── tcp_connection.cpp      S2
│       ├── udp_socket.cpp          S8
│       └── tls_context.cpp         S10
│
├── audio/                          library vc_audio (client only)
│   ├── CMakeLists.txt              S0
│   ├── include/vc/audio/
│   │   ├── audio_device.hpp        S6   miniaudio wrapper
│   │   ├── opus_encoder.hpp        S7
│   │   ├── opus_decoder.hpp        S7
│   │   ├── jitter_buffer.hpp       S9
│   │   ├── mixer.hpp               S9
│   │   ├── vad.hpp                 S9   voice activity detection
│   │   └── denoiser.hpp            S11  RNNoise wrapper
│   └── src/                        one .cpp per header above
│
├── server/
│   ├── CMakeLists.txt              S0
│   └── src/
│       ├── main.cpp                S0
│       ├── server.hpp/.cpp         S2   accept loop, owns services
│       ├── session.hpp/.cpp        S2   one connected client
│       ├── session_manager.hpp/.cpp  S3 who is logged in
│       ├── dispatcher.hpp/.cpp     S4   message type -> handler
│       ├── rate_limiter.hpp/.cpp   S10
│       ├── auth/
│       │   ├── password_hasher.hpp/.cpp  S3  libsodium Argon2id
│       │   ├── auth_service.hpp/.cpp     S3  register, login
│       │   └── token.hpp/.cpp            S3  session and voice tokens
│       ├── db/
│       │   ├── database.hpp/.cpp         S3  SQLite wrapper
│       │   ├── user_repo.hpp/.cpp        S3
│       │   ├── room_repo.hpp/.cpp        S4
│       │   └── migrations/
│       │       ├── 001_users.sql         S3
│       │       └── 002_rooms.sql         S4
│       ├── rooms/
│       │   ├── room.hpp/.cpp             S4
│       │   ├── room_manager.hpp/.cpp     S4
│       │   └── permission_service.hpp/.cpp  S4
│       ├── chat/
│       │   └── chat_service.hpp/.cpp     S4  text broadcast
│       ├── admin/
│       │   └── admin_commands.hpp/.cpp   S4  ban, delete any room
│       └── voice/
│           ├── voice_server.hpp/.cpp     S8  UDP hello and receive
│           └── voice_router.hpp/.cpp     S8  forward within a room
│
├── client/
│   ├── CMakeLists.txt              S0
│   └── src/
│       ├── main.cpp                S0
│       ├── app.hpp/.cpp            S5   top-level UI loop
│       ├── state/
│       │   └── app_state.hpp       S5   current user, room, messages
│       ├── net/
│       │   ├── control_client.hpp/.cpp   S2  TCP side (grows through S4)
│       │   └── voice_client.hpp/.cpp     S8  UDP side
│       ├── audio/
│       │   └── audio_engine.hpp/.cpp     S8  mic > encode > send, receive > decode > play
│       ├── commands/
│       │   └── command_parser.hpp/.cpp   S4  /join, /kick, /mute
│       └── ui/
│           ├── theme.hpp                 S5
│           ├── screens/
│           │   ├── login_screen.hpp/.cpp      S5
│           │   ├── register_screen.hpp/.cpp   S5
│           │   ├── room_list_screen.hpp/.cpp  S5
│           │   └── room_screen.hpp/.cpp       S5  (speaking marks S9)
│           └── components/
│               ├── chat_view.hpp/.cpp     S5
│               ├── member_list.hpp/.cpp   S5
│               ├── input_line.hpp/.cpp    S5
│               ├── status_bar.hpp/.cpp    S5
│               └── vu_meter.hpp/.cpp      S9
│
├── tools/                          small programs for testing one layer
│   ├── CMakeLists.txt              S2
│   ├── tcp_echo/main.cpp           S2
│   ├── cli_client/main.cpp         S4   throwaway text client
│   ├── audio_loopback/main.cpp     S6
│   ├── opus_roundtrip/main.cpp     S7
│   ├── packet_dump/main.cpp        S8
│   └── fake_client/main.cpp        S8   fake voice sender (loss added S9)
│
└── tests/
    ├── CMakeLists.txt              S0
    ├── common/
    │   └── test_ring_buffer.cpp    S6
    ├── protocol/
    │   ├── test_framing.cpp        S1
    │   ├── test_messages.cpp       S1
    │   ├── test_permissions.cpp    S1
    │   └── test_voice_packet.cpp   S1
    ├── server/
    │   ├── test_password_hasher.cpp  S3
    │   ├── test_user_repo.cpp        S3
    │   └── test_room_manager.cpp     S4
    ├── audio/
    │   ├── test_opus_roundtrip.cpp   S7
    │   ├── test_jitter_buffer.cpp    S9
    │   └── test_mixer.cpp            S9
    └── integration/
        ├── test_login_flow.cpp       S3
        └── test_room_chat.cpp        S4
```

**Dependency rule:** `common` ← `protocol` and `net` ← `server` and `client`. `audio` is linked only by the client. A lower layer must never include a header from a higher one; if you feel the urge, the code belongs one layer down. This rule is what keeps you from getting lost.

## What to learn, and when

Learn each topic just before the stage that needs it, not all at once. Before Stage 0, make sure the C++ basics below feel comfortable; if they do not, spend two to three weeks on them first.

**Prerequisites:** pointers and references, classes, RAII (resources cleaned up by destructors), `std::vector`, `std::map`, `std::string`, `std::unique_ptr` and `std::shared_ptr`, lambdas, and a working idea of what `std::move` does.

| Stage | Learn first | Why it matters here |
| --- | --- | --- |
| 0 | CMake targets, `target_link_libraries`, PUBLIC vs PRIVATE; compiling vs linking; AddressSanitizer | Your whole layered design is expressed as CMake targets |
| 1 | Bytes and endianness; `std::span`, `std::string_view`; `std::variant` with `std::visit`; unit testing | Messages are bytes on a wire, and a variant models "one of many message types" |
| 2 | TCP as a byte stream; the Asio model (`io_context`, `async_read`, completion handlers); `shared_from_this`; strands | Most network bugs here are lifetime bugs inside async callbacks |
| 3 | Basic SQL and prepared statements; why SQL injection happens; salting and Argon2id; wrapping C handles in `unique_ptr` with a custom deleter | Secure login and clean use of the SQLite and libsodium C APIs |
| 4 | Data races, `std::mutex` or strands; `std::function` dispatch tables; designing authorization checks | Many clients touch shared room state at once |
| 5 | Event loops; FTXUI components; passing events between threads | The UI thread and network thread must never touch the same data unguarded |
| 6 | Sample rate, frames, mono vs stereo; real-time rules; `std::atomic` and acquire/release ordering; ring buffers | The audio callback has a hard deadline and cannot wait on a lock |
| 7 | What a voice codec does; Opus frame sizes and bitrates; opaque-pointer C APIs; packet loss concealment | You are wrapping a C library that returns error codes |
| 8 | UDP vs TCP; packet loss; MTU (keep packets under about 1,200 bytes); Wireshark; NAT basics | Voice tolerates loss but not delay |
| 9 | Jitter and the latency-vs-smoothness trade-off; 16-bit sequence wraparound; mixing and clipping; simple VAD | This stage is what makes voice sound good instead of just working |
| 10 | TLS and certificates; token-bucket rate limiting; defensive parsing; signal handling; basic fuzzing | Anything on the internet receives hostile input |

**Good free resources**

- learncpp.com for C++ fundamentals, and cppreference.com as your daily reference
- Beej's Guide to Network Programming, for sockets and how TCP and UDP behave
- The official Asio tutorial examples (start with the timer and daytime examples)
- miniaudio's documentation examples: simple capture, simple playback, and duplex
- The FTXUI repository's examples folder; copy and modify one first
- Ross Bencina's article "Real-time audio programming 101: time waits for nothing", before Stage 6
- The official CMake tutorial, and the Opus and libsodium documentation sites

## Pitfalls and pace

Most of the pain in this project comes from a short list of mistakes. Knowing them in advance saves weeks.

**Pitfalls**

- **One read is not one message.** TCP delivers a stream, so always use the length prefix and the `FrameDecoder`.
- **Never block the audio thread.** No locks, no `new`, no logging inside the audio callback.
- **Validate before you trust.** Check every length field before allocating memory or indexing; the server must survive random bytes.
- **Async lifetime.** If a callback can run after an object is destroyed, keep it alive with `shared_from_this`.
- **Never log or store a plain-text password.**
- **Echo when testing.** Two clients on one computer will feed back through the speakers; use headphones.
- **Windows Subsystem for Linux (WSL).** Microphone access is unreliable there; run the client natively if audio will not open.
- **Test with generated sound before using the mic.** A sine wave tells you instantly whether a glitch is in your code or your room.

**Habits that keep you from getting lost**

- Commit at the end of every stage and tag it (`git tag stage-3`) so you can always return to a working state.
- When something breaks, reproduce it in a small program under `tools/` before debugging it inside the full app.
- Write a few lines in `docs/learning-log.md` after each stage: what worked, what confused you, what you would change.

**Rough pace (my estimate):** at 8–10 hours a week, plan about 4–5 months in total. Stages 0–1 take about two weeks, Stages 2–3 about three, Stage 4 two, Stage 5 two, Stages 6–7 about three, Stage 8 two, Stage 9 two to three, and Stage 10 two. Voice quality in Stage 9 is where learners most often need extra time, so do not rush it.
