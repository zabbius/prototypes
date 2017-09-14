package ru.zabbius;

import android.media.AudioManager;
import android.media.MediaPlayer;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.CommandLineParser;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;

import java.io.FileDescriptor;
import java.io.PrintWriter;

public class PlaySound {

    public static void main(String[] args)
    {
        System.exit((new PlaySound()).Run(args));
    }

    private int Run(String[] args)
    {

        Options options = new Options();

        options.addOption(Option.builder("f").argName("file").longOpt("file").desc("path to sound file")
                .hasArg().numberOfArgs(1).required().build());
        options.addOption(Option.builder("v").argName("volume").longOpt("volume").desc("volume from 0 to 1, float")
                .hasArg().numberOfArgs(1).type(Float.class).required(false).build());
        options.addOption(Option.builder("t").argName("type").longOpt("type")
                .desc("audio stream type, one of: ALARM, DTMF, MUSIC, NOTIFICATION, RING, SYSTEM, VOICE_CALL")
                .hasArg().numberOfArgs(1).required(false).build());
        options.addOption(Option.builder("d").argName("duration").longOpt("duration").desc("duration limit in seconds, int")
                .hasArg().numberOfArgs(1).type(Integer.class).required(false).build());


        String soundFile;
        int streamType = AudioManager.STREAM_NOTIFICATION;
        float volume = 1f;
        int duration = 60*60*24;

        CommandLineParser parser = new DefaultParser();

        try
        {
            CommandLine cmd = parser.parse(options, args);

            soundFile = cmd.getOptionValue("file");

            if (cmd.hasOption("volume"))
                volume = Float.parseFloat(cmd.getOptionValue("volume"));

            if (cmd.hasOption("duration"))
                duration = Integer.parseInt(cmd.getOptionValue("duration"));

            if (cmd.hasOption("type"))
            {
                String strType = cmd.getOptionValue("type");
                if (strType.equalsIgnoreCase("ALARM"))
                    streamType = AudioManager.STREAM_ALARM;
                if (strType.equalsIgnoreCase("DTMF"))
                    streamType = AudioManager.STREAM_DTMF;
                if (strType.equalsIgnoreCase("MUSIC"))
                    streamType = AudioManager.STREAM_MUSIC;
                if (strType.equalsIgnoreCase("NOTIFICATION"))
                    streamType = AudioManager.STREAM_NOTIFICATION;
                if (strType.equalsIgnoreCase("RING"))
                    streamType = AudioManager.STREAM_RING;
                if (strType.equalsIgnoreCase("SYSTEM"))
                    streamType = AudioManager.STREAM_SYSTEM;
                if (strType.equalsIgnoreCase("VOICE_CALL"))
                    streamType = AudioManager.STREAM_VOICE_CALL;
            }
        }
        catch (Exception e)
        {
            System.err.println(e.getMessage());
            (new HelpFormatter()).printUsage(new PrintWriter(System.err), HelpFormatter.DEFAULT_WIDTH, "PlaySound", options);
            return 1;
        }


        MediaPlayer mp = new MediaPlayer();

        try
        {
            if (soundFile.equalsIgnoreCase("-"))
                mp.setDataSource(FileDescriptor.in);
            else
                mp.setDataSource(soundFile);

            mp.setAudioStreamType(streamType);
            mp.setVolume(volume, volume);
            mp.prepare();
            mp.start();
        }
        catch (Exception e)
        {
            System.err.println(e.getMessage());
            return 2;
        }

        int i = 0;
        while(mp.isPlaying() && i < duration)
        {
            i++;
            try
            {
                Thread.sleep(1000);
            }
            catch(Exception e)
            {
                System.err.println(e.getMessage());
                return 3;
            }
        }

        mp.stop();
        mp.release();
        return 0;
    }
}
