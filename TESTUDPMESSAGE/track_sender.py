#!/usr/bin/env python3
"""
=============================================================================
Tactical Track UDP Message Generator (Message ID: 613)
=============================================================================
Simulates 5 active tactical tracks dynamically updating their geographic
coordinates, height, direction, and spatial kinematics in real-time, and
transmits packed binary UDP datagrams to the GISLITEAPP UDP receiver (default 127.0.0.1:8540).

Canonical Wire Specification (WireStructures.h):
- Message ID: 613 (MAIN_LITE_TRACK_MSG)
- Header: STRUCT_MESSAGE_HEADER (15 bytes, packed)
- Payload: UINT_16 no_of_tracks (2 bytes) + N * STRUCT_TRACK (304 bytes each)
  * UINT_32 track_id
  * STRING_100 track_name
  * STRUCT_LOCATION track_loc (latatitude, longitude, height, dir)
  * IDENTITY track_identity (UINT_8: 1=HOSTILE, 2=FRIENDLY, 3=NEUTRAL, 4=UNKNOWN)
  * STRUCT_TRACK_ATTRIBUTES (7 bytes: type, sub_type, classification, strength, act_type, act_sub_type, act_classification)
  * SYSTEM_TRACK_TYPE sys_track_type (UINT_8: 1=SYSTEM 1, 2=SYSTEM 2)
  * UINT_8 no_of_sources
  * STRUCT_TRACK_SYMBOL track_symbol (STRING_50)
  * STRUCT_DATE_TIME track_report_time (8 bytes: STRUCT_DATE + STRUCT_TIME)
  * STRING_100 track_remarks
=============================================================================
"""

import socket
import struct
import time
import math
import argparse
import sys
import os
import signal
from datetime import datetime, timezone

# ANSI Terminal Colors
CLR_RESET   = "\033[0m"
CLR_BOLD    = "\033[1m"
CLR_RED     = "\033[31;1m"
CLR_GREEN   = "\033[32;1m"
CLR_YELLOW  = "\033[33;1m"
CLR_BLUE    = "\033[34;1m"
CLR_CYAN    = "\033[36;1m"
CLR_MAGENTA = "\033[35;1m"
CLR_DIM     = "\033[2m"

# Wire Protocol Constants
MESSAGE_ID_TRACK = 613
MAX_UDP_PACKET_SIZE = 1500

IDENTITY_NAMES = {1: "HOSTILE", 2: "FRIENDLY", 3: "NEUTRAL", 4: "UNKNOWN"}
IDENTITY_COLORS = {1: CLR_RED, 2: CLR_GREEN, 3: CLR_CYAN, 4: CLR_YELLOW}
DOMAIN_NAMES = {1: "AIR", 2: "SURFACE", 3: "SUBSURFACE", 4: "LAND"}


class TacticalTrackSimulator:
    """
    Simulates a tactical track with realistic spatial kinematics adhering strictly
    to the canonical STRUCT_TRACK wire specification.
    """
    def __init__(self, track_id, callsign, lat, lon, height, speed_kmh, dir_deg, pattern,
                 identity=1, track_attributes=None, sys_track_type=1,
                 symbol="TACTICAL_SYMBOL_AIR", remarks=""):
        self.track_id = track_id
        self.callsign = callsign
        self.lat = lat
        self.lon = lon
        self.height = height
        self.speed_kmh = speed_kmh  # Internal simulation parameter for spatial updates
        self.dir = dir_deg          # Direction / bearing in degrees [0..360)
        self.pattern = pattern
        self.identity = identity    # 1=HOSTILE, 2=FRIENDLY, 3=NEUTRAL, 4=UNKNOWN
        
        # STRUCT_TRACK_ATTRIBUTES: (type, sub_type, classification, strength, act_type, act_sub_type, act_classification)
        self.track_attributes = track_attributes if track_attributes is not None else (1, 1, 1, 1, 1, 1, 1)
        self.sys_track_type = sys_track_type  # 1=SYSTEM 1, 2=SYSTEM 2
        self.symbol = symbol
        self.remarks = remarks
        
        # Internal kinematic state
        self.initial_lat = lat
        self.initial_lon = lon
        self.elapsed_time = 0.0
        self.orbit_phase = (track_id * 72.0) % 360.0

    def update(self, dt):
        """
        Advances the track's spatial coordinates and kinematics based on its movement pattern.
        """
        self.elapsed_time += dt
        
        # 1 degree latitude ~ 111.32 km
        # 1 degree longitude at ~28.6 deg lat ~ 97.74 km
        deg_lat_km = 111.32
        deg_lon_km = 97.74
        
        distance_km = (self.speed_kmh / 3600.0) * dt

        if self.pattern == "orbit_cw":
            # Circular clockwise orbit around anchor point
            orbit_radius_km = 14.0
            angular_velocity = (self.speed_kmh / orbit_radius_km) * (180.0 / math.pi) / 3600.0
            self.orbit_phase = (self.orbit_phase + angular_velocity * dt) % 360.0
            rad = math.radians(self.orbit_phase)
            self.lat = self.initial_lat + (orbit_radius_km * math.cos(rad)) / deg_lat_km
            self.lon = self.initial_lon + (orbit_radius_km * math.sin(rad)) / deg_lon_km
            self.dir = (self.orbit_phase + 90.0) % 360.0
            self.height = 8500.0 + 300.0 * math.sin(rad * 2.0)

        elif self.pattern == "orbit_ccw":
            # Circular counter-clockwise orbit
            orbit_radius_km = 16.0
            angular_velocity = (self.speed_kmh / orbit_radius_km) * (180.0 / math.pi) / 3600.0
            self.orbit_phase = (self.orbit_phase - angular_velocity * dt) % 360.0
            rad = math.radians(self.orbit_phase)
            self.lat = self.initial_lat + (orbit_radius_km * math.cos(rad)) / deg_lat_km
            self.lon = self.initial_lon + (orbit_radius_km * math.sin(rad)) / deg_lon_km
            self.dir = (self.orbit_phase - 90.0) % 360.0
            self.height = 10200.0 + 400.0 * math.cos(rad)

        elif self.pattern == "figure_eight":
            # Figure-8 (Lemniscate) surveillance orbit
            scale_km = 18.0
            period_s = 240.0
            t = (self.elapsed_time % period_s) / period_s * 2.0 * math.pi
            dx_km = scale_km * math.sin(t)
            dy_km = scale_km * math.sin(t) * math.cos(t)
            
            # Kinematic derivative for direction
            dt_step = 0.05
            t_next = t + dt_step
            dx_next = scale_km * math.sin(t_next)
            dy_next = scale_km * math.sin(t_next) * math.cos(t_next)
            self.dir = (math.degrees(math.atan2(dx_next - dx_km, dy_next - dy_km)) + 360.0) % 360.0
            
            self.lat = self.initial_lat + dy_km / deg_lat_km
            self.lon = self.initial_lon + dx_km / deg_lon_km
            self.height = 6400.0 + 200.0 * math.sin(t * 3.0)

        elif self.pattern == "patrol_linear":
            # Linear back-and-forth patrol with turnarounds
            rad = math.radians(self.dir)
            dlat = (distance_km * math.cos(rad)) / deg_lat_km
            dlon = (distance_km * math.sin(rad)) / deg_lon_km
            self.lat += dlat
            self.lon += dlon
            
            # Boundary checks: reverse direction if too far from base point
            dist_from_origin = math.sqrt(((self.lat - self.initial_lat) * deg_lat_km) ** 2 +
                                         ((self.lon - self.initial_lon) * deg_lon_km) ** 2)
            if dist_from_origin > 25.0:
                self.dir = (self.dir + 180.0) % 360.0
            self.height = 11500.0 + 100.0 * math.sin(self.elapsed_time * 0.1)

        elif self.pattern == "sweep_zigzag":
            # Tactical zig-zag border sweep
            rad = math.radians(self.dir)
            dlat = (distance_km * math.cos(rad)) / deg_lat_km
            dlon = (distance_km * math.sin(rad)) / deg_lon_km
            self.lat += dlat
            self.lon += dlon
            
            # Oscillate direction slightly for a tactical weaving path
            self.dir = (self.dir + 4.0 * math.sin(self.elapsed_time * 0.2)) % 360.0
            dist_from_origin = math.sqrt(((self.lat - self.initial_lat) * deg_lat_km) ** 2 +
                                         ((self.lon - self.initial_lon) * deg_lon_km) ** 2)
            if dist_from_origin > 30.0:
                self.dir = (self.dir + 180.0) % 360.0
            self.height = 3200.0 + 150.0 * math.cos(self.elapsed_time * 0.15)

    def pack_payload(self):
        """
        Serializes the track into 304-byte packed STRUCT_TRACK binary format.
        Strictly conforms to WireStructures.h:
          - UINT_32 track_id                  (4 bytes)
          - STRING_100 track_name             (100 bytes)
          - STRUCT_LOCATION track_loc         (32 bytes: latatitude, longitude, height, dir)
          - IDENTITY track_identity           (1 byte: UINT_8)
          - STRUCT_TRACK_ATTRIBUTES attributes (7 bytes: type, sub_type, classification, strength, act_type, act_sub_type, act_classification)
          - SYSTEM_TRACK_TYPE sys_track_type  (1 byte: UINT_8)
          - UINT_8 no_of_sources              (1 byte)
          - STRUCT_TRACK_SYMBOL track_symbol  (50 bytes: STRING_50)
          - STRUCT_DATE_TIME track_report_time(8 bytes: STRUCT_DATE + STRUCT_TIME)
          - STRING_100 track_remarks          (100 bytes)
          Total: 4 + 100 + 32 + 1 + 7 + 1 + 1 + 50 + 8 + 100 = 304 bytes.
        """
        # 1. track_id (UINT_32: 4 bytes)
        b_id = struct.pack("<I", self.track_id)

        # 2. track_name (STRING_100: 100 bytes, UTF-8 zero padded)
        b_name = self.callsign.encode('utf-8')[:99].ljust(100, b'\x00')

        # 3. track_loc (STRUCT_LOCATION: 4 doubles = 32 bytes)
        # latatitude, longitude, height, dir
        b_loc = struct.pack("<dddd", self.lat, self.lon, self.height, self.dir)

        # 4. track_identity (UINT_8: 1 byte)
        b_ident = struct.pack("<B", self.identity)

        # 5. track_attributes (STRUCT_TRACK_ATTRIBUTES: 7 bytes)
        t_type, s_type, cls, strength, act_t, act_sub, act_cls = self.track_attributes
        b_attr = struct.pack("<BBBBBBB", t_type, s_type, cls, strength, act_t, act_sub, act_cls)

        # 6. sys_track_type (UINT_8: 1 byte)
        b_sys = struct.pack("<B", self.sys_track_type)

        # 7. no_of_sources (UINT_8: 1 byte, 0 dynamic sources present in 304-byte fixed record)
        b_sources = struct.pack("<B", 0)

        # 8. track_symbol (STRUCT_TRACK_SYMBOL: STRING_50 = 50 bytes)
        b_sym = self.symbol.encode('utf-8')[:49].ljust(50, b'\x00')

        # 9. track_report_time (STRUCT_DATE_TIME: 8 bytes)
        now = datetime.now(timezone.utc)
        b_date = struct.pack("<BBH", now.day, now.month, now.year)
        b_time = struct.pack("<BBH", now.hour, now.minute, now.second)
        b_datetime = b_date + b_time

        # 10. track_remarks (STRING_100: 100 bytes)
        b_remarks = self.remarks.encode('utf-8')[:99].ljust(100, b'\x00')

        raw_track = b_id + b_name + b_loc + b_ident + b_attr + b_sys + b_sources + b_sym + b_datetime + b_remarks
        assert len(raw_track) == 304, f"Track size expected 304 bytes, got {len(raw_track)}"
        return raw_track


def build_message_header(source_id, dest_id, message_id, message_len, seq_no=1, total_packets=1):
    """
    Constructs the 15-byte packed STRUCT_MESSAGE_HEADER.
    """
    precedence = 0
    ws_index = 0
    sucomt_index = 0
    header = struct.pack("<HHHHBBBHH", source_id, dest_id, message_id, message_len,
                         precedence, ws_index, sucomt_index, seq_no, total_packets)
    assert len(header) == 15, f"Header size expected 15 bytes, got {len(header)}"
    return header


def create_track_datagram(tracks_batch, source_id=1, dest_id=2):
    """
    Constructs a complete UDP datagram containing a batch of tracks.
    """
    no_of_tracks = len(tracks_batch)
    tracks_payload = b"".join(t.pack_payload() for t in tracks_batch)
    body = struct.pack("<H", no_of_tracks) + tracks_payload
    header = build_message_header(source_id, dest_id, MESSAGE_ID_TRACK, len(body), seq_no=1, total_packets=1)
    return header + body


def create_default_5_tracks(speed_factor=1.0):
    """
    Creates and returns the 5 tactical track simulators configured for New Delhi operational airspace.
    Adheres strictly to canonical wire attributes and identities.
    """
    return [
        TacticalTrackSimulator(
            track_id=101,
            callsign="VIPER-01",
            lat=28.6139,
            lon=77.2090,
            height=8500.0,
            speed_kmh=520.0 * speed_factor,
            dir_deg=45.0,
            pattern="orbit_cw",
            identity=1, # HOSTILE
            track_attributes=(1, 1, 1, 1, 1, 1, 1), # AIR, FIXED_WING, CLASS_1, STR=1, PATROL
            sys_track_type=1, # SYSTEM 1
            symbol="AIR_HOSTILE_FIGHTER",
            remarks="Combat Air Patrol Sector Alpha"
        ),
        TacticalTrackSimulator(
            track_id=102,
            callsign="FALCON-02",
            lat=28.7500,
            lon=77.3500,
            height=9800.0,
            speed_kmh=480.0 * speed_factor,
            dir_deg=225.0,
            pattern="patrol_linear",
            identity=2, # FRIENDLY
            track_attributes=(1, 2, 2, 2, 2, 1, 1), # AIR, ROTARY_WING, CLASS_2, STR=2, ESCORT
            sys_track_type=2, # SYSTEM 2 / FUSED
            symbol="AIR_FRIENDLY_HELO",
            remarks="Friendly Escort Flight"
        ),
        TacticalTrackSimulator(
            track_id=103,
            callsign="EAGLE-03",
            lat=28.5200,
            lon=77.1000,
            height=6400.0,
            speed_kmh=410.0 * speed_factor,
            dir_deg=135.0,
            pattern="figure_eight",
            identity=3, # NEUTRAL
            track_attributes=(2, 1, 1, 1, 1, 1, 1), # SURFACE, SUBTYPE 1, CLASS 1, STR=1, SURVEILLANCE
            sys_track_type=1, # SYSTEM 1
            symbol="SURFACE_NEUTRAL_VESSEL",
            remarks="Commercial Transit Corridor"
        ),
        TacticalTrackSimulator(
            track_id=104,
            callsign="HAWK-04",
            lat=28.4500,
            lon=77.3000,
            height=11500.0,
            speed_kmh=600.0 * speed_factor,
            dir_deg=315.0,
            pattern="patrol_linear",
            identity=4, # UNKNOWN
            track_attributes=(4, 1, 1, 3, 1, 1, 1), # LAND, ARMORED, CLASS 1, STR=3, RECON
            sys_track_type=1, # SYSTEM 1
            symbol="LAND_UNKNOWN_UNIT",
            remarks="Unidentified Convoy Formation"
        ),
        TacticalTrackSimulator(
            track_id=105,
            callsign="COBRA-05",
            lat=28.7200,
            lon=77.0500,
            height=3200.0,
            speed_kmh=310.0 * speed_factor,
            dir_deg=180.0,
            pattern="sweep_zigzag",
            identity=1, # HOSTILE
            track_attributes=(1, 1, 3, 1, 3, 2, 1), # AIR, FIXED_WING, CLASS 3, STR=1, RECON
            sys_track_type=1, # SYSTEM 1
            symbol="AIR_HOSTILE_UAV",
            remarks="Low-Altitude Ingress Detected"
        )
    ]


def main():
    parser = argparse.ArgumentParser(
        description="Simulate and transmit 5 moving tactical tracks over UDP (Message ID: 613) to GISLITEAPP."
    )
    parser.add_argument("--host", type=str, default="127.0.0.1", help="Target IP address (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=8540, help="Target UDP port (default: 8540)")
    parser.add_argument("--interval", type=float, default=1.0, help="Update transmission interval in seconds (default: 1.0)")
    parser.add_argument("--speed-factor", type=float, default=1.0, help="Speed multiplier for simulation (default: 1.0)")
    parser.add_argument("--count", type=int, default=0, help="Total updates to send (0 = infinite continuous loop, default: 0)")
    args = parser.parse_args()

    tracks = create_default_5_tracks(args.speed_factor)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    target_addr = (args.host, args.port)

    # Clean signal handling for Ctrl+C
    running = True
    def handle_sigint(sig, frame):
        nonlocal running
        print(f"\n{CLR_YELLOW}[!] Received interrupt signal. Stopping track generator...{CLR_RESET}")
        running = False
    signal.signal(signal.SIGINT, handle_sigint)

    print(f"{CLR_BOLD}{CLR_CYAN}========================================================================================================{CLR_RESET}")
    print(f"{CLR_BOLD}{CLR_GREEN}   GISLITE Tactical Track Generator (Message ID: 613 - STRUCT_TRACK Canonical Wire)                     {CLR_RESET}")
    print(f"{CLR_BOLD}{CLR_CYAN}========================================================================================================{CLR_RESET}")
    print(f" Target Endpoint    : {CLR_BOLD}{args.host}:{args.port}{CLR_RESET}")
    print(f" Update Interval    : {CLR_BOLD}{args.interval}s{CLR_RESET} | Speed Multiplier: {CLR_BOLD}{args.speed_factor}x{CLR_RESET}")
    print(f" Total Active Tracks: {CLR_BOLD}{len(tracks)}{CLR_RESET} (Multi-Identity: Hostile, Friendly, Neutral, Unknown)")
    print(f"{CLR_DIM} Press Ctrl+C to stop.{CLR_RESET}\n")

    iteration = 0
    last_time = time.time()

    try:
        while running:
            iteration += 1
            now_time = time.time()
            dt = now_time - last_time
            if dt <= 0: dt = args.interval
            last_time = now_time

            # 1. Update all tracks kinematics
            for t in tracks:
                t.update(dt)

            # 2. Transmit tracks to GISLITEAPP:
            # Batch cleanly to ensure adherence to Ethernet MTU (MAX_UDP_PACKET_SIZE = 1500 bytes)
            batch1 = tracks[0:3] # Tracks 101, 102, 103 (929 bytes)
            batch2 = tracks[3:5] # Tracks 104, 105 (625 bytes)

            dgram1 = create_track_datagram(batch1)
            dgram2 = create_track_datagram(batch2)

            sock.sendto(dgram1, target_addr)
            sock.sendto(dgram2, target_addr)

            # 3. Print live telemetry dashboard matching canonical wire properties
            timestamp_str = datetime.now().strftime("%H:%M:%S")
            print(f"{CLR_BOLD}[{timestamp_str}] Sent Update #{iteration} | 5 Tracks Dispatched (Datagrams: {len(dgram1)}+{len(dgram2)} bytes){CLR_RESET}")
            print(f"{CLR_DIM}--------------------------------------------------------------------------------------------------------{CLR_RESET}")
            print(f" {CLR_BOLD}{'ID':<6} {'Callsign':<11} {'Identity':<10} {'Domain':<8} {'Latitude':<12} {'Longitude':<12} {'Height':<9} {'Dir':<7} {'Remarks'}{CLR_RESET}")
            print(f"{CLR_DIM}--------------------------------------------------------------------------------------------------------{CLR_RESET}")
            for t in tracks:
                ident_str = IDENTITY_NAMES.get(t.identity, "UNKNOWN")
                ident_clr = IDENTITY_COLORS.get(t.identity, CLR_RESET)
                domain_str = DOMAIN_NAMES.get(t.track_attributes[0], "UNKNOWN")
                print(f" {ident_clr}{t.track_id:<6}{CLR_RESET} {t.callsign:<11} {ident_clr}{ident_str:<10}{CLR_RESET} {domain_str:<8} {t.lat:10.5f}°  {t.lon:10.5f}°  {int(t.height):<6}m   {int(t.dir):>3}°   {t.remarks}")
            print(f"{CLR_DIM}--------------------------------------------------------------------------------------------------------{CLR_RESET}\n")

            if args.count > 0 and iteration >= args.count:
                print(f"{CLR_GREEN}[✓] Completed {args.count} requested iterations. Exiting.{CLR_RESET}")
                break

            time.sleep(args.interval)

    except KeyboardInterrupt:
        pass
    finally:
        sock.close()
        print(f"{CLR_GREEN}[✓] Socket closed. Dispatched {iteration} updates for 5 tracks successfully.{CLR_RESET}")


if __name__ == "__main__":
    main()
