spnavcfg
========

About
-----
Spacenav daemon interactive configuration program.

![shot](http://spacenav.sourceforge.net/images/spnavcfg1-shot-thumb.png)


Compatibility
-------------
The current version of spnavcfg works with spacenavd v0.9 or higher (requires a
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
  - libspnav v0.4 or higher
  - Qt 5 (core, gui, and widgets).

To build just run `./configure`, `make`, and `make install` as usual.
The `make install` part will probably need to be executed as root, if you're
installing system-wide.

For build options, see `./configure --help`.

License
-------
Copyright (C) 2007-2022 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute
it under the terms of the GNU General Public License version 3, or (at your
option) any later version published by the Free Software Foundation.
See COPYING for details.
