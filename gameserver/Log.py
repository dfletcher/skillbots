#!/usr/bin/python

logf = open('log', 'w')

def log(msg=''):
  logf.write(msg)
  logf.write('\n')
  logf.flush()
