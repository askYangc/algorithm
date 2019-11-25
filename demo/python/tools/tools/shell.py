#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a interesting module '

__author__ = 'Yang Chuan'

import commands

def doShell(cmd, show_cmd=False):
    if(show_cmd == True):
        print cmd
    return commands.getstatusoutput(cmd)

def doShellFork(cmd, show_cmd=False):
    if(show_cmd == True):
        print cmd
    return subprocess.call([cmd], shell=True)