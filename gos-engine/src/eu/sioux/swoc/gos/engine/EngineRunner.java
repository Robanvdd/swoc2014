package eu.sioux.swoc.gos.engine;

import java.io.IOException;

import eu.sioux.swoc.gos.engine.io.IORobot;

public class EngineRunner implements AutoCloseable
{
	private final IORobot botWhite;
	private final IORobot botBlack;

	private final Board board;
	
	public EngineRunner(String executableWhite, String executableBlack) throws IOException
	{
		botWhite = new IORobot(executableWhite);
		botBlack = new IORobot(executableBlack);
		
		board = new Board();
	}

	public void run()
	{
		System.out.println("Game started");

		try
		{
			SetupBots();
			
			FirstRound();
			
			board.Dump();

			NormalRound();
			
			board.Dump();
		}
		catch (Exception ex)
		{
			System.out.println("Run failed: " + ex.getMessage());
		}

		System.out.println("---- START OF DUMP WHITE ----");
		System.out.println(botWhite.getDump());
		System.out.println("---- END OF DUMP WHITE ----");
		System.out.println("---- START OF DUMP BLACK ----");
		System.out.println(botBlack.getDump());
		System.out.println("---- END OF DUMP BLACK ----");
		System.out.println("Game ended");
	}

	@Override
	public void close() throws Exception
	{
		botWhite.close();
		Thread.sleep(200);

		botBlack.close();
		Thread.sleep(200);

		Thread.sleep(200);
	}

	private static final long SetupTimeOut = 2000;
	private static final long FirstMoveTimeOut = 2000;
	private static final long NormalRoundTimeOut = 2000;
	
	private void SetupBots()
	{
        InitiateRequest initReqW = new InitiateRequest(Board.OwnerWhite);
		StatusResponse statusW = botWhite.writeAndReadMessage(initReqW, StatusResponse.class, SetupTimeOut);

        InitiateRequest initReqB = new InitiateRequest(Board.OwnerBlack);
        StatusResponse statusB= botBlack.writeAndReadMessage(initReqB, StatusResponse.class, SetupTimeOut);
}
	
	private static final int[] AttackOnly = { Move.Attack };
	private static final int[] AllMoves = { Move.Pass, Move.Attack, Move.Strengthen };
	
	private void FirstRound()
	{
		// TODO: If bot does not give a valid move, then let it loose immediately
		DoOneMove(botWhite, AttackOnly, FirstMoveTimeOut);
	}
	
	private void NormalRound()
	{
		// First black then white, since white may start the game
		DoOneMove(botBlack, AttackOnly, NormalRoundTimeOut); 
		DoOneMove(botBlack, AllMoves, NormalRoundTimeOut); 

		DoOneMove(botWhite, AttackOnly, NormalRoundTimeOut); 
		DoOneMove(botWhite, AllMoves, NormalRoundTimeOut);
	}
	
	private void DoOneMove(IORobot bot, int[] allowedMoves, long timeOut)
	{
		MoveRequest request = new MoveRequest(board, allowedMoves);
		Move move = bot.writeAndReadMessage(request, Move.class, timeOut);

		boolean moveProcessed = false;
		if (IsMoveInAllowedList(move, allowedMoves) && IsMoveValid(bot, move))
		{
			ProcessValidatedMove(move);
			moveProcessed = true;
		}	

		StatusResponse status = new StatusResponse(moveProcessed);
		bot.writeMessage(status);
	}
	
	private void ProcessValidatedMove(Move move)
	{
	    if (move.Type != Move.Pass)
	    {
    		int newOwner = board.GetOwner(move.From);
    		int newStone = board.GetStone(move.From);
    		int newCount = board.GetHeight(move.From);
    
    		int oldOwner = board.GetOwner(move.To);
    		if (newOwner == oldOwner)
    		{
    			// strengthen
    			int oldCount = board.GetHeight(move.To);
    			newCount += oldCount;
    		}
    		else
    		{
    			// attack
    		}
    		
    		board.SetSpace(move.To, newOwner, newStone, newCount);
    		board.ClearSpace(move.From);
	    }
	}

    private boolean IsMoveInAllowedList(Move move, int[] allowedMoves)
    {
        boolean allowedMove = false;
        for (int i = 0; i < allowedMoves.length; i++)
        {
            if (move.Type == allowedMoves[i])
            {
                allowedMove = true;
                break;
            }
        }

        return allowedMove;
    }
    
    private boolean IsMoveValid(IORobot bot, Move move)
    {
        if (move.Type == Move.Pass)
        {
            return (move.From == null && move.To == null);
        }
        else if (move.From == null || move.To == null)
        {
            return false;
        }
        else
        {
            int fromColor = board.GetOwner(move.From);
            int toColor = board.GetOwner(move.To);
            
            int fromHeight = board.GetHeight(move.From);
            int toHeight = board.GetHeight(move.To);
            
            int botColor = (bot == botWhite) ? Board.OwnerWhite : ((bot == botBlack) ? Board.OwnerBlack : Board.OwnerNone);

            if (fromColor != botColor &&             // can only move from places owned by bot
                toColor != Board.OwnerNone)          // can not move to an empty place
            {
                return false;
            }
            else if (move.Type == Move.Attack &&
                    fromColor != toColor &&          // can only attack the other color
                    fromHeight >= toHeight &&        // can only attack equal or lower stacks
                    IsValidPath(move.From, move.To))
            {
                return true;
            }
            else if (move.Type == Move.Strengthen &&
                    fromColor == toColor &&          // can only strengthen yourself
                    IsValidPath(move.From, move.To))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    
    private boolean IsValidPath(BoardLocation from, BoardLocation to)
    {
        boolean movingEast = (from.X < to.X);
        boolean movingWest = (from.X > to.X);
        boolean movingSouth = (from.Y < to.Y);
        boolean movingNorth = (from.Y > to.Y);
        boolean movingEastOrWest = movingEast || movingWest;
        boolean movingNorthOrSouth = movingNorth || movingSouth;
        boolean movingSouthEast = movingEast && movingSouth;
        boolean movingNorthWest = movingWest && movingNorth;

        int minX = movingEast ? from.X : to.X;
        int minY = movingSouth ? from.Y : to.Y;
        int maxX = movingEast ? to.X : from.X;
        int maxY = movingSouth ? to.Y : from.Y;

        // only allowed to move, N, E, S, W, NW, SE

        boolean movingOnlyEastOrWest = movingEastOrWest && !movingNorthOrSouth;
        boolean movingOnlyNorthOrSouth = movingNorthOrSouth && !movingEastOrWest;
        boolean movingDiagonal = (movingSouthEast || movingNorthWest) &&
                ((maxX - minX) == (maxY - minY));
        if (!movingOnlyEastOrWest && !movingOnlyNorthOrSouth && !movingDiagonal)
        {
            return false;
        }

        boolean allEmpty;
        if (movingOnlyEastOrWest)
        {
            allEmpty = true;
            for (int x = minX + 1; x < maxX; x++)
            {
                int owner = board.GetOwner(new BoardLocation(x, from.Y));
                allEmpty &= (owner == Board.OwnerNone);
            }
        }
        else if (movingOnlyNorthOrSouth)
        {
            allEmpty = true;
            for (int y = minY + 1; y < maxY; y++)
            {
                int owner = board.GetOwner(new BoardLocation(from.X, y));
                allEmpty &= (owner == Board.OwnerNone);
            }
        }
        else if (movingDiagonal)
        {
            allEmpty = true;
            for (int x = minX + 1, y = minY + 1; x < maxX && y < maxY; x++, y++)
            {
                int owner = board.GetOwner(new BoardLocation(x, y));
                allEmpty &= (owner == Board.OwnerNone);
            }
        }
        else
        {
            allEmpty = false;
        }
            
        return allEmpty;
    }
}
