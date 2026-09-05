#!/usr/bin/env python3
"""
=============================================================================
Tactical Track UDP Message Generator (Message ID: 613)
=============================================================================
Simulates 5 active tactical tracks dynamically updating their geographic
coordinates, altitude, heading, and kinematics in real-time, and transmits
packed binary UDP datagrams to the GISLITEAPP UDP receiver (default 127.0.0.1:8540).

Wire Specification:
- Message ID: 613 (MAIN_LITE_TRACK_MSG)
- Header: STRUCT_MESSAGE_HEADER (15 bytes, packed)
- Payload: UINT_16 no_of_tracks + N * STRUCT_TRACK_PAYLOAD (304 bytes each)
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

class TacticalTrackSimulator:
    """
    Simulates a tactical airborne or surface track with realistic kinematic movement.
    """
    def __init__(self, track_id, callsign, lat, lon, alt, speed_kmh, heading, pattern, identity=1):
        self.track_id = track_id
        self.callsign = callsign
        self.lat = lat
        self.lon = lon
        self.alt = alt
        self.speed_kmh = speed_kmh
        self.heading = heading
        self.pattern = pattern
        self.identity = identity  # 1 = Hostile (Red Dot), 2 = Friendly, 3 = Neutral
        
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
            self.heading = (self.orbit_phase + 90.0) % 360.0
            self.alt = 8500.0 + 300.0 * math.sin(rad * 2.0)

        elif self.pattern == "orbit_ccw":
            # Circular counter-clockwise orbit
            orbit_radius_km = 16.0
            angular_velocity = (self.speed_kmh / orbit_radius_km) * (180.0 / math.pi) / 3600.0
            self.orbit_phase = (self.orbit_phase - angular_velocity * dt) % 360.0
            rad = math.radians(self.orbit_phase)
            self.lat = self.initial_lat + (orbit_radius_km * math.cos(rad)) / deg_lat_km
            self.lon = self.initial_lon + (orbit_radius_km * math.sin(rad)) / deg_lon_km
            self.heading = (self.orbit_phase - 90.0) % 360.0
            self.alt = 10200.0 + 400.0 * math.cos(rad)

        elif self.pattern == "figure_eight":
            # Figure-8 (Lemniscate) surveillance orbit
            scale_km = 18.0
            period_s = 240.0
            t = (self.elapsed_time % period_s) / period_s * 2.0 * math.pi
            dx_km = scale_km * math.sin(t)
            dy_km = scale_km * math.sin(t) * math.cos(t)
            
            # Kinematic derivative for heading
            dt_step = 0.05
            t_next = t + dt_step
            dx_next = scale_km * math.sin(t_next)
            dy_next = scale_km * math.sin(t_next) * math.cos(t_next)
            self.heading = (math.degrees(math.atan2(dx_next - dx_km, dy_next - dy_km)) + 360.0) % 360.0
            
            self.lat = self.initial_lat + dy_km / deg_lat_km
            self.lon = self.initial_lon + dx_km / deg_lon_km
            self.alt = 6400.0 + 200.0 * math.sin(t * 3.0)

        elif self.pattern == "patrol_linear":
            # Linear back-and-forth patrol with turnarounds
            rad = math.radians(self.heading)
            dlat = (distance_km * math.cos(rad)) / deg_lat_km
            dlon = (distance_km * math.sin(rad)) / deg_lon_km
            self.lat += dlat
            self.lon += dlon
            
            # Boundary checks: reverse heading if too far from base point
            dist_from_origin = math.sqrt(((self.lat - self.initial_lat) * deg_lat_km) ** 2 +
                                         ((self.lon - self.initial_lon) * deg_lon_km) ** 2)
            if dist_from_origin > 25.0:
                self.heading = (self.heading + 180.0) % 360.0
            self.alt = 11500.0 + 100.0 * math.sin(self.elapsed_time * 0.1)

        elif self.pattern == "sweep_zigzag":
            # Tactical zig-zag border sweep
            rad = math.radians(self.heading)
            dlat = (distance_km * math.cos(rad)) / deg_lat_km
            dlon = (distance_km * math.sin(rad)) / deg_lon_km
            self.lat += dlat
            self.lon += dlon
            
            # Oscillate heading slightly for a tactical weaving path
            self.heading = (self.heading + 4.0 * math.sin(self.elapsed_time * 0.2)) % 360.0
            dist_from_origin = math.sqrt(((self.lat - self.initial_lat) * deg_lat_km) ** 2 +
                                         ((self.lon - self.initial_lon) * deg_lon_km) ** 2)
            if dist_from_origin > 30.0:
                self.heading = (self.heading + 180.0) % 360.0
            self.alt = 3200.0 + 150.0 * math.cos(self.elapsed_time * 0.15)

    def pack_payload(self):
        """
        Serializes the track into 304-byte packed STRUCT_TRACK_PAYLOAD binary format.
        """
        # 1. track_id (UINT_32: 4 bytes)
        b_id = struct.pack("<I", self.track_id)

        # 2. track_name (STRING_100: 100 bytes, UTF-8 zero padded)
        b_name = self.callsign.encode('utf-8')[:99].ljust(100, b'\x00')

        # 3. track_loc (STRUCT_LOCATION: 4 doubles = 32 bytes)
        # latatitude, longitude, height, dir
        b_loc = struct.pack("<dddd", self.lat, self.lon, self.alt, self.heading)

        # 4. track_identity (UINT_8: 1 byte)
        b_ident = struct.pack("<B", self.identity)

        # 5. track_attributes (STRUCT_TRACK_ATTRIBUTES: 7 bytes)
        # type=1 (Air), sub_type=0, classification=0, strength=1, act_type=0, act_sub=0, act_c=0
        b_attr = struct.pack("<BBBBBBB", 1, 0, 0, 1, 0, 0, 0)

        # 6. sys_track_type (UINT_8: 1 byte)
        b_sys = struct.pack("<B", 1)

        # 7. no_of_sources (UINT_8: 1 byte)
        b_sources = struct.pack("<B", 1)

        # 8. track_symbol (STRUCT_TRACK_SYMBOL: STRING_50 = 50 bytes)
        b_sym = b"TACTICAL_SYMBOL_AIR".ljust(50, b'\x00')

        # 9. track_report_time (STRUCT_DATE_TIME: 8 bytes)
        now = datetime.now(timezone.utc)
        b_date = struct.pack("<BBH", now.day, now.month, now.year)
        b_time = struct.pack("<BBH", now.hour, now.minute, now.second)
        b_datetime = b_date + b_time

        # 10. track_remarks (STRING_100: 100 bytes)
        remarks = f"Speed: {int(self.speed_kmh)} km/h | Mode: {self.pattern}"
        b_remarks = remarks.encode('utf-8')[:99].ljust(100, b'\x00')

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
    """
    return [
        TacticalTrackSimulator(101, "VIPER-01",  28.6139, 77.2090, 8500.0, 520.0 * speed_factor,  45.0, "orbit_cw",       identity=1),
        TacticalTrackSimulator(102, "FALCON-02", 28.7500, 77.3500, 9800.0, 480.0 * speed_factor, 225.0, "patrol_linear",  identity=1),
        TacticalTrackSimulator(103, "EAGLE-03",  28.5200, 77.1000, 6400.0, 410.0 * speed_factor, 135.0, "figure_eight",   identity=1),
        TacticalTrackSimulator(104, "HAWK-04",   28.4500, 77.3000, 11500.0, 600.0 * speed_factor, 315.0, "patrol_linear",  identity=1),
        TacticalTrackSimulator(105, "COBRA-05",  28.7200, 77.0500, 3200.0, 310.0 * speed_factor, 180.0, "sweep_zigzag",   identity=1)
    ]


def main():
    parser = argparse.ArgumentParser(
        description="Simulate and transmit 5 moving tactical tracks over UDP (Message ID: 613) to GISLITEAPP."
    )
    parser.add_argument("--host", type=str, default="127.0.0.1", help="Target IP address (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=8540, help="Target UDP port (default: 8540)")
    parser.add_argument("--interval", type=float, default=10.0, help="Update transmission interval in seconds (default: 10.0)")
    parser.add_argument("--speed-factor", type=float, default=1.0, help="Speed multiplier for simulation (default: 1.0)")
    parser.add_argument("--count", type=int, default=0, help="Total updates to send (0 = infinite continuous loop, default: 0)")
    args = parser.parse_args()

    # Define exactly 5 active tactical tracks with distinct flight paths around New Delhi
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

    print(f"{CLR_BOLD}{CLR_CYAN}============================================================================={CLR_RESET}")
    print(f"{CLR_BOLD}{CLR_GREEN}   GISLITE Tactical Track Generator (Message ID: 613)                      {CLR_RESET}")
    print(f"{CLR_BOLD}{CLR_CYAN}============================================================================={CLR_RESET}")
    print(f" Target Endpoint    : {CLR_BOLD}{args.host}:{args.port}{CLR_RESET}")
    print(f" Update Interval    : {CLR_BOLD}{args.interval}s{CLR_RESET} | Speed Factor: {CLR_BOLD}{args.speed_factor}x{CLR_RESET}")
    print(f" Total Active Tracks: {CLR_BOLD}{len(tracks)}{CLR_RESET} (All configured as Hostile Red Dots)")
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

            # 1. Update all 5 tracks kinematics
            for t in tracks:
                t.update(dt)

            # 2. Transmit tracks to GISLITEAPP:
            # To ensure strict compliance with Ethernet MTU (MAX_UDP_PACKET_SIZE = 1500 bytes),
            # we send the 5 tracks in two cleanly partitioned batches (3 tracks + 2 tracks),
            # both arriving within milliseconds and updating the repository simultaneously.
            batch1 = tracks[0:3] # Tracks 101, 102, 103
            batch2 = tracks[3:5] # Tracks 104, 105

            dgram1 = create_track_datagram(batch1)
            dgram2 = create_track_datagram(batch2)

            sock.sendto(dgram1, target_addr)
            sock.sendto(dgram2, target_addr)

            # 3. Print live telemetry dashboard
            timestamp_str = datetime.now().strftime("%H:%M:%S")
            print(f"{CLR_BOLD}[{timestamp_str}] Sent Update #{iteration} | Total 5 Tracks Dispatched (Pkts: {len(dgram1)}+{len(dgram2)} bytes){CLR_RESET}")
            print(f"{CLR_DIM}-----------------------------------------------------------------------------{CLR_RESET}")
            print(f" {CLR_BOLD}{'ID':<6} {'Callsign':<11} {'Latitude':<12} {'Longitude':<12} {'Altitude':<10} {'Heading':<9} {'Speed':<10}{CLR_RESET}")
            print(f"{CLR_DIM}-----------------------------------------------------------------------------{CLR_RESET}")
            for t in tracks:
                print(f" {CLR_RED}{t.track_id:<6}{CLR_RESET} {t.callsign:<11} {t.lat:10.5f}°  {t.lon:10.5f}°  {int(t.alt):<6}m  {int(t.heading):>3}°    {int(t.speed_kmh):<4} km/h")
            print(f"{CLR_DIM}-----------------------------------------------------------------------------{CLR_RESET}\n")

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
