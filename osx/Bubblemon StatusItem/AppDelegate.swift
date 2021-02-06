import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  var statusItem: NSStatusItem?
  @IBOutlet weak var menu: NSMenu?

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    // Inspiration: https://www.appcoda.com/macos-status-bar-apps/
    statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
    statusItem?.button?.title = "Johan"

    if let menu = menu {
      statusItem?.menu = menu
    }
  }
}
