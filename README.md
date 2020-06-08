# Deluge MIDI Footswitch

This is bare-bones source code for my DIY project to create a MIDI 'footswitch' controllers for controlling recording, looping and playback of the Synthstrom Deluge synthesizer.

The code detects (PULLUP) digital inputs on 4 of the board pins and uses the AceButtons button handler library to turn these inputs into single clicks, double clicks and long presses. In turn, these events generate unique MIDI notes (the last 12 notes in the 0 to 127 MIDI note range) which can be learned by the Deluge to control various functions. The default MIDI channel is 16 but you can enter a setup mode by holding button 4 for 2 seconds. In setup mode you can increment or decrement the MIDI channel if you do not want to use channel 16. This change is lost on rebooting the device.

Project was designed and coded for a Teensy 3.2 but should be vaguely compatible with other Arduino boards.

Used libraries:

    MIDI
    AceButtons

both of which you can install from Arduino's Library Manager.

Feel free to use any of the code as you wish.

Neil Baldwin, June 2020

