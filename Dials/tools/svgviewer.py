#!/usr/bin/env python
'''
svgviewer.py
'''
import os
import sys      # For stack traces
import time
import multiprocessing
import subprocess
import pygame

def main(script_path, image_path, width, height):
    mdate_last = 0

    pygame.init()
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption('Image REPL')

    script_dir = os.path.dirname(os.path.realpath(script_path))
    image_dir = os.path.dirname(os.path.realpath(image_path))
    
    # Main Loop
    while True:
        # Check if source file has changed
        mdate = os.path.getmtime(script_path)
        
        # If so, spawn new process & run
        if mdate > mdate_last:
            print("-"*60 + "\n" + str(mdate) + " - File change detected.")
            
            # Run python script (should produce output svg at image_path)
            print("Running " + script_path)
            p = subprocess.call(["python", script_path], cwd=script_dir,
                                stdout=subprocess.PIPE)

            # Have inkscape convert the svg to a png
            print("Converting " + image_path + " to png...")
            p = subprocess.call(['inkscape', '-z', '-e', 'temp_img.png',
                                 '-w', str(width), '-h', str(height), image_path],
                                stdout=subprocess.PIPE)

            # Update image display
            print("Redrawing...")
            img = pygame.image.load('temp_img.png')
            screen.fill( (255,255,255) )
            screen.blit(img, (0,0))
            pygame.display.flip()

            print("Done--waiting for new changes.")
            
        # Sleep
        mdate_last = mdate
        time.sleep(1)


if __name__=="__main__":
    import argparse
    parser = argparse.ArgumentParser(
        description='Daemon to run a python script and display its image output every time the script is changed.',
        epilog='Example usage: python svgviewer.py ./gen_aero_dial.py ./aero_copper.svg')
    
    parser.add_argument('script_path', type=str, default=None,
                        help='Path to Python script to be run')

    parser.add_argument('image_path', type=str, default=None,
                        help='Path to svg to be drawn')

    parser.add_argument('width', type=int, default=600,
                        help='Width of display window')

    parser.add_argument('height', type=int, default=600,
                        help='Height of display window')

    args = parser.parse_args()

    main(script_path=args.script_path, image_path=args.image_path,
         width=args.width, height=args.height)
