//
//  MidiController.cpp
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 8/5/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "MidiController.h"

MidiController::MidiController(PolySynth* synth) : _synth(synth), _midiIn(NULL), _midiOut(NULL) {

    try {
        _midiIn = new RtMidiIn();
    }
    catch (RtMidiError &error) {
        printf("%s: ", __PRETTY_FUNCTION__);
        error.printMessage();
        return;
    }

    try {
        _midiOut = new RtMidiOut();
    }
    catch (RtMidiError &error) {
        printf("%s: ", __PRETTY_FUNCTION__);
        error.printMessage();
        return;
    }
    
    if (!rescanInputDevices())
        printf("%s: No MIDI input devices\n", __PRETTY_FUNCTION__);
    if (!rescanOutputDevices())
        printf("%s: No MIDI output devices\n", __PRETTY_FUNCTION__);
    
//    _synth->setMidiController(this);
}

MidiController::~MidiController() {
    
    if (_midiIn)
        delete _midiIn;
    if (_midiOut)
        delete _midiOut;
}

#pragma mark - RtMidi Input Callback
void MidiController::midiInputCallback(double deltaTime, std::vector<unsigned char>* message) {
    
//    unsigned long nBytes = message->size();
//    for ( unsigned int i=0; i<nBytes; i++ )
//        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
//    if ( nBytes > 0 )
//        std::cout << "stamp = " << deltaTime << std::endl;
    
    if (message->size() == 0)
        return;
    
    int statusByte = (int)message->at(0);
    int messageType = statusByte & 0xF0;
    int midiChannel = statusByte & 0x0F;
    
//    printf("Status: %x; ", statusByte);
//    printf("Type = %x; ", messageType);
//    printf("Channel = %x\n", midiChannel);
    
    int byte2, byte3;
    
    switch (messageType) {
            
        case MESSAGE_NOTEON:
            
            byte2 = (int)message->at(1);
            byte3 = (int)message->at(2);
            
            printf("Note ON: %2x %2x %2x\n", statusByte, byte2, byte3);
            if (message->at(2) == 0)
                _synth->noteOff(byte2);
            
            else {
                _synth->noteOn(byte2, byte3);
            }
            
            break;
            
        case MESSAGE_CONTROL_CHANGE:
            
            printf("Control: %2x %2x %2x\n", statusByte, message->at(1), message->at(2));
            _synth->handleMidiControl(message);
            
            break;
            
        default:
            break;
    }
    
    
    return;
}

bool MidiController::setInputDevice(int devIdx) {
    
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
    
    _midiIn->setCallback(&staticMidiInputCallback, this);
    _midiIn->ignoreTypes(true, true, true);  // Ignore sysex, timing, and active sensing messages
    
    printf("%s: Using MIDI input device %s\n", __PRETTY_FUNCTION__, _inputDeviceNames[devIdx].c_str());
    
    return true;
}

bool MidiController::setOutputDevice(int devIdx) {
    
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

/* Rescan for changes in available devices list. Return true if number of devices has changed since the list was last populated */
bool MidiController::rescanInputDevices() {
    
    if ((int)_midiIn->getPortCount() == _inputDeviceNames.size())
        return false;
    
    _inputDeviceNames.clear();
    
    for (int i = 0; i < _midiIn->getPortCount(); i++) {
        _inputDeviceNames.push_back(_midiIn->getPortName(i));
    }
    
    return true;
}

bool MidiController::rescanOutputDevices() {
    
    if ((int)_midiOut->getPortCount() == _outputDeviceNames.size())
        return false;
    
    _outputDeviceNames.clear();
    
    for (int i = 0; i < _midiOut->getPortCount(); i++) {
        _outputDeviceNames.push_back(_midiIn->getPortName(i));
    }
    
    return true;
}

#pragma mark - RtMidi Output
bool MidiController::sendMRPRoutingMessage(int channel, int string) {
    
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
    message.push_back(MESSAGE_CONTROL_CHANGE | (unsigned char)kMRPMIDIOutputChannel);
    message.push_back(CONTROL_MRP_BASE + (unsigned char)channel);
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
















