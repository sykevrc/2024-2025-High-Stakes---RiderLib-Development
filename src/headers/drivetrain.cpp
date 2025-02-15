#include "main.h"
#include "pros/abstract_motor.hpp"
#include "pros/adi.hpp"
#include "pros/misc.h"
#include "pros/motors.hpp"
#include <sys/_intsup.h>

using namespace pros;
using namespace lemlib;

Controller controller(E_CONTROLLER_MASTER);

// motor groups
MotorGroup leftMotors({18,-19,-7},MotorGearset::blue); // left motor group - ports 3 (reversed), 4, 5 (reversed)
MotorGroup rightMotors({-11,12,3}, MotorGearset::blue); // right motor group - ports 6, 7, 9 (reversed)

Motor intake(-20,MotorGearset::green);
Motor fastintake(-14,MotorGearset::blue);

Motor arm(10,MotorGearset::green);
// Inertial Sensor on port 10
Imu imu(8);
Optical color(9);
Distance dist(17);
// tracking wheels
// horizontal tracking wheel encoder. Rotation sensor, port 20, reversed
Rotation horizontalEnc(1);
// vertical tracking wheel encoder. Rotation sensor, port 11, reversed
Rotation verticalEnc(2);
// horizontal tracking wheel. 2.75" diameter, 5.75" offset, back of the robot (negative)
lemlib::TrackingWheel horizontal(&horizontalEnc, 2, -7.5);
// vertical tracking wheel. 2.75" diameter, 2.5" offset, left of the robot (negative)
lemlib::TrackingWheel vertical(&verticalEnc, 2, -3.35);
adi::Pneumatics clamp(1, false);
adi::Pneumatics doink(8, false);



// drivetrain settings
lemlib::Drivetrain drivetrain(&leftMotors, // left motor group
                              &rightMotors, // right motor group
                              1, // 10 inch track width
                              lemlib::Omniwheel::OLD_4, // using new 4" omnis
                              343, // drivetrain rpm is 360
                              2 // horizontal drift is 2. If we had traction wheels, it would have been 8
);

// lateral motion controller
lemlib::ControllerSettings linearController(5, // proportional gain (kP)
                                            0, // integral gain (kI)
                                            0, // derivative gain (kD)
                                            0, // anti windup
                                            0, // small error range, in inches
                                            0, // small error range timeout, in milliseconds
                                            0, // large error range, in inches
                                            0, // large error range timeout, in milliseconds
                                            0 // maximum acceleration (slew)
);

// angular motion controller
lemlib::ControllerSettings angularController(3, // proportional gain (kP)
                                             0, // integral gain (kI)
                                             10, // derivative gain (kD)
                                             0, // anti windup
                                             0, // small error range, in degrees
                                             0, // small error range timeout, in milliseconds
                                             0, // large error range, in degrees
                                             0, // large error range timeout, in milliseconds
                                             0 // maximum acceleration (slew)
);

// sensors for odometry
lemlib::OdomSensors sensors(&vertical, // vertical tracking wheel
                            nullptr, // vertical tracking wheel 2, set to nullptr as we don't have a second one
                            &horizontal, // horizontal tracking wheel
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            &imu // inertial sensor
);

// input curve for throttle input during driver control
lemlib::ExpoDriveCurve throttleCurve(3, // joystick deadband out of 127
                                     10, // minimum output where drivetrain will move out of 127
                                     1.019 // expo curve gain
);

// input curve for steer input during driver control
lemlib::ExpoDriveCurve steerCurve(3, // joystick deadband out of 127
                                  10, // minimum output where drivetrain will move out of 127
                                  1.019 // expo curve gain
);

// create the chassis
lemlib::Chassis chassis(drivetrain, linearController, angularController, sensors, &throttleCurve, &steerCurve);
