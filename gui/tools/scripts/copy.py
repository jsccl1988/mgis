#!/usr/bin/python
import sys
import os
import shutil

def copy(src, target):
    print('[src   ]%s' % src)
    print('[target]%s' % target)

    target_dir = os.path.dirname(target)
    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    if os.path.isdir(src):
        shutil.copytree(src, target_dir)
        print "[copy dir]: src:%s,dest:%s" % \
            (src, target_dir)
    else:
        if os.path.isfile(src):
            shutil.copy2(src, target)
            print "[copy file]: src:%s,dest:%s" % \
                (src, target)
        else:
            print "[ERROR] %s not exist" % (src)
            return -1

    return 0

if __name__ == '__main__':
    if len(sys.argv) < 3 :
        print('usage: copy.py [src] [target]')
        exit(-1)
    print('Copy zip')
    src = sys.argv[1]
    target = sys.argv[2]

    copy(src, target)
