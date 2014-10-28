//
//  SynthVoiceViewController.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/10/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MIDIHandler.h"

#include "PolySynth.h"

@interface SynthVoiceViewController : NSViewController <SynthVoiceMappingDelegate> {
    
    IBOutlet NSSlider* attackSlider;
    IBOutlet NSSlider* decaySlider;
    IBOutlet NSSlider* sustainSlider;
    IBOutlet NSSlider* releaseSlider;
    IBOutlet NSSlider* amplitudeSlider;
}

@property PolySynth* synth;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth*)synth;

- (IBAction)setAttack:      (id)sender;
- (IBAction)setDecay:       (id)sender;
- (IBAction)setSustain:     (id)sender;
- (IBAction)setRelease:     (id)sender;
- (IBAction)setAmplitude:   (id)sender;

#pragma mark - SynthVoiceMappingDelegate method
- (void)updateControlForParameterName:(NSString *)name value:(float)val;

@end
