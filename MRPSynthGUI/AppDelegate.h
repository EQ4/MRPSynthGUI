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
#include "SubtractiveSynthVoice.h"
#include "MidiController.h"

#import "MIDIHandler.h"
#import "IOViewController.h"
#import "SynthVoiceViewController.h"
#import "AdditiveSynthVoiceViewController.h"
#import "SubtractiveSynthVoiceViewController.h"
#import "MappingViewController.h"

#define kSynthVoiceTitle @"Sine Synth"
#define kAdditiveSynthVoiceTitle @"Additive Synth"
#define kSubtractiveSynthVoiceTitle @"Subtractive Synth"
#define kNumHarmonics 8

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    
    IBOutlet NSTabView *leftTabView;
    IBOutlet NSTabView *rightTabView;
    
    /* ----------- */
    /* === I/0 === */
    /* ----------- */
    AudioController *audioController;
    MidiController *midiController;
    MIDIHandler *midiHandler;
    
    IOViewController *ioViewController;
    IBOutlet NSScrollView *ioScrollView;
    
    /* ------------- */
    /* === Synth === */
    /* ------------- */
    PolySynth *synth;
    SynthVoice *synthVoice;
    AdditiveSynthVoice *additiveSynthVoice;
    SubtractiveSynthVoice *subtractiveSynthVoice;
    
    IBOutlet NSScrollView *mappingScrollView;
    MappingViewController *mappingViewController;
    
    IBOutlet NSPopUpButton *synthVoiceSelector;
    NSViewController *synthViewController;
    IBOutlet NSView *synthViewContainer;
    NSArray *synthViewWidthConstraints;
    NSArray *synthViewHeightConstraints;
    
}

@property (assign) IBOutlet NSWindow* window;

@end
