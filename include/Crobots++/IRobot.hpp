#pragma once

#include <string>
#include <cstdint>

namespace Crobots
{

enum class CannonState
{
    Ready,
    Firing,
    Reloading
};

class IRobot
{
friend class Engine;
public:
class Engine;
    virtual std::string GetName() const = 0;
    virtual void Tick() = 0;

    template<typename T>
    static T* Create()
    {
         T* robot = new T();
         Init(robot);
         return robot;
    }

private:
    uint32_t m_locX;
    uint32_t m_locY;

    uint8_t m_desiredSpeed;
    uint8_t m_speed;

    uint16_t m_desiredFacing;
    uint16_t m_facing;

    // default to 65535 for now, so effectively unlimited, planning for the future
    uint16_t m_rounds;
    CannonState m_cstate;

    uint8_t acceleration;
    uint8_t braking;
    uint8_t turnRate;

    uint8_t damage;

    static Engine *m_engine;

    static void Init(IRobot *robot);
    static void SetEngine(Engine *engine);

protected:
    virtual ~IRobot() = default;
    /*
        The Scan() method invokes the robot's scanner, at a specified degree and
        resolution. Scan() returns 0 if no robots are within the scan range or a
        positive integer representing the range to the closest robot. Degree should
        be within the range 0-359, otherwise degree is forced into 0-359 by a modulo
        360 operation, and made positive if necessary. Resolution controls the
        scanner's sensing resolution, up to +/- 10 degrees.
    */
    uint32_t Scan(uint16_t degree, uint16_t resolution);

    /*
        The Cannon() method fires a missile heading a specified range and
        direction. Cannon() returns 1 (true) if a missile was fired, or 0 (false) if
        the cannon is reloading. Degree is forced into the range 0-359 as in scan().
        Range can be 0-700, with greater ranges truncated to 700.
    */
    bool Cannon(uint16_t degree, uint32_t range);

    /*
        The Drive() method activates the robot's drive mechanism, on a specified
        heading and speed. Degree is forced into the range 0-359 as in Scan(). Speed
        is expressed as a percent, with 100 as maximum. A speed of 0 disengages the
        drive. Changes in direction can be negotiated at speeds of less than 50
        percent.
    */
    void Drive(uint16_t degree, uint8_t speed);

    /*
        The Damage() method returns the current amount of damage incurred.
        Damage() takes no arguments, and returns the percent of damage, 0-99. (100
        percent damage means the robot is completely disabled, thus no longer
        running!)
    */
    uint8_t Damage();

    /*
        The Speed() method returns the current speed of the robot. Speed() takes
        no arguments, and returns the percent of speed, 0-100. Note that Speed() may
        not always be the same as the last drive(), because of acceleration and
        deceleration.
    */
    uint8_t Speed();

    /*
        The LocX() method returns the robot's current x axis location. LocX()
        takes no arguments, and returns 0-999. The LocY() method is similar to
        LocX(), but returns the current y axis position.
    */
    uint32_t LocX();
    uint32_t LocY();

    /*
        The Rand() method returns a random number between 0 and limit, up to 32767.
    */
    uint32_t Rand(uint32_t limit);

    /*
        The Sqrt() returns the square root of a number. Number is made positive, if necessary.
    */
    uint32_t Sqrt(uint32_t number);

    /*
        These methods provide trigonometric values. Sin(), Cos(), and Tan(), take
        a degree argument, 0-359, and returns the trigonometric value times 100,000.
        The scaling is necessary since the CROBOT cpu is an integer only machine,
        and trig values are between 0.0 and 1.0. Atan() takes a ratio argument that
        has been scaled up by 100,000, and returns a degree value, between -90 and
        +90. The resulting calculation should not be scaled to the actual value
        until the final operation, as not to lose accuracy. See programming examples
        for usage.
    */
    uint32_t Sin(uint32_t degree);
    uint32_t Cos(uint32_t degree);
    uint32_t Tan(uint32_t degree);
    uint32_t Atan(uint32_t ratio);
};

}
