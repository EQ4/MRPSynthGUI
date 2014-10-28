//
//  SubtractiveSynthVoiceViewController.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 10/16/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MIDIHandler.h"
#import "NSLogSlider.h"

#include "PolySynth.h"
#include "SubtractiveSynthVoice.h"

@interface SubtractiveSynthVoiceViewController : NSViewController <SynthVoiceMappingDelegate> {
    
    IBOutlet NSView *parent;
    
    IBOutlet NSBox *harmonicsBox;
    NSMutableArray *harmonicSliders;
    
    IBOutlet NSBox *adsrBox;
    IBOutlet NSSlider *attackSlider;
    IBOutlet NSSlider *decaySlider;
    IBOutlet NSSlider *sustainSlider;
    IBOutlet NSSlider *releaseSlider;
    
    IBOutlet NSBox *filterBox;
    IBOutlet NSLogSlider *filterCutoffSlider;
    IBOutlet NSSlider *filterResonanceSlider;
    IBOutlet NSSegmentedControl *filterTypeSelector;
    IBOutlet NSButton *invertFilterEnvelopeCheckBox;
    
    IBOutlet NSBox *filterAdsrBox;
    IBOutlet NSSlider *filterAttackSlider;
    IBOutlet NSSlider *filterDecaySlider;
    IBOutlet NSSlider *filterSustainSlider;
    IBOutlet NSSlider *filterReleaseSlider;
    
    IBOutlet NSSlider *amplitudeSlider;
}

@property PolySynth *synth;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth*)synth numHarmonics:(int)n;

- (IBAction)setHarmonicAmp: (NSSlider*)sender;

- (IBAction)setAttack:      (NSSlider*)sender;
- (IBAction)setDecay:       (NSSlider*)sender;
- (IBAction)setSustain:     (NSSlider*)sender;
- (IBAction)setRelease:     (NSSlider*)sender;

- (IBAction)setFilterCutoff:    (NSSlider*)sender;
- (IBAction)setFilterResonance: (NSSlider*)sender;
- (IBAction)setFilterType:      (NSSegmentedControl*)sender;
- (IBAction)setInvertFilterEnvelope:    (NSButton*)sender;
- (IBAction)setFilterAttack:    (NSSlider*)sender;
- (IBAction)setFilterDecay:     (NSSlider*)sender;
- (IBAction)setFilterSustain:   (NSSlider*)sender;
- (IBAction)setFilterRelease:   (NSSlider*)sender;

- (IBAction)setAmplitude:   (NSSlider*)sender;

#pragma mark - SynthVoiceMappingDelegate method
- (void)updateControlForParameterName:(NSString *)name value:(float)val;

@end
