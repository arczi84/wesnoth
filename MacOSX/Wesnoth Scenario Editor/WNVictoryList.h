/* WNVictoryList */

#import <Cocoa/Cocoa.h>

@interface WNVictoryList : NSObject
{
}
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row;
- (void)tableView:(NSTableView *)aTableView 
	setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex;
@end
