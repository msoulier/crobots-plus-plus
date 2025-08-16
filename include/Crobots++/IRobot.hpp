#pragma once

#include <string_view>
#include <cstdint>

#include "Crobots++/Log.hpp"

namespace Crobots
{

// Forward declaration
class Engine;

enum class CannonType
{
    Standard
};

class IRobot
{
private:
    friend class Engine;

protected:
    IRobot();

public:
    virtual ~IRobot() = default;
    virtual std::string_view GetName() const = 0;
    // Tick is where the robot does all of its work. It is the replacement for the main loop
    // in the original game. To avoid abuse of the api in this call, most functions called
    // have a limit allowable of once per tick, like drive, cannon, scan, etc.
    // It has not yet been decided whether to run the robots in their own threads, or give
    // them a time limit, as such implementations can be error prone.
    virtual void Tick() = 0;
    uint32_t GetId() const;

private:
    // This id should be a simple integer uniquely identifying the robot based on the order
    // in which it was loaded.
    uint32_t m_id;

    // Current X and Y location. To allow high resolution of movement, the coordinates that the
    // robot stores are multiplied by 100.
    float m_currentX;
    float m_currentY;
    // Post-move X and Y location.
    float m_nextX;
    float m_nextY;
    // Speed we are trying to achieve.
    uint32_t m_desiredSpeed;
    // Current speed
    uint32_t m_speed;

    // Facing we would like to have.
    uint32_t m_desiredFacing;
    // Facing we currently have.
    uint32_t m_facing;

    // default to 65535 for now, so effectively unlimited, planning for the future
    uint32_t m_rounds;

    // Some performance parameters for the future.
    uint32_t m_acceleration;
    uint32_t m_braking;
    uint32_t m_turnRate;

    // How much we are hurt.
    uint32_t m_damage;

    // A scan counter, reset at the beginning of each Tick.
    uint32_t m_scansDuringTick;
    // The number of scans our scanner is capable of per Tick.
    uint32_t m_scansPerTick;

    // Shot parameters that go into the next shot.
    bool m_cannotShotRegistered;
    uint32_t m_cannonShotDegree;
    uint32_t m_cannonShotRange;
    uint32_t m_cannonShotSpeed;

    // Countdown until done reloading.
    uint32_t m_cannonTimeUntilReload;

    // Cannon parameters.
    CannonType m_cannonType;
    uint32_t m_cannonReloadTime;
    uint32_t m_cannonShotMaxRange;

    void UpdateTickCounters();
    bool RegisterShot(CannonType weapon, uint32_t degree, uint32_t range);

    static Engine *m_engine;

    static void SetEngine(Engine *engine);
    static uint32_t BoundedRand(uint32_t range);

protected:
    /*
        The Scan() method invokes the robot's scanner, at a specified degree and
        resolution. Scan() returns 0 if no robots are within the scan range or a
        positive integer representing the range to the closest robot. Degree should
        be within the range 0-359, otherwise degree is forced into 0-359 by a modulo
        360 operation, and made positive if necessary. Resolution controls the
        scanner's sensing resolution, up to +/- 10 degrees.

        Scan() can only be called at the robot's scansPerTick rate, which is initially 1.
        Any additional scans during the robot's Tick() method will return 0.
    */
    uint32_t Scan(uint32_t degree, uint32_t resolution);

    /*
        The Cannon() method chooses to fire a missile heading a specified range and
        direction. Cannon() returns 1 (true) if a missile was fired, or 0 (false) if
        the cannon is reloading. Degree is forced into the range 0-359 as in scan().
        Range can be 0-700, with greater ranges truncated to 700.

        Calling this multiple times in a Tick is pointless, only the last call matters.
    */
    bool Cannon(uint32_t degree, uint32_t range);

    /*
        The Drive() method activates the robot's drive mechanism, on a specified
        heading and speed. Degree is forced into the range 0-359 as in Scan(). Speed
        is expressed as a percent, with 100 as maximum. A speed of 0 disengages the
        drive. Changes in direction can be negotiated at speeds of less than 50
        percent.

        Calling this multiple times in a Tick is pointless, only the last call matters.
    */
    void Drive(uint32_t degree, uint32_t speed);

    /*
        The Damage() method returns the current amount of damage incurred.
        Damage() takes no arguments, and returns the percent of damage, 0-99. (100
        percent damage means the robot is completely disabled, thus no longer
        running!)
    */
    uint32_t Damage();

    /*
        The Speed() method returns the current speed of the robot. Speed() takes
        no arguments, and returns the percent of speed, 0-100. Note that Speed() may
        not always be the same as the last drive(), because of acceleration and
        deceleration.
    */
    uint32_t Speed();

    /*
        The LocX() method returns the robot's current x axis location. LocX()
        takes no arguments, and returns 0-999. The LocY() method is similar to
        LocX(), but returns the current y axis position. Location is in meters.
    */
    uint32_t LocX();
    uint32_t LocY();

    // Note - The mathematical functions are not required due to the C++ standard library.
    // https://cppreference.com/w/cpp/numeric/math.html

    // FIXME: Should we yank these functions and just use the standard library? I suspect so.
    /*
        The Rand() method returns a random number between 0 and limit, up to 32767.
    */
    uint32_t Rand(uint32_t limit);
};

}
