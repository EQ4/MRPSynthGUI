//
//  SynthVoiceViewController.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/10/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "SynthVoiceViewController.h"

@interface SynthVoiceViewController ()

@end

@implementation SynthVoiceViewController

@synthesize synth = _synth;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth*)synth
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        _synth = synth;
        dispatch_async(dispatch_get_main_queue(), ^{
            [self setAttack:attackSlider];
            [self setDecay:decaySlider];
            [self setSustain:sustainSlider];
            [self setRelease:releaseSlider];
            [self setAmplitude:amplitudeSlider];
        });
    }
    return self;
}

- (IBAction)setAttack:(id)sender {
        _synth->setMasterVoiceParam("Attack", [attackSlider floatValue], true);
}

- (IBAction)setDecay:(id)sender {
        _synth->setMasterVoiceParam("Decay", [decaySlider floatValue], true);
}

- (IBAction)setSustain:(id)sender {
        _synth->setMasterVoiceParam("Sustain", [sustainSlider floatValue], true);
}

- (IBAction)setRelease:(id)sender {
        _synth->setMasterVoiceParam("Release", [releaseSlider floatValue], true);
}

- (IBAction)setAmplitude:(id)sender {
        _synth->setMasterVoiceParam("Amplitude", [amplitudeSlider floatValue], true);
}

- (void)updateControlForParameterName:(NSString *)name value:(float)val {
    
    if ([name isEqualToString:@"Attack"]) {
        [attackSlider setFloatValue:val];
    }
    else if ([name isEqualToString:@"Decay"]) {
        [decaySlider setFloatValue:val];
    }
    else if ([name isEqualToString:@"Sustain"]) {
        [sustainSlider setFloatValue:val];
    }
    else if ([name isEqualToString:@"Release"]) {
        [releaseSlider setFloatValue:val];
    }
    else if ([name isEqualToString:@"Amplitude"]) {
        [amplitudeSlider setFloatValue:val];
    }
}

@end








