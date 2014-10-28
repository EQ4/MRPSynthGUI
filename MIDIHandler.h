//
//  MIDIHandler.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/14/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "RtMidi.h"
#include "PolySynth.h"

#define kMRPMIDIOutputChannel 0

enum {
    kMESSAGE_NOTEOFF = 0x80,
    kMESSAGE_NOTEON = 0x90,
    kMESSAGE_AFTERTOUCH_POLY = 0xA0,
    kMESSAGE_CONTROL_CHANGE = 0xB0,
    kMESSAGE_PROGRAM_CHANGE = 0xC0,
    kMESSAGE_AFTERTOUCH_CHANNEL = 0xD0,
    kMESSAGE_PITCHWHEEL = 0xE0,
    kMESSAGE_SYSEX = 0xF0,
    kMESSAGE_SYSEX_END = 0xF7,
    kMESSAGE_ACTIVE_SENSE = 0xFE,
    kMESSAGE_RESET = 0xFF
};

enum {
    kCONTROL_MRP_BASE = 16
};

#pragma mark - SynthVoiceMappingDelegate
/* This method should be implemented if the Synth Voice's UI is to be updated via MIDI/OSC mappings. The method is called by MIDIHandler or OSCHandler whenever a message has updated a synth parameter. The implementation of this method should set the value of the UI element associated with the specified synth parameter without actually updating the synth voice's parameter */
@protocol SynthVoiceMappingDelegate
- (void)updateControlForParameterName:(NSString *)name value:(float)val;
@end

#pragma mark - MIDIMappingItemDelegate
/* This method is implemented by MappingViewController so that any currently-selected MIDI mapping items can have their parameter UI elements populated automatically when a MIDI control message is received */
@protocol MIDIMappingItemDelegate
- (void)fillMIDIParamsForSelectedCells:(std::vector<unsigned char> *)message;
@end

#pragma mark - MIDIHandler
@interface MIDIHandler : NSObject {
    
    PolySynth *_synth;
    
    RtMidiIn *_midiIn;                      // Main MIDI Input
    RtMidiOut *_midiOut;                    // Main MIDI Output
    
    std::vector<RtMidiIn*> _midiInputs;     // Secondary MIDI Inputs
    
    std::vector<std::string> _inputDeviceNames;
    std::vector<std::string> _outputDeviceNames;
}

@property BOOL isRunning;
@property NSViewController <MIDIMappingItemDelegate> *mappingViewController;
@property NSViewController <SynthVoiceMappingDelegate> *voiceViewController;

- (PolySynth *)synth;

- (id)initWithPolySynth:(PolySynth *)synth;
- (bool)rescanInputDevices;
- (bool)rescanOutputDevices;
- (bool)setInputDevice:(int)devIdx;
- (bool)openSecondaryInputDevice:(int)devIdx;
- (bool)setOutputDevice:(int)devIdx;
- (bool)sendMRPRoutingMessage:(int)channel string:(int)string;

- (std::vector<std::string>)getInputDeviceNames;
- (std::vector<std::string>)getOutputDeviceNames;

@end

