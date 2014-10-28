//
//  AdditiveSynthVoiceViewController.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/10/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MIDIHandler.h"

#include "PolySynth.h"

@interface AdditiveSynthVoiceViewController : NSViewController <SynthVoiceMappingDelegate> {
    
    IBOutlet NSView *parent;
    
    IBOutlet NSBox *harmonicsBox;
    NSMutableArray *harmonicSliders;

    IBOutlet NSBox *adsrBox;
    IBOutlet NSSlider *attackSlider;
    IBOutlet NSSlider *decaySlider;
    IBOutlet NSSlider *sustainSlider;
    IBOutlet NSSlider *releaseSlider;
    IBOutlet NSSlider *amplitudeSlider;
}

@property PolySynth *synth;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth*)synth numHarmonics:(int)n;

- (IBAction)setAttack:      (NSSlider*)sender;
- (IBAction)setDecay:       (NSSlider*)sender;
- (IBAction)setSustain:     (NSSlider*)sender;
- (IBAction)setRelease:     (NSSlider*)sender;
- (IBAction)setAmplitude:   (NSSlider*)sender;
- (IBAction)setHarmonicAmp: (NSSlider*)sender;

#pragma mark - SynthVoiceMappingDelegate method
- (void)updateControlForParameterName:(NSString *)name value:(float)val;

@end
