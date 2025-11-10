# Sâm Lốc Game Server

A C++ implementation of the Sâm Lốc card game with SQLite database and network support.

## Prerequisites

- CMake (3.10 or higher)
- C++ compiler with C++17 support
- SQLite3 development libraries
- Boost libraries
- Docker (optional)

## Project Structure

```
samloc-socket-engine/
├── server/
│   ├── src/
│   │   ├── db/
│   │   ├── game/
│   │   └── network/
│   ├── migrations/
│   ├── CMakeLists.txt
│   └── Dockerfile
├── data/
└── docker-compose.yaml
```

## Building and Running

### Using Docker (Recommended)

1. Build and start the container:
```bash
docker-compose up --build
```

### Manual Build

1. Install dependencies:
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y cmake libsqlite3-dev libboost-all-dev

# Windows with vcpkg
vcpkg install sqlite3:x64-windows boost:x64-windows
```

2. Build the project:
```bash
cd server
mkdir build && cd build
cmake ..
make
```

3. Run the server:
```bash
./samloc_server
```

## Database

The project uses SQLite3 for data storage. The database file will be created at `data/samloc.db` when the server starts.

## Game Rules

Sâm Lốc is a Vietnamese card game played with a standard 52-card deck. The goal is to be the first player to get rid of all their cards.

Basic Rules:
1. Players are dealt 13 cards each
2. Game starts with player holding 3♠
3. Valid card combinations:
   - Single card
   - Pairs
   - Three of a kind
   - Straight (sequence of 3 or more cards)
   - Straight pairs (two or more consecutive pairs)

## License

MIT License