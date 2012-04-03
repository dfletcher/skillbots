#!/usr/bin/python

import os
import math
import json
import random
import MySQLdb
import traceback
import BotProgram
from Log import log
from time import sleep

# from http://snipplr.com/view.php?codeview&id=22482
def bresenham_line((x,y),(x2,y2)):
  """Brensenham line algorithm"""
  steep = 0
  coords = []
  dx = abs(x2 - x)
  if (x2 - x) > 0: sx = 1
  else: sx = -1
  dy = abs(y2 - y)
  if (y2 - y) > 0: sy = 1
  else: sy = -1
  if dy > dx:
    steep = 1
    x,y = y,x
    dx,dy = dy,dx
    sx,sy = sy,sx
  d = (2 * dy) - dx
  for i in range(0,dx):
    if steep: coords.append((y,x))
    else: coords.append((x,y))
    while d >= 0:
      y = y + sy
      d = d - (2 * dx)
    x = x + sx
    d = d + (2 * dy)
  coords.append((x2,y2))
  return coords

class Weapon(object):

  __idgen = 1
  def __id(self):
    rval = Weapon.__idgen
    Weapon.__idgen += 1
    return rval

  def __init__(self, bot):
    global __idgen
    self.aim = 0.0
    self.power = 1.0
    self.id = self.__id()
    self.bot = bot

  def _firetestcoords(self, arenawidth, arenaheight):
    # TODO: randomize aim a bit, have weapons with better accuracy
    a = self.aim
    x = self.bot.x
    y = self.bot.y
    l = max(arenawidth, arenaheight) * 2
    x2 = int(round(x + l * math.cos(a)))
    y2 = int(round(y + l * math.sin(a)))
    log('fire: (' + str(self.aim) + '): ' + str(x) + ',' + str(y) + ' => ' + str(x2) + ',' + str(y2))
    return (bresenham_line((x, y), (x2, y2)), x2, y2, l)

  def fire(self, obstacles, arenawidth, arenaheight):
    coords, x2, y2, l = self._firetestcoords(arenawidth, arenaheight)
    for coord in coords:
      for obstacle in obstacles:
        if obstacle.occupies(coord[0], coord[1]): return ('obstacle', obstacle, coord[0], coord[1], coords)
      for enemy in bot.enemies:
        if enemy.x == coord[0] and enemy.y == coord[1]: return ('enemy', enemy, coord[0], coord[1], coords)
    return ('none', None, x2, y2, coords)

class Bot(object):

  def __init__(self, dbbot):
    self.id = dbbot[0]
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

  def in_range(self, other):
    """ Test if an enemy is in scanning range of this bot. """
    # TODO: this could be better optimized.
    d = math.sqrt( math.pow(self.x - other.x, 2) + math.pow(self.y - other.y, 2) )
    return d <= self.scanning_range

  def damage(self, amount):
    self.condition -= amount

  def is_alive(self):
    return self.condition > 0.0

class Obstacle(object):

  def __init__(self, id, x, y, r):
    self.x = x
    self.y = y
    self.r = r
    self.id = id

  def in_range(self, bot):
    """ Test if this obstacle is in scanning range of a bot. """
    # TODO: this could be better optimized.
    d = math.sqrt( math.pow(self.x - bot.x, 2) + math.pow(self.y - bot.y, 2) )
    return d <= (bot.scanning_range + self.r)

  def occupies(self, x, y):
    """ Test if this obstacle occupies grid location x,y. """
    # TODO: this could be better optimized.
    d = math.sqrt( math.pow(self.x - x, 2) + math.pow(self.y - y, 2) )
    return d <= self.r

db=MySQLdb.connect(user="root", db="damnart_com")

if __name__ == '__main__':

  random.seed()

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

    # internal utility for adding events to the arena event stream
    def streamadd(t, cmd, args):
      c.execute("""
        INSERT INTO skb_arena_stream
        SET command='%s', aid=%d, t=%d, arguments='%s'
      """ % (cmd, arena[0], t, json.dumps(args).replace("'", "''")))

    # build bot objects
    bots = []
    deaths = []
    for dbbot in dbbots:
      bots.append(Bot(dbbot))

    # init arena
    arenaduration = 1000
    arenawidth = len(bots) * 6 + random.randint(2, 20)
    arenaheight = len(bots) * 6 + random.randint(2, 20)
    streamadd(-1, 'init', { 'arena': arena[0], 'w': arenawidth, 'h': arenaheight, 'max_duration': arenaduration })

    # init bots
    for bot in bots:
      try: bot.program.cmd_init(arena[0], arenawidth, arenaheight, arenaduration)
      except BotProgram.BotProgramException as e:
        streamadd(-1, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})

    # init obstacles
    obstacles = []
    numobstacles = int(math.sqrt(arenawidth * arenaheight) / 3)
    numobstacles += random.randint(-3, 3)
    for i in range(numobstacles):
      r = random.randint(1, 4)
      x = random.randint(0, arenawidth)
      y = random.randint(0, arenaheight)
      obstacle = Obstacle(i, x, y, r)
      for bot in bots:
        try: bot.program.cmd_obstacle(obstacle)
        except BotProgram.BotProgramException as e:
          streamadd(-1, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
      obstacles.append(obstacle)
    for obstacle in obstacles:
      streamadd(-1, 'obstacle', { 'id': obstacle.id, 'x': obstacle.x, 'y': obstacle.y, 'r': obstacle.r })

    # init weapons
    for bot in bots:
      for weapon in bot.weapons:
        try: bot.program.cmd_weapon(weapon)
        except BotProgram.BotProgramException as e:
          streamadd(-1, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
        streamadd(-1, 'weapon', { 'bot': bot.id, 'id': weapon.id, 'power': weapon.power, 'aim': weapon.aim })

    # init enemies
    for bot in bots:
      bot.enemies = []
      for enemy in bots:
        if bot.id != enemy.id:
          bot.enemies.append(enemy)

    # bot starting locations
    for bot in bots:
      good = False
      while not good:
        bot.x = random.randint(0, arenawidth)
        bot.y = random.randint(0, arenaheight)
        good = True
        for obstacle in obstacles:
          if obstacle.occupies(bot.x, bot.y):
            good = False
            break
        for enemy in bot.enemies:
          if enemy.x == bot.x and enemy.y == bot.y:
            good = False
            break
      streamadd(-1, 'bot', {
        'id': bot.id,
        'user': bot.uid,
        'training': 1 if bot.training else 0,
        'language': bot.language,
        'x': bot.x,
        'y': bot.y,
        'max_energy': bot.max_energy,
        'energy': bot.energy,
        'condition': bot.condition,
        'speed': bot.speed,
        'scanning_range': bot.scanning_range,
        'dir': bot.dir,
        'state': bot.state
      })

    # Run the arena.
    for t in range(arenaduration):

      # Update everybody's state.
      for bot in bots:

        # Update remote bot.
        try: bot.program.cmd_time(t)
        except BotProgram.BotProgramException as e:
          streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
        try: bot.program.cmd_bot(bot)
        except BotProgram.BotProgramException as e:
          streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})

        # Tell remote if obstacles are in range.
        for obstacle in obstacles:
          try: bot.program.cmd_obstacle_in_range(obstacle, obstacle.in_range(bot))
          except BotProgram.BotProgramException as e:
            streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})

        # Update remote enemy.
        for enemy in bot.enemies:
          try: bot.program.cmd_enemy(enemy, bot.in_range(enemy))
          except BotProgram.BotProgramException as e:
            streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
          for weapon in enemy.weapons:
            try: bot.program.cmd_enemy_weapon(enemy, weapon)
            except BotProgram.BotProgramException as e:
              streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})

        # Invoke stateChange().
        try: bot.state = bot.program.cmd_state_change()
        except BotProgram.BotProgramException as e:
          streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
          bot.state = 'stop'

        streamadd(t, 'bot', {
          'id': bot.id,
          'x': bot.x,
          'y': bot.y,
          'energy': bot.energy,
          'condition': bot.condition,
          'speed': bot.speed,
          'dir': bot.dir,
          'state': bot.state
        })

      # Everybody who's firing, aim.
      for bot in bots:
        if bot.state in ('attack', 'attack+move'):
          for weapon in bot.weapons:
            try: weapon.aim = bot.program.cmd_aim(weapon)
            except BotProgram.BotProgramException as e:
              streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
              weapon.aim = 0.0
            streamadd(t, 'aim', { 'bot': bot.id, 'weapon': weapon.id, 'aim': weapon.aim })

      # Everybody who's moving, move.
      for bot in bots:
        if bot.state in ('move', 'attack+move', 'defend+move'):

          # Change bot direction.
          try: bot.dir, bot.speed = bot.program.cmd_move()
          except BotProgram.BotProgramException as e:
            streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
            bot.dir = 'n'
            bot.speed = 0.0
          if bot.dir == 'n': d = (0, 0-bot.speed)
          elif bot.dir == 'ne': d = (bot.speed, 0-bot.speed)
          elif bot.dir == 'e': d = (bot.speed, 0)
          elif bot.dir == 'se': d = (bot.speed, bot.speed)
          elif bot.dir == 's': d = (0, bot.speed)
          elif bot.dir == 'sw': d = (0-bot.speed, bot.speed)
          elif bot.dir == 'w': d = (0-bot.speed, 0)
          elif bot.dir == 'nw': d = (0-bot.speed, 0-bot.speed)
          else: d = (0, 0)
          d = ( int(math.floor(d[0])), int(math.floor(d[1])) )

          # find desired new location.
          nx = (bot.x + d[0]) % arenawidth
          ny = (bot.y + d[1]) % arenaheight
          collision = False

          # Check for collisions with obstacles.
          for obstacle in obstacles:
            if obstacle.occupies(nx, ny):
              bot.damage(0.1)
              collision = True
              break

          # Check for collisions with enemies.
          for enemy in bot.enemies:
            if nx == enemy.x and ny == enemy.y:
              enemy.damage(0.1)
              bot.damage(0.1)
              collision = True
              break

          # Move to new location if bot didn't smash into something.
          if not collision:
            bot.x = nx
            bot.y = ny
            streamadd(t, 'move', { 'bot': bot.id, 'x': bot.x, 'y': bot.y })

      # Resolve combat.
      shots = []
      for bot in bots:
        if bot.state in ('attack', 'attack+move'):
          for weapon in bot.weapons:
            targettype, target, endx, endy, coords = weapon.fire(obstacles, arenawidth, arenaheight)
            shots.append(coords)
            if targettype == 'enemy':
              damage = 0.3
              if target.state == 'defend': damage = 0.1
              if target.state == 'defend+move': damage = 0.2
              target.damage(damage)
              streamadd(t, 'fire', {
                'bot': bot.id,
                'target_type': targettype,
                'enemy': target.id,
                'x': target.x,
                'y': target.y,
                'damage': damage
              })
              log("bot %d shot bot %d at (%d,%d) for damage %f" % (bot.id, target.id, target.x, target.y, damage))
              # TODO: notify target that it was hit
              # TODO: notify bot that it made a hit
              # TODO: notify other bots in range about the hit
            if targettype == 'obstacle':
              streamadd(t, 'fire', {
                'bot': bot.id,
                'target_type': targettype,
                'obstacle': target.id,
                'x': target.x,
                'y': target.y
              })
              log("bot %d shot %s %d at (%d,%d)" % (bot.id, targettype, target.id, target.x, target.y))
              # TODO: notify bots in range about the shot
            else:
              streamadd(t, 'fire', {
                'bot': bot.id,
                'target_type': targettype,
                'x': endx,
                'y': endy
              })
              log("bot %d fired a shot but hit nothin'" % (bot.id,))
              # TODO: notify bots in range about the shot

      # Remove dead bots.
      for bot in bots:
        if not bot.is_alive():
          streamadd(t, 'death', { 'bot': bot.id })
          log("bot %d has died" % (bot.id,))
          try: bot.program.cmd_quit()
          except BotProgram.BotProgramException as e:
            streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
          deaths.append((bot, t))
      bots = [bot for bot in bots if bot.is_alive()]

      # If only one bot remains it is the winner.
      if len(bots) == 1:
        streamadd(t, 'winner', { 'bot': bots[0].id })
        log("bot %d has won" % (bots[0].id,))
        break

      # If zero bots remain more than one was killed last round, it's a draw.
      elif len(bots) == 0:
        streamadd(t, 'deathdraw', { })
        log("it's a draw, bots died at same time.")
        break

      # Print the arena.
      def _inshot(x,y):
        for coords in shots:
          for coord in coords:
            if coord[0] == x and coord[1] == y: return True
        return False
      log('time: ' + str(t))
      log('-' * (arenawidth+2))
      for y in range(0, arenaheight):
        line = ''
        for x in range(0, arenawidth):
          if _inshot(x,y): v = '?'
          else: v = ' '
          for obstacle in obstacles:
            if obstacle.occupies(x, y): v = '*'
          for bot in bots:
            if bot.x == x and bot.y == y: v = str(bot.id)
          line += v
        log('|' + line + '|')
      log('-' * (arenawidth+2))
      log()

    # If more than one bot remains after end (t), it's a draw.
    if len(bots) > 1:
      for bot in bots: streamadd(t, 'livedraw', { 'bot': bot.id })
      log("it's a draw, bots survived.")

    # Shutdown all remaining bots.
    for bot in bots:
      try: bot.program.cmd_quit()
      except BotProgram.BotProgramException as e:
        streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})

    streamadd(t, 'end', { 'arena': arena[0] })

    c.execute("""
      UPDATE skb_arena
      SET run=1
      WHERE aid=%d
    """ % arena[0])
    db.commit()
