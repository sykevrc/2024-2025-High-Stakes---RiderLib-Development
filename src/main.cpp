#include "main.h"
#include "robodash/api.h"

rd::Selector selector({
    {"Red Solo Winpoint", &redSoloWP},
    {"Blue Solo Winpoint", &blueSoloWP},
    {"Red 9 pt", &redRush},
    {"Blue 9 pt", &blueRush},
    {"Skills", &skills},
    {"Red Elim", &elimRed},
    {"Blue Elim", &elimBlue},
    
    
});

rd::Console console;

bool armpos = false;
bool spin = false;

void initialize()
{
    //pros::lcd::initialize(); // initialize brain screen
    chassis.calibrate(); // calibrate sensors
    
    /* pros::Task screenTask([&]() {
        while (true) {
            // print robot location to the brain screen
            pros::lcd::print(0, "X: %f", chassis.getPose().x); // x
            pros::lcd::print(1, "Y: %f", chassis.getPose().y); // y
            pros::lcd::print(2, "Theta: %f", chassis.getPose().theta); // heading
            // log position telemetry
            lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());
            // delay to save resources
            pros::delay(50);
        }
    });  */
}

void disabled() {}

void competition_initialize()
{
    selector.focus();
   
}

void autonomous()
{
    selector.run_auton();
    //skills();
}

void opcontrol()
{

    while (true)
    {

        // get joystick positions
        int leftY = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int rightX = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
        int intaketest = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R1))
        {
            spin = !spin;
            if (spin)
            {
                intake.move_voltage(-12000);
                fastintake.move_voltage(-10000);
            }
            else
            {
                intake.move_voltage(0);
                fastintake.move_voltage(0);
            }
        }

        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y))
        {
            spin = !spin;
            if (spin)
            {
                fastintake.move_velocity(600);
                intake.move_voltage(10000);
            }
            else
            {
                fastintake.move_velocity(0);
                intake.move_voltage(0);
            }
        }
        
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L1))
        {
            clamp.toggle();
        }
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B))
        {
            doink.toggle();
        }
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2))
        {
            fastintake.move_relative(100, 600);
            arm.move_absolute(1200, 600);
        }
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_R2))
        {
            if (armpos)
            {
                if (arm.get_position() > 170)
                {
                    arm.move_absolute(0, 200);
                    armpos = false;
                }
                else
                {
                    arm.move_relative(260, 200);
                    armpos = false;
                }
            }
            else
            {
                arm.move_absolute(0, 200);
                armpos = true;
            }
        }

        // move the chassis with curvature drive
        chassis.arcade(leftY, rightX);
        // delay to save resources
        pros::delay(10);
    }
}