# Macro and Micro Manual

> Sioux Weekend of Code 2017

## <a name="macro"></a>Macro

> Control your UFOs to conquer planets and gain credits.

#### Starting a macro game

You can start the macro game from the command line with the following arguments:
```
    MacroEngine.exe playername /path/to/macro/script/ /path/to/micro/script ...
```

The first argument is the player name, the second argument is the directory
which contains your Macro script and a runCommand.txt file, and the third
argument is the directory which contains your Micro script and a runCommand.txt
file. Macro will read the command that has to be executed from the
runCommand.txt file.

Example command line:

```
    MacroEngine.exe jesper C:/scripts/macro/ C:/scripts/micro ferdi C:/scripts/macro/ C:/scripts/micro
```

Example runCommand.txt
```
    python C:/scripts/macro/script.py
```

#### Conquering Planets

The goal of Macro is to command your UFOs to move to planets and conquering
them.
You can conquer planets that have already been conquered by other players.
If you try to conquer a planet while one or more enemy UFOs are in nearby the
planet, you will have to battle ([Micro](#micro)) for the right to conquer
the planet.
The winner of the battle will conquer the planet and in case of a draw,
no one will conquer the planet.

See commands: [conquer](#conquer), [moveToPlanet](#moveToPlanet), and
[moveToCoord](#moveToCoord).

#### Credits

When a planet is conquered you gain credits. These are used to buy more UFOs.
The player with the most credits at the end of the game will win so be mindful
of how you are spending your credits.

You get credits for the total number of planet you have conquered per second.
Your total income per second is calculated with the following formula:
```
    10000 * log(X + 5)
    where
        X = number of planets you have conquered
```

At the end of the game you get an additional **10000** credits for each UFO
that you have remaining.

See commands: [buy](#buy)

#### Receiving Game State from Macro

On each tick of the game, Macro will send the **game state** to all Macro
scripts. Macro will wait for **250 milliseconds** during each tick.

The game state consists of the following:
- List of solar systems, each containing a list of planets
- List of players, each containing a list of UFOs

Example game state:
```json
    {
        "id": 1,
        "name": "Match1",
        "tick": 0,
        "solarSystems": [
            {
                "id": 1,
                "name": "S1",
                "coords": {
                    "x": 50,
                    "y": 15
                },
                "planets": [
                    {
                        "id": 1,
                        "name": "S1P1",
                        "orbitDistance": 10,
                        "orbitRotation": 10,
                        "ownedBy": 1
                    }
                ]
            }
        ],
        "players": [
            {
                "id": 1,
                "name": "Player1",
                "credits": 9001,
                "ufos": [
                    {
                        "id": 1,
                        "inFight": true,
                        "coord": {
                            "x": 25,
                            "y": 75
                        }
                    }
                ]
            }
        ]
    }
```

#### Sending Commands to Macro

You can send commands to [Macro](#macro) by writting them to `stdout`.
There are 5 types of Macro commands:
- [conquer](#conquer)
- [fight](#fight)
- [moveToPlanet](#moveToPlanet)
- [moveToCoord](#moveToCoord)
- [buy](#buy)

Macro will wait for **250 milliseconds** (a tick) before processing all
commands. Within that time you can send as many commands as you like and they
will be queued in Macro for processing. At the end of the tick, Macro will
process all the commands

##### <a name="conquer"></a>conquer

In order to conquer a planet you have to have at least one UFO that is within
a radius of **256** "space meters" of the planet that you want to conquer, and
you have to send the `conquer` command with the ID of that planet.

If there are other enemy UFOs in the radius of the planet, a [Micro](#micro)
 battle will start with all the UFOs in the radius of the planet.
UFOs that are already in a battle will not be included.

Example: lets say you have UFO at (45,75) and you want to conquer planet 42
which is located at (50,60). You can calculate the distance with the formula
 sqrt((50-45)(50-45)+(60-75)(60-75)) = 15 which is within the radius of **256**.

Sending the following `conquer` command will let you conquer planet 42.

```json
    {
        "command": "conquer",
        "planetId": 42,
    }
```

**[Pro-tip]** *Planet Sniping*: If you try to conquer a planet where there are
other UFOs in its radius that are already in a fight for that planet, you can
take that planet without starting a battle and you can keep it until the other
UFOs are done fighting (the winner will take the planet).
If the battle is a draw, you get to keep the planet.

##### <a name="fight"></a>fight

In order to fight bots in space, you can send the `fight` command with the ID
of the UFO you want to fight. All your UFOs that are within a radius of **256**
of your attacking UFO will join the battle along with all the enemy's UFOs
that are within the same radius of the UFO you are attacking.

```json
    {
        "command": "fight",
        "ufoId": 1,
    }
```

##### <a name="moveToPlanet"></a>moveToPlanet

To move your UFO(s) towards a planet you can send the `moveToPlanet` command
with the list of UFOs that you want to move and the ID of the planet you want
to move to.

Sending the following `moveToPlanet` command will start moving UFOs 1, 2, and 3
towards planet 42.

```json
  {
      "command": "moveToPlanet",
      "ufos": [1, 2, 3],
      "planetId": 42
  }
```

##### <a name="moveToCoord"></a>moveToCoord

To move your UFO(s) towards a specific coordinate you can send the `moveToCoord`
command with the list of UFOs that you want to move and the coordinate you
want to move to.

Sending the following `moveToCoord` command will start moving UFOs 1, 2, and 3
towards the coordinate (75,25).

```json
    {
        "command": "moveToCoord",
        "ufos": [1, 2, 3],
        "coord": {
            "X": 75,
            "Y": 25
        }
    }
```

If you send multiple `moveToCoord` commands during one tick

##### <a name="buy"></a>buy

To buy additional UFOs you can send the `buy` command with the amount of UFOs
that you want to buy and the ID of the planet where you want your new UFOs
to spawn. The price of a UFO is **20000** credits.

There are some constraints:
1. The buy command will be ignored if you try to buy more UFOs that you can
afford.
2. The buy command will be ignored if you try to buy UFOs on a planet that you
have not conquered.

If you have no conquered planets, you can still buy UFOs using **-1** as the
planet ID. This will spawn the UFOs at random coordinates in the universe.

Sending the following `buy` command will spawn 3 UFOs at planet 42.

```json
    {
        "command": "buy",
        "amount": 3,
        "planet": 42
    }
```

**[Pro-tip]** *Second Life*: If all your UFOs have been destroyed and you
do not have enough credits to buy more UFOs or any conquered planets, you are
allowed to buy **one** UFO and go to negative credits.

#### <a name="macro-commands-reference"></a>Macro Commands Reference

#### conquer

**Properties**
- `command`: Type of command (conquer)
- `planetId`: ID of planet you want to conquer

```json
    {
        "command": "string",
        "planetId": "int",
    }
```

### fight

**Properties**
- `command`: Type of command (fight)
- `ufoId`: ID of UFO you want to fight

```json
    {
        "command": "fight",
        "ufoId": 1,
    }
```

#### moveToPlanet

**Properties**
- `command`: Type of command (moveToPlanet)
- `ufos`: List of UFOs you want to move
- `planetId`: ID of planet you want to move to

```json
  {
      "command": "string",
      "ufos": "[int]",
      "planetId": "int"
  }
```

#### moveToCoord

**Properties**
- `command`: Type of command (moveToCoord)
- `ufos`: List of UFOs you want to move
- `coord`: (X,Y) coordinate you want to move to

```json
    {
        "command": "string",
        "ufos": "[int]",
        "coord": {
            "x": "int",
            "y": "int"
        }
    }
```

#### buy

**Properties**
- `command`: Type of command (buy)
- `amount`: Quantity of UFOs that you want to buy
- `planetId`: ID of planet where your UFOs will spawn


```json
    {
        "command": "string",
        "amount": "int",
        "planet": "int"
    }
```

## <a name="micro"></a>Micro

> Control your UFOs and battle against enemy UFOs.


### Game Mechanics

Players control UFOs using 2 commands: `move` and `shoot`.


#### Moving

The `move` command moves the UFO in the specified `direction` at the specified `speed`.
The movement is based on the [polar coordinate system], `direction` being the *angle* and `speed` being the *radius*.

**Parameters:**

- `direction`: Value between **0** and **360**.
- `speed`: Value between **0** and **10**.


#### Shooting

The `shoot` command shoots a laser from the origin of the UFO in the specified `direction`.
Unlike moving, shooting has a fixed projectile speed.
The movement is based on the [polar coordinate system], `direction` being the *angle* and the projectile speed which is always **15** (fixed speed).
Shooting also has a *cooldown* of **1** seconds meaning that you can only shoot once per second.
Each shot does **25** points of damage. There is no friendly fire.

**Parameters:**

- `direction`: Value between: **0** and **360**.


#### Hitpoints

A UFO starts with **100** `hitpoints` and is destroyed when it's `hitpoints` reaches `0`.


#### Position

All UFOs have a `position` "**x,y**". This represents the UFO's current position on the arena.
The arena has a specified `width` and `height`. The top left corner of the arena represents "**0,0**" and the bottom right corner of the arena represents "**width,height**".
Your UFO can only move within the bounds of the arena. There is no collision between UFOs.


#### The Laser Fence

The arena is surrounded by a laser fence. If any UFO touches this fence, they will immediately be destroyed (**0** `hitpoints`).
In order to keep the games short, the arena will start shrinking after a fixed amount of time and will keep shrinking until the battle is over.


#### Winning Condition

Last man standing: if all your UFOs are destroyed you lose.


### Scripting


#### Game Loop

The game loop of any basic micro UFO is as follows:

1. Read `JSON` formatted game state (input) from micro using `stdin`
2. Script logic (where the magic happens)
3. Write `JSON` formatted commands (output) to micro using `stdout`

*Note: The input and output are in [JSON] format*

Python example:

```python

while True:
    input = input()
    # Script logic
    print(output)

```


##### Reading input from Micro

The input received from micro has the following format:

```json
{
    "tick": "integer",
    "arena": {
        "height": "integer",
        "width": "integer",
        "shrinkRate": "integer",
        "shrinkThreshold": "integer"
    },
    "player": "string",
    "players": [
        {
            "id": "integer",
            "name": "string",
            "ufos": [
                {
                    "id": "integer",
                    "name": "string",
                    "hitpoints": "float",
                    "position": {
                        "x": "float",
                        "y": "float"
                    }
                }
            ]
        }
    ],
    "projectiles": [
        {
            "position": {
                "x": "float",
                "y": "float"
            },
            "direction": "float",
        },
    ]
}
```

Input example:

```json
{
    "tick": 10,
    "arena": {
        "height": 1000,
        "width": 1000,
        "shrinkRate": 10,
        "shrinkThreshold": 2
    },
    "playerId": 0,
    "playerName": "player0",
    "players": [
        {
            "id": 0,
            "name": "player0",
            "ufos": [
                {
                    "id": 0,
                    "name": "ufo0",
                    "hitpoints": 20,
                    "position": {
                        "x": 25,
                        "y": 25
                    }
                },
                {
                    "id": 1,
                    "name": "ufo1",
                    "hitpoints": 10,
                    "position": {
                        "x": 25,
                        "y": 25
                    }
                }
            ]
        },
        {
            "id": 1,
            "name": "player1",
            "ufos": [
                {
                    "id": 0,
                    "name": "ufo0",
                    "hitpoints": 15,
                    "position": {
                        "x": 75,
                        "y": 75
                    }
                },
                {
                    "id": 1,
                    "name": "ufo1",
                    "hitpoints": 15,
                    "position": {
                        "x": 50,
                        "y": 75
                    }
                }
            ]
        }
    ],
    "projectiles": [
        {
            "position": "23,34",
            "direction": "32",
        },
        {
            "position": {
                "x": 25,
                "y": 25
            },
            "direction": 32
        }
    ]
}
```


##### Writing output to Micro

The output sent to micro has the following format:

```json
{
    "commands": [
        {
            "id": "integer",
            "move": {
                "direction": "float",
                "speed": "float"
            },
            "moveTo": {
                "x": "float",
                "y": "float",
                "speed": "float"
            },
            "shoot": {
                "direction": "float"
            },
            "shootAt": {
                "x": "float",
                "y": "float"
            }
        },
    ]
}
```

Output example:

```json
{
    "commands": [
        {
            "id": 0,
        },
        {
            "id": 1,
            "move": {
                "direction": 0,
                "speed": 10
            }
        },
        {
            "id": 2,
            "shoot": {
                "direction": 90
            }
        },
        {
            "id": 3,
            "move": {
                "direction": 180,
                "speed": 10
            },
            "shoot": {
                "direction": 270
            }
        },
    ]
}
```


[polar coordinate system]:https://en.wikipedia.org/wiki/Polar_coordinate_system
[JSON]: https://en.wikipedia.org/wiki/JSON
