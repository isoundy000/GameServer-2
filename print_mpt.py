#!/usr/bin/env python2
#coding: utf8

"""spt(mpt) 地图文件打印及可走点检查

示例:
./print_mpt.py config/map/s40001.spt # 打印地图
./print_mpt.py config/map/s40001.spt -p 2000    # 检查所有x像素为2000 的格子
./print_mpt.py config/map/s40001.spt -p ,120    # 检查所有y像素为120 的格子
./print_mpt.py config/map/s40001.spt -p 200,120 # 检查像素(200,120) 的格子
./print_mpt.py config/map/s40001.spt -c 20,12 # 检查格子(20,12)
"""

import struct
import sys

class MPT(object):
    def __init__(self, mpt):
        self.cell_width = None
        self.cell_height = None
        self.x_len = None 
        self.y_len = None
        self.map_width = None
        self.map_height = None
        self.i_point_y = None

        self.coords = []

        with open(mpt, 'rb') as fo:
            self.read_head(fo)
            fo.seek(40, 1)
            self.read_map(fo)

    def read_head(self, fo):
        (self.cell_width , 
        self.cell_height , 
        self.x_len,
        self.y_len ,
        self.map_width,
        self.map_height ,
        self.i_point_y) = struct.unpack(">%dH" % 7, fo.read(2*7))[:]

    def read_map(self, fo):
        for y in xrange(0, self.y_len):
            self.coords.append([])
            for x in xrange(0, self.x_len):
                self.coords[y].append(ord(fo.read(1)))

    @staticmethod
    def is_movable_coord(coord_value):
        return True if coord_value not in (1, -1) else False

    def check_cell(self, check_cell):
        px = check_cell[0]*self.cell_width if check_cell[0] else None
        py = check_cell[1]*self.cell_width if check_cell[1] else None
        print "Pixel(%r, %r), Cell(%r, %r)" % (px, py, 
						check_cell[0], check_cell[1])
        rv = self.print_map(check_cell)
        print "Pixel(%r, %r), Cell(%r, %r)" % (px, py, 
						check_cell[0], check_cell[1])
        return rv

    def check_pixel(self, check_pixel):
        cx = check_pixel[0]/self.cell_width if check_pixel[0] else None
        cy = check_pixel[1]/self.cell_height if check_pixel[1] else None 
        print "Pixel(%r, %r), Cell(%r, %r)" % (check_pixel[0], check_pixel[1], 
						cx, cy)
        rv = self.print_map((cx, cy))
        print "Pixel(%r, %r), Cell(%r, %r)" % (check_pixel[0], check_pixel[1], 
						cx, cy)
        return rv

    def print_map(self, check_pixel=None, strip=True):
        if check_pixel:
            if check_pixel[0]:
                assert check_pixel[0] < self.x_len, "out of map range"
            if check_pixel[1]:
                assert check_pixel[1] < self.y_len, "out of map range"

        lines = []
        for y in xrange(0, self.y_len):
            line = []
            for x in xrange(0, self.x_len):
                is_movable = self.is_movable_coord(self.coords[y][x])
                is_check_cell = False
                if check_pixel is not None:
                    if check_pixel[1] is not None and check_pixel[0] is not None:
                        is_check_cell = True if x == check_pixel[0] and y == check_pixel[1] else False
                    else:
                        is_check_cell = True if x == check_pixel[0] or y == check_pixel[1] else False

                if is_check_cell:
                    out = 'Y' if is_movable else 'X'
                else:
                    out = 'O' if is_movable else '-'
                line.append(out)
            lines.append("".join(line))

        y_no = 0
        if strip:
            # 去掉地图顶部的空行
            while not lines[0].strip("-"):
                    y_no += 1
                    lines.pop(0)

            # 去掉地图底部的空行
            while not lines[-1].strip("-"):
                    lines.pop()

        x_nos = []
        for i in xrange(0, len(lines[0])):
            out = str(i%10)
            if out == '0':
                out = "\033[31m%d\033[0m" % ((i/10)%10)
            x_nos.append(out)
        x_nos = "".join(x_nos)
       
        CX = "\033[31m~\033[0m"
        CY = "\033[31mQ\033[0m"
        print "   ",x_nos # 开始4个位置留给行号
        for line in lines:
            y_no_s = "%03d" % y_no
            print y_no_s, line.replace("X", CX).replace("Y", CY), y_no_s
            y_no += 1
        print "   ",x_nos

def _print_usage():
    print >> sys.stderr, "Usage: %s <spt file> [-p|-c] [x,y]"
    sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) < 2 or sys.argv[1] in ('-h', '--help'):
        _print_usage()
    mpt = sys.argv[1]

    m = MPT(mpt)

    check_func = m.print_map
    check_coord = None
    if len(sys.argv) > 3:
        coord_type = sys.argv[2]
        if coord_type == '-c':
            check_func = m.check_cell
        elif coord_type == '-p':
            check_func = m.check_pixel
        else:
            _print_usage()
        
        t_coord = sys.argv[3].split(",")
        t_coord.append(None)
        try:
            x = int(t_coord[0]) if t_coord[0] else None
            y = int(t_coord[1]) if t_coord[1] else None
            check_coord = (x,y)
        except Exception, e:
            print >>sys.stderr, e
            _print_usage()

    check_func(check_coord)
