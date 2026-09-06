# TESTUDPMESSAGE - Tactical Track UDP Generator

A Python tactical track simulation tool that generates and transmits live binary UDP datagrams (**Message ID: 613**, `MAIN_LITE_TRACK_MSG`) to the GISLITEAPP receiver on `127.0.0.1:8540`.

---

## Features

- **5 Active Tactical Tracks with Canonical Wire Attributes**:
  - `VIPER-01` (ID: 101) - **Hostile Air** (`AIR 1 Fixed Wing`, `PATROL`, Circular CW Orbit, 8500m).
  - `FALCON-02` (ID: 102) - **Friendly Air** (`AIR 2 Rotary Wing`, `ESCORT`, Linear Patrol, 9800m).
  - `EAGLE-03` (ID: 103) - **Neutral Surface** (`SURFACE 1`, `SURVEILLANCE`, Figure-8 Orbit, 6400m).
  - `HAWK-04` (ID: 104) - **Unknown Land** (`LAND 1 Armored`, Strength 3, Linear Patrol, 11500m).
  - `COBRA-05` (ID: 105) - **Hostile Air** (`AIR 1 Fixed Wing`, `RECON`, Low-Altitude Zigzag, 3200m).
- **Real-Time Spatial Updates**: Continuously updates `latatitude`, `longitude`, `height`, and direction (`dir`) in real time.
- **Strict Wire Alignment with `WireStructures.h`**:
  - 15-byte packed `STRUCT_MESSAGE_HEADER` (`message_id = 613`).
  - 2-byte `no_of_tracks`.
  - 304-byte packed `STRUCT_TRACK` per track (`track_id`, `track_name`, `track_loc`, `track_identity`, `track_attributes`, `sys_track_type`, `no_of_sources`, `track_symbol`, `track_report_time`, `track_remarks`).
  - Clean datagram batching complying with Ethernet MTU (`MAX_UDP_PACKET_SIZE = 1500` bytes).
- **Live Terminal Telemetry**: Formatted dashboard displaying IDs, callsigns, identities, domain types, coordinates, height, direction, and remarks.

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
