# TESTUDPMESSAGE - Tactical Track UDP Generator

A Python tactical track simulation tool that generates and transmits live binary UDP datagrams (**Message ID: 613**, `MAIN_LITE_TRACK_MSG`) to the GISLITEAPP receiver on `127.0.0.1:8540`.

---

## Features

- **5 Active Tactical Tracks**:
  - `VIPER-01` (ID: 101) - Air Hostile, 520 km/h, circular clockwise combat air patrol orbit.
  - `FALCON-02` (ID: 102) - Air Hostile, 480 km/h, diagonal linear patrol pattern.
  - `EAGLE-03` (ID: 103) - Air Hostile, 410 km/h, tactical figure-8 (lemniscate) surveillance orbit.
  - `HAWK-04` (ID: 104) - Air Hostile, 600 km/h, east-west high-altitude strike patrol.
  - `COBRA-05` (ID: 105) - Air Hostile, 310 km/h, low-altitude zigzag tactical sweep.
- **Dynamic Coordinate Updates**: Continuously updates latitude, longitude, altitude, heading, and kinematics in real-time.
- **Strict Wire Alignment**:
  - 15-byte packed `STRUCT_MESSAGE_HEADER` (`message_id = 613`).
  - 2-byte `no_of_tracks`.
  - 304-byte packed `STRUCT_TRACK_PAYLOAD` per track.
  - Batched transmission adhering to Ethernet MTU (`MAX_UDP_PACKET_SIZE = 1500` bytes).
- **Live Terminal Telemetry**: Formatted color dashboard displaying coordinates, altitude, heading, and packet metrics every tick.

---

## How to Run

### Quick Start:
```bash
./run_track_sender.sh
```
or
```bash
python3 track_sender.py
```

### Command Line Options:
```bash
python3 track_sender.py [options]

Options:
  --host HOST           Target IP address (default: 127.0.0.1)
  --port PORT           Target UDP port (default: 8540)
  --interval INTERVAL   Update transmission interval in seconds (default: 1.0)
  --speed-factor FACTOR Speed multiplier for simulation (e.g. 2.0 = 2x speed, default: 1.0)
  --count COUNT         Total updates to send (0 = infinite continuous loop, default: 0)
```

### Examples:
- **Fast 2x speed simulation with 0.5s updates**:
  ```bash
  python3 track_sender.py --interval 0.5 --speed-factor 2.0
  ```
- **Send 10 updates and exit**:
  ```bash
  python3 track_sender.py --count 10
  ```
- **Send to remote node**:
  ```bash
  python3 track_sender.py --host 192.168.1.50 --port 8540
  ```
