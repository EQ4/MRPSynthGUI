//
//  MappingViewController.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/12/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "MappingViewController.h"

@interface MappingViewController ()

@end

@implementation MappingViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil synth:(PolySynth *)synth
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        _synth = synth;
        mappings = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)loadView {
    
    [super loadView];
//    [mappingsTable setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
}

- (IBAction)addMapping:(NSButton *)sender {
    
    MidiMappingItemViewController *mappingItem = [[MidiMappingItemViewController alloc]  initWithNibName:@"MidiMappingItem" bundle:nil synth:_synth];
    [mappings addObject:mappingItem];
    [mappingsTable reloadData];
}

- (IBAction)removeMapping:(NSButton *)sender {
    
    [[mappings objectAtIndex:[mappingsTable selectedRow]] cancel];
    [mappings removeObjectAtIndex:[mappingsTable selectedRow]];
    [mappingsTable reloadData];
}

#pragma mark - NSTableViewDataSource Methods
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [mappings count];
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row {
    return [[mappings objectAtIndex:row] view].frame.size.height;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {

    NSView *view;
    
    if ([[tableColumn identifier] isEqualToString:@"MainCell"])
        view = [[mappings objectAtIndex:row] view];
    
    if ([[mappingsTable selectedRowIndexes] containsIndex:row]) {
        [[[mappings objectAtIndex:row] bgLayer] setBackgroundColor:[NSColor colorWithCalibratedRed:0.2f green:0.2f blue:0.2f alpha:0.5f].CGColor];
    }
    
    return view;
}

- (void)fillMIDIParamsForSelectedCells:(std::vector<unsigned char> *)message {
    
    NSIndexSet *rows = [mappingsTable selectedRowIndexes];
    [rows enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
        [mappings[idx] fillMIDIParams:message];
        [mappingsTable deselectRow:idx];
    }];
}


@end




















