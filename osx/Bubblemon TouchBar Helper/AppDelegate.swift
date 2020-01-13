import Cocoa

@available(OSX 10.12.2, *)
@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  @IBOutlet weak var window: NSWindow!

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    let bubblemonView = BubblemonView(frame: NSMakeRect(
      0, 0,
      // FIXME: Put the correct values here, these 50s are both made up. What should we use?
      50, 50))
    bubblemonView.setTouchBarMode(true)

    NSLog("Adding Bubblemon to the Touch Bar...")
    controlStrippify(bubblemonView, "com.gmail.walles.johan.bubblemon.TouchBarBubbler")
    NSLog("Done adding Bubblemon to the Touch Bar")
  }

  func applicationWillTerminate(_ aNotification: Notification) {
    // Insert code here to tear down your application
  }
}
