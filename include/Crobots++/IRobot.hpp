#pragma once

#include <string>
#include <cstdint>

namespace Crobots
{

class IRobot
{
public:
    virtual ~IRobot() = default;
    virtual std::string GetName() const = 0;

private:
    /*
        The scan() function invokes the robot's scanner, at a specified degree and
        resolution. scan() returns 0 if no robots are within the scan range or a
        positive integer representing the range to the closest robot. Degree should
        be within the range 0-359, otherwise degree is forced into 0-359 by a modulo
        360 operation, and made positive if necessary. Resolution controls the
        scanner's sensing resolution, up to +/- 10 degrees.
    */
    uint32_t scan(uint16_t degree, uint16_t resolution);

    /*
        The cannon() function fires a missile heading a specified range and
        direction. cannon() returns 1 (true) if a missile was fired, or 0 (false) if
        the cannon is reloading. Degree is forced into the range 0-359 as in scan().
        Range can be 0-700, with greater ranges truncated to 700.
    */
    bool cannon(uint16_t degree, uint32_t range);

    /*
        The drive() function activates the robot's drive mechanism, on a specified
        heading and speed. Degree is forced into the range 0-359 as in scan(). Speed
        is expressed as a percent, with 100 as maximum. A speed of 0 disengages the
        drive. Changes in direction can be negotiated at speeds of less than 50
        percent.
    */
    void drive(uint16_t degree, uint8_t speed);

    /*
        The damage() function returns the current amount of damage incurred.
        damage() takes no arguments, and returns the percent of damage, 0-99. (100
        percent damage means the robot is completely disabled, thus no longer
        running!)
    */
    uint8_t damage();

    /*
        The speed() function returns the current speed of the robot. speed() takes
        no arguments, and returns the percent of speed, 0-100. Note that speed() may
        not always be the same as the last drive(), because of acceleration and
        deceleration.
    */
    uint8_t speed();

    /*
        The loc_x() function returns the robot's current x axis location. loc_x()
        takes no arguments, and returns 0-999. The loc_y() function is similar to
        loc_x(), but returns the current y axis position.
    */
    uint32_t loc_x();
    uint32_t loc_y();

    /*
        The rand() function returns a random number between 0 and limit, up to 32767.
    */
    uint16_t rand(uint16_t limit);

    /*
        The sqrt() returns the square root of a number. Number is made positive, if necessary.
    */
    uint32_t sqrt(uint32_t number);

    /*
        These functions provide trigonometric values. sin(), cos(), and tan(), take
        a degree argument, 0-359, and returns the trigonometric value times 100,000.
        The scaling is necessary since the CROBOT cpu is an integer only machine,
        and trig values are between 0.0 and 1.0. atan() takes a ratio argument that
        has been scaled up by 100,000, and returns a degree value, between -90 and
        +90. The resulting calculation should not be scaled to the actual value
        until the final operation, as not to lose accuracy. See programming examples
        for usage.
    */
    uint32_t sin(uint32_t degree);
    uint32_t tan(uint32_t degree);
    uint32_t atan(uint32_t ratio);
};

}
