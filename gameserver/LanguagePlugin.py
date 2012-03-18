#/usr/bin/python

import os

language_plugin_classes = None

class BotProgram(object):
  
  def init(self, w, h, bot, obstacles):
    raise NotImplementedError

  def set(self, bot, time, enemies):
    raise NotImplementedError

  def change_state(self):
    raise NotImplementedError

  def aim(self):
    raise NotImplementedError

  def move(self):
    raise NotImplementedError

class LanguagePlugin(object):

  def init(self):
    raise NotImplementedError
 
  def language(self):
    raise NotImplementedError
 
  def open(self, srcpath):
    raise NotImplementedError

def open_bot_program(srcpath, language):
  return None
