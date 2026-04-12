#!/usr/bin/env python3

# This tool was created with the assistance of Claude AI and ChatGPT
# Claude wanted to add very complex processing that did not fit the scope of our project
# I asked ChatGPT to simplify the script greatly.
# I added the user interaction to select which track to parse and converted to csv as opposed to json

import mido
import sys
import argparse
import os
import csv

# Settings - configure for track
CHANNEL = None
LANE_NOTES = [60, 62, 64, 67]
MIN_VELOCITY = 20


def extract_notes(mid, track_num):
    track = mid.tracks[track_num]

    abs_time = 0
    active = {}
    notes = []

    for msg in track:
        abs_time += msg.time

        if hasattr(msg, "channel") and CHANNEL is not None:
            if msg.channel != CHANNEL:
                continue

        if msg.type == "note_on" and msg.velocity >= MIN_VELOCITY:
            active[msg.note] = abs_time

        elif msg.type in ["note_off", "note_on"]:
            if msg.note in active:
                start = active.pop(msg.note)
                duration = max(abs_time - start, 30)
                notes.append((start, msg.note, duration))

    return notes


def ticks_to_ms(mid, ticks):
    tempo = 500000
    return (ticks * tempo) / (mid.ticks_per_beat * 1000)


def map_to_lane(note):
    if note in LANE_NOTES:
        return LANE_NOTES.index(note)
    return None


def build_tiles(mid, notes):
    tiles = []

    for start, note, dur in notes:
        lane = map_to_lane(note)
        if lane is None:
            continue

        tiles.append({
            "lane": lane,
            "time_ms": round(ticks_to_ms(mid, start), 1),
            "duration_ms": round(ticks_to_ms(mid, dur), 1)
        })

    return sorted(tiles, key=lambda x: x["time_ms"])


def main():
    p = argparse.ArgumentParser()
    p.add_argument("midi")
    p.add_argument("--output", default=None)
    args = p.parse_args()

    if not os.path.exists(args.midi):
        print(f"ERROR: File not found: {args.midi}")
        sys.exit(1)

    print(f"[INFO] MIDI: {args.midi}")
    mid = mido.MidiFile(args.midi)

    print(f"[INFO] type={mid.type}  tpb={mid.ticks_per_beat}  tracks={len(mid.tracks)}")
    for i, t in enumerate(mid.tracks):
        count = sum(1 for m in t if m.type == "note_on")
        print(f"Track {i}: {count} notes")

    while True:
        user_input = input("Select Track: ")
        if len(user_input) == 1 and user_input.isdigit():
            break
        print("Error: Please enter a single digit.")

    notes = extract_notes(mid, int(user_input))
    tiles = build_tiles(mid, notes)

    # Build output path
    midi_filename = os.path.splitext(os.path.basename(args.midi))[0]

    if args.output is None:
        output = os.path.join("..", "beatmaps", f"{midi_filename}.csv")
    else:
        output = args.output

    # Write
    with open(output, "w", newline="") as f:
        writer = csv.writer(f)
        
        writer.writerow(["lane", "time_ms", "duration_ms"])
        
        for t in tiles:
            writer.writerow([t["lane"], t["time_ms"], t["duration_ms"]])

    print(f"Generated {len(tiles)} tiles and wrote to {output}")


if __name__ == "__main__":
    main()