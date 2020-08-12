#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a test module '

__author__ = 'Yang Chuan'

import sys
import os
import re
import logging
import getopt
import mysql.connector
import commands

reload(sys)
sys.setdefaultencoding('utf-8')


COLOR_RED="\033[31m\033[1m"
COLOR_GREEN="\033[32m\033[1m"
COLOR_YELLOW="\033[33m\033[1m"
COLOR_END="\033[0m"

def printInfo(str):
    print COLOR_GREEN + str + COLOR_END

def printErr(str):
    print COLOR_RED + str + COLOR_END

def printWarn(str):
    print COLOR_YELLOW + str + COLOR_END

def doShell(cmd, show_cmd=False):
    if(show_cmd == True):
        print cmd
    return commands.getstatusoutput(cmd)

def doShellFork(cmd, show_cmd=False):
    if(show_cmd == True):
        print cmd
    return subprocess.call([cmd], shell=True)

engine = None
mysqldb = None
    
def create_engine(user, password, database, host='127.0.0.1', port=3306, **kw):
    global engine
    if engine is not None:
        raise DBError('Engine is already initialized.')
    params = dict(user=user, password=password, database=database, host=host, port=port)
    defaults = dict(use_unicode=True, charset='utf8', collation='utf8_general_ci', autocommit=False)
    for k, v in defaults.iteritems():
        params[k] = kw.pop(k, v)
    params.update(kw)
    params['buffered'] = True
    engine = lambda: mysql.connector.connect(**params)
    # test connection...
    logging.info('Init mysql engine <%s> ok.' % hex(id(engine)))    
    
class mysqldbc(object):
    
    def __init__(self):
        self.connection = None
  
    def mysql_connect(self):
        if self.connection is None:
            self.connection = engine()
        return self.connection
        
    def mysql_close(self):
        if self.connection:
            self.connection.close()
            
    def cursor(self):
        self.mysql_connect()
        return self.connection.cursor()
       
    def commit(self):
        if self.connection:
            self.connection.commit()

def Select(sql):
    try:
        cursor = mysqldb.cursor()
        cursor.execute(sql)
        result = [x for x in cursor.fetchall()]
        return result
    except Exception, e:
        print e
        print sql
        exit(0)
    finally:
        if cursor:
            cursor.close()

def Insert(sql):
    try:
        cursor = mysqldb.cursor()
        cursor.execute(sql)
        mysqldb.commit()
    except Exception, e:
        print e
        print sql
        exit(0)
    finally:
        if cursor:
            cursor.close()


            
mems = []

#配置
#检测的程序名
progs = 'queryserver'
#progs = 'linkserver'
#需要过滤的函数名
#func_name_filters = ['ir_lib_load', 'bs_recv_message', 'linkserver_init', 'thread_mm_mysql_init', 'cJSON_New_Item', 'parse_string', 'cJSON_strdup', 'do_parsejson_freedomjson_exec_dev']
func_name_filters = ['libmysqlclient']
#输入文件
LogFileName = 'ex.log'
#输出文件
OutLogFileName = 'ex.log'
#要转换的栈函数层次
FrameToFunc = -1

def get_mem_info():
    global mems, Addr2Func
    with open(LogFileName, "r") as f:
        d = f.read()
        mems = d.split('===========alloc===============')[1:]

def writeLog():
    global mems
    with open(OutLogFileName, "w") as f:       
        f.write('===================mem_alloc_show====================\n===========alloc===============')
        for m in mems:
            f.write(m)
            f.write('===========alloc===============')
    
"""
addr: 0xdcc3b0, stack_num: 5, size: 152
stacktrace:
        frame 0 :./linkserver(__wrap_calloc+0x59) [0x4ece85]
        frame 1 :./linkserver(irlib_dynamic_init+0x1f) [0x4d2540]
        frame 2 :./linkserver(ir_lib_load+0x20) [0x4d2d22]
        frame 3 :./linkserver(linkserver_init+0xf4) [0x43ab15]
        frame 4 :./linkserver(main+0x91) [0x43bf92]
        
stacktrace:
        frame 0 :./linkserver(__wrap_malloc+0x4e) [0x4ecf1d]
        frame 1 :./linkserver() [0x4e0fcd]
        frame 2 :./linkserver() [0x4e2d29]
        frame 3 :./linkserver() [0x4e2429]
        frame 4 :./linkserver() [0x4e2786]
        frame 5 :./linkserver() [0x4e2409]
        frame 6 :./linkserver() [0x4e2ede]
        frame 7 :./linkserver() [0x4e2429]
        frame 8 :./linkserver() [0x4e2786]
        frame 9 :./linkserver() [0x4e2409]        
"""

def check_mem(m):
    #过滤第一遍有函数名字的
    for f in func_name_filters:
        if f in m:
            return False
    
    #过滤第二遍栈结构开始，解析出来函数名字的
        
    r = m.strip()
    if len(r) == 0:
        return False
    
    return True
            
def check_mems():
    global mems
    
    m = []
    print "before len(mems): %s" % len(mems)
    for mem in mems:
        if check_mem(mem):
            m.append(mem)
    
    print "after len(m): %s" % len(m)
    mems = m
    writeLog()
    showAddr()
  
 
def doFrameToFunc():
    global FrameToFunc, progs, mems
    li = []
    print "total %s" % len(mems)
    i = 0
    for m in mems:
        #print m
        frameCount = FrameToFunc+1
        data = m.split('frame')
        if len(data) <= frameCount:
            li.append(m)
            i = i + 1
            continue
        data = m.split('frame')[FrameToFunc+1].strip()
        addr = data.split('[')[1].split(']')[0]
        cmd = 'addr2line -e %s -f -C -a %s -p' % (progs, addr)
        err, r = doShell(cmd)
        func_name = r.split(':')[1].split('at')[0]
        line = '/'.join(r.split('/')[-2:])
        cur = m.find(addr) + len(addr) + 1
        new_m = m[:cur] + func_name + line + m[cur:]
        li.append(new_m)
        i = i + 1
        if i % 100 == 0:
            print "now %s/%s" % (i+1, len(mems))
        
    mems = li

def showAddr():
    print 'addr2line -e linkserver -f -C -a 0x425355 -p'
        
def do_test(): 
    global FrameToFunc
    get_mem_info()
    
    if FrameToFunc != -1:
        doFrameToFunc()
        writeLog()
        return 
    
    check_mems()
   
def do_help():
    print "Uspage %s these options:" % (sys.argv[0], )
    print "\t -c num (指定转换的栈地址为函数名的层次 1, 2, 3)"
    print "\t -r 运行过滤操作"
    exit()

def do_option():
    global FrameToFunc
    shortopts = "hc:r"
    longopts = ["help", 'run', "devtype="]
    try:
        options, args = getopt.getopt(sys.argv[1:], shortopts, longopts)
    except getopt.GetoptError, e:
        print e
        sys.exit()

    if len(options) == 0:
        do_help()
        
    for option, value in options:
        if option in ('-h', '--h', '--help'):
            do_help()   
        if option in ('-r', '--run'):
            continue   
        if option in ('-c'):
            FrameToFunc = int(value)

if __name__=='__main__':
    logging.basicConfig(level=logging.DEBUG)
    #test()
    #do_insert = False
          
    #create_engine("ijmaster", 'ijjazhang', 'ijdbs', host='127.0.0.1', port=3306)
    #mysqldb = mysqldbc()

    do_option()
    do_test()
    
    #mysqldb.mysql_close()