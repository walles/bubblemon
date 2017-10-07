//
//  AppDelegate.swift
//  Bubblemon TouchBar
//
//  Created by Johan Walles on 2017-10-06.
//

import Cocoa

// FIXME: Move these to a file common between Bubblemon TouchBar and BubblemonTouchBarWindow?
extension NSTouchBarItemIdentifier {
  static let touchBarBubbler =
    NSTouchBarItemIdentifier("com.gmail.walles.johan.bubblemon.TouchBarBubbler")
}
extension NSTouchBarCustomizationIdentifier {
  static let touchBarBubbler =
    NSTouchBarCustomizationIdentifier("com.gmail.walles.johan.bubblemon.TouchBarBubbler")
}

@available(OSX 10.12.2, *)
@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  @IBOutlet weak var window: NSWindow!

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    print("Johan says hello!")

    let bubblemonView = BubblemonView(frame: NSMakeRect(
      0, 0,
      // FIXME: Put the correct values here, these 50s are both made up. What should we use?
      50, 50))
    bubblemonView.setTouchBarMode(true)

    controlStrippify(bubblemonView, "com.gmail.walles.johan.bubblemon.TouchBarBubbler")
  }

  func applicationWillTerminate(_ aNotification: Notification) {
    // Insert code here to tear down your application
    print("Johan says goodbye!")
  }
}
