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

  func isStopped() -> Bool {
    let bubblemons =
      NSRunningApplication.runningApplications(
        withBundleIdentifier: "com.gmail.walles.johan.bubblemon.TouchBarHelper")

    NSLog("TouchBar Bubblemon %@", bubblemons.isEmpty ? "not running" : "running")

    return bubblemons.isEmpty
  }

  func shouldBubbleInTouchBar() -> Bool {
    let alert = NSAlert()
    alert.messageText = "Run Bubblemon in the Touch Bar?"
    alert.alertStyle = NSAlertStyle.informational
    alert.addButton(withTitle: "Yes")
    alert.addButton(withTitle: "No")
    return alert.runModal() == NSAlertFirstButtonReturn
  }

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    let shouldBubble = isStopped() || shouldBubbleInTouchBar()
    let result = SMLoginItemSetEnabled(
      "com.gmail.walles.johan.bubblemon.TouchBarHelper" as CFString,
      shouldBubble)
    NSLog("%@ TouchBar Bubblemon %@",
          shouldBubble ? "Enabling" : "Disabling",
          result ? "succeeded" : "failed")

    // Now that we've executed the user's wish our presence is not needed any more
    NSApplication.shared().terminate(self)
  }
}
