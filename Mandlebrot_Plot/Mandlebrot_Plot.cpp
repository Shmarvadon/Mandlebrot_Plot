#include <iostream>
#include <thread>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

#define MAX_ITER 100

// Code written by ShmarvDogg, this is open source by its very nature so do with as you want but pls dont claim as ur own code <3


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
			i * (rez / NumberOfThreds),
			(i + 1) * (rez / NumberOfThreds),
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

	// plot a single image.
	PlotSet(FileName, rez, RE_Start, RE_End, IM_Start, IM_End);

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