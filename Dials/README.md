# Creating Custom Watch Dials With Python and PCBs
<p align="center">
  <img height="400" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/dial_fdai.jpg">  
</p>

## Why use PCBs?
The PCB manufacturing process is great for watch dials for a few reasons:

**Resolution**
It's common even for cheaper board houses to offer 6mil/0.15mm traces, and some fabs do 3mil/0.075mm. On paper these feature sizes correspond roughly to 170 and 340 DPI, which on its face isn't particularly impressive; however, in practice the PCB process offers very sharp, clean edges with tolerances well below these numbers. 

**Materials**
The PCB process offers a surprising variety of options here. Typically board houses offer PCBs with a fiberglass substrate, and this can be easily drilled, sanded, and glued. A wide range of thickness options are also available, and boards as thin as 0.6mm can be ordered at no extra cost. Other substrate options such as aluminum and flexible Kapton are also available, though those are more expensive.

In terms of metallic surfaces, exposed copper can be electroplated in either nickel or gold as part of the standard PCB fab process, and at very little additional cost. It's also not uncommon for an additional electropolishing step to be offered. 

**Cost**
For small, two-layer boards, custom PCBs are very cheap. A dozen custom dials can be had for less than $40. 

### Why not Printing?
While there are a number of photo printing shops that offer 300 DPI prints in various gloss finishes, these tend to look a bit fuzzy and flat: 

<p align="center">
  <img height="300" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/printing_comparison.jpg"> <img height="300" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/resolution_comparison.jpg">    
</p>

The printed paper must also be drilled to allow hand posts to pass through, which can result in fraying of the paper and wrinkling of the gloss. The outer edge must similarly be trimmed, and then the whole thing must be adhered to something such as a steel backing plate. There may be a printing workflow that produces good results, but the path is less clear compared to the PCB process.

### Disadvantages
**Color Choice** The biggest technical disadvantage is that board fabs provide limited choice in colors for soldermask and silkscreen. For one-offs this can be partly overcome by hand-painting additional colors onto the boards using common hobby enamel paints. 

**Lack of Direct PCB Art Tools** Another more practical disadvantage is that PCB layout tools such as KiCad and Eagle are designed to lay out functional electronics--not PCB art. Most likely you will draw the dial in one format and need to convert it to a different format these programs understand. Luckily, there are already well-established processes doing this.

- - -

## Step 1: Generate SVGs with Python and svgwrite
For our example I'll be using the 38mm FDAI dial. The first step was obtaining vector art of the FDAI design. Unfortunately, I was unable to find this anywhere online. Instead, I was able to generate this surprisingly easily with Python and the [svgwrite](https://pypi.org/project/svgwrite/). The full script can be found [here](https://github.com/snietfeld/OpenChronograph/blob/master/Dials/fdai_38mm/draw_fdai_dial.py). This script produces two SVG images:

<p align="center">
  <img width="200" height="200" src="https://github.com/snietfeld/OpenChronograph/blob/master/Dials/fdai_38mm/source_img/fdai_copper.svg">  <img width="200" height="200" src="https://github.com/snietfeld/OpenChronograph/blob/master/Dials/fdai_38mm/source_img/fdai_soldermask.svg">
</p>

The left image will be our copper layer, and will eventually be plated in gold. The right image is our soldermask, which prevents our copper from being covered by the soldermask. We will also use the soldermask image to produce a third image for the white silkscreen.

## Step 2: Prepare Layer Bitmaps
Once our source SVGs are generated, we need to process and re-export them as high-dpi bitmaps. 

1. Open the copper SVG in GIMP
<p align="center">
  <img scale="1" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/gimp_svg_import.png">  
</p>

2. Import the silkscreen svg as a new layer.

3. Add a new layer with a solid white color and put it at the bottom of the layer stack--this will be our white background for the copper and soldermask layers.

4. Create new layer from visible, delete and fill everything but bottom white negative space (this will be for silkscreen:
<p align="center">
  <img height=400 src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/gimp_silkscreen.png">  
</p>

5. Set color mode to 256 colors by going to IMAGE-->MODE-->Indexed…
Set to 256 and hit CONVERT
<p align="center">
  <img scale="1" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/gimp_convert_colors.png">  
</p>

6. Export different layers to copper, soldermask, and silkscreen BMP images. You should end up with three BMP files corresponding to the copper, soldermask, and silkscreen layers:
<p align="center">
  <img height="200" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/gimp_exported_layers.png">  
</p>

## Step 3: Import into EagleCAD
1. Go to File-->Import..-->BMP…

2. Eagle needs to know which pixel color to use for the layer being imported. Click “Scan Colors”, then select black for the copper and soldermask images, and white for the silkscreen image.

3. Set format to DPI, units to inches, and be sure to set the value for DPI to correspond to the DPI setting used when importing the SVG into GIMP (in this case, 2000). Then set the layer ID number to import to (1 - top copper, 21 - top silkscreen, 29 - top soldermask): 
<p align="center">
  <img scale="1" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/eagle_bmp_import.png">  
</p>

Useful layer numbers

Layer ID | Description | Final Color
--- | --- | ---
1 | Top Copper | Gold
21 | Tplace (silkscreen) | White 
29 | Tstop (soldermask) | Black

There’s some discretization that occurs due to the way the BMPs are rasterized, but I’ve set the DPI to 2000 here--much higher level of detail than the fab lab can print.

<p align="center">
  <img scale="1" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/eagle_bmp_rasterization.png">  
</p>


## Step 4: Export GERBER files and send off to fab
This step will vary depending on which board fab you use. I have had great luck with DirtyPCBs, so I'll use them as an example. 

1. Save your Eagle design file
2. Click File-->CAM Processor
2. In the new window, click File-->Open-->Job
<p align="center">
  <img height="200" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/eagle_open_job.PNG">  
</p>
3. Select the DirtyPCBs cam job file:
<p align="center">
  <img height="300" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/eagle_select_job.PNG">  
</p>

4. Click "Process Job"

5. Place the generated GERBER files in to a .zip, and upload using DirtyPCBs website. Be sure to select 0.6mm board thickness.
<p align="center">
  <img height="200" src="https://github.com/snietfeld/OpenChronograph/blob/master/docs/img/dail_howto/dirtypcbs_order_received.png">  
</p>
