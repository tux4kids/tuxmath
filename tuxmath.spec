#
# spec file for package tuxmath 
#
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#

# norootforbuild

Name:           tuxmath
%define         realname tuxmath_w_fonts
Summary:        Tux Math - educational math game
Version:        1.7.0
Release:        1
License:        GPL
Group:          Productivity/Scientific/Math
Url:            http://tux4kids.alioth.debian.org/
Vendor:         openSUSE-Education
Obsoletes:      tuxmath = 2001.09.07
BuildRequires:  SDL_image-devel >= 1.2.2
BuildRequires:  SDL_ttf-devel > 2.0.8
BuildRequires:  SDL-devel
BuildRequires:  SDL_mixer-devel
BuildRequires:  SDL_Pango-devel
%if 0%{?suse_version}
BuildRequires:  update-desktop-files
BuildRequires:  fdupes
%endif
%if 0%{?fedora_version}
BuildRequires:  desktop-file-utils
%endif
%if 0%{?mandriva_version}
Requires(post): desktop-file-utils
Requires(postun): desktop-file-utils
%endif
Source0:        %realname-%version.tar.bz2
Source1:        %name.desktop
Source2:        tuxmath-1.7.0-de.po
Source3:        tuxmath-1.7.0-de.gmo
Patch1:         tuxmath-1.7.0-german.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
An educational math tutorial game starring Tux, the Linux Penguin.

Based on the classic arcade game "Missile Command," Tux must defend his cities. 
In this case, though, he must do it by solving math problems.

Authors:
========
  Bill Kendrick
  Sam 'Criswell' Hart
  Larry Ewing

%prep
%setup -q -n %realname-%version
rm -rf $(find . -type d -name CVS)
rm -rf $(find . -type d -name .svn)
rm -rf $(find . -type d -name .xvpics)
install -m644 %{SOURCE2} po/de.po 
install -m644 %{SOURCE3} po/de.gmo
%patch1 -p0

%build
%configure --enable-sdlpango
make %{?jobs:-j %jobs}

%install
install -d %buildroot/{%_bindir,%_datadir/pixmaps,%_datadir/applications,%_datadir/%name,%_defaultdocdir/%name}
make DESTDIR=%{buildroot} install
install -m 644 data/images/icons/icon.png %buildroot%_datadir/pixmaps/%name.png
%if 0%{?suse_version}
# handle special docdir path
mv %buildroot/%_datadir/doc/%name/* %buildroot/%_defaultdocdir/%name/
rm -rf %buildroot/%_datadir/doc/%name
# install desktop file
%suse_update_desktop_file -i tuxmath Education Math
%fdupes -s %buildroot
%endif
%if 0%{?fedora_version}
# install desktop file
desktop-file-install --vendor="%{vendor}" \
  --dir=%buildroot/%_datadir/applications \
  %{SOURCE1}
%endif
%if 0%{?mandriva_version}
desktop-file-install --vendor="%{vendor}" \
  --dir=%buildroot/%_datadir/applications \
  %{SOURCE1}
%endif
# remove invalid locale directories
rm -rf %buildroot/%{_datadir}/locale/en@*
%find_lang %name

%if 0%{?mandriva_version}
%post
%{update_menus}

%postun
%{clean_menus}
%endif

%clean
rm -rf %buildroot

%files -f %name.lang
%defattr(-,root,root)
%doc %_defaultdocdir/%name
%_bindir/*
%_datadir/pixmaps/*
%_datadir/applications/*
%_datadir/%name

%changelog 
s