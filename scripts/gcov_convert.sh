#!/bin/bash

# Typical usage: ./gcov_convert.sh ../test01_serial_log.txt

# Serial log file can have null characters in it
# if the system rebooted during the log.
# Also remove any other non-ASCII chars (only allow specific octal character values through)
# (thanks to https://alvinalexander.com/blog/post/linux-unix/how-remove-non-printable-ascii-characters-file-unix/)
tr -cd '\11\12\15\40-\176' < $1 > ${1%.*}_nonulls.txt

# Convert from DOS test file
dos2unix ${1%.*}_nonulls.txt

# Create separate .gcda.xxd files from the serial log
# The files can be created at the full pathname specified in the log
# or can be created in the current directory, see serial_split.awk.
# Current directory is more convenient for us here.
cat ${1%.*}_nonulls.txt | awk -f serial_split.awk

# Move the .gcda.xxd files from here to ../objs
# which is where the object files and .gcno files
# should already be
mv *.gcda.xxd ../objs

# Convert the separate .gcda.xxd files to separate binary .gcda files
# And remove the .xxd files
for i in `find ../objs -name '*.gcda.xxd'`;do
	cat "$i" | xxd -r > "${i/\.xxd/}"
	rm "$i"
done

# embedded-gcov gcov_convert.sh script to split serial output to separate gcda files
#
# Copyright (c) 2021 California Institute of Technology (“Caltech”).
# U.S. Government sponsorship acknowledged.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#    Redistributions of source code must retain the above copyright notice,
#        this list of conditions and the following disclaimer.
#    Redistributions in binary form must reproduce the above copyright notice,
#        this list of conditions and the following disclaimer in the documentation
#        and/or other materials provided with the distribution.
#    Neither the name of Caltech nor its operating division, the Jet Propulsion Laboratory,
#        nor the names of its contributors may be used to endorse or promote products
#        derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
