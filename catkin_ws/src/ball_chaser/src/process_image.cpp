#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <math.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    if (!client.call(srv)) {
        ROS_ERROR("command_robot service error");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int left_pixels_cnt = 0;
    int midle_pixels_cnt = 0;
    int right_pixels_cnt = 0;
    int left_max_index = floor(img.step / 3);
    int right_max_index = left_max_index * 2;

    // image height, that is, number of rows
    // ROS_INFO("Image rows %d", img.height);
    // image width, that is, number of columns
    // ROS_INFO("Image columns %d", img.width);
    // image width, that is, number of columns
    // ROS_INFO("Image encoding %s", img.encoding);
    // ROS_INFO("Image step %i", img.step);
    
    for (int row = 0; row < img.height; row++) {
        int row_offset = img.step * row;
        // Read 3 bytes of a row - RGB
        for (int step = 0; step < img.step; step += 3) {
            // printf( "step is %d\n", img.step );
            // printf( "left max index is %d\n", left_max_index );
            // printf( "right max index is %d\n", right_max_index );
            if (img.data[row_offset+ step] == white_pixel && img.data[row_offset + step + 1] == white_pixel && img.data[row_offset + step+2] == white_pixel) {
                // ROS_INFO("Step is %d", step);
		if (step < left_max_index) {
		    left_pixels_cnt++;
		}
		if (step >= left_max_index && step < right_max_index) {
		    midle_pixels_cnt++;
		}
                if (step >= right_max_index) {
		    right_pixels_cnt++;
		}
	    }
        }
    }

    ROS_INFO("Left pixels %d", left_pixels_cnt);
    ROS_INFO("Middle pixels %d", midle_pixels_cnt);
    ROS_INFO("Right pixels %d", right_pixels_cnt);

    if (left_pixels_cnt+midle_pixels_cnt+right_pixels_cnt == 0) {
        drive_robot(0.0, 0.0);
    } else {
	if (left_pixels_cnt >= midle_pixels_cnt && left_pixels_cnt >= right_pixels_cnt) {
            // Go left
            drive_robot(0.2, 4.0);
        }
        if (midle_pixels_cnt >=left_pixels_cnt && midle_pixels_cnt >= right_pixels_cnt) {
            // Go forward
            drive_robot(0.2, 0.0);
        }
	if (right_pixels_cnt >=left_pixels_cnt && right_pixels_cnt >= midle_pixels_cnt) {
            // Go right
            drive_robot(0.2, -4.0);
        }
    }
    
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);
    ROS_INFO("Subscribed to camera/rgb/image_raw");

    // Handle ROS communication events
    ros::spin();

    return 0;
}
