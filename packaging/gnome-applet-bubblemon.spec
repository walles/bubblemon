%define app_name bubblemon
%define platform gnome3

Name:		gnome-applet-%{app_name}
Version:	3.0
Release:	1%{?dist}
Summary:	Bubbling Load Monitoring Applet for the GNOME 3 Panel

URL:		http://savannah.nongnu.org/projects/%{app_name}
Source:		http://download.savannah.gnu.org/releases/%{app_name}/%{app_name}-%{version}.tar.gz
License:	GPLv2+
Group:		User Interface/X
Requires:	gnome-panel >= 3.0

BuildRequires:	gettext
BuildRequires:	gnome-panel-devel >= 3.0
BuildRequires:	gtk3-devel
BuildRequires:	intltool
BuildRequires:	libgtop2-devel


%description
The Bubbling Load Monitor displays CPU and memory load as a bubbling liquid.

- The water level indicates how much memory is in use.
- The color of the liquid indicates how much swap space is used (watery blue
  means none and angry red means all).
- The system CPU load is indicated by bubbles floating up through the liquid;
  lots of bubbles mean high CPU load. On SMP systems CPU load distribution is
  visualized by having the most heavily loaded CPU's bubbles in the middle and
  the others nearer to the edges.
- Seaweed growing up from the bottom indicate IO load; taller means high load.
- If you have unread mail, a message in a bottle falls into the water. 

Choose "Add to Panel" -> "Bubbling Load Monitor" in your GNOME Panel.


%prep
%setup -q -n %{app_name}-%{version}

%build
%configure
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot} INSTALL="install -p"
%find_lang %{app_name}

%clean
rm -rf %{buildroot}


%files -f %{app_name}.lang
%defattr(-,root,root,-)
%{_libexecdir}/%{app_name}-%{platform}
%{_datadir}/dbus-1/services/org.gnome.panel.applet.BubblemonAppletFactory.service
%{_datadir}/gnome-panel/4.0/applets/org.gnome.panel.BubblemonApplet.panel-applet
%{_datadir}/gnome-panel/ui/%{app_name}-menu.xml
%{_datadir}/pixmaps/%{app_name}.png

%doc AUTHORS ChangeLog COPYING FAQ PROFILING README TODO TRANSLATIONS
%{_mandir}/man1/%{app_name}-%{platform}.1.gz
%lang(hu) %{_mandir}/hu/man1/%{app_name}-%{platform}.1.gz
%lang(sv) %{_mandir}/sv/man1/%{app_name}-%{platform}.1.gz


%changelog
* Wed Feb 29 2012 Honore Doktorr <hdfssk@gmail.com> - 3.0-1
- Upgraded to 3.0, for Gnome 3 panel/fallback mode support.

* Thu Dec 24 2009 Edwin ten Brink <fedora at tenbrink-bekkers.nl> - 2.0.15-1
- Upgraded to 2.0.15, which fixes a crash which occurs in some locales.

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 2.0.14-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Mon May 11 2009 Edwin ten Brink <edwin.ten.brink at gmail.com> - 2.0.14-1
- Upgraded to version 2.0.14.

* Sat Apr 25 2009 Edwin ten Brink <edwin.ten.brink at gmail.com> - 2.0.13-3
- Incorporated following suggestions from Fedora package review process:
  Changed name to gnome-applet-bubblemon. Changed license to GPLv2+.
  Removed AutoReqProv. Added Requires: gnome-panel. Removed doc ABOUT-NLS.
  Modified make install to preserve timestamps.

* Mon Apr 21 2009 Edwin ten Brink <edwin.ten.brink at gmail.com> - 2.0.13-2
- Added seaweeds to description. Added ABOUT-NLS to documentation.
  Fine-tuned BuildRequires (removed packages which were also included via
  dependencies in the mentioned BuildRequires).

* Sat Apr 19 2009 Edwin ten Brink <edwin.ten.brink at gmail.com> - 2.0.13-1
- Converted to use Fedora packaging guidelines and additional clean-up for
  submission into Fedora. Updated description. Removed the (now obsoleted)
  French translations. Updated BuildRequires to reflect reality.

* Thu Jul 10 2008 Edwin ten Brink <edwin.ten.brink at gmail.com> - 2.0.9
- Changed to newer RPM format: Copyright -> License, removed old bug
  work-around for install, fixed name and version macro's, changed
  Requires: from libgtop -> Automatic Dependencies, added clean-up of
  the build root, fixed man page locations.

* Mon Mar 03 2003 Eric Lassauge <lassauge at mail.dotcom.fr>
- Added french translations