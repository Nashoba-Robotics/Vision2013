
Our FIRST robotic team ([FIRST Team 1768, Nashoba Robotics](http://www.nashobarobotics.org)) came up with the goal of having a COTS system board that plugs into the wireless switch and preprocesses the images from the camera. As such, we had to write code to faciliate communication between the camera, the coprocessor, and the cRIO. We route this information all over ethernet, through the switch.

Our plan is to have the coprocessor run a version of FreeBSD and Python, with image parsing provided by [OpenCV](http://opencv.willowgarage.com/wiki/).

The cRIO
========

The robot code is fairly simple: set up a UDP connection and poll the coprocessor for targets. This code depends on the networking and concurrency components of our [main robot code](https://github.com/Nashoba-Robotics/Nashoba-Robotics).

The Coprocessor
===============

The coprocessor connects to both the cRIO and the Camera, requesting images from the camera, parsing them, and providing the robot with feedback. It gets images from the camera over a TCP connection and hosts a UDP server for the robot to listen on.

Testing a Recognition Algorithm
===============================

We have a test suite for various target detection algorithms. Given a manifest file containing image file names and target coordinates, the test suite runs the image detection code and ranks the algorithm based on two factors: accuracy and precision (Interested in the difference? See [here](http://en.wikipedia.org/wiki/Accuracy_and_precision)). The ranking algorithm is tolerant of small mistakes, but if you get a single target more than 40px off, you're probably going to get a negative score. I consider "good scores" (having not tested this with my own code) to be positive scores. After that, most image detection becomes unusable.

Run the ranking algorithm with `./algtest.py ExampleManifest algorithmFile.py`
