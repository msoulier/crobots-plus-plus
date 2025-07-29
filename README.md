# crobots-plus-plus
This project is a re-implementation of the classic
[Crobots](https://en.wikipedia.org/wiki/Crobots) MS-DOS program, the point of
which was to design a virtual robot using a simple K&R C syntax and API, and
enter it into combat with up to three other such robots. 

The interface to the original was a simple ASCII text "rendering", and the
source code for the robots was compiled into memory each time the game was run,
compiling for what was essentially an emulator. The source code could be
identified by their .r file extensions.

# Goals
We started this project with the following goals in mind:

- It should be fun to implement, and fun to use
- We can do better than ASCII graphics, even though I still kind of like them
  myself
- The robots should be object-oriented, implementing a pre-defined interface
- Improved debugging capabilities for troubleshooting one's robot

# Design
Implementing a robot is as simple as including
```C++
#include <Crobots++/Crobots++.hpp>
```

and subclassing the IRobot class, and implementing its interface.

## The API
The interface is based on the original API. I am using [this one](https://tpoindex.github.io/crobots/docs/crobots_manual.html#8-1).
Here are the functions that we need to implement, at a minimum, in the API:

```C
scan (degree,resolution)
```

The scan() function invokes the robot's scanner, at a specified degree and resolution. scan() returns 0 if no robots are within the scan range or a positive integer representing the range to the closest robot. Degree should be within the range 0-359, otherwise degree is forced into 0-359 by a modulo 360 operation, and made positive if necessary. Resolution controls the scanner's sensing resolution, up to +/- 10 degrees.

```C
Examples:
   range = scan(45,0); /* scan 45, with no variance */
   range = scan(365,10); /* scans the range from 355 to 15 */
```

```C
cannon (degree,range)
```

The cannon() function fires a missile heading a specified range and direction. cannon() returns 1 (true) if a missile was fired, or 0 (false) if the cannon is reloading. Degree is forced into the range 0-359 as in scan(). Range can be 0-700, with greater ranges truncated to 700.

```C
Examples:
   degree = 45;    /* set a direction to test */
   if ((range=scan(degree,2)) > 0) /* see if a target is there */
     cannon(degree,range);  /* fire a missile */
```

```C
drive (degree,speed)
```

The drive() function activates the robot's drive mechanism, on a specified heading and speed. Degree is forced into the range 0-359 as in scan(). Speed is expressed as a percent, with 100 as maximum. A speed of 0 disengages the drive. Changes in direction can be negotiated at speeds of less than 50 percent.

```C
Examples:
   drive(0,100);  /* head due east, at maximum speed */
   drive(90,0);   /* stop motion */
```

```C
damage ()
```

The damage() function returns the current amount of damage incurred. damage() takes no arguments, and returns the percent of damage, 0-99. (100 percent damage means the robot is completely disabled, thus no longer running!)

```C
Examples:
   d = damage();       /* save current state */
   ; ; ;               /* other instructions */
   if (d != damage())  /* compare current state to prior state */
   {
     drive(90,100);    /* robot has been hit, start moving */
     d = damage();     /* get current damage again */
   }
```

```C
speed ()
```

The speed() function returns the current speed of the robot. speed() takes no arguments, and returns the percent of speed, 0-100. Note that speed() may not always be the same as the last drive(), because of acceleration and deceleration.

```C
Examples:
   drive(270,100);   /* start drive, due south */
   ; ; ;             /* other instructions */
   if (speed() == 0) /* check current speed */
   {
     drive(90,20);   /* ran into the south wall, or another robot */
   }
```

```C
loc_x ()
loc_y ()
```

The loc_x() function returns the robot's current x axis location. loc_x() takes no arguments, and returns 0-999. The loc_y() function is similar to loc_x(), but returns the current y axis position.

```C
Examples:
   drive (180,50);  /* start heading for west wall */
   while (loc_x() > 20)
     ;              /* do nothing until we are close */
   drive (180,0);   /* stop drive */
```

```C
rand (limit)
```

The rand() function returns a random number between 0 and limit, up to 32767.

```C
Examples:
   degree = rand(360);     /* pick a random starting point */
   range = scan(degree,0); /* and scan */
```

```C
sqrt (number)
```

The sqrt() returns the square root of a number. Number is made positive, if necessary.

```C
Examples:
   x = x1 - x2;     /* compute the classical distance formula */
   y = y1 - y2;     /* between two points (x1,y1) (x2,y2) */
   distance = sqrt((x*x) - (y*y));
```

```C
sin (degree)
cos (degree)
tan (degree)
atan (ratio)
```

These functions provide trigonometric values. sin(), cos(), and tan(), take a degree argument, 0-359, and returns the trigonometric value times 100,000. The scaling is necessary since the CROBOT cpu is an integer only machine, and trig values are between 0.0 and 1.0. atan() takes a ratio argument that has been scaled up by 100,000, and returns a degree value, between -90 and +90. The resulting calculation should not be scaled to the actual value until the final operation, as not to lose accuracy. See programming examples for usage.

More to come.
