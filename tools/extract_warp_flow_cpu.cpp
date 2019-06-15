#include "dense_flow.h"
#include "utils.h"

INITIALIZE_EASYLOGGINGPP

using namespace cv::cuda;

int main(int argc, char** argv){
	// IO operation
	const char* keys =
		{
			"{ f vidFile   | ex2.avi | filename of video }"
			"{ x xFlowFile | flow_x  | filename of flow x component }"
			"{ y yFlowFile | flow_y  | filename of flow y component }"
			"{ b bound     | 15      | specify the maximum of optical flow}"
			"{ t type      | 0       | specify the optical flow algorithm }"
			"{ d device_id | 0       | set gpu id}"
			"{ s step      | 1       | specify the step for frame sampling}"
			"{ o out       | zip     | output style}"
		};

	CommandLineParser cmd(argc, argv, keys);
	std::string vidFile = cmd.get<std::string>("vidFile");
	std::string xFlowFile = cmd.get<std::string>("xFlowFile");
	std::string yFlowFile = cmd.get<std::string>("yFlowFile");
	std::string output_style = cmd.get<std::string>("out");
	int bound = cmd.get<int>("bound");
    int type  = cmd.get<int>("type");
    int device_id = cmd.get<int>("device_id");
    int step = cmd.get<int>("step");

    std::cout << "vid file "<< vidFile << std::endl;
    std::cout << "x flow "<< xFlowFile << std::endl;
    std::cout << "y flow "<< yFlowFile << std::endl;
    std::cout << "output style "<< output_style << std::endl;

	std::vector<std::vector<uchar> > out_vec_x, out_vec_y;

	calcDenseWarpFlowCPU(vidFile, bound, type, step, device_id,
					 out_vec_x, out_vec_y);

	if (output_style == "dir") {
		writeImages(out_vec_x, xFlowFile);
		writeImages(out_vec_y, yFlowFile);
	}else{
	}

	return 0;
}
