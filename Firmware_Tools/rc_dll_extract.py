# Copyright 2021, Pantman <https://github.com/pantman>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License at <http://www.gnu.org/licenses/> for
# more details.

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

#!/usr/local/opt/python/bin/python2.7
# read rc_vec.dll and rc.dll, and export png files

import zlib
import struct
import png

def readString(myfile):
    chars = []
    while True:
        c = myfile.read(1)
        if c == None or c == chr(0):
            return "".join(chars)
        chars.append(c)

class Resource:

	def __init__(self):
		width = 0
		height = 0
		offset = 0
		stride = 0
		path = ""
		filename = ""

	def Print(self):
		print("width = {}").format(res.width)
		print("height = {}").format(res.height)
		print("offset = {}").format(res.offset)
		print("stride = {}").format(res.stride)
		print("path = {}").format(res.path)
		print("filename = {}").format(res.filename)

	@staticmethod
	def Read(myfile):

		chunk = myfile.read(80);
		
		if not chunk:
			return None

		res = Resource()

		header = struct.unpack("6I12s2I8s7I",chunk) # read struct
		res.path = readString(myfile) 
		padding = (4 - (len(res.path) + 1) % 4) % 4
		myfile.read(padding) #skip padding

		res.width = header[7]
		res.height = header[8]
		res.offset = header[13]
		res.stride = header[14]	

		res.filename = "./" + res.path.split("\\")[-1];

		return res


rc = open("./rc.dll", mode="rb")
compressed_bytes = rc.read()
rc.close()

rcvec = open("./rcvec.dll", mode="rb")

while True:

	res = Resource.Read(rcvec)

	if not res:
		break	

	res.Print()

	decoded = zlib.decompress(compressed_bytes[res.offset:])

	png_file = open(res.filename, "wb")

	png_writer = png.Writer(res.width, res.height, greyscale=False, alpha=True, bitdepth=8)

	png_writer.write_array(png_file, decoded)

	png_file.close()


rcvec.close()