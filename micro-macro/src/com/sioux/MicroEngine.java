package com.sioux;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.TypeAdapter;
import com.google.gson.stream.JsonReader;
import com.google.gson.stream.JsonToken;
import com.google.gson.stream.JsonWriter;
import com.sioux.game_objects.Game;

import java.awt.Point;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by Ferdinand on 4-7-17.
 */
public class MicroEngine {
    private MicroTick state;
    private Map<String, BotProcess> scripts;
    private Gson gson;
    private GsonBuilder gsonBuilder;
    private Integer tickCounter;
    private Boolean gameRunning;

    MicroEngine() {
        this.state = new MicroTick();
        this.scripts = new HashMap<>();
        this.gsonBuilder = new GsonBuilder();
        this.gsonBuilder.registerTypeAdapter(Point.class, new PointAdapter());
        this.gson = this.gsonBuilder.create();
        this.tickCounter = 0;
        this.gameRunning = false;
    }

    public void Run(Game start) {
        Initialize(start);

        while (gameRunning) {
            GameLogic();

            // Run for a specific amount of ticks.
            gameRunning = (++this.tickCounter < 1);
        }

        Cleanup();
    }

    private void Initialize(Game start) {
        // TODO Game data -> MicroTick state

        for (int i = 0; i < 2; i++) {
            MicroPlayer player = new MicroPlayer("Player#" + i);

            for (int j = 0; j < 4; j++) {
                Point position = new Point(10 * i, 10 * j);
                player.Add(new MicroBot("Bot#" + j, 100, position));
            }

            state.Add(player);

            final String dir = "/home/luke/Projects/swoc2017/test-scripts/";
            final String cmd = "python3.5 micro-bot.py";

            try {
                scripts.put(player.name, new BotProcess(dir, cmd));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        gameRunning = true;
    }

    private void Cleanup() {
        for (Map.Entry<String, BotProcess> entry : scripts.entrySet()) {
            if (entry.getValue().isRunning()) {
                entry.getValue().close();
            }
        }
    }

    private void GameLogic() {
        SendGameState();
        WaitForCommands();
        SaveGameState();
    }

    private void SendGameState() {
        for (MicroPlayer player : state.players) {
            String stateJson = gson.toJson(this.state, MicroTick.class);
            scripts.get(player.name).writeLine(stateJson);
        }
    }

    private void WaitForCommands() {
        for (MicroPlayer player : state.players) {
            String inputJson = scripts.get(player.name).readLine(1000);
            MicroInput input = gson.fromJson(inputJson, MicroInput.class);

            ExecuteCommands(player.name, input);
        }
    }

    private void ExecuteCommands(String name, MicroInput input) {
        // TODO run commands for player
    }

    private void SaveGameState() {
        final String path = "/home/luke/Projects/swoc2017/test-scripts/ticks/tick_" + this.tickCounter + ".json";
        final String data = gson.toJson(state, MicroTick.class);

        try {
            File file = new File(path);
            file.getParentFile().mkdir();
            file.createNewFile();
            FileWriter writer = new FileWriter(file);
            writer.write(data);
            writer.flush();
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private class MicroTick {
        private MicroArena arena;
        private List<MicroPlayer> players;
        private List<MicroProjectile> projectiles;

        public MicroTick() {
            this.arena = new MicroArena(1000, 1000);
            this.players = new ArrayList<>();
            this.projectiles = new ArrayList<>();
        }

        public void Add(MicroPlayer player) {
            this.players.add(player);
        }

        public void Add(MicroProjectile projectile) {
            this.projectiles.add(projectile);
        }
    }

    private class MicroArena {
        private Integer height;
        private Integer width;

        public MicroArena(Integer height, Integer width) {
            this.height = height;
            this.width = width;
        }
    }

    private class MicroPlayer {
        private String name;
        private List<MicroBot> bots;

        public MicroPlayer(String name) {
            this.name = name;
            this.bots = new ArrayList<>();
        }

        public void Add(MicroBot bot) {
            this.bots.add(bot);
        }
    }

    private class MicroBot {
        private String name;
        private Integer hitpoints;
        private Point position;

        public MicroBot(String name, Integer hp, Point pos) {
            this.name = name;
            this.hitpoints = hp;
            this.position = pos;
        }

        public Boolean Move(Move cmd) {
            // TODO
            return true;
        }

        public MicroProjectile Shoot(Shoot cmd) {
            // TODO
            return new MicroProjectile();
        }

        public Boolean Hit(Shoot cmd) {
            // TODO
            return false;
        }
    }

    private class MicroProjectile {
        private String name;
        private Point position;
    }

    private class MicroInput {
        private List<BotCommand> bots;

        public MicroInput() {
            bots = new ArrayList<>();
        }
    }

    private class BotCommand {
        private String name;
    }

    private class Move extends BotCommand {
        private Point direction;
        private Integer speed;
    }

    private class Shoot extends BotCommand {
        private Point direction;
    }

    private class PointAdapter extends TypeAdapter<Point> {
        public Point read(JsonReader reader) throws IOException {
            if (reader.peek() == JsonToken.NULL) {
                reader.nextNull();
                return null;
            }
            String[] point = reader.nextString().split(",");
            Integer x = Integer.parseInt(point[0]);
            Integer y = Integer.parseInt(point[1]);
            return new Point(x, y);
        }
        public void write(JsonWriter writer, Point value) throws IOException {
            if (value == null) {
                writer.nullValue();
                return;
            }
            String point = value.getX() + "," + value.getY();
            writer.value(point);
        }
    }
}