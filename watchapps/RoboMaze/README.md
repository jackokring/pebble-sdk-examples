RoboMaze
========
This source is free to use with proper acknowledgement of the author. The game and watch is easy
to understand, and should not need changing much. The stopwatch uses a time subtraction method of 
aparently working while the app is suspended. This is to keep the worker thread free, and perhaps
used to provide other functionality, such as running some backgound code. This limits the accuracy
to within 1 second of button pushes, and with a 40 day return to zero count. If the count goes
past a new year, the leap year status is checked. The stopwatch will stop and reset before 2 years.

Working
=======
* Clock
* Date
* Stopwatch
* Maze Animation

Not Working
===========
* Number Calculator
* Console Input
* Maze Game
* Worker Thread

(Under Construction)
====================
The fourth mode of the clock is dificult to explain and does require some indepth knowledge. The
UP and DOWN buttons in many clock modes show the score and switches on motion interaction for
the robot. They may select one of two game possibilities at this point. This interaction is showed
by the dot to the right of the score, and controlled by UP and DOWN. The top is anticlockwise,
and the bottom is clockwise. Second from the top and bottom are straight on but with an
anticlockwise or clockwise bias at T junctions. The centre position (default) is an automatic mode
to increase background play options by auto using one of the other 4 turning modes.

The date may be difficult to read for some, as some robo letters are simplified to a large extent.
These are just larger versions of the letters used on the console when SELECT is pressed, with
some slight stylization. Not all large letters have been defined for your own extensions to this
source and they are not on the primary resource bitmap, which does contain all the small versions.

The BACK button becomes the clock mode select, with a long press escaping from this application.
A further extension set of menus can be activated by double or long pressing SELECT. The source
could do with further optimization, but is quite efficient on memory. I hope you enjoy!
