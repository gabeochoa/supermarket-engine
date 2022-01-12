# supermarket


Libraries this uses:
(Libs with * I edited, look for @SUPERMARKET for the changes)

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

https://github.com/bombela/backward-cpp
- * added extra signal 
- TODO change output format to be readable
- TODO remove if cant get good format
- stack traces


```

TODOs

Game 

player should be able to control price of items
+ player gets feedback from customers on price and quantity 
+ feedback should be centralized on item so if you miss popups its findable
player should have to buy items 
+ which take time to get shipped to store
+ can hire an inventory manager to do this automatically

Allow player to label aisles 
+ makes it easier for customers to find the item they want
+ gives stockers faster stocking times since they only need to stock when aisle is empty

Customers leaving items around
+ Another stock job to put items back

Customers will leave if lines are too long 

Stealing items
+ Hire security manager
+ Hire security employees 
+ Video Cameras


Weather and day night system
+ Different desires based on these (less ice cream in winter) 

Highlight text in inventory manager when not ideal? 
+ Maybe an upgrade if you buy a "Buyer" manager or something
+ Red means too expensive etc 

Can you price all your items at $0? 
+ Whats the min price / Whats the max price? 

Can you offer sales when you need to get rid of things? 
Do items spoil? so you have to sell or eat the cost ... 



Middleware 

- keybindings on the layer level 
    without giving away what each layer does? 

Engine Ideas

- UI screenshot / render tests ... 
    output rendered UI to json on end during debug or something 
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
- add support for editing values on the fly (DONE) 
     some way to register values with global storage (DONE)
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



