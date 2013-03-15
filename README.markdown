Our FIRST robotic team ([FIRST Team 1768, Nashoba Robotics](http://www.nashobarobotics.org)) came up with the goal of having a COTS system board that plugs into the wireless switch and preprocesses the images from the camera. As such, we had to write code to faciliate communication between the camera, the coprocessor, and the cRIO. We route this information all over ethernet, through the switch.

Our plan is to have the coprocessor run a version of Arch Linux and C++ with image parsing provided by [OpenCV](http://opencv.willowgarage.com/wiki/).

The cRIO
========

The robot code is fairly simple: set up a UDP connection and poll the coprocessor for targets. This code depends on the networking and concurrency components of our [main robot code](https://github.com/Nashoba-Robotics/Nashoba-Robotics2013).

The Coprocessor
===============

The coprocessor connects to both the cRIO and the Camera, requesting images from the camera, parsing them, and providing the robot with feedback. It gets images from the camera over a TCP connection and hosts a UDP server for the robot to listen on.
