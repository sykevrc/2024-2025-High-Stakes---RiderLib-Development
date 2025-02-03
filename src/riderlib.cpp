#include <iostream>
#include <vector>
#include <cmath>
#include "Eigen/Dense"

struct Pose {
    double x, y, theta;
};

struct MotionProfile {
    double x, y, theta, vx, vy, omega;
};

struct WheelSpeeds {
    double left, right;
};

// Quintic Spline Coefficients
struct QuinticCoeffs {
    double a0, a1, a2, a3, a4, a5;
};

// Solve for quintic polynomial coefficients
QuinticCoeffs computeQuinticSpline(double p0, double v0, double a0, double pf, double vf, double af, double tf) {
    Eigen::Matrix<double, 6, 6> T;
    Eigen::Matrix<double, 6, 1> B, X;

    T << 1, 0, 0, 0, 0, 0,
         0, 1, 0, 0, 0, 0,
         0, 0, 2, 0, 0, 0,
         1, tf, tf*tf, tf*tf*tf, tf*tf*tf*tf, tf*tf*tf*tf*tf,
         0, 1, 2*tf, 3*tf*tf, 4*tf*tf*tf, 5*tf*tf*tf*tf,
         0, 0, 2, 6*tf, 12*tf*tf, 20*tf*tf*tf;

    B << p0, v0, a0, pf, vf, af;
    X = T.colPivHouseholderQr().solve(B);

    return {X(0), X(1), X(2), X(3), X(4), X(5)};
}

// Evaluate position, velocity, acceleration
void evaluateQuintic(QuinticCoeffs c, double t, double& pos, double& vel, double& acc) {
    pos = c.a0 + c.a1*t + c.a2*t*t + c.a3*t*t*t + c.a4*t*t*t*t + c.a5*t*t*t*t*t;
    vel = c.a1 + 2*c.a2*t + 3*c.a3*t*t + 4*c.a4*t*t*t + 5*c.a5*t*t*t*t;
    acc = 2*c.a2 + 6*c.a3*t + 12*c.a4*t*t + 20*c.a5*t*t*t;
}

// Generate a motion profile
std::vector<MotionProfile> generateMotionProfile(QuinticCoeffs xC, QuinticCoeffs yC, QuinticCoeffs thetaC, double tf, int steps) {
    std::vector<MotionProfile> profile;
    for (int i = 0; i <= steps; ++i) {
        double t = (tf / steps) * i;
        double x, vx, ax, y, vy, ay, theta, omega, alpha;
        
        evaluateQuintic(xC, t, x, vx, ax);
        evaluateQuintic(yC, t, y, vy, ay);
        evaluateQuintic(thetaC, t, theta, omega, alpha);

        profile.push_back({x, y, theta, vx, vy, omega});
    }
    return profile;
}

// Convert velocity to wheel speeds
WheelSpeeds tankDriveWheelSpeeds(double v, double omega, double track_width) {
    return { v - (omega * track_width / 2), v + (omega * track_width / 2) };
}

// RAMSETE Controller
WheelSpeeds ramseteControl(Pose robotPose, MotionProfile target, double b, double zeta, double track_width) {
    double ex = cos(robotPose.theta) * (target.x - robotPose.x) + sin(robotPose.theta) * (target.y - robotPose.y);
    double ey = -sin(robotPose.theta) * (target.x - robotPose.x) + cos(robotPose.theta) * (target.y - robotPose.y);
    double etheta = target.theta - robotPose.theta;

    double k1 = 2 * zeta * sqrt(target.omega * target.omega + b * target.vx * target.vx);
    double k3 = b * target.vx;

    double v = target.vx * cos(etheta) + k1 * ex;
    double omega = target.omega + k3 * ey + k1 * etheta;

    return tankDriveWheelSpeeds(v, omega, track_width);
}

// Clamp voltage to max/min limits
double clampVoltage(double voltage, double maxVoltage) {
    return std::max(-maxVoltage, std::min(voltage, maxVoltage));
}

// Update odometry
Pose updateOdometry(double v_left, double v_right, double track_width, double dt) {
    static Pose pose = {0, 0, 0};
    double v = (v_left + v_right) / 2;
    double omega = (v_right - v_left) / track_width;

    pose.x += v * cos(pose.theta) * dt;
    pose.y += v * sin(pose.theta) * dt;
    pose.theta += omega * dt;

    return pose;
}

// Set motor voltage
void setMotorVoltage(double left_voltage, double right_voltage) {
    std::cout << "Left Voltage: " << left_voltage << "V, Right Voltage: " << right_voltage << "V\n";
}

// Execute motion profile with voltage limits
void executePath(std::vector<std::vector<MotionProfile>> waypoints, std::vector<double> maxVoltages, double track_width, double dt) {
    Pose robotPose = {0, 0, 0};

    for (size_t i = 0; i < waypoints.size(); ++i) {
        auto profile = waypoints[i];
        double maxVoltage = maxVoltages[i];

        for (const auto& target : profile) {
            WheelSpeeds speeds = ramseteControl(robotPose, target, 2.0, 0.7, track_width);

            // Clamp voltages
            double left_voltage = clampVoltage(speeds.left * 12, maxVoltage);
            double right_voltage = clampVoltage(speeds.right * 12, maxVoltage);

            setMotorVoltage(left_voltage, right_voltage);

            robotPose = updateOdometry(speeds.left, speeds.right, track_width, dt);
        }
    }
}

// Main function
int main() {
    double tf = 5.0; // Time per segment
    int steps = 100; // Points per path
    double track_width = 0.5; // Meters
    double dt = tf / steps;

    std::vector<std::vector<MotionProfile>> waypoints;
    std::vector<double> maxVoltages = {12.0, 6.0, 10.0}; // Different voltages per movement

    // Define 3 movement segments with different voltage caps
    std::vector<std::pair<Pose, Pose>> targets = {
        {{0, 0, 0}, {2, 1, M_PI/4}}, // Move to (2,1) with 45°
        {{2, 1, M_PI/4}, {4, 2, M_PI/2}}, // Move to (4,2) with 90°
        {{4, 2, M_PI/2}, {5, 3, M_PI}} // Move to (5,3) with 180°
    };

    for (auto& [start, end] : targets) {
        QuinticCoeffs xC = computeQuinticSpline(start.x, 0, 0, end.x, 0, 0, tf);
        QuinticCoeffs yC = computeQuinticSpline(start.y, 0, 0, end.y, 0, 0, tf);
        QuinticCoeffs thetaC = computeQuinticSpline(start.theta, 0, 0, end.theta, 0, 0, tf);

        waypoints.push_back(generateMotionProfile(xC, yC, thetaC, tf, steps));
    }

    executePath(waypoints, maxVoltages, track_width, dt);
    return 0;
}
