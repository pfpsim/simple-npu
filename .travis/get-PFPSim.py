#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# pfpgen: Code generation tool for creating models in the PFPSim Framework
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


try:
  # For Python 3.0 and later
  from urllib.request import urlopen, Request
except ImportError:
  # Fall back to Python 2's urllib2
  from urllib2 import urlopen, Request

import json
from sys import argv, exit

if len(argv) != 3:
  print("Usage: " + argv[0] + " <tag name> <CXX>")
  exit(1)


tag_name   = argv[1]

cxx = argv[2]
plus_idx = cxx.find('+')
if plus_idx >= 0:
    cxx = cxx[0:plus_idx]
deb_suffix = '-' + cxx + '.deb'

r = Request("https://api.github.com/repos/pfpsim/PFPSim/releases/tags/" + tag_name)

resp = urlopen(r).read()
try:
    resp = json.loads(resp)
except:
    resp = json.loads(resp.decode("utf-8"))

print("Got list of assets")

for asset in resp['assets']:
  if asset['name'].endswith(deb_suffix):
    print("found " + asset['name'])
    r = Request(asset['url'],
            None, # No data
            {"Accept":"application/octet-stream"})
    deb_resp = urlopen(r).read()
    print("Successfully downloaded")
    with open(asset['name'], 'wb') as debfile:
      debfile.write(deb_resp)
