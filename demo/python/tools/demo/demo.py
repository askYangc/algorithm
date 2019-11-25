#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a interesting module '

__author__ = 'Yang Chuan'

import log
from db import mysqldbc
from tools import Dict
import tools


def do_test1():
    log.LogInit("udpserver-out", "debug", log.LOG_STD)

    log.ColorDebug(log.GREEN, "%s okok", 2)
    log.ColorInfo(log.GREEN, "%s okok", 2)
    log.ColorWarning(log.YELLOW, "%s okok" % (2,))
    log.ColorError(log.RED, "%s okok", 2)

def do_test2():
    import time
    log.LogInit("udpserver-out", "debug", log.LOG_STD)
    db = mysqldbc("ijmaster", "ijjazhang", "sync_ijdbs")


    db.begin()

    db.begin()
    print "begin"
    sql = 'select * from test'
    print db.select(sql)

    sql = 'insert into test(name, num) value("nnn", 1)'
    #sql = 'update test set num=1 where id=83'
    db.update(sql)
    db.rollback()

    time.sleep(5)
    db.commit()

def do_test3():
    import time
    filename = "/home/yangchuan/git_core/algorithm/demo/python/tools/1.log"
    try:
        tools.do_daemon(filename)
        print "haha"
    except Exception as e:
        print e

def do_test4():
    d = {"a": 1, "b": "hello"}
    d = Dict(a='1', b='2')
    #d = tools.todict(d)
    print type(d)

if __name__ == "__main__":
    do_test4()