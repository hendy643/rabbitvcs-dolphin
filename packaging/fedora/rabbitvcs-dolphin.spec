Name:           rabbitvcs-dolphin
Version:        0.1
Release:        1%{?dist}
Summary:        RabbitVCS integration for the Dolphin file manager
License:        GPL-2.0-or-later
URL:            https://rabbitvcs.org
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  extra-cmake-modules
BuildRequires:  kf6-rpm-macros
BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6Widgets)
BuildRequires:  cmake(Qt6Test)
BuildRequires:  cmake(KF6CoreAddons)
BuildRequires:  cmake(KF6KIO)
BuildRequires:  dolphin-devel
BuildRequires:  git-core

Requires:       dolphin
Recommends:     rabbitvcs

%description
Dolphin version-control plugins for Git and Subversion. They show the
status of files as overlays and add a RabbitVCS submenu to the context
menu. Status is read with the native git/svn tools; the menu actions
start the rabbitvcs command line client.

%prep
%autosetup

%build
%cmake_kf6
%cmake_build

%install
%cmake_install

%check
%ctest

%files
%license COPYING
%doc README
%{_qt6_plugindir}/dolphin/vcs/rabbitvcsgitplugin.so
%{_qt6_plugindir}/dolphin/vcs/rabbitvcssvnplugin.so

%changelog
* Tue Nov 03 2026 hendy643 <phenderson643@gmail.com> - 0.1-1
- Initial packaging
