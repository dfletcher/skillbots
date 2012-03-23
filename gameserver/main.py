#!/usr/bin/python

import os
import math
import random
import MySQLdb
import traceback
import BotProgram
from time import sleep

class Weapon(object):

  def __init__(self, bot):
    self.aim = 0.0
    self.power = 1.0
    self.id = 1
    self.bot = bot

  def fire(self):
    pass

class Bot(object):

  def __init__(self, dbbot):
    self.bid = dbbot[0]
    self.uid = dbbot[1]
    self.nid = dbbot[2]
    self.ispublic = dbbot[3]
    self.training = dbbot[4]
    self.language = dbbot[5]
    self.path = dbbot[6]
    self.program = BotProgram.open(dbbot[6], dbbot[5])
    self.x = 0
    self.y = 0
    self.max_energy = 25
    self.energy = 25
    self.condition = 1.0
    self.speed = 1.0
    self.scanning_range = 8
    self.dir = 'n'
    self.state = 'move'
    self.weapons = [ Weapon(self) ]

  def inrange(self, other):
    """ Test if an enemy is in scanning range of this bot. """
    # TODO: this could be better optimized.
    d = sqrt( math.pow(self.x - other.x, 2) + math.pow(self.y - other.y, 2) )
    return d <= self.scanning_range

class Obstacle(object):

  def __init__(self, id, x, y, r):
    self.x = x
    self.y = y
    self.r = r
    self.id = id

  def inrange(self, bot):
    """ Test if this obstacle is in scanning range of a bot. """
    # TODO: this could be better optimized.
    d = sqrt( math.pow(self.x - bot.x, 2) + math.pow(self.y - bot.y, 2) )
    return d <= bot.scanning_range

  def occupies(self, x, y):
    """ Test if this obstacle occupies grid location x,y. """
    # TODO: this could be better optimized.
    d = sqrt( math.pow(self.x - x, 2) + math.pow(self.y - y, 2) )
    return d <= self.r

db=MySQLdb.connect(user="root", db="damnart_com")

if __name__ == '__main__':

  c = db.cursor()

  while True:

    sleep(1)

    # fetch an arena
    c.execute("""
      SELECT aid,run
      FROM skb_arena
      WHERE NOT run
      LIMIT 1
    """)
    arena = c.fetchone()
    if not arena: continue

    # fetch bots in arena
    c.execute("""
      SELECT b.bid,b.uid,b.nid,b.ispublic,b.training,b.language,b.path
      FROM skb_bot b
      LEFT JOIN skb_arena_bot a ON b.bid = a.bid
      WHERE a.aid = %d
    """ % arena[0])
    dbbots = c.fetchall()

    # build bot objects
    bots = []
    for dbbot in dbbots:
      bots.append(Bot(dbbot))

    # init arena
    arenaduration = 1000
    arenawidth = len(bots) * 6 + random.randint(2, 20)
    arenaheight = len(bots) * 6 + random.randint(2, 20)
    for bot in bots:
      bot.program.cmd_init(arenawidth, arenaheight, arenaduration)
      for weapon in bots.weapons: bot.program.cmd_weapon(weapon)
    print('TODO: create db arena ' + str(arenawidth) + ' ' + str(arenaheight) + str(arenaduration))

    # init obstacles
    obstacles = []
    numobstacles = int(math.sqrt(arenawidth * arenaheight) / 2)
    numobstacles += random.randint(-5, 5)
    for i in range(numobstacles):
      r = random.randint(1, 4)
      x = random.randint(0, arenawidth)
      y = random.randint(0, arenaheight)
      obstacle = Obstacle(i, x, y, r)
      for bot in bots: bot.program.cmd_obstacle(obstacle)
      obstacles.append(obstacle)
      print('TODO: create db obstacle ' + str(obstacle))

    # bot starting locations
    print('TODO: add bots to arena ' + str(bots))

    # init enemies
    for bot in bots:
      bot.enemies = []
      for enemy in bots:
        if bot.bid != enemy.bid:
          bot.enemies.append(enemy)
          bot.program.cmd_enemy(enemy)
          for weapon in enemy.weapon:
            bot.program.cmd_enemy_weapon(enemy, weapon)

    # Run the arena.
    for t in range(arenaduration):

      # Update everybody's state.
      for bot in bots:

        # Update time.
        bot.program.cmd_time(t)

        # Update remote bot.
        bot.program.cmd_bot(bot)

        # Tell remote if obstacles are in range.
        for obstacle in obstacles:
          bot.program.cmd_obstacle_in_range(obstacle, obstacle.inrange(bot))

        # Update remote enemy.
        for enemy in bot.enemies:
          bot.program.cmd_enemy(enemy, enemy.inrange(bot))

        # Invoke stateChange().
        bot.state = bot.program.cmd_state_change()

      # Everybody who's firing, aim.
      for bot in bots:
        if bot.state in ('attack', 'attack+move'):
          for weapon in bot.weapons:
            weapon.aim = bot.program.cmd_aim(weapon)

      # Everybody who's moving, move.
      for bot in bots:
        s = bot.state
        if bot.state in ('move', 'attack+move', 'defend+move'):

          # Change bot direction.
          bot.dir, bot.speed = bot.program.cmd_move()
          if bot.dir == 'n': d = (0.0, 0.0-bot.speed)
          elif bot.dir == 'ne': d = (bot.speed, 0.0-bot.speed)
          elif bot.dir == 'e': d = (bot.speed, 0.0)
          elif bot.dir == 'se': d = (bot.speed, bot.speed)
          elif bot.dir == 's': d = (0.0, bot.speed)
          elif bot.dir == 'sw': d = (0.0-bot.speed, bot.speed)
          elif bot.dir == 'w': d = (0.0-bot.speed, 0.0)
          elif bot.dir == 'nw': d = (0.0-bot.speed, 0.0-bot.speed)
          else: d = (0.0, 0.0)

          # find desired new location.
          nx = (bot.x + d[0]) % arenawidth
          ny = (bot.y + d[1]) % arenaheight
          collision = False

          # Check for collisions with obstacles.
          for obstacle in obstacles:
            if obstacle.occupies(nx, ny):
              # TODO: collision with obstacle, damage
              break

          # Check for collisions with enemies.
          for enemy in bot.enemies:
            if nx == enemy.x and ny == enemy.y:
              # TODO: collision with enemy, damage
              break

          # Move to new location if bot didn't smash into something.
          if not collision:
            bot.x = nx
            bot.y = ny

      # Resolve combat.
      for bot in bots:
        if bot.state in ('attack', 'attack+move'):
          for weapon in bot.weapons:
            targettype, target, endx, endy = weapon.fire()
            if targettype == 'enemy':
              # TODO: calculate damage here
              if target.state == 'defend': pass
              elif target.state == 'defend+move': pass
              else: pass
            elif targettype == 'obstacle':
              pass
            else:
              pass
            # TODO: notify bot and enemies in range about shot

    for bot in bots: bot.program.cmd_quit()
