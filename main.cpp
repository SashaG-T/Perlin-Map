#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "GenericMap.h"

const int MAP_WIDTH(400);
const int MAP_HEIGHT(400);

GenericMap<float> randomData[]
{
	GenericMap<float>(MAP_WIDTH, MAP_HEIGHT),
	GenericMap<float>(MAP_WIDTH, MAP_HEIGHT),
};

// Function to linearly interpolate between a0 and a1
// Weight w should be in the range [0.0, 1.0]
float lerp(float a0, float a1, float w) {
	return (1.0 - w)*a0 + w * a1;

	// as an alternative, this slightly faster equivalent formula can be used:
	// return a0 + w*(a1 - a0);
}

// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y) {

	// Precomputed (or otherwise) gradient vectors at each grid node
	//extern float Gradient[200][200][2];

	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// Compute the dot-product
	//return (dx*Gradient[iy][ix][0] + dy * Gradient[iy][ix][1]);
	return dx * randomData[0][iy][ix] + dy * randomData[1][iy][ix];
}

// Compute Perlin noise at coordinates x, y
float perlin_b(float x, float y) {

	// Determine grid cell coordinates
	int x0 = int(x);
	int x1 = x0 + 1;
	int y0 = int(y);
	int y1 = y0 + 1;

	// Determine interpolation weights
	// Could also use higher order polynomial/s-curve here
	float sx = x - (float)x0;
	float sy = y - (float)y0;

	// Interpolate between grid point gradients
	float n0, n1, ix0, ix1, value;
	n0 = dotGridGradient(x0, y0, x, y);
	n1 = dotGridGradient(x1, y0, x, y);
	ix0 = lerp(n0, n1, sx);
	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	ix1 = lerp(n0, n1, sx);
	value = lerp(ix0, ix1, sy);

	return value;
}

float perlin_r(float x, float y, int d) {
	//return (perlin(x / d, y / d) + ((d -= 1, (d >= 1 ? perlin_r(x, y, d) * d : 0)))) / (d + 1);
	return (perlin_b(x / d, y / d) + (d > 1 ? perlin_r(x, y, d / 2) : 0));
}

float perlin(float x, float y, int r) {
	int d(1);
	int rr(r);
	for (int rr = r; rr > 0; d++) {
		rr /= 2;
	}
	return perlin_r(x, y, r) / d;
}

unsigned char colourFromHeight(int colour, unsigned char height) {
	unsigned char colours[8][4]
	{
		{255, 0, 0, 64},
		{255, 0, 0, 128},
		{255, 0, 0, 255},
		{255, 128, 128, 255},
		{0, 255, 0, 255},
		{0, 255, 0, 128},
		{255, 255, 255, 128},
		{255, 255, 255, 255},
	};
	unsigned char val(height / 32);
	return colours[val][colour];
}

int main(void) {

	const int windowWidth(MAP_WIDTH);
	const int windowHeight(MAP_HEIGHT);
	const int pixelSize = 2;

	char field_window[] = "Field";
	cv::Mat nnt = cv::Mat::zeros(windowWidth*pixelSize, windowHeight*pixelSize, CV_8UC4);

	GenericMap<float> mapData(windowWidth, windowHeight);

	srand(time(0));	

	for (;;) {
		int max(windowWidth*windowHeight);
		for (int i = 0; i < max; i++) {
			for (int j = 0; j < 2; j++) {
				randomData[j].ptr()[i] = ((float)(rand() % UCHAR_MAX)) / (float)UCHAR_MAX;
			}
		}

		CV_Assert(nnt.channels() == 4);

		float maxHeight(0.f), minHeight(1.f);
		for (int x = 0; x < MAP_WIDTH; x++) {
			for (int y = 0; y < MAP_HEIGHT; y++) {
				float p(perlin(x, y, 1000));
				if (p > maxHeight) {
					maxHeight = p;
				}
				if (p < minHeight) {
					minHeight = p;
				}
				mapData[x][y] = p;
			}
		}

		float scale(1.f / (maxHeight - minHeight));
		for (int x = 0; x < MAP_WIDTH; x++) {
			for (int y = 0; y < MAP_HEIGHT; y++) {
				mapData[x][y] -= minHeight;
				mapData[x][y] *= scale;
			}
		}

		for (int i = 0; i < nnt.rows; i++) {
			for (int j = 0; j < nnt.cols; j++) {
				int x(i / pixelSize);
				int y(j / pixelSize);
				unsigned char val(mapData[x][y] * UCHAR_MAX);
				cv::Vec4b& bgra = nnt.at<cv::Vec4b>(i, j);
				
				bgra[0] = colourFromHeight(0, val);
				bgra[1] = colourFromHeight(1, val);
				bgra[2] = colourFromHeight(2, val);
				bgra[3] = colourFromHeight(3, val);
				
				/*
				bgra[0] = val;
				bgra[1] = val;
				bgra[2] = val;
				bgra[3] = 255;
				*/
				
			}
		}

		cv::imshow(field_window, nnt);
		cv::waitKey(0);
	}

	cv::imshow(field_window, nnt);

	cv::waitKey(0);
	return 0;
}