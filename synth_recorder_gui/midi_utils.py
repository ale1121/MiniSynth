import os
import time
import math
from threading import Event
from mido import MidiFile, MidiTrack, Message, second2tick, bpm2tempo
import pygame.midi

TICKS_PER_BEAT = 480
BPM = 120

pause_event = Event()

def save_recording(buffer):
    if not buffer:
        # dont save empty recordings
        return
    
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    file_name = f"rec_{timestamp}.mid"
    file_path = os.path.join("recordings", file_name)
    
    mid = MidiFile(ticks_per_beat=TICKS_PER_BEAT)
    track = MidiTrack()
    mid.tracks.append(track)

    tempo = bpm2tempo(BPM)
    last_timestamp = None
    active_note = None

    for line in buffer:
        try:
            msg_type, timestamp_ms, freq = map(int, line.split())
            timestamp = timestamp_ms / 1000.0

            if last_timestamp is None:
                # first note
                delta_ticks = 0
            else:
                # calculate ticks since last message
                delta_seconds = timestamp - last_timestamp
                delta_ticks = round(second2tick(delta_seconds, TICKS_PER_BEAT, tempo))

            last_timestamp = timestamp

            if msg_type == 1:
                # note on

                note = freq_to_midi(freq)

                if active_note is not None:
                    # end unfinished previous note
                    track.append(Message('note_off', note=active_note, velocity=64, time=delta_ticks))
                    delta_ticks = 0
                
                track.append(Message('note_on', note=note, velocity=64, time=delta_ticks))
                active_note = note  # update active note

            elif msg_type == 0:
                # note off

                if active_note is not None:
                    track.append(Message('note_off', note=active_note, velocity=64, time=delta_ticks))

                active_note = None  # clear active note

        except Exception as e:
            print(e)
            continue

    if active_note is not None:
        # end unfinished last note
        track.append(Message('note_off', note=active_note, velocity=64, time=500))

    mid.save(file_path)


def get_recordings():
    return [f for f in os.listdir("recordings") if f.endswith(".mid")]


def play_recording(rec_name):
    file_path = os.path.join("recordings", rec_name)
    if not os.path.exists(file_path):
        print("no such file")
        return

    pygame.midi.init()
    player = pygame.midi.Output(0)
    mid = MidiFile(file_path)

    for msg in mid.play():
        if pause_event.is_set():
            # stop playing
            break

        if msg.type == 'note_on':
            player.note_on(msg.note, msg.velocity)
        elif msg.type == 'note_off':
            player.note_off(msg.note, msg.velocity)

    player.close()
    pygame.midi.quit()
    pause_event.clear()


def freq_to_midi(freq):
    return round(69 + 12 * math.log2(freq / 440.0))
