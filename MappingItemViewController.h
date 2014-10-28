//
//  MappingItemViewController.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/12/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include "PolySynth.h"
#include "SynthVoice.h"

@interface MidiMappingItemViewController : NSViewController {
    
    PolySynth *_synth;
    SynthVoice *_voice;
    MidiMapping *mapping;
    
    IBOutlet NSPopUpButton *synthParameterSelector;
    IBOutlet NSPopUpButton *mappingTypeSelector;
    IBOutlet NSPopUpButton *messageTypeSelector;
    IBOutlet NSPopUpButton *midiChannelSelector;
    IBOutlet NSTextField *dataByteField;
    IBOutlet NSTextField *minField;
    IBOutlet NSTextField *maxField;
    IBOutlet NSButton *rampSwitch;
    IBOutlet NSPopUpButton *mappingScaleSelector;
}

@property CALayer *bgLayer;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth *)synth;

- (void)apply;
- (void)cancel;
- (IBAction)synthParameterSelected:(id)sender;
- (IBAction)mappingTypeSelected:(id)sender;
- (IBAction)messageTypeSelected:(id)sender;
- (IBAction)midiChannelSelected:(id)sender;
- (IBAction)dataByteSet:(id)sender;
- (IBAction)minSet:(id)sender;
- (IBAction)maxSet:(id)sender;
- (IBAction)rampSet:(id)sender;
- (IBAction)mappingScaleSelected:(id)sender;

- (void)fillMIDIParams:(std::vector<unsigned char> *)message;


@end
