// Code written by ShmarvDogg, this is open source by its very nature so do with as you want but pls dont claim as ur own code <3

// Plotters.h includes all the needed headers!
#include "Plotters.h"

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

	if (argc == 3) {
		rez = std::atoi(argv[1]);

		if (argv[2] == (std::string)"DPCPP") {
			std::cout << "Selected Intel DPC++ renderer." << std::endl;
			PlotSet_DPCPP(FileName, rez, RE_Start, RE_End, IM_Start, IM_End);
		}

		else if (argv[2] == (std::string)"ST") {
			std::cout << "Selected single threaded renderer." << std::endl;
			PlotSet_SingleThreaded(FileName, rez, RE_Start, RE_End, IM_Start, IM_End);
		}

		else if (argv[2] == (std::string)"MT") {
			std::cout << "Selected multi threaded renderer." << std::endl;
			PlotSet_MultiThreaded(FileName, rez, RE_Start, RE_End, IM_Start, IM_End);
		}
		std::cout << "End of program." << std::endl;
		return 0;

	}

	std::cout << "Automatically selected Intel DPC++ renderer." << std::endl;
	PlotSet_DPCPP(FileName, rez, RE_Start, RE_End, IM_Start, IM_End);

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