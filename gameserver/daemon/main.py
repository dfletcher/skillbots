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
    x = self.bot.get('x')
    y = self.bot.get('y')
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
        if enemy.get('x') == coord[0] and enemy.get('y') == coord[1]: return ('enemy', enemy, coord[0], coord[1], coords)
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
    self.weapons = [ Weapon(self) ]
    self.data = {}
    self.set('x', 0, True)
    self.set('y', 0, True)
    self.set('max_energy', 25, True)
    self.set('energy', 25, True)
    self.set('condition', 1.0, True)
    self.set('speed', 1.0, True)
    self.set('scanning_range', 12, True)
    self.set('dir', 'n', True)
    self.set('state', 'move', True)

  def in_range(self, other):
    """ Test if an enemy is in scanning range of this bot. """
    # TODO: this could be better optimized.
    d = math.sqrt( math.pow(self.get('x') - other.get('x'), 2) + math.pow(self.get('y') - other.get('y'), 2) )
    return d <= self.get('scanning_range')

  def damage(self, amount):
    self.set('condition', self.get('condition') - amount)

  def is_alive(self):
    return self.get('condition') > 0.0

  def get(self, key, dflt=0):
    if not key in self.data: return dflt
    return self.data[key][0]
  
  def set(self, key, value, initialval=False):
    if not key in self.data: self.data[key] = []
    self.data[key].insert(0, value)
    if initialval: self.data[key].insert(0, value)

  def diffs(self):
    d = {}
    for key in self.data:
      if len(self.data[key]) > 1:
        v = self.data[key][0]
        self.data[key] = [ v ]
        d[key] = v
    return d

class Obstacle(object):

  def __init__(self, id, x, y, r):
    self.x = x
    self.y = y
    self.r = r
    self.id = id

  def in_range(self, bot):
    """ Test if this obstacle is in scanning range of a bot. """
    # TODO: this could be better optimized.
    d = math.sqrt( math.pow(self.x - bot.get('x'), 2) + math.pow(self.y - bot.get('y'), 2) )
    return d <= (bot.get('scanning_range') + self.r)

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
        bot.set('x', random.randint(0, arenawidth))
        bot.set('y', random.randint(0, arenaheight))
        good = True
        for obstacle in obstacles:
          if obstacle.occupies(bot.get('x'), bot.get('y')):
            good = False
            break
        for enemy in bot.enemies:
          if enemy.get('x') == bot.get('x') and enemy.get('y') == bot.get('y'):
            good = False
            break
      args = bot.diffs()
      if len(args):
        args['id'] = bot.id
        args['user'] = bot.uid
        args['training'] = 1 if bot.training else 0
        args['language'] = bot.language
        streamadd(-1, 'bot', args)

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
        try: bot.set('state', bot.program.cmd_state_change())
        except BotProgram.BotProgramException as e:
          streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
          bot.set('state', 'stop')

        # If bot values changed, output them.
        args = bot.diffs()
        if len(args):
          args['id'] = bot.id
          streamadd(t, 'bot', args)

      # Everybody who's firing, aim.
      for bot in bots:
        if bot.get('state') in ('attack', 'attack+move'):
          for weapon in bot.weapons:
            try: weapon.aim = bot.program.cmd_aim(weapon)
            except BotProgram.BotProgramException as e:
              streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
              weapon.aim = 0.0
            streamadd(t, 'aim', { 'bot': bot.id, 'weapon': weapon.id, 'aim': weapon.aim })

      # Everybody who's moving, move.
      for bot in bots:
        if bot.get('state') in ('move', 'attack+move', 'defend+move'):

          # Change bot direction.
          try:
            dir, speed = bot.program.cmd_move()
            bot.set('dir', dir)
            bot.set('speed', speed)
          except BotProgram.BotProgramException as e:
            streamadd(t, 'error', { 'bot': bot.id, 'message': e.message, 'line': e.line, 'column': e.column})
            bot.set('dir', 'n')
            bot.set('speed', 0.0)
          dir = bot.get('dir')
          speed = bot.get('speed')
          if dir == 'n': d = (0, 0-speed)
          elif dir == 'ne': d = (speed, 0-speed)
          elif dir == 'e': d = (speed, 0)
          elif dir == 'se': d = (speed, speed)
          elif dir == 's': d = (0, speed)
          elif dir == 'sw': d = (0-speed, speed)
          elif dir == 'w': d = (0-speed, 0)
          elif dir == 'nw': d = (0-speed, 0-speed)
          else: d = (0, 0)
          d = ( int(math.floor(d[0])), int(math.floor(d[1])) )

          # find desired new location.
          nx = (bot.get('x') + d[0]) % arenawidth
          ny = (bot.get('y') + d[1]) % arenaheight
          collision = False

          # Check for collisions with obstacles.
          for obstacle in obstacles:
            if obstacle.occupies(nx, ny):
              bot.damage(0.1)
              collision = True
              break

          # Check for collisions with enemies.
          for enemy in bot.enemies:
            if nx == enemy.get('x') and ny == enemy.get('y'):
              enemy.damage(0.1)
              bot.damage(0.1)
              collision = True
              break

          # Move to new location if bot didn't smash into something.
          if not collision:
            bot.set('x', nx)
            bot.set('y', ny)
            streamadd(t, 'move', { 'bot': bot.id, 'x': bot.get('x'), 'y': bot.get('y') })

      # Resolve combat.
      shots = []
      for bot in bots:
        if bot.get('state') in ('attack', 'attack+move'):
          for weapon in bot.weapons:
            targettype, target, endx, endy, coords = weapon.fire(obstacles, arenawidth, arenaheight)
            shots.append(coords)
            if targettype == 'enemy':
              damage = 0.3
              if target.get('state') == 'defend': damage = 0.1
              elif target.get('state') == 'defend+move': damage = 0.2
              target.damage(damage)
              streamadd(t, 'fire', {
                'bot': bot.id,
                'target_type': targettype,
                'enemy': target.id,
                'x': target.get('x'),
                'y': target.get('y'),
                'damage': damage
              })
              log("bot %d shot bot %d at (%d,%d) for damage %f" % (bot.id, target.id, target.get('x'), target.get('y'), damage))
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
            if bot.get('x') == x and bot.get('y') == y: v = str(bot.id)
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
