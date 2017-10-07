@import Foundation;

#include "applePrivateHelpers.h"

// See: https://github.com/a2/touch-baer
extern void DFRSystemModalShowsCloseBoxWhenFrontMost(BOOL);
extern void DFRElementSetControlStripPresenceForIdentifier(NSString *string, BOOL gris);

@interface NSTouchBarItem ()
+ (void)addSystemTrayItem:(NSTouchBarItem *)item;
@end

@interface NSTouchBar ()
+ (void)presentSystemModalFunctionBar:(NSTouchBar *)touchBar systemTrayItemIdentifier:(NSString *)identifier;
@end

void controlStrippify(NSView *view, NSString *identifier) {
  if (@available(macOS 10.12.2, *)) {
    DFRSystemModalShowsCloseBoxWhenFrontMost(YES);

    NSCustomTouchBarItem *panda = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
    panda.view = view;
    [NSTouchBarItem addSystemTrayItem:panda];
    DFRElementSetControlStripPresenceForIdentifier(identifier, YES);
  } else {
    // FIXME: Fail!
  }
}
