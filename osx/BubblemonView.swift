import Cocoa

class BubblemonView: NSView, NSDockTilePlugIn {
  private var _bubblemon: UnsafeMutablePointer<bubblemon_t>
  private var _picture: UnsafePointer<bubblemon_picture_t>?
  private var _dockTile: NSDockTile?
  private var _dockMenu: NSMenu?
  private var _windowFrame: CGImage?
  private var _scaledWindowFrame: CGLayer?

  private var _touchBarMode: Bool

  func setDockTile(_ dockTile: NSDockTile?) {
    if dockTile != nil {
      _dockTile = dockTile
      _dockTile!.contentView = self
    }
  }

  func dockMenu() -> NSMenu? {
    NSLog("Setting up dock menu")

    if _dockMenu != nil {
      return _dockMenu
    }

    let menu = NSMenu()
    let helpItem = NSMenuItem(title: "Help", action: #selector(self.openLegend), keyEquivalent: "")
    helpItem.target = self
    menu.addItem(helpItem)
    let aboutItem = NSMenuItem(title: "About", action: #selector(self.openAboutPanel), keyEquivalent: "")
    aboutItem.target = self
    menu.addItem(aboutItem)
    let shareItem = NSMenuItem(title: "Share on Facebook", action: #selector(self.shareOnFacebook), keyEquivalent: "")
    shareItem.target = self
    menu.addItem(shareItem)

    // From: https://stackoverflow.com/a/55403087/473672
    let submenu = NSMenu()
    let mainDropdown = NSMenuItem(title: "Debug", action: nil, keyEquivalent: "")
    menu.addItem(mainDropdown)
    menu.setSubmenu(submenu, for: mainDropdown)

    let showNumbersItem = NSMenuItem(title: "Show Numbers", action:
      #selector(self.showLoadNumbers), keyEquivalent: "")
    showNumbersItem.target = self
    submenu.addItem(showNumbersItem)

    _dockMenu = menu
    return _dockMenu
  }

  @IBAction func openLegend(_ sender: Any) {
    NSLog("Opening help in browser...")
    // From: http://lists.apple.com/archives/xcode-users/2016/Feb/msg00111.html
    let url = URL(string: "https://walles.github.io/bubblemon/")
    NSWorkspace.shared.open(url!)
  }

  @IBAction func shareOnFacebook(_ sender: Any) {
    NSLog("Opening browser to share on Facebook...")
    // From: http://lists.apple.com/archives/xcode-users/2016/Feb/msg00111.html
    let url = URL(string: "https://www.facebook.com/sharer/sharer.php?u=http%3A//walles.github.io/bubblemon/")
    NSWorkspace.shared.open(url!)
  }

  @IBAction func openAboutPanel(_ sender: Any) {
    let credits = NSAttributedString(
      string: "https://walles.github.io/bubblemon",
      attributes: [
        NSAttributedString.Key.link: "https://walles.github.io/bubblemon"
      ]
    )

    // The Git hash and version get filled in by a "Run Script" build step
    let bundle = Bundle(for: BubblemonView.self)
    let version = bundle.infoDictionary?["CFBundleShortVersionString"] as? String
    let gitHash = bundle.infoDictionary?["GitHash"] as? String
    let icon = bundle.image(forResource: "icon.png.icns")
    let aboutOptions: [NSApplication.AboutPanelOptionKey: Any] = [
      NSApplication.AboutPanelOptionKey.applicationName: "Bubblemon",
      NSApplication.AboutPanelOptionKey.applicationVersion: version!,
      NSApplication.AboutPanelOptionKey.version: gitHash!,
      NSApplication.AboutPanelOptionKey.applicationIcon: icon!,
      NSApplication.AboutPanelOptionKey.credits: credits,
    ]
    NSApplication.shared.activate(ignoringOtherApps: true)
    NSApplication.shared.orderFrontStandardAboutPanel(options: aboutOptions)
  }

  @IBAction func showLoadNumbers(_ sender: Any) {
    let alert = NSAlert()

    alert.messageText = String(cString: bubblemon_getTooltip(_bubblemon)!)

    alert.alertStyle = NSAlert.Style.informational
    alert.addButton(withTitle: "OK")

    // This might freeze the physics, but give us the numbers. Call it an MVP!
    alert.runModal()
  }

  override init(frame: NSRect) {
    _touchBarMode = false

#if DEBUG
    bubblemon_selftest()
#endif
    _bubblemon = bubblemon_init()
    _picture = nil
    // Load window frame graphics
    let bundle = Bundle(for: BubblemonView.self)
    let windowFrameUrl = bundle.urlForImageResource("window-frame")
    let windowFrameNsUrl = NSURL(string: (windowFrameUrl?.absoluteString)!)

    let dataProvider = CGDataProvider(url: windowFrameNsUrl!)
    _windowFrame = CGImage(
      pngDataProviderSource: dataProvider!,
      decode: nil,
      shouldInterpolate: false,
      intent: CGColorRenderingIntent.defaultIntent)

    super.init(frame: frame)

    // Inspired by http://stackoverflow.com/questions/1449035/how-do-i-use-nstimer
    Timer.scheduledTimer(
      timeInterval: (1.0 / 10.0),
      target: self,
      selector: #selector(self.timerTriggered),
      userInfo: nil,
      repeats: true)
  }

  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }

  func setTouchBarMode(_ touchBarMode: Bool) {
    _touchBarMode = touchBarMode
  }

  func getCachedWindowFrame(baseContext: CGContext) -> CGLayer {
    // Can we use our cached window frame?
    if _scaledWindowFrame == nil {
      // No; we don't have one
    } else if (!bounds.size.equalTo(_scaledWindowFrame!.size)) {
      // No; the size doesn't match
    } else {
      // Yes!
      return _scaledWindowFrame!
    }

    // All of these numbers are because we want to keep the frame width the same
    // all around the frame, even if the frame is wider than it is high.
    //
    // The way we do this is by scaling the width in three parts. The leftmost
    // third is scaled the same in both directions, as is the rightmost third.
    //
    // The middle third of the original image is then scaled to cover the gap in
    // between the right and left parts. x1 and x2 are one and two thirds into
    // the frame image respectively.
    let yFactor = bounds.size.height / CGFloat(_windowFrame!.height)
    let x1before = CGFloat(_windowFrame!.width / 3)
    let x2before = CGFloat((2 * _windowFrame!.width) / 3)
    let middleWidthBefore = x2before - x1before
    let rightWidthBefore = CGFloat(_windowFrame!.width - 1) - x2before
    let heightBefore = CGFloat(_windowFrame!.height)

    let x1after = x1before * yFactor
    let x2after = bounds.size.width - 1 - x1after
    let middleWidthAfter = 1 + x2after - x1after
    let rightWidthAfter = bounds.size.width - 1 - x2after
    let heightAfter = bounds.size.height

    let scaledLayer = CGLayer(baseContext, size: bounds.size, auxiliaryInfo: nil)
    let scaledContext = scaledLayer!.context

    // Draw the left part
    scaledContext!.draw(
      _windowFrame!.cropping(to: CGRect(x: CGFloat(0), y: CGFloat(0), width: x1before, height: heightBefore))!,
      in: CGRect(x: 0, y: 0, width: x1after + 1, height: heightAfter))

    // Draw the middle part
    scaledContext!.draw(
      _windowFrame!.cropping(to: CGRect(
        x: x1before, y: CGFloat(0), width: middleWidthBefore, height: heightBefore))!,
      in: CGRect(x: x1after, y: CGFloat(0), width: middleWidthAfter, height: heightAfter))

    // Draw the right part
    scaledContext!.draw(
      _windowFrame!.cropping(to: CGRect(
        x: x2before, y: 0, width: rightWidthBefore, height: heightBefore))!,
      in: CGRect(x: x2after, y: CGFloat(0), width: rightWidthAfter, height: heightAfter))

    _scaledWindowFrame = scaledLayer!
    return _scaledWindowFrame!
  }

  @objc
  func timerTriggered() {
    if (!self.window!.occlusionState.contains(.visible)) {
      // Ref: https://developer.apple.com/forums/thread/71171
      return
    }

    // Compute a new image to display
    if (_touchBarMode) {
      // Touch Bar is tiny, make few bubbles and scale everything up to be big
      bubblemon_setSize(_bubblemon, 60, 30)
    } else {
      // The Dock won't tell us its size, so this is a guess at roughly how many pixels
      // the bubblemon will get on screen.
      bubblemon_setSize(_bubblemon, 50, 50)
    }
    _picture = bubblemon_getPicture(_bubblemon)
    let tooltip = String(utf8String: bubblemon_getTooltip(_bubblemon))
    toolTip = tooltip
    if _dockTile != nil {
      _dockTile!.display()
    } else {
      needsDisplay = true
    }
  }

  override func setFrameSize(_ newSize: NSSize) {
    if(_touchBarMode) {
      // Landscape mode on the touchbar because it is tiny
      let width = min(newSize.height * 2, newSize.width)
      let height = newSize.height

      // Center ourselves horizontally
      setFrameOrigin(NSPoint.init(x: (newSize.width - width) / 2, y: 0))

      super.setFrameSize(NSSize.init(width: width, height: height))
    } else {
      super.setFrameSize(newSize)
    }
  }

  override func draw(_ dirtyRect: NSRect) {
    if _picture == nil {
      return
    }
    let picture = _picture!.pointee

    // Redraw ourselves
    let imageDataSize = Int(picture.width * picture.height) * MemoryLayout<bubblemon_color_t>.size
    let dataProviderRef = CGDataProvider(
      dataInfo: nil,
      data: picture.pixels,
      size: imageDataSize,
      releaseData: releaseDataProvider)

    let bitsPerComponent: size_t = 8
    let bitsPerPixel = MemoryLayout<bubblemon_color_t>.size * 8
    let bytesPerRow = Int(picture.width) * MemoryLayout<bubblemon_color_t>.size
    let shouldInterpolate: Bool = false
    let rgb = CGColorSpaceCreateDeviceRGB()
    let cgImage = CGImage(
      width: Int(picture.width), height: Int(picture.height),
      bitsPerComponent: bitsPerComponent,
      bitsPerPixel: bitsPerPixel,
      bytesPerRow: bytesPerRow,
      space: rgb,
      bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipLast.rawValue),
      provider: dataProviderRef!,
      decode: nil,
      shouldInterpolate: shouldInterpolate,
      intent: CGColorRenderingIntent.defaultIntent)

    let nsGraphicsContext = NSGraphicsContext.current
    guard let cgContext = nsGraphicsContext?.cgContext else {
      fatalError("unable to get a CGContext, can't draw")
    }

    // Draw the bubblemon image

    // The frame width is the same on all sides, and determined by the height of
    // the image.
    let frameWidth = 0.08 * bounds.size.height

    let bubbleViewRect = CGRect(
      x: frameWidth,
      y: frameWidth,
      width: bounds.size.width - 2.0 * frameWidth,
      height: bounds.size.height - 2.0 * frameWidth)
    cgContext.setAlpha(1.0)
    cgContext.interpolationQuality = .none
    cgContext.draw(cgImage!, in: bubbleViewRect)

    if (!_touchBarMode) {
      // Draw the window frame
      cgContext.setAlpha(1.0)
      cgContext.draw(getCachedWindowFrame(baseContext: cgContext), at: CGPoint(x: 0, y: 0))
    }
  }
}

func releaseDataProvider(_ context:UnsafeMutableRawPointer?, data:UnsafeRawPointer, size:Int) {
  // No need to actually free anything here, the data is handled by
  // the bubblemon code.
}
