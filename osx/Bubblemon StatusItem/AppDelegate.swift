import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  var statusItem: NSStatusItem?

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    // Inspiration: https://www.appcoda.com/macos-status-bar-apps/
    statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
    statusItem?.button?.title = "Johan"
  }
}
