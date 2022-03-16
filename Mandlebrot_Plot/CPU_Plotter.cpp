#include "Plotters.h"


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
	// Initialise our variables needed for plotting and a pointer to interface with the images memory.
	uint8_t* pixelptr = (uint8_t*)Image->data;
	int n = 0;
	std::complex<float> z = 0,
		c = 0;

	// Loop over each pixel in the image calculating if it is in the set or not.
	for (uint64_t i = x_start; i < x_end; i++) {
		for (uint64_t j = y_start; j < y_end; j++) {
			z = 0;
			// mapping the pixel position in the image to the domain of the plot.
			c = std::complex(RE_Start + ((float)i / Image_Width) * (RE_End - RE_Start),
				IM_Start + ((float)j / Image_Height) * (IM_End - IM_Start));
			n = 0;

			// This loop does the check to see if the position falls within the set.
			while (abs(z) <= 2 && n < MAX_ITER) {
				z = z * z + c;
				n++;
			}

			// Setting the pixel values depending on how many times the while loop has iterated.
			// The blue channel is amplified by 2 to give the image a blue hue.
			pixelptr[i * Image->cols * 3 + j * 3 + 0] = 255 * ((double)n * 2 / MAX_ITER);
			pixelptr[i * Image->cols * 3 + j * 3 + 1] = 255 * ((double)n / MAX_ITER);
			pixelptr[i * Image->cols * 3 + j * 3 + 2] = 255 * ((double)n / MAX_ITER);
		}
	}
};

void PlotSet_MultiThreaded(
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
	for (uint64_t i = 0; i < NumberOfThreds; i++) {
		Threads.push_back(std::thread(CalculateSetThread,
			0,
			rez,
			i * (rez  / (NumberOfThreds + 1)),
			(i + 1) * (rez  / (NumberOfThreds + 1)),
			&image,
			rez,
			rez,
			RE_Start,
			RE_End,
			IM_Start,
			IM_End));
	}

	Threads.push_back(std::thread(CalculateSetThread,
		0,
		rez,
		(NumberOfThreds) * (rez / (NumberOfThreds + 1)),
		rez,
		&image,
		rez,
		rez,
		RE_Start,
		RE_End,
		IM_Start,
		IM_End));

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


void PlotSet_SingleThreaded(std::string FileName, uint64_t rez, double RE_Start, double RE_End, double IM_Start, double IM_End) {
	int n = 0;
	std::complex<float> z = 0,
		c = 0;

	std::cout << "Creating Image object." << std::endl;
	cv::Mat image(rez, rez, CV_8UC3);

	std::cout << "Image object created." << std::endl;

	uint8_t* pixelptr = (uint8_t*)image.data;

	std::cout << "Looping through the mandelbrot set." << std::endl;
	for (uint64_t i = 0; i < image.rows; i++) {
		for (uint64_t j = 0; j < image.cols; j++) {
			// complex number things
			z = 0;
			c = std::complex(RE_Start + ((float)i / image.cols) * (RE_End - RE_Start),
				IM_Start+ ((float)j / image.rows) * (IM_End - IM_Start));
			n = 0;


			while (abs(z) <= 2 && n < MAX_ITER) {
				z = z * z + c;
				n++;
			}

			pixelptr[i * image.cols * 3 + j * 3 + 0] = 255 * ((double)n / MAX_ITER);
			pixelptr[i * image.cols * 3 + j * 3 + 1] = 255 * ((double)n / MAX_ITER);
			pixelptr[i * image.cols * 3 + j * 3 + 2] = 255 * ((double)n / MAX_ITER);
		}
	}
	std::cout << "Loop through the set complete." << std::endl;

	std::string FileWriteName = "./" + FileName + ".png";
	bool check = cv::imwrite(FileWriteName, image);

	if (!check) {
		std::cout << "Shit, something went wrong." << std::endl;
	}

	std::cout << "Image written?" << std::endl;
};