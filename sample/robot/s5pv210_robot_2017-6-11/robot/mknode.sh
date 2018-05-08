#!/bin/sh

echo "mknod in /dev/snd ..."

mkdir /dev/snd
mknod /dev/snd/controlC0 c 116 0
mknod /dev/snd/mixer     c 14  0
mknod /dev/snd/timer     c 116 33

mknod /dev/snd/pcmC0D0c  c 116 24
mknod /dev/snd/pcmC0D0p  c 116 16

echo "done"
