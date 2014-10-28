//
//  MappingViewController.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/12/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MIDIHandler.h"
#import "MappingItemViewController.h"

#include "PolySynth.h"

@interface MappingViewController : NSViewController <NSTableViewDataSource, NSTableViewDelegate, MIDIMappingItemDelegate> {
    
    PolySynth *_synth;
    
    NSMutableArray *mappings;       // Array of MappingItem ViewControllers
    IBOutlet NSTableView *mappingsTable;    // Table to display their views
    
    IBOutlet NSButton *addButton;
    IBOutlet NSButton *removeButton;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth *)synth;

- (void)fillMIDIParamsForSelectedCells:(std::vector<unsigned char> *)message;

@end
