import serial
import threading
import time
import midi_utils
import pygame

class Recorder:
    def __init__(self, port):
        self.port = port
        self.is_recording = threading.Event()
        self.serial = None
        self.listen_thread = None
        self.buffer = []


    def is_connected(self):
        return self.serial is not None and self.serial.is_open


    def connect(self):
        # try connection for 10s
        try:
            self.serial = serial.Serial(self.port, 9600, timeout=10)
            self.serial.reset_input_buffer()
            return True
        except:
            return False


    def disconnect(self):
        self.serial.close()
        self.serial = None


    def start_recording(self):
        # record in a separate thread
        self.is_recording.set()
        self.buffer = []
        self.listen_thread = threading.Thread(target=self.read_loop)
        self.listen_thread.start()


    def stop_recording(self):
        self.serial.reset_input_buffer()
        self.is_recording.clear()
        midi_utils.save_recording(self.buffer)
        self.buffer = []
        pygame.event.post(pygame.event.Event(pygame.USEREVENT, {"custom_type": "RECORDING_ENDED"}))


    def read_loop(self):
        self.serial.reset_input_buffer()  # clear serial buffer
        self.buffer = []                  # clear data buffer

        # read data until "Stop Recording" is clicked
        while self.is_recording.is_set():
            try:
                if not self.serial or not self.serial.is_open:
                    raise Exception
                
                line = self.serial.readline().decode("utf-8").strip()

                if line:
                    if line == "RECORD_OFF":
                        self.stop_recording()
                        return
                    if line == "RECORD_ON":
                        # clear messages before "RECORD_ON" flag
                        self.buffer = []
                        continue
                    self.buffer.append(line)
                    print(line)

            except:
                print(f"disconnected")
                self.serial.close()
                self.stop_recording()
                self.serial = None
                return