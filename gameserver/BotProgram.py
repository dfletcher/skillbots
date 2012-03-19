#/usr/bin/python

import os
import threading
import subprocess
from time import sleep

SLEEP_TIME = 0.03
MAX_EXECUTION_TIME = 3.00

languages = {
  'javascript': './jsbot'
}

class BotProgramException(Exception):
  def __init__(self, message):
    Exception.__init__(self)
    self.message = message

class BotProgram(threading.Thread):
  
  def __init__(self, program, botpath):
    threading.Thread.__init__(self)
    self.input = None
    self.error = None
    self.program = subprocess.Popen(
      [program, botpath],
      bufsize=256,
      close_fds=True,
      stdin=subprocess.PIPE,
      stdout=subprocess.PIPE,
      stderr=subprocess.PIPE
    )

  def send(self, command):
    if self.program.poll() != None: return None
    self.input = None
    self.program.stdin.write(command + "\n")
    self.program.stdin.flush()
    if command == 'quit':
      self.program.kill()
      return 'ok'
    else:
      timeout = 0.0
      while (True):
        if self.input: return self.input
        elif self.error: raise BotProgramException(self.error)
        elif timeout > MAX_EXECUTION_TIME:
          self.program.kill()
          raise BotProgramException("Timeout waiting for command: " + command)
        sleep(SLEEP_TIME)
        timeout += SLEEP_TIME

  def run(self):
    while self.program.poll() == None:
      try:
        self.input = self.program.stdout.readline().strip()
      except:
        self.error = self.program.stderr.readline().strip()
      sleep(0.5)
    print "bot exit"

def open(botpath, language):
  program = BotProgram(languages[language], botpath)
  program.start()
  return program

