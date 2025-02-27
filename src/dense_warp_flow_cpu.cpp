#include "common.h"
#include "dense_flow.h"

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <opencv2/optflow.hpp>

#include <stdio.h>
#include <iostream>

#include "warp_flow.h"

using namespace cv;
using namespace std;

void calcDenseWarpFlowCPU(string file_name, int bound, int type, int step, int dev_id,
					  std::vector<std::vector<uchar> >& output_x,
					  std::vector<std::vector<uchar> >& output_y){
	VideoCapture video_stream(file_name);
	CHECK(video_stream.isOpened())<<"Cannot open video stream \""
								  <<file_name
								  <<"\" for optical flow extraction.";

    // OpenCV 3.1.0 SURF interface
    //
    // source: http://stackoverflow.com/a/27533437/957997 
    //  http://stackoverflow.com/questions/27533203/how-do-i-use-sift-in-opencv-3-0-with-c
    cv::Ptr<Feature2D> detector_orb = cv::ORB::create(200);
	std::vector<Point2f> prev_pts_flow, pts_flow;
	std::vector<Point2f> prev_pts_surf, pts_surf;
	std::vector<Point2f> prev_pts_all, pts_all;
	std::vector<KeyPoint> prev_kpts_surf, kpts_surf;
	Mat prev_desc_surf, desc_surf;

	Mat capture_frame, capture_image, prev_image, capture_gray, prev_gray, human_mask;
	Mat flow, flow_x, flow_y;


    cv::Ptr<cv::optflow::DualTVL1OpticalFlow> alg_tvl1 = cv::optflow::DualTVL1OpticalFlow::create();

	bool initialized = false;
	int cnt = 0;
	while(true){

		//build mats for the first frame
		if (!initialized){
			video_stream >> capture_frame;
			if (capture_frame.empty()) return; // read frames until end
			initializeMats(capture_frame, capture_image, capture_gray,
						   prev_image, prev_gray);
			capture_frame.copyTo(prev_image);
			cvtColor(prev_image, prev_gray, cv::COLOR_BGR2GRAY);

			//detect key points
			human_mask = Mat::ones(capture_frame.size(), CV_8UC1);
			detector_orb->detect(prev_gray, prev_kpts_surf, human_mask); 
			detector_orb->compute(prev_gray, prev_kpts_surf, prev_desc_surf); 

			initialized = true;
			for(int s = 0; s < step; ++s){
				video_stream >> capture_frame;
				cnt ++;
				if (capture_frame.empty()) return; // read frames until end
			}
		}else {
			capture_frame.copyTo(capture_image);
			cvtColor(capture_image, capture_gray, cv::COLOR_BGR2GRAY);

			switch(type){
				case 0: {
					break;
				}
				case 1: {
					alg_tvl1->calc(prev_gray, capture_gray, flow);
					break;
				}
				case 2: {
					break;
				}
				default:
					LOG(ERROR)<<"Unknown optical method: "<<type;
			}

            Mat planes[2];
            cv::split(flow, planes);

			//get back flow map
            Mat flow_x(planes[0]);
            Mat flow_y(planes[1]);

			// warp to reduce holistic motion
			detector_orb->detect(capture_gray, kpts_surf, human_mask);
			detector_orb->compute(capture_gray, kpts_surf, desc_surf);
			ComputeMatch(prev_kpts_surf, kpts_surf, prev_desc_surf, desc_surf, prev_pts_surf, pts_surf);
			MatchFromFlow_copy(capture_gray, flow_x, flow_y, prev_pts_flow, pts_flow, human_mask);
			MergeMatch(prev_pts_flow, pts_flow, prev_pts_surf, pts_surf, prev_pts_all, pts_all);
			Mat H = Mat::eye(3, 3, CV_64FC1);
			if(pts_all.size() > 50) {
				std::vector<unsigned char> match_mask;
				Mat temp = findHomography(prev_pts_all, pts_all, RANSAC, 1, match_mask);
				if(cv::countNonZero(Mat(match_mask)) > 25)
					H = temp;
			}

			Mat H_inv = H.inv();
			Mat gray_warp = Mat::zeros(capture_gray.size(), CV_8UC1);
			MyWarpPerspective(prev_gray, capture_gray, gray_warp, H_inv);


			switch(type){
				case 0: {

					LOG(ERROR)<<"of method not supported"<<type;
					break;
				}
				case 1: {
					alg_tvl1->calc(prev_gray, gray_warp, flow);
					break;
				}
				case 2: {
					LOG(ERROR)<<"of method not supported"<<type;
					break;
				}
				default:
					LOG(ERROR)<<"Unknown optical method: "<<type;
			}


			//get back flow map
            cv::split(flow, planes);
            Mat flow_x_warp(planes[0]);
            Mat flow_y_warp(planes[1]);

			vector<uchar> str_x, str_y;
			encodeFlowMap(flow_x_warp, flow_y_warp, str_x, str_y, bound);

			output_x.push_back(str_x);
			output_y.push_back(str_y);

			std::swap(prev_gray, capture_gray);
			std::swap(prev_image, capture_image);


			//get next frame
			bool hasnext = true;
			for(int s = 0; s < step; ++s){
				video_stream >> capture_frame;
				cnt ++;
				hasnext = !capture_frame.empty();
				// read frames until end
			}
			if (!hasnext){
				return;
			}
		}


	}
}
