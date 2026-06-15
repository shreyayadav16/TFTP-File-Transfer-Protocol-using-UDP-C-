# TFTP-File-Transfer-Protocol-using-UDP-C-
# TFTP File Transfer Protocol using UDP (C)

## Overview

This project implements the **Trivial File Transfer Protocol (TFTP)** using the **C programming language** and **UDP socket programming** on Linux.

The application consists of a **client** and a **server** that can transfer files over a network using TFTP packet formats such as **RRQ (Read Request), WRQ (Write Request), DATA, ACK, and ERROR** packets.

The project demonstrates concepts of **socket programming, networking, file handling, packet processing, and protocol implementation**.

---

# Features

* File upload (Client → Server)
* File download (Server → Client)
* UDP socket communication
* TFTP packet implementation
* ACK and ERROR packet handling
* Multiple transfer modes:

  * Default Mode
  * Octet Mode
  * Net-ASCII Mode
* Command-line interface
* Linux compatible

---

# Technologies Used

* C Programming
* Linux
* UDP Socket Programming
* File Handling
* POSIX System Calls
* Networking

---

# Folder Structure

```
TFTP/
│
├── tftp_client.c
├── tftp_client.h
├── tftp_server.c
├── tftp.c
├── tftp.h
├── Makefile
└── README.md
```

---

# Build

Compile the server:

```
gcc tftp_server.c tftp.c -o server
```

Compile the client:

```
gcc tftp_client.c tftp.c -o client
```

---

# Run

Start the server:

```
./server
```

Start the client:

```
./client
```

---

# Client Commands

Connect to server

```
connect 127.0.0.1
```

Download file

```
get filename.txt
```

Upload file

```
put filename.txt
```

Default mode

```
mode default
```

Octet mode

```
mode octet
```

Net ASCII mode

```
mode net-ascii
```

---

# TFTP Packet Types

* RRQ (Read Request)
* WRQ (Write Request)
* DATA Packet
* ACK Packet
* ERROR Packet

---

# Working

1. Client connects to the server.
2. Client sends RRQ or WRQ request.
3. Server validates the request.
4. File is transferred in DATA packets.
5. Receiver sends ACK for every DATA packet.
6. Transfer ends when the last packet contains less than 512 bytes.

---

# Concepts Covered

* UDP Communication
* Client–Server Architecture
* File Transfer Protocol
* Socket Programming
* Packet Structures
* Error Handling
* File I/O
* Network Programming

---

# Learning Outcomes

* Understanding of UDP sockets
* TFTP protocol implementation
* Reliable communication using ACK packets
* File handling in C
* Network application development
* Embedded Linux networking concepts

---

# Future Improvements

* Timeout and retransmission support
* Multi-client support
* Concurrent transfers
* Better error recovery
* IPv6 support
* Secure authentication
* Logging support

---

# Author

**Shreya Yadav**

Embedded Systems | Linux | C Programming | Networking
