//
//  AppDelegate.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 8/4/14.
//  Copyright (c) 2014 ___FULLUSERNAME___. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    
    /* -------------------------- */
    /* === GUI Keyboard setup === */
    /* -------------------------- */
    [[c4Button cell] setBackgroundColor:[NSColor whiteColor]];
    [[d4Button cell] setBackgroundColor:[NSColor whiteColor]];
    [[e4Button cell] setBackgroundColor:[NSColor whiteColor]];
    [[f4Button cell] setBackgroundColor:[NSColor whiteColor]];
    [[g4Button cell] setBackgroundColor:[NSColor whiteColor]];
    [[a4Button cell] setBackgroundColor:[NSColor whiteColor]];
    [[b4Button cell] setBackgroundColor:[NSColor whiteColor]];
    [[c5Button cell] setBackgroundColor:[NSColor whiteColor]];
    
    [[cS4Button cell] setBackgroundColor:[NSColor blackColor]];
    [[dS4Button cell] setBackgroundColor:[NSColor blackColor]];
    [[fS4Button cell] setBackgroundColor:[NSColor blackColor]];
    [[gS4Button cell] setBackgroundColor:[NSColor blackColor]];
    [[aS4Button cell] setBackgroundColor:[NSColor blackColor]];
    
    /* ------------------- */
    /* === Audio setup === */
    /* ------------------- */
    masterVoice = new SynthVoice();
    synth = new PolySynth(masterVoice, 0);  // No voices until we set the number of audio channels
    audioController = new AudioController(synth);
    
    /* Populate output device list */
    std::vector<const PaDeviceInfo*> audioDevs = audioController->getAvailableOutputDevices();
    for (int i = 0; i < audioDevs.size(); i++) {
        NSString* devName = [NSString stringWithFormat:@"%s", audioDevs[i]->name];
        [outputDeviceSelector addItemWithTitle:devName];
        
        if ([devName isEqualToString:kDefaultAudioOutputDeviceName]) {
            [outputDeviceSelector selectItemAtIndex:i];
        }
    }
    
    [self setOutputDevice:self];
    
    /* ------------------ */
    /* === MIDI setup === */
    /* ------------------ */
    midiController = new MidiController(synth);
    
    /* Populate the input devices list */
    [midiInputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"None"]];
    std::vector<std::string> midiDevs = midiController->getInputDeviceNames();
    for (int i = 0; i < midiDevs.size(); i++) {
        [midiInputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"%s", midiDevs[i].c_str()]];
    }
    /* Pick the first device on the list if one exists */
    if ([midiInputDeviceSelector numberOfItems] > 1)
        [midiInputDeviceSelector selectItemAtIndex:1];
    [self setMIDIInputDevice:self];
    
    /* Populate the output devices list */
    midiDevs = midiController->getOutputDeviceNames();
    [midiOutputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"None"]];
    for (int i = 0; i < midiDevs.size(); i++) {
        [midiOutputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"%s", midiDevs[i].c_str()]];
    }
    
    [midiOutputDeviceSelector selectItemWithTitle:@"None"]; // No output device by default
    
    /* Rescan for new devices on a 5-second timer */
    [NSTimer scheduledTimerWithTimeInterval:5.0
                                     target:self
                                   selector:@selector(rescanMIDIDevices)
                                   userInfo:nil
                                    repeats:true];
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    
    if (audioController)
        delete audioController;
    if (midiController)
        delete midiController;
    if (masterVoice)
        delete masterVoice;
    if (synth)
        delete synth;
}

#pragma mark - Audio Parameter Events
- (IBAction)setOutputDevice:(id)sender {
    
    int devIdx = (int)[outputDeviceSelector indexOfSelectedItem];
    
    if (!audioController->setOutputDevice(devIdx)) {
        [outputNumChannelsSelector setEnabled:false];
        [outputSampleRateSelector setEnabled:false];
        [startStopButton setEnabled:false];
    }
    
    std::vector<const PaDeviceInfo*> devices = audioController->getAvailableOutputDevices();
    
    /* Populate the number of output channels list */
    int maxNumChannels = devices[devIdx]->maxOutputChannels;
    for (int i = 1; i <= maxNumChannels; i++) {
        [outputNumChannelsSelector addItemWithTitle:[NSString stringWithFormat:@"%d", i]];
        
        if (i == kDefaultAudioOutputNumChannels) {
            [outputNumChannelsSelector selectItemAtIndex:i-1];
        }
    }
    
    /* Set the number of output channels to the default */
    [self setOutputNumChannels:self];
    
    /* Popuate sample rate list */
    std::vector<float> sampleRates = audioController->getSupportedOutputSampleRates(devIdx);
    for (int i = 0; i < sampleRates.size(); i++) {
        [outputSampleRateSelector addItemWithTitle:[NSString stringWithFormat:@"%.0f", sampleRates[i]]];
        
        if (sampleRates[i] == kDefaultAudioOutputSampleRate) {
            [outputSampleRateSelector selectItemAtIndex:i];
        }
    }
    
    /* Set the output sample rate to the default */
    [self setOutputSampleRate:self];
    
//    if (audioController->isRendering())
//        audioController->stopAudioRender();
//    
//    if (audioController->streamIsOpen())
//        audioController->closeStream();
    
    /* Open the stream */
    if (!audioController->openStream()) {
        [startStopButton setEnabled:false];
    }
}

- (IBAction)setOutputNumChannels:(id)sender {
    
    if (!audioController->setNumOutputChannels([[[outputNumChannelsSelector selectedItem] title] intValue])) {
        [outputSampleRateSelector setEnabled:false];
        [startStopButton setEnabled:false];
    }
}

- (IBAction)setOutputSampleRate:(id)sender {
    
    if (!audioController->setOutputSampleRate([[[outputSampleRateSelector selectedItem] title] floatValue])) {
        [startStopButton setEnabled:false];
    }
}

- (IBAction)toggleOutput:(id)sender {
    
    if ([sender state] == NSOnState) {
        if (!audioController->startAudioRender()) {
            Pa_Terminate();
            [startStopButton setEnabled:false];
        }
        else [startStopButton setTitle:@"Stop"];
    }
    else {
        if (!audioController->stopAudioRender()) {
            Pa_Terminate();
            [startStopButton setEnabled:false];
        }
        else [startStopButton setTitle:@"Start"];
    }
}

#pragma mark - MIDI Parameter Events
- (IBAction)setMIDIInputDevice:(id)sender {
    
    if (midiController->setInputDevice((int)[midiInputDeviceSelector indexOfSelectedItem]-1)) {
        
    }
    else {
        
    }
}

- (IBAction)setMIDIOutputDevice:(id)sender {
    
    if (midiController->setOutputDevice((int)[midiOutputDeviceSelector indexOfSelectedItem]-1)) {
        
    }
    else {
        
    }
}

- (void)rescanMIDIDevices {
    
    std::vector<std::string> midiDevs;
    
    /* If the number of input devices has changed */
    if (midiController->rescanInputDevices()) {
        
        midiDevs = midiController->getInputDeviceNames();
        
        /* Automatically set the first device as the MIDI input device if we went from zero available devices to one */
        bool selectFirst = midiDevs.size() == 1 && [midiInputDeviceSelector numberOfItems] == 1;
        
        [midiInputDeviceSelector removeAllItems];
    
        /* Populate the input devices list */
        [midiInputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"None"]];
        for (int i = 0; i < midiDevs.size(); i++) {
            [midiInputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"%s", midiDevs[i].c_str()]];
        }
        
        if (selectFirst) {
            [midiInputDeviceSelector selectItemAtIndex:1];
            [self setMIDIInputDevice:self];
        }
        
    }
    
    /* If the number of output devices has changed */
    if (midiController->rescanOutputDevices()) {
        
        [midiOutputDeviceSelector removeAllItems];
        
        /* Populate the output devices list */
        midiDevs = midiController->getOutputDeviceNames();
        [midiOutputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"None"]];
        for (int i = 0; i < midiDevs.size(); i++) {
            [midiOutputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"%s", midiDevs[i].c_str()]];
        }
    }
}

#pragma mark - GUI Keyboard Events
- (IBAction)blackKeyEvent:(NSButton*)sender {
    
    int midiNum = (int)[sender tag];
    bool noteOn = [sender state] == NSOnState;
    
    if (noteOn) {
        synth->noteOn(midiNum, 127);
        [[sender cell] setBackgroundColor:[NSColor darkGrayColor]];
    }
    else {
        synth->noteOff(midiNum);
        [[sender cell] setBackgroundColor:[NSColor blackColor]];
    }
    
    NSLog(@"MIDI Note %d %s", midiNum, noteOn ? "ON" : "OFF");
}

- (IBAction)whiteKeyEvent:(NSButton*)sender {
    
    int midiNum = (int)[sender tag];
    bool noteOn = [sender state] == NSOnState;
    
    if (noteOn) {
        synth->noteOn(midiNum, 127);
        [[sender cell] setBackgroundColor:[NSColor lightGrayColor]];
    }
    else {
        synth->noteOff(midiNum);
        [[sender cell] setBackgroundColor:[NSColor whiteColor]];
    }
    
    NSLog(@"MIDI Note %d %s", midiNum, noteOn ? "ON" : "OFF");
}

@end
