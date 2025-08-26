Name:           Silo
Version:        9999
Release:        1%{?dist}
Summary:        Mesh and Field I/O Library and Scientific Database

License:        BSD-3-Clause 
URL:            https://silo.llnl.gov/
Source0:        https://github.com/LLNL/Silo/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake >= 3.12
BuildRequires:  hdf5-devel 
BuildRequires:  perl 

%global silo_desc \
Silo is a C/Fortran API for reading and writing a wide variety of \
scientific data to binary files. Silo files can be easily shared and \
exchanged between wholly independently developed applications running \
on disparate computing platforms. Consequently, Silo facilitates the \
development of general purpose tools for processing scientific data. \
One of the more popular tools to process Silo data is the VisIt \
visualization tool. Silo supports a variety of mesh types including \
simple curves, gridless (point), structured, unstructured-zoo and \
unstructured-arbitrary, block structured AMR, and constructive solid \
geometry (CSG).

%description
%{silo_desc}

%package devel
Summary:        Development package for  %{name} packages
Requires:       %{name} = %{version}-%{release}
%description devel
%{silo_desc}

This package contains the development files of %{name}.

%prep
%autosetup -p1 -n %{name}

%build
%cmake \
  -DSILO_ENABLE_SHARED=ON \
  -DSILO_ENABLE_SILOCK=ON \
  -DSILO_ENABLE_SILEX=OFF \
  -DSILO_ENABLE_BROWSER=ON \
  -DSILO_ENABLE_FORTRAN=ON \
  -DSILO_ENABLE_HDF5=ON \
  -DSILO_ENABLE_JSON=OFF \
  -DSILO_ENABLE_PYTHON_MODULE=OFF \
  -DSILO_ENABLE_TESTS=ON \
  -DSILO_BUILD_FOR_BSD_LICENSE=ON \
  %{nil}
%cmake_build

%install
%cmake_install

%check
%cmake_build --target check

%files devel
%doc README.md
%license LICENSE
%files
...
%{_prefix}/SiloInstall/bin/browser
%{_prefix}/SiloInstall/bin/silock
%{_prefix}/SiloInstall/bin/silodiff
%{_prefix}/SiloInstall/bin/silofile
%{_prefix}/SiloInstall/cmake/SiloConfig.cmake
%{_prefix}/SiloInstall/cmake/SiloConfigVersion.cmake
%{_prefix}/SiloInstall/cmake/SiloTargets-release.cmake
%{_prefix}/SiloInstall/cmake/SiloTargets.cmake
%{_prefix}/SiloInstall/include/lite_pdb.h
%{_prefix}/SiloInstall/include/lite_score.h
%{_prefix}/SiloInstall/include/pdform.h
%{_prefix}/SiloInstall/include/silo.h
%{_prefix}/SiloInstall/include/silo.inc
%{_prefix}/SiloInstall/include/silo_FC.h
%{_prefix}/SiloInstall/include/silo_exports.h
%{_prefix}/SiloInstall/include/silo_f9x.inc
%{_prefix}/SiloInstall/lib/libsiloh5.so
%{_prefix}/lib/debug/.../browser-9999-1.fc44.x86_64.debug
%{_prefix}/lib/debug/.../silock-9999-1.fc44.x86_64.debug
