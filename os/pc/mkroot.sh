#!/bin/sh

r=/work/inferno

data2c nvramc_ nvram-c
data2c nvramh_ nvram-h
data2c mount_ ./mount.dis
data2c osinit_ ../init/$1.dis
data2c daytime_ $r/dis/lib/daytime.dis
data2c login_ $r/dis/lib/login.dis
data2c ppp_ ./ppp.dis
data2c ssl_ $r/dis/lib/ssl.dis
data2c xd_ ./xd.dis
data2c _9660srv_ ./cdfs/9660srv.dis
data2c cdfs_ ./cdfs/cdfs.dis
data2c dirmod_ ./cdfs/dirmod.dis
data2c iobuf_ ./cdfs/iobuf.dis
data2c iso9660_ ./cdfs/iso9660.dis
data2c styx_ ./cdfs/styx.dis
data2c styxreply_ ./cdfs/styxreply.dis
