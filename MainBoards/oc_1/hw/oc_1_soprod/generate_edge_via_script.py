# -*- coding: utf-8 -*-
"""
Generates Eagle script to draw radial array of button edge vias.

To use:
    1. In Eagle, set mark at center of array
    2. Run script
"""
from math import sin, cos, pi
import numpy as np

def rotate(pt, theta, center=(0,0)):
    x,y = pt
    x -= center[0]
    y -= center[1]
    
    x_new = x*cos(theta) - y*sin(theta)
    y_new = x*sin(theta) + y*cos(theta)
    
    x_new += center[0]
    y_new += center[1]
    return (x_new,y_new)


    
# Find locations of all via centers
dial_center = (0,0)
via_hole_radius = 120.0

# Need to know the right clocking angle, which we can get
# if we already know where one of the vias needs to go.
# In this case, the position of the first via is (1114, 186)
# and the center of the array is (650,650).
pt_start = np.array( [1114.03002 - 650, 186.03999 - 650] )

via_centers = []
for i in range(24):
    new_pt = rotate(pt_start, 2*pi/24.0*i, center=dial_center)
    print(new_pt)
    via_centers.append(rotate(pt_start, 2*pi/24.0*i))
    

# Set via size and shape
s = """SET Wire_Bend 2;
CHANGE DRILL 120;
CHANGE SHAPE Round;
"""

button_nets = ["BTN-0", "BTN-2", "BTN-1"]

# Add edge vias
for i,center in enumerate(via_centers):
    net = button_nets[i % len(button_nets)]
    s += "VIA '%s' (R%2.2f %2.2f);\n" % (net, center[0], center[1])
    
# Add silkscreen labels
s += "LAYER 25\nCHANGE SIZE 24\n"
for i,via_center in enumerate(via_centers):
    net = button_nets[i % len(button_nets)]
    text_pos = (via_center[0]*0.825, via_center[1]*0.85)
    text_str = net[-1]
    s += "TEXT '%s' R0 (R%2.2f %2.2f);\n" % (text_str, text_pos[0], text_pos[1])
    
print(s)
    
