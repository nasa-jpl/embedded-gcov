#!/bin/bash

# Typical usage: ./lcov_combine_new_total.sh test02 test01 test0102

# First argument (optional) is newcov input filename label
# as used in preceding lcov_newcoverage.sh
# such as "test01"
# Will be used for input and output if no second argument is provided.
if [ -z "$1" ]
then
	newlbl=""
else
	newlbl=_"$1"
fi

# Second argument (optional) is input/output totcov filename label
# such as "test01"
# If not provided, first argument (if any) will be used.
# Will be used for input and output if no third argument is provided.
if [ -z "$2" ]
then
	totlbl_in="$newlbl"
else
	totlbl_in=_"$2"
fi

# Third argument (optional) is output totcov filename label
# such as "test01"
# If not provided, second argument (if any) will be used for input and output.
if [ -z "$3" ]
then
	totlbl_out="$totlbl_in"
else
	totlbl_out=_"$3"
fi

lcov -a ../results/totcov${totlbl_in}.info \
	-a ../results/newcov${newlbl}.info \
	-o ../results/totcov${totlbl_out}.info

# embedded-gcov lcov_combine_new_total.sh script to combine new lcov output with total
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
