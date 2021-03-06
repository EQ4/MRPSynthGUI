//
//  SubtractiveSynthVoiceViewController.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 10/16/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "SubtractiveSynthVoiceViewController.h"

@interface SubtractiveSynthVoiceViewController ()

@end

@implementation SubtractiveSynthVoiceViewController

@synthesize synth = _synth;

//- (void)viewDidLoad {
//    [super viewDidLoad];
//    // Do view setup here.
//}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth*)synth numHarmonics:(int)n
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        
        _synth = synth;
        
        dispatch_async(dispatch_get_main_queue(), ^{
            
            [self setAttack:attackSlider];
            [self setDecay:decaySlider];
            [self setSustain:sustainSlider];
            [self setRelease:releaseSlider];
            
            [self setFilterCutoff:filterCutoffSlider];
            [self setFilterResonance:filterResonanceSlider];
            [self setFilterType:filterTypeSelector];
            [self setInvertFilterEnvelope:invertFilterEnvelopeCheckBox];
            [self setFilterAttack:filterAttackSlider];
            [self setFilterDecay:filterDecaySlider];
            [self setFilterSustain:filterSustainSlider];
            [self setFilterRelease:filterReleaseSlider];
            
            [self setAmplitude:amplitudeSlider];
        });
        
        harmonicSliders = [[NSMutableArray alloc] init];
        [self createHarmonicSliderArray:n];
    }
    return self;
}

- (void)createHarmonicSliderArray:(int)numHarmonics {
    
    CGFloat spacing = 45.0f;
    CGFloat leadingSpace = 20.0f;
    CGSize sliderSize = CGSizeMake(21.0f, 150.0f);
    
    NSRect frame;
    NSSlider* hSlider;
    NSTextField* label;
    for (int i = 0; i < numHarmonics; i++) {
        frame = NSMakeRect(leadingSpace + i * spacing,
                           23.0f,
                           sliderSize.width,
                           sliderSize.height);
        hSlider = [[NSSlider alloc] initWithFrame:frame];
        [hSlider setMinValue:0];
        [hSlider setMaxValue:1];
        [hSlider setFloatValue:(i == 0 ? 1.0f : 0.0f)];
        [hSlider setTag:i+1];
        [hSlider setTarget:self];
        [hSlider setAction:@selector(setHarmonicAmp:)];
        [harmonicSliders addObject:hSlider];
        
        frame.origin.y += frame.size.height + 3.0f;
        frame.size.height = 20.0f;
        label = [[NSTextField alloc] initWithFrame:frame];
        [label setStringValue:[NSString stringWithFormat:@"H%d", i+1]];
        [label setBezeled:NO];
        [label setDrawsBackground:NO];
        [label setEditable:NO];
        [label setSelectable:NO];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [harmonicsBox addSubview:hSlider];
            [harmonicsBox addSubview:label];
        });
    }
    
    /* Set the synth's harmonic amplitudes to the slider values */
    dispatch_async(dispatch_get_main_queue(), ^{
        for (int i = 0; i < [harmonicSliders count]; i++)
            [self setHarmonicAmp:harmonicSliders[i]];
    });
}

- (IBAction)setHarmonicAmp:(NSSlider*)sender {
    
    char name[12];
    sprintf(name, "Harmonic %d", (int)[sender tag]);
    _synth->setMasterVoiceParam(name, [sender floatValue], true);
}

- (IBAction)setAttack:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Attack", [attackSlider floatValue], true);
}

- (IBAction)setDecay:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Decay", [decaySlider floatValue], true);
}

- (IBAction)setSustain:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Sustain", [sustainSlider floatValue], true);
}

- (IBAction)setRelease:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Release", [releaseSlider floatValue], true);
}

- (IBAction)setFilterCutoff:(NSSlider*)sender {
    printf("Cutoff = %f\n", [filterCutoffSlider logValue]);
    _synth->setMasterVoiceParam("Cutoff Freq", [filterCutoffSlider logValue], false);
}

- (IBAction)setFilterResonance:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Resonance", [filterResonanceSlider floatValue], false);
}

- (IBAction)setFilterType:(NSSegmentedControl*)sender {
    
    /* The filter type is not a key-value assigned parameter, so we need to get the master voice, modify it, then reset the master voice */
    SubtractiveSynthVoice *voice = (SubtractiveSynthVoice *)_synth->masterVoice();
    
    switch ([sender selectedSegment]) {
            
        case 0:
            voice->setFilterType(kBiquadFilterType_LowPass);
            break;
        case 1:
            voice->setFilterType(kBiquadFilterType_HighPass);
            break;
        case 2:
            voice->setFilterType(kBiquadFilterType_BandPass);
            break;
    }
    
    _synth->setMasterVoice(voice);
}

- (IBAction)setInvertFilterEnvelope:(NSButton*)sender {
    
}

- (IBAction)setFilterAttack:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Filter Attack", [filterAttackSlider floatValue], true);
}

- (IBAction)setFilterDecay:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Filter Decay", [filterDecaySlider floatValue], true);
}

- (IBAction)setFilterSustain:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Filter Sustain", [filterSustainSlider floatValue], true);
}

- (IBAction)setFilterRelease:(NSSlider*)sender {
    _synth->setMasterVoiceParam("Filter Release", [filterReleaseSlider floatValue], true);
}

- (IBAction)setAmplitude:(NSSlider *)sender {
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
    else if ([name isEqualToString:@"Cutoff Freq"]) {
        [filterCutoffSlider setLogValue:val];
    }
    else if ([name isEqualToString:@"Resonance"]) {
        [filterResonanceSlider setFloatValue:val];
    }
    else if ([name isEqualToString:@"Filter Attack"]) {
        [filterAttackSlider setFloatValue:val];
    }
    else if ([name isEqualToString:@"Filter Decay"]) {
        [filterDecaySlider setFloatValue:val];
    }
    else if ([name isEqualToString:@"Filter Sustain"]) {
        [filterSustainSlider setFloatValue:val];
    }
    else if ([name isEqualToString:@"Filter Release"]) {
        [filterReleaseSlider setFloatValue:val];
    }
    else {
        NSArray *tokens = [name componentsSeparatedByString:@" "];
        
        if ([tokens[0] isEqualToString:@"Harmonic"]) {
            if ([tokens count] == 2) {
                NSNumberFormatter *nf = [[NSNumberFormatter alloc] init];
                NSNumber *num = [nf numberFromString:tokens[1]];
                if (num != nil) {
                    NSSlider *hSlider = [harmonicSliders objectAtIndex:[num integerValue]-1];
                    [hSlider setFloatValue:val];
                }
            }
        }
    }
}

@end
