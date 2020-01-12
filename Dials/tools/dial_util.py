


import svgwrite
from svgwrite import cm, mm
from math import pi, sin, cos


def line_radial(cx, cy, length, angle, r):
    angle *= -1
    p1 = rotate((r,0), angle)
    p2 = rotate((r+length,0), angle)

    p1 = (p1[0] + cx, p1[1] + cy)
    p2 = (p2[0] + cx, p2[1] + cy)
    return (p1,p2)

def line_tangential(length, angle, r):
    angle *= -1
    p1 = rotate((r,length/2.), angle)
    p2 = rotate((r,-length/2.), angle)
    return (p1,p2)
                             

def arc(cx, cy, r, theta_start, theta_end, n_segs):
    arclen = theta_end - theta_start

    points = []
    for i in range(n_segs+1):
        theta_0 = theta_start + arclen/n_segs*i
        p = rotate((r,0), theta_0)
        p = (p[0] + cx, p[1] + cy)
        points.append(p)

    return points
    

def rotate(p, theta):
    return (cos(theta)*p[0]-sin(theta)*p[1], sin(theta)*p[0]+cos(theta)*p[1])


'''
Normally we'd use the alignment_baseline or dominant_baseline attributes
to center text vertically, but Inkscape doesn't implement this correctly.
Instead, we manually add a vertical position offset with text_offset arg.
'''
def draw_dial(dwg, elems,
              cx, cy, r,
              outline_lw,
              theta_start, theta_end,
              major_inc, major_lw, major_len,
              med_inc=None, med_lw=None, med_len=None,
              minor_inc=None, minor_lw=None, minor_len=None,
              color='black', fill_color='none',
              major_labels=[],
              font_style="font-size:1.5px; font-family:Comfortaa; font-weight:bold",
              text_r_multiplier=0.75,
              text_offset=(0,0)):

    # Top arcs
    points = arc(cx, cy, r, pi, pi*2, 30)
    elems.add(dwg.polyline(points, stroke_width=outline_lw, fill=fill_color, stroke=color)) 

    # Bottom arcs
    points = arc(cx, cy, r, 0, pi, 30)
    elems.add(dwg.polyline(points, stroke_width=outline_lw, fill=fill_color, stroke=color))

    # Draw major ticks
    theta = theta_start
    i = 0
    while theta < theta_end:
        # Calculate tick points & draw
        points = line_radial(cx, cy, -major_len, theta, r)
        elems.add(dwg.line(start=points[0], end=points[1], stroke_width=major_lw, stroke=color))

        text_xy = rotate( (r*text_r_multiplier, 0), theta)
        text_xy = (text_xy[0] + cx + text_offset[0], text_xy[1] + cy + text_offset[1])

        if i < len(major_labels):            
            dwg.add(dwg.text(major_labels[i],
                             insert=text_xy,
                             stroke='none',
                             fill=svgwrite.rgb(15, 15, 15, '%'),
                             style=font_style,
                             text_anchor="middle"))
        
        theta += major_inc
        i += 1

    # Draw medium ticks
    if med_inc is not None and med_lw is not None and med_len is not None:
        theta = theta_start
        i = 0
        while theta < theta_end:
            # Calculate tick points & draw
            points = line_radial(cx, cy, -med_len, theta, r)
            elems.add(dwg.line(start=points[0], end=points[1], stroke_width=med_lw, stroke=color))
        
            theta += med_inc
            i += 1

    # Draw minor ticks
    if minor_inc is not None and minor_lw is not None and minor_len is not None:
        theta = theta_start
        i = 0
        while theta < theta_end:
            # Calculate tick points & draw
            points = line_radial(cx, cy, -minor_len, theta, r)
            elems.add(dwg.line(start=points[0], end=points[1], stroke_width=minor_lw, stroke=color))
        
            theta += minor_inc
            i += 1
