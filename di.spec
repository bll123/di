Name:           di
Version:        4.35a
Release:        1%{?dist}
Summary:        'di' is a disk information utility, displaying everything (and more) that your 'df' command does.

Group:          System Environment/Base
License:        zlib/libpng
URL:            http://www.gentoo.com/di/
Source0:        http://www.gentoo.com/di/di-%{version}.tar.gz
Source1:        http://www.fossies.org/unix/misc/di-%{version}.tar.gz
BuildRoot:      %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

# Build requires: cat cp grep ln msgfmt mv rm sed sort test uname tr wc
#BuildRequires:
#Requires:

%description
'di' is a disk information utility, displaying everything (and
more) that your 'df' command does. It features the ability to display
your disk usage in whatever format you prefer. It also
checks the user and group quotas, so that the user
sees the space available for their use, not the system wide
disk space. It is designed to be portable across many platforms
and is great for heterogenous networks.

%prep
%setup -q

%build
make -e LOCALEDIR=/usr/share/locale

%install
test -d $RPM_BUILD_ROOT || mkdir $RPM_BUILD_ROOT
test -d $RPM_BUILD_ROOT/usr || mkdir $RPM_BUILD_ROOT/usr
make -e prefix=$RPM_BUILD_ROOT/usr install

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
# For Mandriva 2011, change di.1.gz to di.1.xv

%changelog
