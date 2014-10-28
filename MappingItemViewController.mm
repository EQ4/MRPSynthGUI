//
//  MappingItemViewController.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/12/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "MappingItemViewController.h"

@implementation MidiMappingItemViewController

@synthesize bgLayer;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth *)synth {
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        
        _synth = synth;
        std::vector<std::string> params = _synth->masterVoice()->getParameterNames();
        SynthParameter *param = _synth->masterVoice()->getParameterWithName(params[0]);
        
        mapping = new MidiMapping();
        mapping->byte1 = 0x00;
        mapping->byte2 = 0x00;
        mapping->parameterName = param->name();
        mapping->type = kMappingTypeAssign;
        mapping->ramp = false;
        mapping->min = param->minVal();
        mapping->max = param->maxVal();
        mapping->scale = kMappingScaleLinear;
        
        bgLayer = [[CALayer alloc] init];
        [bgLayer setBorderColor:[NSColor darkGrayColor].CGColor];
        [bgLayer setBorderWidth:2.0f];
        [bgLayer setCornerRadius:6.0f];
        [[self view] setWantsLayer:true];
        [[self view] setLayer:bgLayer];
    }
    return self;
}

/* Override [NSViewController loadView] to do UI setup after the .xib has loaded since UI updates performed in [NSViewController initWithNibName:] don't work because the view hasn't fully loaded yet */
- (void)loadView {
    
    [super loadView];
    
    [dataByteField setStringValue:[NSString stringWithFormat:@"%d", mapping->byte2]];
    [minField setStringValue:[NSString stringWithFormat:@"%1.1f", mapping->min]];
    [maxField setStringValue:[NSString stringWithFormat:@"%1.1f", mapping->max]];
    
    /* Populate the parameter selector menu with the voice's parameters */
    std::vector<std::string> params = _synth->masterVoice()->getParameterNames();
    
    /* Clear and re-populate the output device list */
    [synthParameterSelector removeAllItems];
    for (int i = 0; i < params.size(); i++) {
        NSString *name = [NSString stringWithFormat:@"%s", params[i].c_str()];
        [synthParameterSelector addItemWithTitle:name];
    }
    
    _synth->addMasterVoiceMidiMapping(mapping);
}

- (void)apply {
    _synth->addMasterVoiceMidiMapping(mapping);
}

- (void)cancel {
    _synth->removeMasterVoiceMidiMapping(mapping);
}

- (IBAction)synthParameterSelected:(id)sender {
    
    [self cancel];
    mapping->parameterName = [[synthParameterSelector titleOfSelectedItem] UTF8String];
    SynthParameter *param = _synth->masterVoice()->getParameterWithName(mapping->parameterName);
    mapping->min = param->minVal();
    [minField setFloatValue:mapping->min];
    mapping->max = param->maxVal();
    [maxField setFloatValue:mapping->max];
    [self apply];
}

- (IBAction)mappingTypeSelected:(id)sender {
    
    [self cancel];
    switch ([mappingTypeSelector indexOfSelectedItem]) {
        case 0:
            mapping->type = kMappingTypeAssign;
            break;
        case 1:
            mapping->type = kMappingTypeAdd;
            break;
            
        case 2:
            mapping->type = kMappingTypeMultiply;
            break;
    }
    [self apply];
}

- (IBAction)messageTypeSelected:(id)sender {
    
    [self cancel];
    int messageType = (int)[[messageTypeSelector selectedCell] tag];
    int channel = (int)[midiChannelSelector indexOfSelectedItem];
    mapping->byte1  = (messageType << 4) | channel;
    [self apply];
}

- (IBAction)midiChannelSelected:(id)sender {
    
    [self cancel];
    int messageType = (int)[[messageTypeSelector selectedCell] tag];
    int channel = (int)[midiChannelSelector indexOfSelectedItem];
    mapping->byte1  = (messageType << 4) | channel;
    [self apply];
}

- (IBAction)dataByteSet:(id)sender {
    
    [self cancel];
    mapping->byte2 = (int)[[dataByteField stringValue] integerValue];
    [self apply];
}

- (IBAction)minSet:(id)sender {
    
    [self cancel];
    mapping->min = (int)[[minField stringValue] integerValue];
    [self apply];
}

- (IBAction)maxSet:(id)sender {
    
    [self cancel];
    mapping->max = (int)[[maxField stringValue] integerValue];
    [self apply];
}

- (IBAction)rampSet:(id)sender {
    
    [self cancel];
    mapping->ramp = [rampSwitch state] == NSOnState;
    [self apply];
}

- (IBAction)mappingScaleSelected:(id)sender {
    
    [self cancel];
    switch ([mappingScaleSelector indexOfSelectedItem]) {
        case 0:
            mapping->scale = kMappingScaleLinear;
            break;
        case 1:
            mapping->scale = kMappingScaleLogarithmic;
            if (mapping->min <= 0.0f)
                mapping->min = 0.01f;
            if (mapping->max <= 0.0f)
                mapping->max = 1.0f;
            break;
    }
    [self apply];
}

- (void)fillMIDIParams:(std::vector<unsigned char> *)message {
    
    int statusByte = (int)message->at(0);
    int messageType = (statusByte & 0xF0) >> 4;
    int midiChannel = statusByte & 0x0F;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [messageTypeSelector selectItemWithTag:messageType];
        [self messageTypeSelected:self];
        [midiChannelSelector selectItemWithTag:midiChannel];
        [self midiChannelSelected:self];
    });
    
    if (message->size() > 1) {
        int dataByte = message->at(1);
        dispatch_async(dispatch_get_main_queue(), ^{
            [dataByteField setIntegerValue:dataByte];
            [self dataByteSet:self];
        });
    }
}

@end



























