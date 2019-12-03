#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a interesting module '

__author__ = 'Yang Chuan'

import log.log as log
from db import mysqldbc
from tools import Dict
import tools


def do_test1():
    log.LogInit("udpserver-out", "debug", log.LOG_STD)


    log.Debug("%s okok", 2)
    log.Info("%s okok", 2)
    log.Warning("%s okok" % (2,))
    log.Error("%s okok", 2)

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

def do_test5():
    import time
    log.LogInit("udpserver-out", "debug", log.LOG_STD)
    k = {"autocommit":True}
    #db = mysqldbc("ijmaster", "ijjazhang", "sync_ijdbs", **k)
    db = mysqldbc("ijmaster", "ijjazhang", "sync_ijdbs")
    db.mysql_connect()

    db.begin()
    #time.sleep(10)
    sql = 'insert into test(name, num) value("nnn", 1)'
    r = db.insert(sql)
    db.info()
    #db.commit()
    if r is not None:
        print "insert ok"
        db.begin()
        #time.sleep(10)
        db.insert(sql)
        db.commit()
        db.commit()
    else:
        print "insert failed"
        db.rollback()
        db.info()

    sql = 'select * from test'
    print db.select(sql)
    db.info()
    sql = 'insert into test(name, num) value("nnn", 1)'
    #sql = 'update test set num=1 where id=83'
    #db.update(sql)


    #time.sleep(10)
    db.begin()
    #sql = 'select * from test'
    print db.insert(sql)
    db.info()
    db.commit()
    db.info()

    print "over"


if __name__ == "__main__":
    do_test5()