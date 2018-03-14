#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Tool for generating WEBFS file system image
# Copyright © 2017 Magres
# https://bitbucket.org/magres/webfs_tool
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
# Street, Fifth Floor, Boston, MA 02110-1301 USA.

__version__='0.9'

import os, struct, re, zlib, gzip, fnmatch, argparse, posixpath, collections

def webfs_hash(filename):
    r = 0;
    for l in filename:
        r = r + ord(l)
        r = r << 1
        r = r & 0xFFFF
    return r

def check_match(filename, extensions=[]):
    for extension in extensions:
        if fnmatch.fnmatch(filename, extension): return True
    return False

def write_image(files_dict, output, dynamic_files=['*.htm', '*.html', '*.cgi', '*.xml', '*.bin', '*.txt', '*.wav'], nozip_files=['*.inc', 'snmp.bib']):
    file_refs=[]
    idx = re.compile('~([^~]{0,127})~')
    with open(output, 'wb') as f:
        f.seek(12+6*len(files_dict))
        for file_name, file_path in files_dict.iteritems():
            with open(file_path, 'r') as f1:
                file_refs.append(f.tell())
                cnt = f1.read()
                f.write(struct.pack('<1L', len(cnt)+len(file_name)+9 ))
                f.write(struct.pack('<1H', len(file_name)+9 ))
                flags = 0

                if check_match(file_name, dynamic_files) and idx.findall(cnt):
                    flags = flags | 0x2
                elif not check_match(file_name, nozip_files):
                    flags = flags | 0x1

                f.write(struct.pack('<1H', flags))
                f.write(file_name)
                f.write('\x00')
                if flags & 0x1:
                    with gzip.GzipFile(fileobj=f, mode='w') as g:
                        g.write(cnt)
                else:
                    f.write(cnt)
        img_len = f.tell()
        f.seek(0)
        f.write('FWEB\x01\x00')
        f.write(struct.pack('<1H', len(files_dict)))
        f.write(struct.pack('<1L', img_len-7))
        for file_name in files_dict.keys():
            f.write(struct.pack('<1H', webfs_hash(file_name)))
        for file_ref in file_refs:
            f.write(struct.pack('<1L', file_ref))

def get_files_dict(dir):
    r = collections.OrderedDict()
    for dirpath, dirnames, filenames in os.walk(dir):
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            rel_path = os.path.relpath(file_path, dir)
            r[posixpath.join(*rel_path.split(os.sep))] = file_path
    return r

if __name__ == "__main__":

    #parser = argparse.ArgumentParser("webfs_tool.py", description="Tools for working with WEBFS")
    m = argparse.ArgumentParser(description="Tools for working with WEBFS")
    subp = m.add_subparsers()
    parser = subp.add_parser('build')

    parser.add_argument('output', help='Output image file')
    parser.add_argument('-s', '--source', help='Source directory', action='append', default=[])
    parser.add_argument('-d', '--dynamic_files', help='Files with dynamic content (default "*.htm, *.html, *.cgi, *.xml, *.bin, *.txt, *.wav")', default='*.htm, *.html, *.cgi, *.xml, *.bin, *.txt, *.wav')
    parser.add_argument('-n', '--nozip_files', help='Files not to be gzipped', default='*.inc, snmp.bib')
    parser.add_argument('-f', '--file', help='Add file to image in format path/in/image:path/on/disc', action='append', default=[])

    def cmd_1(args):
        files = collections.OrderedDict()
        for s in args.source:
            files.update(get_files_dict(s))
        for fd in args.file:
            fdl = fd.split('@')
            if len(fdl)>2: raise Exception('Invalid file format "%s"'%fd)
            elif len(fdl)==2:
                files[fdl[0]]=fdl[1]
            else:
                files[os.path.basename(fdl[0])]=fdl[0]
        write_image(files, args.output, dynamic_files=args.dynamic_files.split(','),nozip_files=args.nozip_files.split(',') )

    parser.set_defaults(func=cmd_1)

    args = m.parse_args()
    args.func(args)
