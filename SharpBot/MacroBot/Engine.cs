﻿using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using static SwocIO.Pipeline;

namespace MacroBot
{
    public abstract class Engine<GameStateTemplate> where GameStateTemplate : class
    {
        private readonly Thread readThread;
        private readonly List<GameStateTemplate> gameStates = new List<GameStateTemplate>();

        public Engine()
        {
            readThread = new Thread(new ThreadStart(PollState));
        }

        public void Run()
        {
            try
            {
                DoRun();
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Run failure: " + ex.Message);
            }
        }

        private void DoRun()
        {
            mStop = false;
            readThread.Start();
            while (!mStop)
            {
                var states = GameStates();
                if (states.Count > 0)
                {
                    Response(states);
                }
                Thread.Sleep(100);
            }
        }

        private void PollState()
        {
            try
            {
                DoPollState();
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Poll state failure: " + ex.Message);
            }
        }

        private void DoPollState()
        {
            while (!mStop)
            {
                var state = ReadMessage<GameStateTemplate>();
                lock (gameStates)
                {
                    gameStates.Add(state);
                }
            }
        }

        private List<GameStateTemplate> GameStates()
        {
            lock (gameStates)
            {
                var states = gameStates.ToList();
                gameStates.Clear();
                return states;
            }
        }

        public abstract void Response(List<GameStateTemplate> gameStates);

        private bool mStop;
        public void Stop()
        {
            mStop = true;
            readThread.Join();
        }

        private static T ReadMessage<T>()
        {
            string line = ReadLine();
            if (String.IsNullOrEmpty(line))
                return default(T);

            try
            {
                return JsonConvert.DeserializeObject<T>(line);
            }
            catch (Exception)
            {
                return default(T);
            }
        }

        public static void WriteMessage<T>(T message)
        {
            WriteLine(JsonConvert.SerializeObject(message, Formatting.None));
        }
    }
}
