# Samloc Socket Engine - Client Setup

## Linux Dependencies (Ubuntu/Debian)

Before building the client, install SFML dependencies:

```bash
sudo apt-get update
sudo apt-get install -y libudev-dev libx11-dev libxrandr-dev libxcursor-dev libxi-dev libgl1-mesa-dev libfreetype6-dev libopenal-dev libvorbis-dev libflac-dev
```

## Build Instructions

```bash
# From project root:
cmake -B build -S .
cmake --build build --target samloc_client -j$(nproc)
```

## Running the Client

```bash
./build/client/samloc_client
```

## Font Path

The client tries to load `/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf`.  
If this font is not available, install it:

```bash
sudo apt-get install -y fonts-dejavu
```

Or edit `client/main.cpp` to use a different font path.

## Architecture

- **Main Thread**: SFML UI event loop and rendering
- **Network Thread**: Background socket I/O, pushes events to ThreadSafeQueue
- **State Machine**: LoginState → LobbyState → InGameState
- **Challenge Notifications**: Overlay UI in LobbyState with Accept/Reject buttons
