#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a interesting module '

__author__ = 'Yang Chuan'

import beanstalkc

class BSQueueC(object):

    def __init__(self, host='localhost', port=11300):
        self.host = host
        self.port = port
        self.__watch_tube = []
        self.__use_tube = ""
        self.__conn = beanstalkc.Connection(host, port)
        self.__use_tube = self.__conn.using()
        for w in self.__conn.watching():
            self.__watch_tube.append(w)

    def __del__(self):
        self.close()

    def close(self):
        if self.__conn:
            self.__conn.close()
        self.__conn = None

    def use(self, tube):
        self.__use_tube = tube
        self.__conn.use(tube)

    def watch(self, tube):
        if tube not in self.__watch_tube:
            self.__watch_tube.append(tube)
        self.__conn.watch(tube)

    def watching(self):
        return self.__watch_tube

    def using(self):
        return self.__use_tube

    def ignore(self, tube):
        if tube in self.__watch_tube:
            self.__watch_tube.remove(tube)
            self.__conn.ignore(tube)

    def put(self, body, priority=2 ** 31, delay=0, ttr=120):
        return self.__conn.put(body, priority, delay, ttr)

    def putTube(self, tube, body, priority=2 ** 31, delay=0, ttr=120):
        self.use(tube)
        return self.__conn.put(body, priority, delay, ttr)

    def reserve(self, timeout=None):
        return self.__conn.reserve(timeout)

    def reserveTube(self, tube, timeout=None):
        self.watch(tube)
        for t in self.watching():
            if t != tube:
                self.ignore(t)
        return self.reserve(timeout)

    def clear(self, tube):
        try:
            while 1:
                job = self.reserve(tube, 1)
                if job is None:
                    break
                else:
                    job.delete()
        except Exception, e:
            print e

    def stats_tube(self, tube):
        return self.__conn.stats_tube(tube)

    def __str__(self):
        s =  "host: %s port: %s\nuse_tube: \n\t%s\nwatch_tube:\n" % (self.host, self.port, self.__use_tube)
        for w in self.__watch_tube:
            s = s + '\t' + w + '\n'
        return s

    __repr__ = __str__

def bsSendProc(bs, q):
    while True:
        t = q.get()
        bs.put(t)

def bsRecvProc(bs, q):
    while True:
        c = bs.reserve()
        q.put(c.body)
        c.delete()


def do_test():
    bs = BSQueueC()
    bs.watch("bar")
    bs.ignore("default")
    job = bs.reserve()
    print job.body
    print job.stats()
    job.delete()


if __name__=='__main__':
    do_test()