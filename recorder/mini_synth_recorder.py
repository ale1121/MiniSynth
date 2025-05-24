import pygame
import pygame_gui
import os
import midi_utils
import threading
from recorder import Recorder

pygame.init()
pygame.midi.init()

SCREEN_WIDTH = 700
SCREEN_HEIGHT = 500
FPS = 60

PORT = 'COM11'

os.makedirs("recordings", exist_ok=True)


window_surface = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption('Mini Synth Recorder')

background = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
background.fill(pygame.Color('#1e1e1e'))

manager = pygame_gui.UIManager((SCREEN_WIDTH, SCREEN_HEIGHT), 'theme.json')
recorder = Recorder(PORT)


record_tab_button = pygame_gui.elements.UIButton(pygame.Rect(20, 10, 100, 30), 'Record', manager)
view_tab_button = pygame_gui.elements.UIButton(pygame.Rect(130, 10, 120, 30), 'Recordings', manager)


# record tab
record_panel = pygame_gui.elements.UIPanel(pygame.Rect(20, 50, 660, 430), starting_height=1, manager=manager)

port_label = pygame_gui.elements.UILabel(pygame.Rect(10, 10, 80, 30), 'Port Name:', manager, container=record_panel)
port_input = pygame_gui.elements.UITextEntryLine(pygame.Rect(100, 10, 120, 30), manager, container=record_panel)
port_input.set_text("COM11")

status_label = pygame_gui.elements.UILabel(pygame.Rect(10, 50, -1, 30), 'Status: Disconnected', manager, container=record_panel)

connect_button = pygame_gui.elements.UIButton(pygame.Rect(255, 120, 150, 40), 'Connect', manager, container=record_panel)
disconnect_button = pygame_gui.elements.UIButton(pygame.Rect(255, 170, 150, 40), 'Disconnect', manager, container=record_panel)
record_button = pygame_gui.elements.UIButton(pygame.Rect(255, 220, 150, 50), 'Start Recording', manager, container=record_panel, object_id='record-btn')

feedback_label = pygame_gui.elements.UILabel(pygame.Rect(0, 280, 660, 30), '', manager, container=record_panel)


# view tab
view_panel = pygame_gui.elements.UIPanel(pygame.Rect(20, 50, 660, 430), starting_height=0, manager=manager)

recordings_list = pygame_gui.elements.UISelectionList(pygame.Rect(10, 10, 300, 350), item_list=[], manager=manager, container=view_panel)
refresh_button = pygame_gui.elements.UIButton(pygame.Rect(10, 370, 100, 30), 'Refresh', manager, container=view_panel)

play_button = pygame_gui.elements.UIButton(pygame.Rect(430, 140, 100, 40), 'Play', manager, container=view_panel)
delete_button = pygame_gui.elements.UIButton(pygame.Rect(430, 190, 100, 40), 'Delete', manager, container=view_panel)

is_playing = False

def update_recordings():
    recordings = midi_utils.get_recordings()
    recordings_list.set_item_list(recordings)
    play_button.disable()
    delete_button.disable()

def on_recording_finished():
    status_label.set_text('Status: Connected')
    record_button.set_text('Start Recording')
    feedback_label.set_text('Recording finished')
    disconnect_button.enable()
    update_recordings()

def update_state():
    connected = recorder.is_connected()
    recording = recorder.is_recording.is_set()

    if connected:
        status = 'Recording...' if recording else 'Connected'
    else:
        status = 'Disconnected'

    status_label.set_text(f'Status: {status}')

    connect_button.disable() if connected else connect_button.enable()
    disconnect_button.enable() if connected and not recording else disconnect_button.disable()
    record_button.enable() if connected else record_button.disable()
    record_button.set_text('Stop Recording' if recording else 'Start Recording')

def update_ui(time_delta):
    manager.update(time_delta)
    window_surface.blit(background, (0, 0))
    manager.draw_ui(window_surface)
    pygame.display.update()

def delete_recording(name):
    base = os.path.splitext(name)[0]
    os.remove(f"recordings/{base}.mid")

def play_rec():
    global is_playing
    midi_utils.play_recording(selected)
    is_playing = False
    play_button.set_text("Play")


record_panel.show()
view_panel.hide()

update_recordings()
update_state()

clock = pygame.time.Clock()
is_running = True


while is_running:
    time_delta = clock.tick(FPS) / 1000.0

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            is_running = False
            if recorder.is_connected():
                recorder.disconnect()

        if event.type == pygame_gui.UI_BUTTON_PRESSED:
            if event.ui_element == record_tab_button:
                # switch to record tab, pause if playing
                if is_playing: midi_utils.pause_event.set()
                record_panel.show()
                view_panel.hide()

            elif event.ui_element == view_tab_button:
                # switch to view tab
                record_panel.hide()
                view_panel.show()

            elif event.ui_element == connect_button:
                # connect to bluetooth device

                feedback_label.set_text('Connecting...')
                update_ui(time_delta)

                port_text = port_input.get_text().strip()
                if port_text:
                    recorder.port = port_text
                    if recorder.connect():
                        status_label.set_text('Status: Connected')
                        feedback_label.set_text('')
                    else:
                        feedback_label.set_text('Connection failed')
                update_state()

            elif event.ui_element == disconnect_button:
                # disconnect
                recorder.disconnect()
                update_state()

            elif event.ui_element == record_button:
                # start/stop recording
                if recorder.is_recording.is_set():
                    recorder.stop_recording()
                    on_recording_finished()
                else:
                    feedback_label.set_text('')
                    recorder.start_recording()
                update_state()

            elif event.ui_element == play_button:
                # play/stop selected recording, in a separate thread
                selected = recordings_list.get_single_selection()
                if selected:
                    if not is_playing:
                        is_playing = True
                        play_button.set_text("Stop")
                        threading.Thread(target=play_rec, daemon=True).start()
                    else:
                        midi_utils.pause_event.set()

            elif event.ui_element == refresh_button:
                # refresh recordings list
                update_recordings()

            elif event.ui_element == delete_button:
                # delete selected recording
                selected = recordings_list.get_single_selection()
                if selected:
                    if is_playing: midi_utils.pause_event.set()
                    delete_recording(selected)
                    update_recordings()

        if event.type == pygame_gui.UI_SELECTION_LIST_NEW_SELECTION:
            if event.ui_element == recordings_list:
                # select recording, enable option buttons
                play_button.enable()
                delete_button.enable()

        if event.type == pygame.USEREVENT and event.dict.get("custom_type") == "RECORDING_ENDED":
            # update ui after recording stopped
            on_recording_finished()

        manager.process_events(event)

    update_ui(time_delta)

pygame.quit()
