import Cocoa

@available(OSX 10.12.2, *)
@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  @IBOutlet weak var window: NSWindow!

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    // Tell macOS we're going to be touchbaring
    NSApplication.shared.isAutomaticCustomizeTouchBarMenuItemEnabled = true
  }

  func applicationWillTerminate(_ aNotification: Notification) {
    // Insert code here to tear down your application
  }
}
