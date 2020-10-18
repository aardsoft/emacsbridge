Name: emacsbridge
Version: VERSION
Release: RELEASE
Summary: Emacs bridge
Group: System/Base
License: GPLv2
#Source0: %{name}-%{version}.tar.gz
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5QuickControls2)
BuildRequires: pkgconfig(Qt5WebSockets)
BuildRequires: pkgconfig(Qt5RemoteObjects)
BuildRequires: pkgconfig(Qt5Network)
BuildRequires: libQt5Core-private-headers-devel
%{?systemd_requires}

%description
%{summary}.

%files
%defattr(-,root,root,-)
%{_bindir}/emacsbridge
%{_libdir}/*.so.*
%{_userunitdir}/*.service

%package devel
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description devel
%{summary}.

%files devel
%defattr(-,root,root,-)
%{_includedir}/qt5/QtHttpServer/*
%{_includedir}/qt5/QtSslServer/*
%{_libdir}/pkgconfig/*.pc
%{_libdir}/cmake/Qt5HttpServer/*.cmake
%{_libdir}/cmake/Qt5SslServer/*.cmake
%{_libdir}/qt5/examples/httpserver/*
%{_libdir}/qt5/mkspecs/modules/*.pri
%{_libdir}/*.so
%{_libdir}/*.la
%{_libdir}/*.prl

%prep
%setup -c -q

%build
mkdir -p pc
pushd pc
qmake-qt5 QMAKE_CXX=clang++ QMAKE_LINK=clang++ ..
make %{?_smp_mflags}
popd

%install
pushd pc
make INSTALL_ROOT=%{buildroot} install
popd

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
