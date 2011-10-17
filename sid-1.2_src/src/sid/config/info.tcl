# info.tcl.in -*- Tcl -*-

# Copyright (C) 1999, 2000, 2001 Red Hat.
# This file is part of SID and is licensed under the GPL.
# See the file COPYING.SID for conditions for redistribution.

# Additional configure-time settings.

global prefix
set prefix "/opt/sid-1.2"
set enable_shared "yes"
set target_alias "i686-pc-linux-gnu"

# pull out "--enable-targets"/"--target" settings
global sidtarget
set sidtarget(arm) 1
set sidtarget(x86) 1
set sidtarget(mips) 1
set sidtarget(m32r) 1
set sidtarget(m68k) 1
set sidtarget(mep) 1
set sidtarget(mt) 1
set sidtarget(ppc) 1
set sidtarget(sh) 1
set sidtarget(sh64) 1
set sidtarget(xstormy16) 1
