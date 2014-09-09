package gos.engine;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class BotProcess implements AutoCloseable
{
	private final Process child;

	private final OutputStreamWriter outputWriter;
	
	private final StreamGobbler inputReader;
	private final StreamGobbler errorReader;

    public BotProcess(String command) throws IOException
    {
        File file = new File(command);
        if (!file.exists() || !file.canExecute())
        {
            throw new IllegalArgumentException("File does not exist or is not executable.");
        }
        File parent = file.getParentFile();
        
        System.out.println("Starting new bot process " + command + " in " + parent);
        
        child = Runtime.getRuntime().exec(command, null, parent);

        inputReader = new StreamGobbler(child.getInputStream());
        errorReader = new StreamGobbler(child.getErrorStream());
        outputWriter = new OutputStreamWriter(child.getOutputStream());
        
        inputReader.start();
        errorReader.start();
    }

    @Override
    public void close()
    {
        child.destroy();

        try
        {
            child.waitFor();
            inputReader.join();
            errorReader.join();
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }

        try
        {
            outputWriter.close();
        }
        catch (IOException e)
        {
        }
    }

	public String readLine(long timeOut)
	{
		if (!isRunning())
		{
		    return null;
		}
		
		return inputReader.readLine(timeOut, TimeUnit.MILLISECONDS);
	}
	
	public boolean writeLine(String line)
	{
	    try
	    {
    	    outputWriter.write(line.trim());
    	    outputWriter.write("\n");
    	    outputWriter.flush();
            return true;
	    }
	    catch (IOException ex)
	    {
	        return false;
	    }
	}
	
	public boolean isRunning()
	{
		try
		{
			child.exitValue();
			return false;
		}
		catch(IllegalThreadStateException ex)
		{
			return true;
		}
	}
	
	public String getErrors()
	{
	    StringBuilder builder = new StringBuilder();

        String line;
        while ((line = errorReader.readLine(0, TimeUnit.SECONDS)) != null)
        {
            builder.append(line);
            builder.append('\n');
        }
        
	    return builder.toString();
	}
}
