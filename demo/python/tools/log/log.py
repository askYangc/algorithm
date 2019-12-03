#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a interesting module '

__author__ = 'Yang Chuan'

import sys
import os
import logging
from logging import handlers
import time

reload(sys)
sys.setdefaultencoding('utf-8')

COLOR_RED="\033[31m\033[1m"
COLOR_GREEN="\033[32m\033[1m"
COLOR_YELLOW="\033[33m\033[1m"
COLOR_END="\033[0m"

GREEN = 1
YELLOW = 2
RED = 3

ColorMap = {
    GREEN:COLOR_GREEN,
    YELLOW:COLOR_YELLOW,
    RED:COLOR_RED
}

def printInfo(str):
    print COLOR_GREEN + str + COLOR_END

def printErr(str):
    print COLOR_RED + str + COLOR_END

def printWarn(str):
    print COLOR_YELLOW + str + COLOR_END


#备份数据
def backuplog(filename):
    if os.path.exists(filename):
        newName = filename + "-" + time.strftime("%Y-%m-%d-%H-%M-%S", time.localtime())
        os.rename(filename, newName)

class MyRotatingFileHandler(handlers.BaseRotatingHandler):
    """
    Handler for logging to a set of files, which switches from one file
    to the next when the current file reaches a certain size.
    """
    def __init__(self, filename, mode='a', maxBytes=0, suffix="%Y-%m-%d-%H-%M-%S", encoding=None, delay=0):
        """
        Open the specified file and use it as the stream for logging.
        By default, the file grows indefinitely. You can specify particular
        values of maxBytes and backupCount to allow the file to rollover at
        a predetermined size.
        Rollover occurs whenever the current log file is nearly maxBytes in
        length. If backupCount is >= 1, the system will successively create
        new files with the same pathname as the base file, but with extensions
        ".1", ".2" etc. appended to it. For example, with a backupCount of 5
        and a base file name of "app.log", you would get "app.log",
        "app.log.1", "app.log.2", ... through to "app.log.5". The file being
        written to is always "app.log" - when it gets filled up, it is closed
        and renamed to "app.log.1", and if files "app.log.1", "app.log.2" etc.
        exist, then they are renamed to "app.log.2", "app.log.3" etc.
        respectively.
        If maxBytes is zero, rollover never occurs.
        """
        # If rotation/rollover is wanted, it doesn't make sense to use another
        # mode. If for example 'w' were specified, then if there were multiple
        # runs of the calling application, the logs from previous runs would be
        # lost if the 'w' is respected, because the log file would be truncated
        # on each run.
        if maxBytes > 0:
            mode = 'a'
        handlers.BaseRotatingHandler.__init__(self, filename, mode, encoding, delay)
        self.maxBytes = maxBytes
        self.suffix = suffix

    def doRollover(self):
        """
        Do a rollover, as described in __init__().
        """
        if self.stream:
            self.stream.close()
            self.stream = None

        backuplog(self.baseFilename)
        #logging version > 2.6 has delay
        if hasattr(self, "delay"):
            if not self.delay:
                self.stream = self._open()
        else:
            self.stream = self._open()
        sys.stdout = open(self.baseFilename, 'w')

    def shouldRollover(self, record):
        """
        Determine if rollover should occur.
        Basically, see if the supplied record would cause the file to exceed
        the size limit we have.
        """
        if self.stream is None:                 # delay was set...
            self.stream = self._open()
        if self.maxBytes > 0:                   # are we rolling over?
            msg = "%s\n" % self.format(record)
            self.stream.seek(0, 2)  #due to non-posix-compliant Windows feature
            if self.stream.tell() + len(msg) >= self.maxBytes:
                return 1
        return 0

#位与关系
#日志记录
LOG_FILE = 0x1
#文件输出
LOG_STD = 0x2



class Logger(object):
    level_relations = {
        'debug':logging.DEBUG,
        'info':logging.INFO,
        'warning':logging.WARNING,
        'error':logging.ERROR,
        'crit':logging.CRITICAL
    }#日志级别关系映射

    def __init__(self,filename,level='info', mode=LOG_STD, fmt='%(asctime)s - %(filename)s[%(module)s line:%(lineno)d] - %(levelname)s: %(message)s'):
        self.logger = logging.getLogger(filename)
        self.format_str = logging.Formatter(fmt)#设置日志格式
        #self.format_str = logging.Formatter(fmt, datefmt='%y-%m-%d %H:%M:%S')  # 设置日志格式
        self.logger.setLevel(self.level_relations.get(level))#设置日志级别
        self.level = level

        if LOG_FILE & mode:
            #self.addRotatingFileHandler(filename, maxBytes=268435456, backupCount=256)
            self.addMyRotatingFileHandler(filename, maxBytes=268435456)
        if LOG_STD & mode:
            self.addStreamHandler()

        #self.addStreamHandler()
        #self.addTimedRotatingFileHandler(filename)

    def addRotatingFileHandler(self, filename, mode='a', maxBytes=0, backupCount=0, encoding='utf-8', delay=0):
        rh = handlers.RotatingFileHandler(filename, mode, maxBytes, backupCount, encoding, delay)
        rh.setFormatter(self.format_str)  # 设置屏幕上显示的格式
        self.logger.addHandler(rh)

    def addMyRotatingFileHandler(self, filename, mode='a', maxBytes=0, suffix='%Y-%m-%d-%H-%M-%S', encoding='utf-8', delay=0):
        rh = MyRotatingFileHandler(filename, mode, maxBytes, suffix, encoding, delay)

        rh.setFormatter(self.format_str)  # 设置屏幕上显示的格式
        self.logger.addHandler(rh)

    def addStreamHandler(self):
        sh = logging.StreamHandler()  # 往屏幕上输出
        sh.setFormatter(self.format_str)  # 设置屏幕上显示的格式
        self.logger.addHandler(sh)  # 把对象加到logger里


    def addTimedRotatingFileHandler(self, filename, when='S', backCount=0,fmt='%(asctime)s - %(pathname)s[line:%(lineno)d] - %(levelname)s: %(message)s'):
        th = handlers.TimedRotatingFileHandler(filename=filename,when=when,backupCount=backCount,encoding='utf-8')#往文件里写入#指定间隔时间自动生成文件的处理器
        #实例化TimedRotatingFileHandler
        #interval是时间间隔，backupCount是备份文件的个数，如果超过这个个数，就会自动删除，when是间隔的时间单位，单位有以下几种：
        # S 秒
        # M 分
        # H 小时、
        # D 天、
        # W 每星期（interval==0时代表星期一）
        # midnight 每天凌晨
        th.setFormatter(self.format_str)#设置文件里写入的格式
        self.logger.addHandler(th)

    def setLevel(self, level):
        self.logger.setLevel(self.level_relations.get(level))  # 设置日志级别

    def getLevel(self):
        return self.level

    def getLevelInfo(self):
        return self.level_relations.keys()


    def logEnable(self):
        self.logger.disabled = False

    def logDisable(self):
        self.logger.disabled = True
    """
    def debug(self, *args, **kw):
        self.logger.debug(*args, **kw)

    def info(self, *args, **kw):
        self.logger.info(*args, **kw)

    def warning(self, *args, **kw):
        self.logger.warning(*args, **kw)

    def error(self, *args, **kw):
        self.logger.error(*args, **kw)
    """

"""
def Debug(*args, **kw):
    if logger is None:
        printErr("LogInit.debug is err")
        return
    print logger.logger.findCaller()
    logger.logger.debug(*args, **kw)

def Info(*args, **kw):
    if logger is None:
        printErr("LogInit.info is err")
        return
    logger.logger.info(*args, **kw)

def Warning(*args, **kw):
    if logger is None:
        printErr("LogInit.warning is err")
        return
    logger.logger.warning(*args, **kw)

def Error(*args, **kw):
    if logger is None:
        printErr("LogInit.error is err")
        return
    logger.logger.error(*args, **kw)
"""

def ColorDebug(color=GREEN, *args, **kw):
    if logger is None:
        printErr("LogInit.debug is err")
        return
    if len(args) > 0:
        args = list(args)
        args[0] = ColorMap[color] + args[0] + COLOR_END
    logger.logger.debug(*args, **kw)

def ColorInfo(color=GREEN, *args, **kw):
    if logger is None:
        printErr("LogInit.info is err")
        return
    if len(args) > 0:
        args = list(args)
        args[0] = ColorMap[color] + args[0] + COLOR_END
    logger.logger.info(*args, **kw)

def ColorWarning(color=GREEN, *args, **kw):
    if logger is None:
        printErr("LogInit.warning is err")
        return
    if len(args) > 0:
        args = list(args)
        args[0] = ColorMap[color] + args[0] + COLOR_END
    logger.logger.warning(*args, **kw)

def ColorError(color=GREEN, *args, **kw):
    if logger is None:
        printErr("LogInit.error is err")
        return
    if len(args) > 0:
        args = list(args)
        args[0] = ColorMap[color] + args[0] + COLOR_END
    logger.logger.error(*args, **kw)

def setLevel(level):
    logger.setLevel(level)

def getLevel():
    return logger.getLevel()

def getLevelInfo():
    return logger.getLevelInfo()

logger = None
Debug = None
Info = None
Warning = None
Error = None

def LogInit(filename="all.log", level='warning', mode=LOG_STD):
    global logger,Debug,Info,Warning,Error
    logger = Logger(filename, level, mode=mode)
    Debug = logger.logger.debug
    Info = logger.logger.info
    Warning = logger.logger.warning
    Error = logger.logger.error



if __name__ == '__main__':
    LogInit("udpserver-out", level="warning", mode=LOG_STD)
    #LogInit("udpserver-out", mode="desk")
    a = 0
    while a < 1:
        a = a + 1
        Debug('debug111')
        #Debug('debug222 %s', 4)
        Info('info')
        setLevel("info")
        Info('info2')
        Warning('警告')
        Error('报错')
        time.sleep(1)
    #log.logger.critical('严重')
    #Logger('error.log', level='error').logger.error('error')
