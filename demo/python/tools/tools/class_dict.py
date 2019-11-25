#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a interesting module '

__author__ = 'Yang Chuan'


class Dict(dict):
    '''
    Simple dict but support access as x.y style.
    '''
    def __init__(self, names=(), values=(), **kw):
        super(Dict, self).__init__(**kw)
        for k, v in zip(names, values):
            self[k] = v

    def __getattr__(self, key):
        try:
            return self[key]
        except KeyError:
            raise AttributeError(r"'Dict' object has no attribute '%s'" % key)

    def __setattr__(self, key, value):
        self[key] = value

def toDict(d):
    D = Dict()
    for k, v in d.iteritems():
        D[k] = toDict(v) if isinstance(v, dict) else v
    return D

def todict(D):
    d = dict()
    for k, v in D.iteritems():
        d[k] = todict(v) if isinstance(v, Dict) else v
    return d
