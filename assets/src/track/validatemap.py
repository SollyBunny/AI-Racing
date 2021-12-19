with open("map.map", "rb") as file:
    out = file.read()

if len(out) < 5:
    print("File too small to be valid")
    exit(1)

size = (
    out[0] * 256 + out[1],
    out[2] * 256 + out[3]
)
print("File size is %d by %d", size[0], size[1])

if size[0] * size[1] + 4 != len(out):
    print("File size doesnt match data")
    exit(1)
    
# for i in range(4, len(out)):
    # if (out[i] == 0): continue
    # if (out[i] == 1): continue
    # print("Invalid byte found \"%s\"" % out[i])
    # exit(1)
