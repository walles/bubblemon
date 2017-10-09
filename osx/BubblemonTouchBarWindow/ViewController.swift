import Cocoa

class ViewController: NSViewController {
}

extension NSTouchBarItemIdentifier {
  static let touchBarBubbler =
    NSTouchBarItemIdentifier("com.gmail.walles.johan.bubblemon.TouchBarBubbler")
}
extension NSTouchBarCustomizationIdentifier {
  static let touchBarBubbler =
    NSTouchBarCustomizationIdentifier("com.gmail.walles.johan.bubblemon.TouchBarBubbler")
}

// MARK: - TouchBar Delegate

@available(OSX 10.12.2, *)
extension ViewController: NSTouchBarDelegate {
  override func makeTouchBar() -> NSTouchBar? {
    let touchBar = NSTouchBar()
    touchBar.delegate = self
    touchBar.customizationIdentifier = .touchBarBubbler
    touchBar.defaultItemIdentifiers = [.touchBarBubbler]
    touchBar.customizationAllowedItemIdentifiers = [.touchBarBubbler]
    return touchBar
  }

  func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItemIdentifier) -> NSTouchBarItem? {
    switch identifier {
    case NSTouchBarItemIdentifier.touchBarBubbler:
      let customViewItem = NSCustomTouchBarItem(identifier: identifier)

      let bubblemonView = BubblemonView(frame: NSMakeRect(
        0, 0,
        // FIXME: Put the correct values here, these 50s are both made up. What should we use?
        50, 50))
      bubblemonView.setTouchBarMode(true)
      customViewItem.view = bubblemonView

      return customViewItem
    default:
      return nil
    }
  }
}
