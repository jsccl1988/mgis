#!/usr/bin/python
import sys
import os
import shutil

def unzip(zip_exe, src_zip, target_dir):
    print('[zip_exe   ]%s' % zip_exe)
    print('[src_zip   ]%s' % src_zip)
    print('[target_dir]%s' % target_dir)

    if not os.path.isfile(src_zip):
        return -1;

    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    zip_exe = os.path.normpath(zip_exe)
    src_zip = os.path.normpath(src_zip)
    target_dir = os.path.normpath(target_dir)

    unzip_command = r'%s\7z.exe x -r -o"%s" -y "%s" *' % (
        zip_exe, target_dir, src_zip)
    print('[command ]%s' % unzip_command)

    if 0 != os.system(unzip_command):
        print "[ERROR]unzip_dump_report failed"
        return -1

    return 0

if __name__ == '__main__':
    if len(sys.argv) < 4 :
        print('usage: unzip.py [zip_exe] [src_zip.zip] [target_dir]')
        exit(-1)
    print('Unzip zip...')
    zip_exe = sys.argv[1]
    src_zip = sys.argv[2]
    target_dir = sys.argv[3]

    unzip(zip_exe, src_zip, target_dir)
