// 1-D linear altitude hold simulator
#include <iostream>
#include "math.h"
using namespace std;

const double g = 9.81;      // Gravitational constant
//
class MAV
{
private:
    protected mass;         // Mass of MAV
public:
    // Constructor
    MAV() : mass(0.0)
    {

    }
};
///
class AltHold
{
private:
    double alt_;            // Desired altitude
    double vel_;            // Desired velocity
    double accel_;          // Desired acceleration
    double kp_;             // Proportional gain
    double kd_;             // Derivative gain
    double mass_;           // Mass of MAV
    double u_;              // Initialize acceleration
    double alt0_;           // Initialize altitude
    double vel0_;           // Initialize velocity
    unsigned int seg_;      // Number of segments to divide the total time
public:
    // Constructor with no args
    AltHold() : alt_(0.0), vel_(0.0), accel_(0.0), kp_(0.0), kd_(0.0), mass_(0.0), seg_(0.0)
    {

    }
    // Constructor with args
    AltHold(double alt, double vel, double accel, double kp, double kd, double m) : alt_(alt), vel_(vel), accel_(accel), kp_(kp), kd_(kd), mass_(m)
    {

    }
    // Setting computation parameters
    void setParam(unsigned int seg, double u, double alt, double vel)
    {
        seg_ = seg;
        u_ = u;
        alt0_ = alt;
        vel0_ = vel;
    }
    // Function to controlling the MAV altitude
    void controlAltitude(AltHold a1);
    // Destructor
    ~AltHold()
    {

    }
};
void AltHold::controlAltitude(AltHold a1)
{
    double t;
    double del_t;
    double alt, vel, u;     // Current altitude, velocity, and acceleration variables

    t = alt_/0.5;           // 1 second to climb every 5 meters
    del_t = t/seg_;         // Time step
    alt = alt0_;            // Initialize altitude
    vel = vel0_;            // Initialize velocity
    u = u_;                 // Initialize acceleration

    for(unsigned int i=0; i<=seg_; i++)
    {
        alt = (-g*pow(del_t,2.0) + u_*pow(del_t,2.0)/mass_) / 2 + vel*del_t + alt;
        vel = -g*del_t + u*del_t/mass_ + vel;
        u = mass_*(g + accel_ + kd_*(vel_-vel) + kp_*(alt_-alt));
    }
    cout << alt << endl;
}

int main()
{
    AltHold a1(10.0, 0.0, 0.0, 2.0, 1.1, 1.0);
    a1.setParam(100, 0.1, 0.0, 0.0);
    a1.controlAltitude(a1);
    return 0;
}
