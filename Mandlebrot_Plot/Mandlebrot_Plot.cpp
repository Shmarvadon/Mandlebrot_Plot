#include <iostream>
#include <thread>

#include <CL/sycl.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

#include <cmath>

#define MAX_ITER 100

// Code written by ShmarvDogg, this is open source by its very nature so do with as you want but pls dont claim as ur own code <3


// This exception code is not mine and is obtained from SYCL example code written by intel.
static auto exception_handler = [](sycl::exception_list e_list) {
	for (std::exception_ptr const& e : e_list) {
		try {
			std::rethrow_exception(e);
		}
		catch (std::exception const& e) {
#if _DEBUG
			std::cout << "Failure" << std::endl;
#endif
			std::terminate();
		}
	}
};

void CalculateSetThread(uint64_t x_start,
	uint64_t x_end,
	uint64_t y_start,
	uint64_t y_end,
	cv::Mat* Image,
	uint64_t Image_Width,
	uint64_t Image_Height,
	double RE_Start,
	double RE_End,
	double IM_Start,
	double IM_End) {
	uint8_t* pixelptr = (uint8_t*)Image->data;
	int n = 0;
	std::complex<float> z = 0,
		c = 0;

	for (uint64_t i = x_start; i < x_end; i++) {
		for (uint64_t j = y_start; j < y_end; j++) {
			z = 0;
			c = std::complex(RE_Start + ((float)i / Image_Width) * (RE_End - RE_Start),
				IM_Start + ((float)j / Image_Height) * (IM_End - IM_Start));
			n = 0;

			while (abs(z) <= 2 && n < MAX_ITER) {
				z = z * z + c;
				n++;
			}

			pixelptr[i * Image->cols * 3 + j * 3 + 0] = 255 * ((double)n * 2 / MAX_ITER);
			pixelptr[i * Image->cols * 3 + j * 3 + 1] = 255 * ((double)n / MAX_ITER);
			pixelptr[i * Image->cols * 3 + j * 3 + 2] = 255 * ((double)n / MAX_ITER);
		}
	}
};

void PlotSet(
	std::string FileName,
	uint64_t rez,
	double RE_Start,
	double RE_End,
	double IM_Start,
	double IM_End) {

	std::vector<std::thread> Threads;
	uint64_t NumberOfThreds;

	cv::Mat image(rez, rez, CV_8UC3);

	NumberOfThreds = std::thread::hardware_concurrency();

	std::cout << "Threads Launched.\n";
	for (uint64_t i = 0; i <= NumberOfThreds; i++) {
		Threads.push_back(std::thread(CalculateSetThread,
			0,
			rez,
			i * (rez / (NumberOfThreds + 1)),
			(i + 1) * (rez / (NumberOfThreds + 1)),
			&image,
			rez,
			rez,
			RE_Start,
			RE_End,
			IM_Start,
			IM_End));
	}

	for (std::thread& th : Threads) {
		th.join();
	}
	std::cout << "Threads Finished.\n";

	std::string FileWriteName = "./" + FileName + ".png";
	bool check = cv::imwrite(FileWriteName, image);

	if (!check) {
		std::cout << "Something went wrong." << std::endl;
	}

	std::cout << "Image written?" << std::endl;
}

void PlotSet_DPCPP(
	std::string FileName,
	uint64_t rez,
	double RE_Start,
	double RE_End,
	double IM_Start,
	double IM_End) {

	// Create an OpenCV Mat object as our image.
	cv::Mat image(rez, rez, CV_8UC3);

	// define device selector. This will select the most capable device in the system to execute our kernels.
	sycl::default_selector d_selector;
	// define a queue. This is what we will use to submit kernels for execution to our device.
	sycl::queue q(d_selector, exception_handler);

	// display the chosen device in console to the user.
	std::cout << "Using device: " << q.get_device().get_info<sycl::info::device::name>() << std::endl;

	// setup the kernels resources.

	// This is the range object which specifies how big to make the buffers for SYCL.
	sycl::range<1> num_items{ rez * rez * 3};

	// Initialisng a device buffer that is backed by the host, this means that when the buffer is destroyed
	//  the information is copied back to the host memory at the location proided by pointer.
	sycl::buffer<uint8_t> img_buff((uint8_t*)image.data, num_items);

	// creating a buffer from a vector to feed in the domain of the plot (this is the real and imaginary bounds of the plot).
	// This can be thought of as the x,y coordinate limits of the plot.
	std::vector<float> domain = { (float)RE_Start, (float)RE_End, (float)IM_Start, (float)IM_End };
	sycl::buffer domain_buff(domain);

	// Specifying the image dimensions and storing them in a buffer such that a perspective transform can take place in the kernel.
	// This allows us to map the Mandelbrot set plot onto our image.
	std::vector<int> rez_vec = { (int)image.rows, (int)image.cols };
	sycl::buffer rez_buff(rez_vec);

	// this range object specifies a 2D for loop range, this is used to launch kernels with 2 iteration indexes.
	// this is similar to idx & idy in CUDA.
	sycl::range<2> num_pixels{ rez , rez };

	// use a lambda function to setup access to resources and submit the kernel for exectuion on the device.
	q.submit([&](sycl::handler& h) {
		// define some accessors so that the device can access the buffers defined earlier.
		// Some access flags are also defined such as read only or write only or read/write.
		sycl::accessor img_buff_access(img_buff, h, sycl::read_write, sycl::noinit);
		sycl::accessor domain_buff_access(domain_buff, h, sycl::read_only);
		sycl::accessor rez_buff_access(rez_buff, h, sycl::read_only);

		// launch the kernel on the device, in this case a parralel for loop which will launch a kernal for
		// each instance of 2D coordinate pair between 0,0 and image.rows, image.cols.

		h.parallel_for(num_pixels, [=](sycl::id<2> i) {
			int n = 0;
			int y = i[1];
			int x = i[0];

			// SYCL supports some std lib funcitons and templates!!!!!
			// This is setting up our complex / imaginary numbers.
			// As per the plotting algorithm, Z = 0
			std::complex<float> z = 0;
			// C is the result of mapping the x,y coordinates onto the domain of the set plot.
			std::complex<float> c(domain_buff_access[0] + ((float)x / rez_buff_access[0]) * (domain_buff_access[1] - domain_buff_access[0]), 
				domain_buff_access[2] + ((float)y / rez_buff_access[1]) * (domain_buff_access[3] - domain_buff_access[2]));

			// This will run untill either we reach MAX_ITER or the value is greater than or equal to 2.
			while (sqrt((z._Val[1] * z._Val[1]) + (z._Val[0] * z._Val[0])) <= 2 && n < MAX_ITER) {
				z = z * z + c;
				n++;
			}

			// x * rez_buff_access[0] * 3 is to calculate the position in the 1D vector of the row we are accessing.
			// this is becayse the image is a contiguous 1D block of memory not 2D and the * 3 is for the 3 colours per pixel
			// (uint8_t*) pointer so each index is a sub pixel.
			// y * 3 + <int> is to calculate the colum we are accessing, again each index is subpixel and y represents our position.
			// In the row (which colum we want to access).
			// the + <int> is which subpixel / colour channel we want to modify.

			img_buff_access[x * rez_buff_access[0] * 3 + y * 3 + 0] = 2 * (uint8_t)(255 * ((float)n / MAX_ITER));
			img_buff_access[x * rez_buff_access[0] * 3 + y * 3 + 1] = (uint8_t)(255 * ((float)n / MAX_ITER));
			img_buff_access[x * rez_buff_access[0] * 3 + y * 3 + 2] = (uint8_t)(255 * ((float)n / MAX_ITER));

			// above is our colouring algorithm. Essentially if we reach max iteration then the pixel is going to be white.
			// anything below that it will decrease in brightness while maintaining a blue hue till at n = 0 it is black.

			});

		});
	
	std::cout << "Sycl kernels launched and complted." << std::endl;

	// we manually call the destructor so that the data is written back to the host memory.
	img_buff.~buffer();

	// save the image and do a check to ensure it is saved properly.
	bool check = cv::imwrite("./TestImage.png", image);

	if (!check) {
		std::cout << "Something went wrong." << std::endl;
	}

	// Alert the user that something might have happened but probably not?
	std::cout << "Image written?" << std::endl;
};

int main(int argc, char*argv[]) {
	std::string FileName = "Mandelbrot_Plot";
	uint64_t rez = 1920;
	double RE_Start = -2,
		RE_End = 1,
		IM_Start = -1,
		IM_End = 1;

	if (argc <= 1) {
		std::cout << "No args passed, defaulting to defaults..." << std::endl;
	}
	if (argc == 2) {
		rez = std::atoi(argv[1]);
		std::cout << rez << std::endl;
	}
	// plot the image using SYCL / DPC++
	
	PlotSet_DPCPP(FileName, rez, RE_Start, RE_End, IM_Start, IM_End);


	// plot a single image.
	//PlotSet(FileName, rez, RE_Start, RE_End, IM_Start, IM_End);

	/*
	// plot a zoming set of images.
	double zoom_factor = 0.95;
	double x_shift = 0.05;
	double y_shift = -0.00535;
	for (size_t i = 0; i < 200; i++) {
		RE_Start = RE_Start * zoom_factor + zoom_factor * y_shift;
		RE_End = RE_End * zoom_factor + zoom_factor * y_shift;
		IM_Start = IM_Start * zoom_factor + zoom_factor * x_shift;
		IM_End = IM_End * zoom_factor + zoom_factor * x_shift;

		PlotSet("Zoom Sequence/"+FileName + std::to_string(i), rez, RE_Start, RE_End, IM_Start, IM_End);
	}
	*/

	std::cout << "End of program." << std::endl;
	return 0;
};