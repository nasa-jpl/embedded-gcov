#!/bin/bash

# Typical usage: ./lcov_newcoverage.sh test01

# First argument (optional) is output filename label
# such as "test01"
if [ -z "$1" ]
then
	newlbl=""
else
	newlbl=_"$1"
fi

# Second argument (optional) is lcov test data label
# such as "test01"
# Do not use an lcov test data label if you plan to
# submit to JPL coveralls server, not compatible
if [ -z "$2" ]
then
	tname_arg=""
else
	tname_arg="--test-name $2"
fi

# NOTE: The --gcov-tool argument must specify a gcov executable
# that goes with the compiler that build the program you are analyzing.
# In most embedded sytems, that means a gcov version matching
# your cross-compiler for the embedded system.
# Example: --gcov-tool /opt/tools/target-tools/bin/sparc-rtems5-gcov

lcov --gcov-tool gcov \
	--capture ${tname_arg} \
	--directory ../objs/ \
	-o ../results/newcov${newlbl}.info

# embedded-gcov lcov_newcoverage.sh script to generate new lcov data
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
