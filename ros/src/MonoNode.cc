/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/


#include "MonoNode.h"

int main(int argc, char **argv)
{
    ros::init(argc, argv, "Mono");
    ros::start();

    if(argc != 3)
    {
        ROS_ERROR ("Path to vocabulary and path to settings need to be set.");
        ros::shutdown();
        return 1;
    }

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM2::System SLAM(argv[1],argv[2],ORB_SLAM2::System::MONOCULAR);
    ros::NodeHandle node_handle;
    image_transport::ImageTransport image_transport (node_handle);

    MonoNode node (&SLAM, node_handle, image_transport);

    ros::spin();

    // Stop all threads
    SLAM.Shutdown();

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");

    ros::shutdown();

    return 0;
}


MonoNode::MonoNode (ORB_SLAM2::System* pSLAM, ros::NodeHandle &node_handle, image_transport::ImageTransport &image_transport) {
  orb_slam = pSLAM;
  image_subscriber = image_transport.subscribe ("/camera/image_raw", 1, &MonoNode::ImageCallback, this);
  rendered_image_publisher = image_transport.advertise ("/orbslam2/debug_image", 1);
}


MonoNode::~MonoNode () {
}


void MonoNode::ImageCallback (const sensor_msgs::ImageConstPtr& msg) {
  cv_bridge::CvImageConstPtr cv_in_ptr;
  try {
      cv_in_ptr = cv_bridge::toCvShare(msg);
  } catch (cv_bridge::Exception& e) {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
  }

  orb_slam->TrackMonocular(cv_in_ptr->image,cv_in_ptr->header.stamp.toSec());

  /*cv_bridge::CvImage rendered_image_msg;
  rendered_image_msg.header   = msg->header;
  rendered_image_msg.encoding = sensor_msgs::image_encodings::TYPE_32FC1;
  rendered_image_msg.image    = orb_slam->DrawCurrentFrame(); // cv::Mat*/
  const sensor_msgs::ImagePtr rendered_image_msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", orb_slam->DrawCurrentFrame()).toImageMsg();

  rendered_image_publisher.publish (rendered_image_msg);
}