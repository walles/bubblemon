//
//  Bubblemon-Bridging-Header.h
//  bubblemon
//
//  Created by Johan Walles on 2017-09-08.
//
//

#ifndef Bubblemon_Bridging_Header_h
#define Bubblemon_Bridging_Header_h

#include "bubblemon.h"

//
//
// vvvv  Start of Apple-private API declarations  vvvv
//
//

// See: https://github.com/a2/touch-baer/blob/master/TouchBarTest/TouchBar.h

#import <AppKit/AppKit.h>

extern void DFRElementSetControlStripPresenceForIdentifier(NSString *, BOOL);
extern void DFRSystemModalShowsCloseBoxWhenFrontMost(BOOL);

@interface NSTouchBarItem ()
+ (void)addSystemTrayItem:(NSTouchBarItem *)item;
@end

@interface NSTouchBar ()
+ (void)presentSystemModalFunctionBar:(NSTouchBar *)touchBar systemTrayItemIdentifier:(NSString *)identifier;
@end

//
//
// ^^^^  End of Apple-private API declarations  ^^^^
//
//

#endif /* Bubblemon_Bridging_Header_h */
