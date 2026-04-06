#!/usr/bin/env python3

import mido
import json
import sys
import argparse
import os

# ---- SETTINGS (tune these for Let It Be) ----
TRACK_INDEX = 1        # try 0,1,2... until it looks right
CHANNEL = None         # or set 0–15 if needed
LANE_NOTES = [60, 62, 64, 67]  # C4, D4, E4, G4 (good for melody)
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
    # SIMPLE conversion (assumes constant tempo ~120 BPM)
    tempo = 500000  # default
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
    p.add_argument("--output",        default=None)
    args = p.parse_args()

    if not os.path.exists(args.midi):
        print(f"ERROR: File not found: {args.midi}")
        usage()
        sys.exit(1)

    print(f"[INFO] MIDI: {args.midi}")
    mid = mido.MidiFile(args.midi)
    print(f"[INFO] type={mid.type}  tpb={mid.ticks_per_beat}  tracks={len(mid.tracks)}")
    for i, t in enumerate(mid.tracks):
        count = sum(1 for m in t if m.type == "note_on")
        print(f"Track {i}: {count} notes")

    while True:
        user_input = input("Select Track: ")
        if len(user_input) == 1:
            break
        print("Error: Please enter only one character.")

    notes = extract_notes(mid, int(user_input))
    tiles = build_tiles(mid, notes)

    beatmap = {
        "meta": {
            "song": args.midi,
            "tile_count": len(tiles)
        },
        "tiles": tiles,
    }

    # Build output path
    script_dir = os.path.dirname(os.path.abspath(__file__))
    midi_filename = os.path.splitext(os.path.basename(args.midi))[0]
    
    if args.output is None:
        output = os.path.join("..", "beatmaps", f"{midi_filename}.json")
    else:
        output = args.output

    with open(output, "w") as f:
        json.dump(beatmap, f, indent=2)

    print(f"Generated {len(tiles)} tiles and wrote to {output}")


if __name__ == "__main__":
    main()