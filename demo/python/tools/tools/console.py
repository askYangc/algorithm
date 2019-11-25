#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a interesting module '

__author__ = 'Yang Chuan'


from gevent import monkey;monkey.patch_all()
import gevent
import sys
import os
import select
import time
import log.log as log


class FileLogger(object):
    def __init__(self, filename="Default.log"):
        self.terminal = sys.stdout
        self.log = open(filename, "a")

    def __del__(self):
        self.log.close()
        
    def write(self, message):
        #self.terminal.write(message)
        self.log.write(message)
        #log.Warning("FileLogger.write %s" % message)

    def flush(self):
        #log.Debug("FileLogger.flush")
        self.log.flush()

def config_debug(params):
    if len(params) != 1 or params[0] not in log.getLevelInfo():
        print "Use level %s" % '/'.join(log.getLevelInfo())
        return

    print "before level is %s" % log.getLevel()
    log.setLevel(params[0])
    print "now level is %s" % params[0]

commands = {
    "configdebug":config_debug,
}

def show_commands():
    print "supported linkserver console commands:"
    for cmd in commands:
        print "\t" + cmd

    sys.stdout.flush()

def do_console_cmd(cmd):
    command = cmd.strip(" \n").split(" ")
    if command[0] not in commands:
        show_commands()
        return
    commands[command[0]](command[1:])


def try_open_fifo_fd(pipe_file):
    fd = -1
    try:
        fd = os.open(pipe_file, os.O_RDONLY | os.O_NONBLOCK)
        return fd
    except Exception as e:
        print e

    print "open fifo"
    if os.path.exists(pipe_file):
        os.remove(pipe_file)

    os.mkfifo(pipe_file, 0666)
    try:
        fd = os.open(pipe_file, os.O_RDONLY | os.O_NONBLOCK)
    except Exception as e:
        print e
    finally:
        return fd

def consoleProc1():
    pipe_file = "/dsserver/sms_manager-in"
    pipe_file = "sms_manager-in"

    epoll = select.epoll()

    while True:
        try:
            fd = try_open_fifo_fd(pipe_file)
            epoll.register(fd, select.EPOLLIN)
            running = 1

            while running:
                poll_list = epoll.poll()
                print poll_list
                for sock, events in poll_list:
                    if events & select.EPOLLIN:
                        l = os.read(fd, 1024)
                        print l
                    if events & (select.EPOLLHUP|select.EPOLLERR):
                        #断开
                        running = 0
                        break
        except OSError as e:
            continue
        except Exception as e:
            continue
        finally:
            if fd > 0:
                print "fd: %s" % fd
                epoll.unregister(fd)
                os.close(fd)

    epoll.close()


def consoleProc():
    pipe_file = "/dsserver/sms_manager-in"
    #pipe_file = "sms_manager-in"

    poll = select.poll()

    while True:
        try:
            fd = try_open_fifo_fd(pipe_file)
            poll.register(fd, select.POLLIN)
            running = 1

            while running:
                poll_list = poll.poll()
                for sock,events in poll_list:
                    if events & select.POLLIN:
                        l = os.read(sock, 1024)
                        if len(l) == 0:
                            running = 0
                            break
                        do_console_cmd(l)
                    if events & (select.POLLHUP|select.POLLERR):
                        running = 0
                        break
        except OSError as e:
            print e
            continue
        except Exception as e:
            print e
            continue
        finally:
            if fd > 0:
                poll.unregister(fd)
                os.close(fd)

    poll.close()


def consoleInit():
    gevent.spawn(consoleProc)


if __name__ == "__main__":
    #consoleProc()
    #exit()
    #do_test()
    gevent.joinall([
        gevent.spawn(consoleProc)
    ])