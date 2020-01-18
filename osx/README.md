These instructions will tell you how to make an installation package starting
only from source code, and how to install that package.

These instructions have been tested on macOS 10.12.6 El Capitan with Xcode
version 9.0.1 (9A1004).


Developing
----------
To debug the visualization, the *Bubblemon Window* target is debugger friendly.

To debug the Touch Bar flavor, the *Touch Bar Window* target is debugger friendly.

What actually goes into [the dist archives](https://github.com/walles/bubblemon/releases/latest) is:
* *Release*
* *Bubblemon TouchBar*


Making a Release
----------------
* `git tag | cat` to show previous releases; release tags are the ones named osx-versionnumber.

* `git tag --annotate osx-1.2.3` to tag version `1.2.3`. Remember that the text
you write will become readable by end users on the download page, write
something nice! Also, the first line is a heading, write something heady!

* Run `./osx/makeDist.sh` to get a release zip file

* Go to <https://github.com/walles/bubblemon/releases> and make a new release for
  the version you just tagged and built. Don't forget to upload the dist files.


Installing the Dockapp
----------------------
* Double click the dmg file in the Finder to mount it
* Drag Bubblemon into the *Applications* folder
* Double click the *Applications* folder to open it
* Right click Bubblemon and choose *Open*
* Say yes to the security question (if any)

Bubblemon should now restart the Dock to install itself.

Right click Bubblemon in the Dock and choose *Help* if you have questions.

Left click Bubblemon in the Dock to launch Activity Monitor.


Installing Bubblemon on the Touch Bar
-------------------------------------
* `unzip dist/bubblemon-touchbar-*.zip`
* `open bubblemon-touchbar-*/"Bubblemon TouchBar.app"`


TODO
====
* If `$OLDVERSION` is running and `$NEWVERSION` is launched, make sure
  `$NEWVERSION` replaces $OLDVERSION in the TouchBar. Maybe by adding the
  version number to the plugin directory's file name?

* If Bubblemon.app has a file system timestamp later than when the
  Dock process started, then we should replace the running Bubblemon
  with ourselves. Nice during development.

* Improve install instructions, make sure there's a step-by-step list in the
  `.dmg` that anybody could follow. See Installing the Dockapp above.

* TouchBar: Clicking Bubblemon should bring up the Activity Monitor

* TouchBar: Consider making the air transparent and changing the color scheme
 to better blend in with the other things on the TouchBar

* CI: Make sure CI fails if the code analysis finds problems

* Make the frame thinner and glossier

* Somehow make the Travis CI build run the self-checks that are built
  into Bubblemon.

* Think about using NSDefaults to query the Dock about our tile
  size. Maybe not every frame (for performance reasons), but every
  five seconds or so.


WONTFIX
-------
* Sign the app to remove the unknown-developer warning and to enable
  people to just double-click after download. This would cost me $99
  per year, which is just plain silly:
  https://developer.apple.com/support/compare-memberships/

* Enable all warnings and warnings-as-errors.
  Don't know how, have asked:
  http://stackoverflow.com/q/26624470/473672

DONE
----
* Point the old web page to http://walles.github.io/bubblemon.

* Make sure http://walles.github.io/bubblemon has a link to
  https://github.com/walles/bubblemon/releases.

* Release zip files should go into ~/Downloads.

* Unzipping bubblemon-osx-1234.zip should give you the app in
  bubblemon-osx-1234/Bubblemon.app.

* Fix the copyright information you get if you right-click
  Bubblemon.app in the Finder and do "Get Info". It should be the same
  as what you get in the About box.

* Profile running on a really slow system

* Enable easy upgrades. If some other Bubblemon is running in the
  Dock, we should replace it with ourselves.

* Set up a Travis CI job analyzing macOS Debug for all pull requests and
  commits.

* Make sure Travis CI fails on Analyze warnings.

* Not our fault, didn't do anything: Deal with the timer syslog
  message that shows up every time Bubblemon has to install itself.

* Make a 1024x1024 app icon. This is a requirement for App Store
  release, and while releasing to App Store won't be possible we
  should follow their guidelines as well as we can. The icon should
  show up in the About box and when you look at Bubblemon.app in
  Finder.

* TouchBar: Make sure the .app has our icon

* TouchBar: Make sure the .app has the correct version when doing cmd-i in
 Finder

* TouchBar: Make sure the .app has the correct copyright when doing cmd-i in
 Finder

* TouchBar: Position the Bubblemon so it fills its allotted slot perfectly

* TouchBar: Figure out how to survive reboots

* TouchBar: When re-launched, pop up a dialog asking whether we should stay on
 the TouchBar or remove ourselves

* TouchBar: Add the TouchBar Bubblemon to the release zip file

* TouchBar: Give more pixels to the visualization by removing the frame

* TouchBar: Release a new Bubblemon with TouchBar support
