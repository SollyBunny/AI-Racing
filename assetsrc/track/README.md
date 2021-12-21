# How to make a map

#### Creating an image

First open up an editor of your choice (gimp) and create a red background  
Then draw a line with a gradient from black at the start to white at the end, this is the track  
You can add barriers by drawing blue  
An example is map.png in this folder  

#### Formatting the map  

Run `python main.py` in this directory and the picture called img.png will be converted to `map.map`  
This file contains the image in a format which the game can understand  

#### Validating the map

To check if the map is valid you can run `python main.py validate` which will print any errors found in the file called `map.map`  

#### Drag n Drop

To "install" the map simply drop in the folder with the game's executable in it
