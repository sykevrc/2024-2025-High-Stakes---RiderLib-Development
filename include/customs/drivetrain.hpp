#include "pros/adi.hpp"
#include "pros/imu.hpp"
#include "pros/motor_group.hpp"
#include "pros/rotation.hpp"
#include "lemlib/chassis/chassis.hpp"
#include "lemlib/chassis/trackingWheel.hpp"

using namespace pros;
using namespace lemlib;

extern Controller controller;

extern MotorGroup leftMotors;
extern MotorGroup rightMotors;

extern Motor intake;
extern Motor fastintake;

extern Motor arm;
extern Imu imu;


extern adi::Pneumatics clamp;
extern adi::Pneumatics doink;

extern Rotation verticalEnc;
extern Rotation horizontalEnc;

extern Drivetrain drivetrain;

extern TrackingWheel horizontal;
extern TrackingWheel vertical;

extern Chassis chassis;