spnavcfg
========

About
-----
Spacenav daemon interactive configuration program.

![shot](http://spacenav.sourceforge.net/images/spnavcfg1-shot-thumb.png)


Compatibility
-------------
The current version of spnavcfg works with spacenavd v1.0 or higher (requires a
spacenavd which speaks the spnav protocol v1).

This break in compatibility is necessary because the new protocol allows
spnavcfg to act as a regular libspnav client and send configuration commands
through the spacenavd socket, instead of having to modify `/etc/spnavrc` and
send `SIGHUP` to the daemon, as was the old mode of operation.

This is a huge security improvement, as it makes it no longer necessary to
install spnavcfg as setuid-root, and frankly much less clunky and error-prone.
Plus it opens the way for a new and improved GUI with much more functionality
and user feedback.

Therefore, you are advised to update your spacenavd to the latest version if at
all possible. If you can't update spacenavd, the last version of spnavcfg
which will work with older versions of the daemon is 0.3.1.

Installation
------------
First make sure you have the dependencies installed:
  - libspnav v1.0 or higher
  - Qt 5 (core, gui, and widgets).

To build just run `./configure`, `make`, and `make install` as usual.
The `make install` part will probably need to be executed as root, if you're
installing system-wide.

For build options, see `./configure --help`.

> Note: if you cloned the source code from the git repo without GIT-LFS, the
> image in `icons/devices.png` will be invalid leading to an incorrect build and
> crashes on startup. If you don't want to install GIT-LFS, you can grab the
> file from the latest release archive and drop it in place.

License
-------
Copyright (C) 2007-2022 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute
it under the terms of the GNU General Public License version 3, or (at your
option) any later version published by the Free Software Foundation.
See COPYING for details.

FAQ
---
  1. **Q**: I'm trying to build spnavcfg, but the linker complains about missing
     functions starting with `spnav_cfg_`.
     
     **A**: You need libspnav v1.0 or higher to build spnavcfg. If it is installed,
     but you also have a previous version installed in a different path, please
     remove it in case the linker finds that one first.

  2. **Q**: When I ran spnavcfg, it immediately crashes with a SIGFPE.
     
     **A**: This is most likely caused by bulding with an invalid device atlas
     image. If you cloned the source code from git, you need GIT-LFS to
     correctly retreive the image from the repo. Without GIT-LFS, the file
     `icons/devices.png` will be a text file with a hash instead of a PNG file.
     If you don't want to install GIT-LFS, grab the file from the latest release
     archive and drop it in.
