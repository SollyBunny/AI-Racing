# AI Racing Game
Racing made in c sdl2 but with neurel network AI

## Note
I have no idea how the maths in actuall neurel networks work  
I just made educated guessed from a lot of youtube videos and wikipedia :P  
Thusly, it doesn't evolve very well/quickly
Also keep close look of cpu/gpu temperatures, this program does not care about your device's health

## Linux
You need sdl2, gcc (or clang) and make

### Arch
    # pacman -Sy sdl2 gcc make
##### Gpu Acceleration (Optional)
###### Nvidia
    # pacman -Sy opencl-nvidia ocl-icd opencl-headers
###### Amd (Untested)
    # pacman -Sy opencl-mesa ocl-icd opencl-headers
###### Intel (Redundant / Untested)
    # pacman -Sy intel-compute-runtime ocl-icd opencl-headers
    
### Ubloatu
    # apt update
    # apt install libsdl2-dev build-essential
##### Gpu Acceleration (Optional / Untested)
###### Nvidia
Fish out whichever package you need from  
https://packages.ubuntu.com/search?keywords=nvidia+opencl&searchon=names  
Then install with  

	# apt install <package name>
###### Amd
	# apt install mesa-opencl-icd
###### Intel (Redundant)
	# apt install intel-opencl-icd

### Compiling (All Linux)
Then you can simple make and run ./build/racer (or just `make run`)  

    $ make
    $ make run
    
## Winblows  

### Mingw
Compiler has a c compiler needed to, well, compile  
https://sourceforge.net/projects/mingw/  
Make sure to also install make in the installer gui to make stuff easier (optional)  

### Compiling
Compile by running in a terminal in the folder  

    $ make winblows
or if you dont have make  

    $ x86_64-w64-mingw32-gcc main.c controller.c -o build\\racer.exe -L%cd%\\SDL2\\x86_64-w64-mingw32\\lib -pthread -I%cd%\\SDL2\\x86_64-w64-mingw32\\include -Dmain=SDL_main -lmingw32 -lSDL2main -lSDL2 -lm -w -Wl,-subsystem,windows
you can then double click on racer.exe
