#pragma once

#include <cstdint>
#include <iostream>
#include <random>
#include <string_view>
#include <sstream>
#include <vector>

#include "Crobots++/Log.hpp"

namespace Crobots
{

// Forward declaration
class InternalRobotProxy;

enum class DamageType
{
    Alive,
    Cannon,
    HitRobot,
    HitWall,
}; 

struct CollisionDeathData
{
    float VelocityX;
    float VelocityY;
    float Mass;
    float Heading;
};

struct CannonDeathData
{
    float VelocityX;
    float VelocityY;
    float Mass;
    float Heading;
};

struct DeathData
{
    DamageType Type;
    union
    {
        CannonDeathData CannonData;
        CollisionDeathData CollisionData;
        // other stuff
    };
};

enum class CannonType
{
    Standard
};

class ContactDetails
{
public:
    ContactDetails(float fromx, float fromy, float tox, float toy, float bearing, float range)
    : m_fromx{fromx}
    , m_fromy{fromy}
    , m_tox{tox}
    , m_toy{toy}
    , m_bearing{bearing}
    , m_range{range}
    {}

    std::string ToString()
    {
        std::stringstream ss;
        ss << "ContactDetails: " << m_fromx << " " << m_fromy << " -> "
           << m_tox << " " << m_toy << " "
           << m_bearing << " " << m_range << std::endl;
        return ss.str();
    }

private:
    float m_fromx;
    float m_fromy;
    float m_tox;
    float m_toy;
    float m_bearing;
    float m_range;
};

class IRobot
{
private:
    friend class Engine;

protected:
    IRobot();

public:
    virtual ~IRobot();
    virtual std::string_view GetName() const = 0;
    // Tick is where the robot does all of its work. It is the replacement for the main loop
    // in the original game. To avoid abuse of the api in this call, most functions called
    // have a limit allowable of once per tick, like drive, cannon, scan, etc.
    // It has not yet been decided whether to run the robots in their own threads, or give
    // them a time limit, as such implementations can be error prone.
    virtual void Tick() = 0;
    uint32_t GetId() const;
    void SetId(uint32_t id);
    float GetX() const;
    float GetY() const;
    float GetFacing() const;
    float GetScanDir() const;
    float GetResolution() const;
    bool IsDetected() const;

    struct DeathData GetDeathData() const;

    static float ToDegrees(float radians);
    static float ToRadians(float degrees);

    template<typename T>
    static IRobot* Create(InternalRobotProxy* proxy)
    {
        IRobot* robot = new T();
        robot->m_proxy = proxy;
        return robot;
    }
    void AddContact(std::unique_ptr<ContactDetails>& contact);
    void ClearContacts();


private:
    // Current X and Y location. Floats for more accurate resolution, but the LocX and LocY
    // methods return integers, rounded off.
    float m_currentX;
    float m_currentY;
    // Post-move X and Y location.
    float m_nextX;
    float m_nextY;
    // Speed we are trying to achieve.
    float m_desiredSpeed;
    // Current speed
    float m_speed;

    float m_scan_dir;
    float m_resolution;

    // Facing we would like to have.
    float m_desiredFacing;
    // Facing we currently have.
    float m_facing;

    // default to 65535 for now, so effectively unlimited, planning for the future
    uint32_t m_rounds;

    // Some performance parameters for the future.
    float m_acceleration;
    float m_braking;
    float m_turnRate;

    // How much we are hurt.
    float m_damage;

    // A scan counter, reset at the beginning of each Tick.
    uint32_t m_scanCountDown;
    // The number of ticks that must pass between scans.
    uint32_t m_ticksPerScan;

    // Shot parameters that go into the next shot.
    bool m_cannotShotRegistered;
    float m_cannonShotDegree;
    float m_cannonShotRange;
    float m_cannonShotSpeed;

    // Countdown until done reloading.
    uint32_t m_cannonTimeUntilReload;

    // Cannon parameters.
    CannonType m_cannonType;
    uint32_t m_cannonReloadTime;
    float m_cannonShotMaxRange;

    // Robot detected by another robot's scan?
    bool m_detected;

    // Set robot to indestructible
    bool m_indestructible;

    struct DeathData m_deathdata;

    void TickInit();
    bool RegisterShot(CannonType weapon, float degree, float range);
    void AccelRobot();
    void MoveRobot();
    void HitTheWall();
    bool IsDead();
    float GetActualSpeed();
    void Detected();

    std::vector<std::unique_ptr<ContactDetails>> m_contacts;

    InternalRobotProxy* m_proxy;

    static uint32_t BoundedRand(uint32_t range);
    static std::random_device rd;
    static std::mt19937 gen;

protected:
    /*
     * Return the robot's current facing, where 0 is to the right and the facing
     * increases positively counter-clockwise.
     */
    float Facing();
    // Return the robot's current desired facing.
    float GetDesiredFacing() const;
    /*
        The Scan() method invokes the robot's scanner, at a specified degree and
        resolution. Scan() returns 0 if no robots are within the scan range or a
        positive integer representing the range to the closest robot. Degree should
        be within the range 0-359, otherwise degree is forced into 0-359 by a modulo
        360 operation, and made positive if necessary. Resolution controls the
        scanner's sensing resolution, up to +/- 5 degrees, so 10 degrees in total.

        Scanning takes time, so a scan result can only be returned at a rate of m_ticksPerScan.
        Each Scan call reduces a counter by 1 until it reaches 0 and a scan can be made.
        Scan returns -1 if a scan cannot be performed yet.
    */
    float Scan(float degree, float resolution);

    /*
        The Cannon() method chooses to fire a missile heading a specified range and
        direction. Cannon() returns true if a missile was fired, or false if
        the cannon is reloading. Degree is forced into the range 0-359 as in scan().
        Range can be 0-700, with greater ranges truncated to 700. FIXME

        Calling this multiple times in a Tick is pointless, only the last call matters.
    */
    bool Cannon(float degree, float range);

    /*
        The Drive() method activates the robot's drive mechanism, on a specified
        heading and speed. Degree is forced into the range 0-359 as in Scan(). Speed
        is expressed as a percent, with 100 as maximum. A speed of 0 disengages the
        drive. Changes in direction can be negotiated at speeds of less than 50
        percent.

        Calling this multiple times in a Tick is pointless, only the last call matters.
    */
    void Drive(float degree, float speed);

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
    float Speed();

    /*
        The LocX() method returns the robot's current x axis location. LocX()
        takes no arguments, and returns 0-999. The LocY() method is similar to
        LocX(), but returns the current y axis position. Location is in meters.
    */
    float LocX();
    float LocY();

    // Note - The mathematical functions are not required due to the C++ standard library.
    // https://cppreference.com/w/cpp/numeric/math.html

    // FIXME: Should we yank these functions and just use the standard library? I suspect so.
    /*
        The Rand() method returns a random number between 0 and limit, up to 32767.
    */
    uint32_t Rand(uint32_t limit);

    // Fetch the X dimension of the arena.
    float GetArenaX();

    // Fetch the Y dimension of the arena.
    float GetArenaY();

    // Modulo 360 operation.
    float Mod360(float number);

    void SetDeathData(struct DeathData deathdata);
};

}
