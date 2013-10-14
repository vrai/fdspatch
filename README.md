fdspatch : Famicom Disk System image patcher
============================================

The combination of the FDSLoadr software[1] and parallel port/RAM
adapter cable[2] allows an old PC to be used as a replacement for
Famicom Disk System hardware. However the .FDS images found online
often lack the header required by FDSLoadr.

The `fdspatch` tool will, by default, confirm that the image is
valid and display whether it has the required header. For example,
when run on a "Monty On The Run" image that lacks the header:

    $ fdspatch ./monty.fds
    Disk count: 2
    FDS header expected: no
    Image header valid: yes
    Game vendor id: 10
    Game ident: "MDD "
    Game version: 0

The image is valid, but no FDS header is found. Running `fdspatch`
in convert mode will fix this:

    $ fdspatch -c monty.fds
    Patching /Users/vrai/Downloads/monty.fds ........OK

Re-running in default mode confirms the fix has worked:

    $ fdspatch monty.fds
    Disk count: 2
    FDS header expected: yes
    FDS header valid: yes
    Image header valid: yes
    Game vendor id: 10
    Game ident: "MDD "
    Game version: 0

Note that the FDS header is found and is valid. Running the tool in
convert mode over an image that already has the FDS header will
leave the file untouched. Only valid images that lack the header
will be modified.

Build and installation
----------------------

To build `fdspatch` on a Unix style system (including Linux and
OS X), simply run `make`. This will produce the `fdspatch` binary
that can be copied to the system's standard binary directory if
required.

To build on DOS, the DJGPP development environment[3] is required.
Once this is installed and configured, running `make dos` will
build `fdspatch.exe`. This binary takes the same options and works
the same way as the Unix version.

Note that the DOS version has only been tested on FreeDOS. Any
version of DOS capable of running the compiler should work.

Updates and license
-------------------

The latest version of the application can be downloaded from Github:

    https://github.com/vrai/fdspatch

The fdspatch is free software and is licensed under version 2 of the GNU
General Public License as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along
with this program (see the COPYING file); if not, write to the Free
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Footnotes
---------

1. FDSLoadr can be downloaded from http://nesdev.com/fdsloadr.zip
2. The FDS RAM adapter to parallel port cable can be home build or
   bought from http://www.tototek.com/
3. The DJGPP compiler and development environment can be downloaded
   from http://www.delorie.com/djgpp/

----

Copyright 2013
Vrai Stacey <vrai.stacey@gmail.com>

