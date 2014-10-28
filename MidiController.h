//
//  MidiController.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 8/5/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRPSynthGUI__MidiController__
#define __MRPSynthGUI__MidiController__

#include <iostream>
#include "RtMidi.h"
#include "PolySynth.h"

#define kMRPMIDIOutputChannel 0

/* MIDI message types */
enum {
    MESSAGE_NOTEOFF = 0x80,
    MESSAGE_NOTEON = 0x90,
    MESSAGE_AFTERTOUCH_POLY = 0xA0,
    MESSAGE_CONTROL_CHANGE = 0xB0,
    MESSAGE_PROGRAM_CHANGE = 0xC0,
    MESSAGE_AFTERTOUCH_CHANNEL = 0xD0,
    MESSAGE_PITCHWHEEL = 0xE0,
    MESSAGE_SYSEX = 0xF0,
    MESSAGE_SYSEX_END = 0xF7,
    MESSAGE_ACTIVE_SENSE = 0xFE,
    MESSAGE_RESET = 0xFF
};

//enum {
//    CONTROL_BANK_SELECT = 0,
//    CONTROL_MODULATION_WHEEL = 1,
//    CONTROL_VOLUME = 7,
//    CONTROL_AUX_PEDAL = 15,
//    CONTROL_MRP_BASE = 16,
//    CONTROL_BANK_SELECT_LSB = 32,
//    CONTROL_MODULATION_WHEEL_LSB = 33,
//    CONTROL_VOLUME_LSB = 39,
//    CONTROL_DAMPER_PEDAL = 64,
//    CONTROL_SOSTENUTO_PEDAL = 66,
//    CONTROL_SOFT_PEDAL = 67,
//    CONTROL_ALL_SOUND_OFF = 120,
//    CONTROL_ALL_CONTROLLERS_OFF = 121,
//    CONTROL_LOCAL_KEYBOARD = 122,
//    CONTROL_ALL_NOTES_OFF = 123,
//    CONTROL_OMNI_OFF = 124,
//    CONTROL_OMNI_ON = 125,
//    CONTROL_MONO_OPERATION = 126,
//    CONTROL_POLY_OPERATION = 127
//};

enum {
    CONTROL_MRP_BASE = 16
};

class PolySynth;

class MidiController {
    
    PolySynth *_synth;
    
    RtMidiIn *_midiIn;
    RtMidiOut *_midiOut;
    
    std::vector<std::string> _inputDeviceNames;
    std::vector<std::string> _outputDeviceNames;
    
#pragma mark - Private Methods
    
#pragma mark - RtMidi Input Callback
    /* RtMidi requires a static callback method, so the staticMidiInputCallback() method passes control to the instance-specified midiInputCallback() */
    void midiInputCallback(double deltaTime, std::vector<unsigned char> *message);
    static void staticMidiInputCallback(double deltaTime, std::vector<unsigned char> *message, void *userData) {
		MidiController *controller = (MidiController *)userData;
		controller->midiInputCallback(deltaTime, message);
	}
    
public:
    
    MidiController(PolySynth *synth);
    ~MidiController();
    
    std::vector<std::string> getInputDeviceNames() { return _inputDeviceNames; }
    std::vector<std::string> getOutputDeviceNames() { return _outputDeviceNames; }
    bool setInputDevice(int devIdx);
    bool setOutputDevice(int devIdx);
    
    /* Rescan for changes in available devices list. Return true if number of devices has changed since the list was last populated */
    bool rescanInputDevices();
    bool rescanOutputDevices();
    
#pragma mark - RtMidi Output
    bool sendMRPRoutingMessage(int channel, int string);
};

#endif /* defined(__MRPSynthGUI__MidiController__) */
