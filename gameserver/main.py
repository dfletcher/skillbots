#!/usr/bin/python

import os
import MySQLdb
from time import sleep
from LanguagePlugin import open_bot_program

db=MySQLdb.connect(user="root",db="damnart_com")

if __name__ == '__main__':

  c = db.cursor()

  while True:

    sleep(1)

    # fetch an arena
    c.execute("SELECT aid,run FROM skb_arena WHERE NOT run LIMIT 1")
    arena = c.fetchone()
    if not arena:
      continue

    # fetch an arena
    c.execute("SELECT b.bid,b.uid,b.nid,b.ispublic,b.training,b.language,b.path FROM skb_bot b LEFT JOIN skb_arena_bot a ON b.bid = a.bid WHERE a.aid = %d" % arena[0])
    bots = c.fetchall()
    print "bots: " + str(bots)

    bot_programs = []
    for bot in bots:
      bot_programs.append(open_bot_program(bot[6], bot[5]))
    print "loaded bot programs: " + str(bot_programs)



  #c=db.cursor()
  #max_price=5
  #c.execute("""SELECT * FROM node""")
  #print(c.fetchall())
