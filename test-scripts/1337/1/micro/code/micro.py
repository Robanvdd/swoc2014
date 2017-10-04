"""
Example micro bot script.

"""

import sys
import json
import logging
import math

fh = logging.FileHandler('script.log')
fh.setLevel(logging.DEBUG)

formatter = logging.Formatter('%(asctime)s - %(message)s')
fh.setFormatter(formatter)

logger = logging.getLogger('script')
logger.setLevel(logging.DEBUG)
logger.addHandler(fh)

time = 0
shotOnce = False

def game_loop():
    # Wait for input from micro.
    micro_json = input()
    logger.info('input' + micro_json)
    micro = json.loads(micro_json)

    arenaWidth = micro['arena']['width']
    arenaHeight = micro['arena']['height']
    player = micro['playerId']

    global time
    time = time + 0.1
    
    bots = []
    for p in micro['players']:
        if p['id'] == player:
            bots = p['bots']

    otherBots = []
    for p in micro['players']:
        if p['id'] != player:
            otherBots += p['bots']

    targetX = 0
    targetY = 0
    if otherBots != []:
        targetBot = otherBots[0]
        target = [float(i) for i in targetBot['position'].split(',')]
        targetX = target[0]
        targetY = target[1]

    commands = {
        'commands': []
    }
    
    global shotOnce
    if not shotOnce:
        for bot in bots:
            commands['commands'].append({
                'id': bot['id'],
                'move': {
                    'direction': math.cos(time) * 180,
                    'speed': math.sin(time) * 10
                },
#                'shoot': {
#                    'direction': 180
#                }
                'shootAt': {
                    'x': targetX,
                    'y': targetY
                }
            })
    
    shotOnce = True

    # Send output to micro.
    commands_json = json.dumps(commands)
    logger.info('output' + commands_json)
    print(commands_json)


def main():
    while True:
        try:
            game_loop()
        except:
            logger.exception()


if __name__ == '__main__':
    main()