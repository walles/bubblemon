//
//  AppDelegate.swift
//  Bubblemon TouchBar
//
//  Created by Johan Walles on 2017-10-20.
//

import Cocoa
import ServiceManagement

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {

  @IBOutlet weak var window: NSWindow!

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    if (SMLoginItemSetEnabled("com.gmail.walles.johan.bubblemon.TouchBarHelper" as CFString, true)) {
      NSLog("TouchBar Bubblemon started")
    } else {
      NSLog("TouchBar Bubblemon failed to start")
    }
  }
}
