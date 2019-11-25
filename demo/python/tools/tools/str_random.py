#!/usr/bin/env python
# -*- coding: utf-8 -*-

' a interesting module '

__author__ = 'Yang Chuan'
import random
import string

def rand_char():
    return random.choice('abcdefghijklmnopqrstuvwxyz!@#$%^&*()')

def rand_int(a,b):
    return random.randint(a, b)

def rand_list(n):
    return random.sample('zyxwvutsrqponmlkjihgfedcba1234567890', n)

def get_rand(n):
    return  ''.join(random.sample(string.ascii_letters + string.digits, n))

def do_test():
    print get_rand(8)

if __name__ == '__main__':
    do_test()