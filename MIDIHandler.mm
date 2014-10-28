//
//  MIDIHandler.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/14/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "MIDIHandler.h"

#pragma mark - RtMidi Input Callback
static void midiInputCallback(double deltaTime, std::vector<unsigned char> *message, void *userData) {
    
//    unsigned long nBytes = message->size();
//    for ( unsigned int i=0; i<nBytes; i++ )
//        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
//    if ( nBytes > 0 )
//        std::cout << "stamp = " << deltaTime << std::endl;
    
    MIDIHandler *midi = (__bridge MIDIHandler *)userData;
    
    if (message->size() == 0)
        return;
    
    int statusByte = (int)message->at(0);
    int messageType = statusByte & 0xF0;
    int midiChannel = statusByte & 0x0F;
    
    int byte2, byte3;
    std::vector<std::pair<std::string, float> > updatedParams;
    
    switch (messageType) {
            
        case kMESSAGE_NOTEON:
            
            byte2 = (int)message->at(1);
            byte3 = (int)message->at(2);
            
            printf("Note ON: %2x %2x %2x\n", statusByte, byte2, byte3);
            if (message->at(2) == 0)
                [midi synth]->noteOff(byte2);
            
            else {
                /* PolySynth::noteOn() returns the index of the audio channel allocated for this note. Use it and the MIDI note number to send the MRP routing message after allocation. */
                int channel = [midi synth]->noteOn(byte2, byte3);
                [midi sendMRPRoutingMessage:channel string:byte2-21];
            }
            break;
            
        case kMESSAGE_CONTROL_CHANGE:
            
            printf("Control: %2x %2x %2x\n", statusByte, message->at(1), message->at(2));
            
            /* Forward the control message to PolySynth::handleMidiControl(), which returns a vector<pair<string, int>> with the name and updated value of any parameters that were updated. Use this vector to update the UI accordingly via the SynthVoiceMappingDelegate protocol */
            updatedParams = [midi synth]->handleMidiControl(message);
            for (int i = 0; i < updatedParams.size(); i++) {
                
                if ([midi voiceViewController] != nil) {
                    if ([[midi voiceViewController] respondsToSelector:@selector(updateControlForParameterName:value:)])
                        [[midi voiceViewController] updateControlForParameterName:[NSString stringWithFormat:@"%s", updatedParams[i].first.c_str()] value:updatedParams[i].second];
                }
            }
            break;
            
        case kMESSAGE_PITCHWHEEL:
            
            printf("Pitch Wheel: %2x %2x %2x\n", statusByte, message->at(1), message->at(2));
            [midi synth]->handleMidiControl(message);
            break;
            
        case kMESSAGE_AFTERTOUCH_CHANNEL:
            
            break;
            
        default:
            break;
    }
    
    if (messageType == kMESSAGE_CONTROL_CHANGE ||
        messageType == kMESSAGE_AFTERTOUCH_CHANNEL ||
        messageType == kMESSAGE_PITCHWHEEL ||
        messageType == kMESSAGE_AFTERTOUCH_POLY) {
        
        [[midi mappingViewController] fillMIDIParamsForSelectedCells:message];
    }
    
    return;
}

@implementation MIDIHandler

@synthesize isRunning = _isRunning;
@synthesize mappingViewController;
@synthesize voiceViewController;

- (id)initWithPolySynth:(PolySynth *)synth {
    
    self = [super init];
    if (self) {
        _synth = synth;
        _isRunning = NO;
    
        try {
            _midiIn = new RtMidiIn();
        }
        catch (RtMidiError &error) {
            printf("%s: ", __PRETTY_FUNCTION__);
            error.printMessage();
            return nil;
        }
        
        try {
            _midiOut = new RtMidiOut();
        }
        catch (RtMidiError &error) {
            printf("%s: ", __PRETTY_FUNCTION__);
            error.printMessage();
            return nil;
        }
        
        if (![self rescanInputDevices])
            printf("%s: No MIDI input devices\n", __PRETTY_FUNCTION__);
        if (![self rescanOutputDevices])
            printf("%s: No MIDI output devices\n", __PRETTY_FUNCTION__);
    }
    
    return self;
}

- (PolySynth *)synth {
    return _synth;
}

/* Rescan for changes in available devices list. Return true if number of devices has changed since the list was last populated */
- (bool)rescanInputDevices {
    
    if ((int)_midiIn->getPortCount() == _inputDeviceNames.size())
        return false;
    
    _inputDeviceNames.clear();
    
    for (int i = 0; i < _midiIn->getPortCount(); i++) {
        _inputDeviceNames.push_back(_midiIn->getPortName(i));
    }
    
    return true;
}

- (bool)rescanOutputDevices {
    
    if ((int)_midiOut->getPortCount() == _outputDeviceNames.size())
        return false;
    
    _outputDeviceNames.clear();
    
    for (int i = 0; i < _midiOut->getPortCount(); i++) {
        _outputDeviceNames.push_back(_midiIn->getPortName(i));
    }
    
    return true;
}

- (bool)setInputDevice:(int)devIdx {
    
    /* Make sure we have a valid device index */
    if (devIdx < 0 || devIdx >= _inputDeviceNames.size()) {
        printf("%s: Invalid device index %d. %lu available MIDI input devices.\n", __PRETTY_FUNCTION__, devIdx, _inputDeviceNames.size());
        return false;
    }
    
    /* Open the device */
    try {
        _midiIn->openPort(devIdx);
    }
    catch (RtMidiError &error) {
        printf("%s: ", __PRETTY_FUNCTION__);
        error.printMessage();
        return false;
    }
    
    _midiIn->setCallback(&midiInputCallback, (__bridge void *)self);
    _midiIn->ignoreTypes(true, true, true);  // Ignore sysex, timing, and active sensing messages
    
    printf("%s: Using MIDI input device %s\n", __PRETTY_FUNCTION__, _inputDeviceNames[devIdx].c_str());
    _isRunning = YES;
    
    return true;
}

- (bool)openSecondaryInputDevice:(int)devIdx {
    
    /* Make sure we have a valid device index */
    if (devIdx < 0 || devIdx >= _inputDeviceNames.size()) {
        printf("%s: Invalid device index %d. %lu available MIDI input devices.\n", __PRETTY_FUNCTION__, devIdx, _inputDeviceNames.size());
        return false;
    }
    
    try {
        _midiInputs.push_back(new RtMidiIn());
    }
    catch (RtMidiError &error) {
        printf("%s: ", __PRETTY_FUNCTION__);
        error.printMessage();
        return nil;
    }
    
    /* Open the device */
    try {
        _midiInputs.back()->openPort(devIdx);
    }
    catch (RtMidiError &error) {
        printf("%s: ", __PRETTY_FUNCTION__);
        error.printMessage();
        return false;
    }
    
    _midiInputs.back()->setCallback(&midiInputCallback, (__bridge void *)self);
    _midiInputs.back()->ignoreTypes(true, true, true);  // Ignore sysex, timing, and active sensing messages
    
    return true;
}

- (bool)setOutputDevice:(int)devIdx {
    
    /* Make sure we have a valid device index */
    if (devIdx < 0 || devIdx >= _outputDeviceNames.size()) {
        printf("%s: Invalid device index %d. %lu available MIDI output devices.\n", __PRETTY_FUNCTION__, devIdx, _outputDeviceNames.size());
        return false;
    }
    
    /* Open the device */
    try {
        _midiOut->openPort(devIdx);
    }
    catch (RtMidiError &error) {
        printf("%s: ", __PRETTY_FUNCTION__);
        error.printMessage();
        return false;
    }
    
    printf("%s: Using MIDI output device %s\n", __PRETTY_FUNCTION__, _outputDeviceNames[devIdx].c_str());
    
    return true;
}

- (std::vector<std::string>)getInputDeviceNames {
    return _inputDeviceNames;
}

- (std::vector<std::string>)getOutputDeviceNames {
    return _outputDeviceNames;
}

- (bool)sendMRPRoutingMessage:(int)channel string:(int)string {
    
    /* Make sure the RtMidiOut instance exists */
    if (_midiOut == NULL) {
        printf("%s: Error sending MRP routing message. No RtMidiOut instance exists.\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    /* Make sure an output port is open */
    if (!_midiOut->isPortOpen()) {
        printf("%s: Error sending MRP routing message. No MIDI output port is open.\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    /* Construct MRP routing message */
    vector<unsigned char> message;
    message.push_back(kMESSAGE_CONTROL_CHANGE | (unsigned char)kMRPMIDIOutputChannel);
    message.push_back(kCONTROL_MRP_BASE + (unsigned char)channel);
    message.push_back((unsigned char)(string & 0xFF));
    
    //    printf("%s: Sending MRP routing message %u %u %u\n", __PRETTY_FUNCTION__, message[0], message[1], message[2]);
    
    /* Send the message */
    try {
        _midiOut->sendMessage(&message);
    }
    catch(RtMidiError& err) {
        printf("%s: Error sending MRP routing message. RtMidiError: ", __PRETTY_FUNCTION__);
        err.printMessage();
        printf("\n");
        return false;
    }
    
    return true;
}


@end











