from PIL import Image
import sys

img = Image.open("img.png")

size = img.size
data = list(img.getdata())

out = []

out.append(size[0] // 256)
out.append(size[0] %  256)
out.append(size[1] // 256)
out.append(size[1] %  256)

for i in range(len(data)):
    
    if (data[i][0] != data[i][1]):
        out.append(0)
    else:
        out.append(data[i][0])
    
    if (i + 1) % size[0] == 0:
        print();

with open("map.map", "wb") as file:
    file.write(bytes(out))
