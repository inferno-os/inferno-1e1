#!/bin/sh

sed 's/extern //;s,;.*/\* \(.*\) \*/, = "\1";,' < ../port/error.h
exit 0
