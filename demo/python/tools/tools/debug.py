#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a test module '

__author__ = 'Yang Chuan'

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