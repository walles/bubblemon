@import Foundation;

#include "applePrivateHelpers.h"

// See: https://github.com/a2/touch-baer
extern void DFRSystemModalShowsCloseBoxWhenFrontMost(BOOL);
extern void DFRElementSetControlStripPresenceForIdentifier(NSString *string, BOOL enabled);

@interface NSTouchBarItem ()
+ (void)addSystemTrayItem:(NSTouchBarItem *)item;
@end

@interface NSTouchBar ()
+ (void)presentSystemModalFunctionBar:(NSTouchBar *)touchBar systemTrayItemIdentifier:(NSString *)identifier;
@end

void controlStrippify(NSView *view, NSString *identifier) {
  if (@available(macOS 10.12.2, *)) {
    DFRSystemModalShowsCloseBoxWhenFrontMost(YES);

    NSCustomTouchBarItem *touchBarItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
    touchBarItem.view = view;
    [NSTouchBarItem addSystemTrayItem:touchBarItem];
    DFRElementSetControlStripPresenceForIdentifier(identifier, YES);
    NSLog(@"Done adding View to the Touch Bar");
  } else {
    NSLog(@"Touch Bar support not available until macOS 10.12.2, please upgrade");
  }
}
