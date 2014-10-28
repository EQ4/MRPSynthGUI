//
//  IOViewController.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/11/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MIDIHandler.h"
#import	"CustomOpenGLView.h"

#include "AudioController.h"

#undef check    /* 'check' is a reserved word in Objective C,
                    which conflicts with boost libraries used
                    by the touchkeys source */

#include "TouchkeyDevice.h"

#include <string>
#include <vector>

#define kDefaultAudioOutputDeviceName @"Built-in Output"
#define kDefaultAudioOutputNumChannels 2
#define kDefaultAudioOutputSampleRate 44100.0f

@interface IOViewController : NSViewController <NSApplicationDelegate> {
    
    /* Audio */
    AudioController *audioController;
    IBOutlet NSPopUpButton *audioOutputDeviceSelector;
    IBOutlet NSPopUpButton *audioOutputNumChannelsSelector;
    IBOutlet NSPopUpButton *audioOutputSampleRateSelector;
    IBOutlet NSButton *audioStartStopButton;
    
    /* Touchkeys */
    IBOutlet NSButton      *touchkeyInputEnable;
    IBOutlet NSPopUpButton *touchkeyDeviceSelect;
    IBOutlet NSPopUpButton *touchkeyLowestOctaveSelect;
    IBOutlet NSButton      *touchkeyStartButton;
    IBOutlet NSButton      *touchkeyCalibrateButton;
    IBOutlet NSButton      *touchkeyCalibrationLoadButton;
    IBOutlet NSButton      *touchkeyCalibrationSaveButton;
    IBOutlet NSButton      *touchkeyCalibrationClearButton;
    IBOutlet NSTextField   *touchkeyCalibrationStatusField;
    IBOutlet NSTextField   *touchkeyStatusField;
    
    // TODO: Set these externally from preferences viewController
    NSString *touchkeyCalibrationDirectoryPath;
    NSString *touchkeyLogDirectoryPath;
    
    std::vector<std::string> touchkeyDevicePaths;
    TouchkeyDevice  *touchkeyController;
    PianoKeyboard   *keyboardController;
//    KeyboardDisplay *keyboardDisplay;
    
    /* MIDI */
    MIDIHandler *midiHandler;
    IBOutlet NSPopUpButton *midiInputDeviceSelector;
    IBOutlet NSPopUpButton *midiOutputDeviceSelector;
    
    /* OSC */
}

@property IBOutlet NSPopUpButton *audioOutputDeviceSelector;

- (id)initWithNibName:(NSString *)nibNameOrNil
               bundle:(NSBundle *)nibBundleOrNil
                audio:(AudioController *)aCont
                 midi:(MIDIHandler *)mHandler;

@end
