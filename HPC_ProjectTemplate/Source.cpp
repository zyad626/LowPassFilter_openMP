#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>
#include <omp.h>

using namespace std;
using namespace msclr::interop;

struct Filter
{
	int size;
	int* data;
};
struct Image_struct {
	int width;
	int height;
	int* data;
};
#include <iostream>
#include <iomanip>

void blur(Filter filter, Image_struct input_img, Image_struct* output_img);
Image_struct add_zero_padding(Image_struct input_img, int filter_size);

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int* Red = new int[BM.Height * BM.Width];
	int* Green = new int[BM.Height * BM.Width];
	int* Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height * BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i * BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

		}

	}
	return input;
}


void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i * width + j] < 0)
			{
				image[i * width + j] = 0;
			}
			if (image[i * width + j] > 255)
			{
				image[i * width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//outputRes" + index + ".png");
	cout << "result Image Saved " << index << endl;
}

int main()
{
	int start_s, stop_s, TotalTime = 0;

	//Image Path
	System::String^ imagePath;
	std::string img;
	img = "..//Data//Input//lena.png";

	//Filter intialization
	Filter filter;
	cout << "Enter kernel size (must be an odd number)" << endl;
	cin >> filter.size;
	if (filter.size % 2 == 0)
		return 0;
	filter.data = new int[filter.size * filter.size];
	for (int i = 0; i < filter.size; i++) {
		for (int j = 0; j < filter.size; j++) {
			filter.data[i * filter.size + j] = 1;
		}
	}

	//get number of threads
	int n_threads = 1;
	cout << "Enter number of threads" << endl;
	cin >> n_threads;
	omp_set_num_threads(n_threads);

	//input the image
	Image_struct input_img;
	imagePath = marshal_as<System::String^>(img);
	input_img.data = inputImage(&input_img.width, &input_img.height, imagePath);

	cout << "input Image width = " << input_img.width << "\n" << "input Image height = " << input_img.height << endl;

	//add zero padding to input image
	Image_struct padded_img = add_zero_padding(input_img, filter.size);

	//start timer
	start_s = clock();

	//blur the image
	Image_struct* output_img = new Image_struct();
	blur(filter, padded_img, output_img);

	//stop timer
	stop_s = clock();

	cout << "output Image width = " << output_img->width << "\n" << "output Image height = " << output_img->height << endl;

	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;

	//create the image
	createImage(output_img->data, output_img->width, output_img->height, 1);


	cout << "time: " << TotalTime << endl;

	//free all allocated memory
	delete output_img->data;
	delete output_img;
	delete input_img.data;
	delete filter.data;
	delete padded_img.data;
	int c; cin >> c;
	return 0;

}

void blur(Filter filter, Image_struct input_img, Image_struct* output_img) {
	output_img->height = input_img.height - filter.size + 1;
	output_img->width = input_img.width - filter.size + 1;

	output_img->data = new int[output_img->height * output_img->width];
	int sop = 0;
#pragma omp parallel for private(sop)
	for (int i = 0; i < output_img->height; i++) {
		for (int j = 0; j < output_img->width; j++) {
			sop = 0;
			for (int h = 0; h < filter.size; h++) {
				for (int w = 0; w < filter.size; w++) {
					sop += filter.data[h * filter.size + w] * input_img.data[(i + h) * input_img.width + (j + w)];
				}
			}
			output_img->data[i * output_img->width + j] = sop / (filter.size * filter.size);
		}
	}
	
}
Image_struct add_zero_padding(Image_struct input_img, int filter_size) {
	Image_struct padded_img;
	padded_img.height = input_img.height + filter_size - 1;
	padded_img.width = input_img.width + filter_size - 1;
	padded_img.data = new int[padded_img.height * padded_img.width];
	/* by default the padded image is initialized with zeros. We just have to fill in with the input image */
	int input_img_start_row = (filter_size - 1) / 2;//where the input image should start inside the padded image
	int input_img_start_col = (filter_size - 1) / 2;
	for (int i = 0; i < input_img.height; i++) {
		for (int j = 0; j < input_img.width; j++) {
			padded_img.data[(i + input_img_start_row) * padded_img.width + (j + input_img_start_col)]
				= input_img.data[i * input_img.width + j];
		}
	}
	cout << "Padded image height and width = " << padded_img.height << endl;

	return padded_img;
}
