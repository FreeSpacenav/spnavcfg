spnavcfg
========

About
-----
Spacenav daemon interactive configuration program.

![shot](http://spacenav.sourceforge.net/images/spnavcfg-shot-thumb.png)

Installation
------------
Requires GTK+ 2 and Xlib headers to be installed.
./configure, make, make install, as usual (make install part as root).

Security
--------
The binary is installed setuid root in order to be able to modify /etc/spnavrc
and send signals to the daemon. However, it was designed from start to be as
secure as possible. spnavcfg launches two processes (see front.c and back.c).
The frontend that runs all the GUI code drops priviledges to the original uid,
while the backend that does all the gruntwork keeps effective uid root.
As long as you don't run the program while logged in as root, it should be very
secure.

License
-------
Copyright (C) 2007-2020 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute
it under the terms of the GNU General Public License version 3, or (at your
option) any later version published by the Free Software Foundation.
See COPYING for details.
