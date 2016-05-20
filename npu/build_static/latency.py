#!/usr/bin/python
#
# simple-npu: Example NPU simulation model using the PFPSim Framework
#
# Copyright (C) 2016 Concordia Univ., Montreal
#     Samar Abdi
#     Umair Aftab
#     Gordon Bailey
#     Faras Dewal
#     Shafigh Parsazad
#     Eric Tremblay
#
# Copyright (C) 2016 Ericsson
#     Bochra Boughzala
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

import sys
import csv
import operator
import itertools
import time

if __name__=="__main__":

    IngressTrace = list(csv.reader(open("IngressTrace.csv"), delimiter=","))
    EgressTrace = list(csv.reader(open("EgressTrace.csv"), delimiter=","))
    sortedEgressTrace = sorted(EgressTrace, key=lambda x: float(x[0]), reverse=False)
    # print "---- Egress Sort----"
    # for line in sortedEgressTrace:
    #    print line
    # print "---- Ingress assumed to be sorted----"
    # for line in IngressTrace:
    #    print line
    #print "---- Latency----"

    avgsum =0
    count =0
    for IngressVal,EgressVal in itertools.izip(IngressTrace,sortedEgressTrace):
        #print IngressVal + EgressVal
        diff = float(EgressVal[1])-float(IngressVal[1])
        avgsum = avgsum + diff
        count = count + 1

    avg_latency = avgsum/count


    # print EgressTrace
    sortedEgressTimeTrace = sorted(EgressTrace, key=lambda x: float(x[1]), reverse=False)
    avg_thrp =len(sortedEgressTimeTrace) / (float(sortedEgressTimeTrace[-1][1]) - float(sortedEgressTimeTrace[0][1]))



   # print "Latency "+str(avg_latency)
   # print "Throupt "+str(avg_thrp)
    runname = ""
    if sys.argv[1:]:
        runname = str(sys.argv[1])
    else:
        runname = "Simualtion Run@ "+str(time.asctime( time.localtime(time.time()) ))
    avgfile = "avg.txt"
    with open(avgfile,"a") as avgfh :
        avgfh.write(str(runname)+" ,")
        avgfh.write(str(avg_latency))
        avgfh.write(", ")
        avgfh.write(str(avg_thrp))
        avgfh.write('\n')
    print "Latency:    ", avg_latency
    print "Throughput: ", avg_thrp
