#!/usr/bin/env python

from os import listdir
import sys

if len(sys.argv) > 1 and sys.argv[1] == "validate":

    try:
        with open("map.map", "rb") as file:
            out = file.read()
    except FileNotFoundError:
        print("Cannot find file \"map.map\"")
        exit(1)
        
    if len(out) < 5:
        print("File too small to be valid")
        exit(1)
    
    size = (
        out[0] * 256 + out[1],
        out[2] * 256 + out[3]
    )
    print(f"Image size is {size[0]}x{size[1]}")
    
    if size[0] * size[1] + 4 != len(out):
        print("File size doesnt match data")
        exit(1)

    flagbarrier = 0;
    flagroad = 0;
    flaggrass = 0;
    for i in range(4, len(out)):
        if i == 255:
            print("Premature EOF found")
        elif i == 0:
            flaggrass = 1
        elif i == 1:
            flagbarrier = 1
        else:
            flagroad = 1
    if flagbarrier == 0: print("No barriers found") 
    if flagroad    == 0: print("No road found") 
    if flaggrass   == 0: print("No grass found") 
    
else:

    from PIL import Image

    img = None
    for i in listdir():
        if i[:4] == "map.":
            img = Image.open(i)
            break;
    if img == None:
        print("Cannot find file \"map.*\"")
        exit(1) 
        
    size = img.size
    data = list(img.getdata())

    out = []

    out.append(size[0] // 256)
    out.append(size[0] %  256)
    out.append(size[1] // 256)
    out.append(size[1] %  256)

    for i in range(len(data)):
        
        if data[i][0] == 255 and data[i][1] == 0: # red
            out.append(0)
        elif data[i][0] == 0 and data[i][2] == 255: # blue
            out.append(1)
        else:
            if (data[i][0] == 255):
                out.append(254)
            elif (data[i][0] == 1):
                out.append(2)
            else:
                out.append(data[i][0])
                
    with open("map.map", "wb") as file:
        file.write(bytes(out))
