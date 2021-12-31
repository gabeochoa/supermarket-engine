# supermarket


Libraries this uses:

https://github.com/vallentin/glText
- debug text drawing

https://github.com/fmtlib/fmt
- for log formatting :) 

https://github.com/nothings/stb
- truetype for fontfile loading 
- image_read for texture loading 
- image_write for texture export debugging (manual for now) 

https://github.com/samhocevar/portable-file-dialogs
- file dialoges


```
TODOs

Game Ideas

Allow player to label aisles which make it easier for people to find the item
the want

Customers leaving items around
+ Another stock job to put items back
Stealing items

Weather and day night system
+ Different desires based in


Engine Ideas

- theres tons of Ideas in engine/ui.h
    not moving them here, so go look there :) 
- Setup visual tracing so we can use chrome::tracing to analyze
    https://www.youtube.com/watch?v=qiD39bB7DvA
- support for defer/runlater
    for example:
         {
         auto X = new int[10];
         defer(free x)
         ... rest of function
         } // free happens here
- LRU cache for temporary textures
- base64 encode, is this already built in c++ ?
- add support for editing values on the fly
     some way to register values with global storage
     hook up a UI dropdown to edit it
     support new debug layer for holding the ui dropdown
 add support for adding a struct (is there some kind of Reflection in cpp?)
    automatically discover AND create a UI to edit the values
- add charts to UI lib
- build Task and Task managing into the engine?
    other threads for really simple stuff only?
- support for playing audio
- ai behavior tree ?
    i prefer a job based system but it doesnt support interruptions right now
- decompression library
     create / read bins so textures/fonts/etc arent leaked
     probably can do some kind of encryption on the file too
- radar ui would look cool
     https://github.com/s9w/oof/blob/master/demos/radar_demo.cpp
- async coroutines
     http://libdill.org/structured-concurrency.html
- Make UI state reactive
     https://github.com/snakster/cpp.react
- networking
     socket.io?
- support for OTF font formats
     https://github.com/caryll/otfcc
- add a default texture that isnt just white
     make it obvious that the texture you are trying to draw doesnt exist
```



