These instructions will tell you how to make an installation package starting
only from source code, and how to install that package.

These instructions have been tested on macOS 10.12.6 El Capitan with Xcode
version 9.0.1 (9A1004).


Developing
----------
To debug the visualization, the *Bubblemon Window* target is debugger friendly.

To debug the Touch Bar flavor, the *Touch Bar Window* target is debugger friendly.

What actually goes into [the release zip](github.com/walles/bubblemon/releases/latest) is:
* *Release*
* *Bubblemon TouchBar*


Making a Release
----------------
* `git tag | cat` to show previous releases; release tags are the ones named
osx-versionnumber.

* `git tag --annotate osx-1.2.3` to tag version `1.2.3`. Remember that the text
you write will become readable by end users on the download page, write
something nice!

* Run `./osx/makeDistZip.sh` to get a release zip file

* `git push --tags`

* Go to https://github.com/walles/bubblemon/releases and make a new release for
  the version you just tagged and built. Don't forget to upload the zip file.


Installing
----------
* Double click the zip file in the Finder. This will create
  Bubblemon.app.

* Right click Bubblemon and choose *Open*.

* Say yes to the security question (if any).

Bubblemon should now be running in your Dock.

Right click Bubblemon in the Dock and choose *Help* if you have questions.
