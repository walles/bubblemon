Summary: GNOME Panel Bubbling Load Monitor
Name: bubblemon
Version: 0.1.0
Release: 1
Source: bubblemon-0.1.0.tgz
Buildroot: /tmp/bubblemon-0.1.0
Packager: Johan Walles (d92-jwa@nada.kth.se)
URL: -
Copyright: GPL
Group: User Interface/GNOME Applets
Requires: gnome-libs >= 1.0

%description
This is a panel applet that displays the CPU and memory load as a
bubbling liquid.  It is based heavily on Merlin's CPU Fire Applet
available at http://nitric.com/freeware.
Choose Add Applet->Utility->Bubbling Load Monitor in your GNOME Panel.

%changelog

* Wed Dec 22 1999 Johan Walles <d92-jwa@nada.kth.se>

- Started reworking code into a CPU+memory monitoring bubbling-liquid applet

* Sun Nov 28 1999 Merlin Hughes <merlin@merlin.org>

- Wrote and released it.
 
%prep

%setup

%build

make mode=release

%install

rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT/usr etcdir=$RPM_BUILD_ROOT/etc install

%clean

rm -rf $RPM_BUILD_ROOT

%files 

/usr/bin/bubblemon_applet
/etc/CORBA/servers/bubblemon_applet.gnorba
/usr/share/applets/Utility/bubblemon_applet.desktop

%doc AUTHORS CHANGES INSTALL LICENSE README URL VERSION
