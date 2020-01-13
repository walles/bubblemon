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

  private func shouldBubbleInTouchBar() -> Bool {
    let alert = NSAlert()
    alert.messageText = "Run Bubblemon in the Touch Bar?"
    alert.alertStyle = NSAlert.Style.informational
    alert.addButton(withTitle: "Yes")
    alert.addButton(withTitle: "No")
    return alert.runModal() == NSApplication.ModalResponse.alertFirstButtonReturn
  }

  private func setEnabled(enable: Bool) {
    let result = SMLoginItemSetEnabled(
      "com.gmail.walles.johan.bubblemon.TouchBarHelper" as CFString,
      enable)
    NSLog("%@ TouchBar Bubblemon %@",
          enable ? "Enabling" : "Disabling",
          result ? "succeeded" : "failed")
  }

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    NSLog("Bubblemon Touch Bar launcher started")

    // Disabling here enables us to restart Bubblemon if it's running, and
    // upgrade it if there's an old version running. Checking which version is
    // running, if any, would be better, but I don't know how to do that.
    // Anybody?
    setEnabled(enable: false)

    let shouldBubble = shouldBubbleInTouchBar()
    if (shouldBubble) {
      setEnabled(enable: shouldBubble)
    }

    // Now that we've executed the user's wish our presence is not needed any more
    NSApplication.shared.terminate(self)
  }
}
