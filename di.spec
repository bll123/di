Name:           di
Version:        4.27
Release:        1%{?dist}
Summary:        'di' is a disk information utility, displaying everything (and more) that your 'df' command does.

Group:          System Environment/Base
License:        zlib/libpng
URL:            http://www.gentoo.com/di/
Source0:        http://www.gentoo.com/di/di-%{version}.tar.gz
Source1:        http://www.sfr-fresh.com/unix/misc/di-%{version}.tar.gz
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

# Build requires: cat cp grep ln msgfmt mv rm sed sort test uname uniq
#BuildRequires:
#Requires:

%description
'di' is a disk information utility, displaying everything
(and more) that your 'df' command does. It features the
ability to display your disk usage in whatever format you
desire. It is designed to be highly portable across many
platforms.  Great for heterogenous networks.

%prep
%setup -q

%build
make LOCALEDIR=/usr/share/locale

%install
test -d $RPM_BUILD_ROOT || mkdir $RPM_BUILD_ROOT
test -d $RPM_BUILD_ROOT/usr || mkdir $RPM_BUILD_ROOT/usr
make prefix=$RPM_BUILD_ROOT/usr install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc README LICENSE MANIFEST
/usr/bin/di
/usr/bin/mi
/usr/share/locale/de/LC_MESSAGES/di.mo
/usr/share/locale/en/LC_MESSAGES/di.mo
/usr/share/locale/es/LC_MESSAGES/di.mo
/usr/share/man/man1/di.1.gz

%changelog
