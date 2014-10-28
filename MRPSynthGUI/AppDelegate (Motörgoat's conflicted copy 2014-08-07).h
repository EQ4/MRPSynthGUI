//
//  AppDelegate.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 8/4/14.
//  Copyright (c) 2014 ___FULLUSERNAME___. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <vector>

#include "AudioController.h"
#include "PolySynth.h"
#include "AdditiveSynthVoice.h"
#include "MidiController.h"

#define kDefaultAudioOutputDeviceName @"Built-in Output"
#define kDefaultAudioOutputNumChannels 2
#define kDefaultAudioOutputSampleRate 44100.0f

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    
    AudioController* audioController;
    MidiController* midiController;
    
    SynthVoice* masterVoice;
    PolySynth* synth;

    IBOutlet NSPopUpButton* outputDeviceSelector;
    IBOutlet NSPopUpButton* outputNumChannelsSelector;
    IBOutlet NSPopUpButton* outputSampleRateSelector;
    
    IBOutlet NSPopUpButton* midiInputDeviceSelector;
    IBOutlet NSPopUpButton* midiOutputDeviceSelector;
    
    IBOutlet NSButton* startStopButton;
    
    IBOutlet NSButton* c4Button;
    IBOutlet NSButton* d4Button;
    IBOutlet NSButton* e4Button;
    IBOutlet NSButton* f4Button;
    IBOutlet NSButton* g4Button;
    IBOutlet NSButton* a4Button;
    IBOutlet NSButton* b4Button;
    IBOutlet NSButton* c5Button;
    IBOutlet NSButton* cS4Button;
    IBOutlet NSButton* dS4Button;
    IBOutlet NSButton* fS4Button;
    IBOutlet NSButton* gS4Button;
    IBOutlet NSButton* aS4Button;
}

@property (assign) IBOutlet NSWindow *window;

#pragma mark - Audio Parameter Events
- (IBAction)setOutputDevice:(id)sender;
- (IBAction)setOutputNumChannels:(id)sender;
- (IBAction)setOutputSampleRate:(id)sender;
- (IBAction)toggleOutput:(id)sender;

#pragma mark - MIDI Parameter Events
- (IBAction)setMIDIInputDevice:(id)sender;
- (IBAction)setMIDIOutputDevice:(id)sender;

- (void)rescanMIDIDevices;

#pragma mark - GUI Keyboard Events
- (IBAction)blackKeyEvent:(NSButton*)sender;
- (IBAction)whiteKeyEvent:(NSButton*)sender;

@end
