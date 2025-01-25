di Version 5.x beta

Website: https://diskinfo-di.sourceforge.io/
SourceForge: https://sourceforge.net/projects/diskinfo-di/

See: README-new.txt
This README will be replaced with README-new.txt
when 5.x reaches production status.

Version 5 beta Timeline

2025-1-20 Very early days, wrote this readme...
2025-1-23 Releasing a very early beta version.  Still a very long road ahead.
          The wiki is a bit raw.  Almost no cross-platform testing has
          been done.  The mkconfig tests are currently broken.
2025-1-24 FreeBSD is now compiling w/cmake.  This fixes a lot of cmake issues.
2025-1-25 More cmake issues fixed, mkconfig issues fixed.  Everything is
          a lot cleaner now, and very basic build/run testing
          should go more smoothly.

Version 5.x beta

- re-worked to have a library and proper api
- cmake configuration
- wiki pages to document the api
- optionally use gmp or libtommath to handle large numbers

Version 5 Roadmap (more or less)

- done: move the totals processing into the main library.
- done: finish the api, there's still a couple of routines to write
- done: linux tirpc library (nfs quotas)
- done: get installation working
- started: wiki documentation for the api
- get all the basics working
- make the beta available to download
- work on the test suite
- get basic di working on all platforms I have access to.
- check the cmake configuration against mkconfig.  check again. re-check.
- I have macos now, test quotas and nfs quotas there
- write the manual page for the library
- re-write the readme, get ready for production release
- wiki page updates and rewrite
- production release

Most of the following _should_ work...but I may have easily added
a typo or copy/paste error.  The basic code structure hasn't changed.

- apfs testing on macos
- test quota handling
- test nfs quota handling
- xfs quotas
- dragonflybsd quotas
- zfs testing on solaris

Version 5 beta Notes

GMP and libtommath work fine.  Long doubles seem to be quite
capable (where long doubles are supported).  If you have long double
support, it seems reasonable to not use the multi-precision libraries
(in 2025).

I have a *lot* of VMs.  These are mostly old Linux and old *BSD,
though there are a couple of others.

But access to non-Linux and non-*BSD systems is limited. Amazingly, I
still have access to a Tru64 machine (so slow), QNX and SCO Openserver
(thanks to polarhome).

For MacOS, I removed some of my older versions (no disk space).  I
have Big Sur, Ventura, Sonoma and Sequoia.  I did something wrong and
apparently removed or overwrote the Monterey partitions.  I don't
think that will be an issue.  Someone tell the Mac people to fix the
damned audiomxd bug.

I don't have access to the old Solaris machines.  OpenCSW appears to
be up, but I don't remember my password (the problem with using ssh
keys, sometimes they get changed).  I have a few VMs with Solaris.

I do not have access to any HP-UX machine.

I should have (or be able to get) access to modern AIX on the gcc
farm. I no longer have access to AIX 5.
