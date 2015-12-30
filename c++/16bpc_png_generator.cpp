/**
 * @file
 * @brief Program to generate a 16 bpc(bits per component) PNG image.
 * @details
 * To compile this program, do as described below.
 * @code
 * $ g++ --std=c++14 generate_png.cpp -lpng
 * @endcode
 * Then, to run this program, do as described below.
 * @code
 * $ ./a.out
 * @endcode
 */

#define PNG_NO_SETJMP

#include <cstdio>
#include <functional>
#include <memory>
#include <png.h>

constexpr png_uint_32 width  = 1920;               // number of horizontal pixels
constexpr png_uint_32 height = 1080;               // number of vertical pixels
constexpr int bitdepth       = 16;                 // bitdepth per component
constexpr int colortype      = PNG_COLOR_TYPE_RGB;
constexpr int pixelsize      = 6;                  // bytes per pixel

struct RGB{
	png_byte Rmsb_;
	png_byte Rlsb_;
	png_byte Gmsb_;
	png_byte Glsb_;
	png_byte Bmsb_;
	png_byte Blsb_;
	RGB(png_uint_16 R, png_uint_16 G, png_uint_16 B): Rmsb_(R>>8), Rlsb_(R&0xff), Gmsb_(G>>8), Glsb_(G&0xff), Bmsb_(B>>8), Blsb_(B&0xff){}
};

struct Row{
	png_bytep row_;
	Row(png_bytep row): row_(row){}
	RGB& operator[](int column){return *(RGB*)(row_ + column*pixelsize);}
};

struct Image{
	png_bytep block_;
	Image(png_bytep block): block_(block){}
	Row operator[](int row){return Row(block_ + row*width*pixelsize);}
};

int main(int argc, char* argv[])
{
	std::unique_ptr<FILE, decltype(&std::fclose)> fp(std::fopen("16bpc_img.png", "wb"), std::fclose);
	if(!fp){
		std::perror(argv[0]);
		return -1;
	}
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(!png_ptr){
		return -1;
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr){
		png_destroy_write_struct(&png_ptr, nullptr);
		return -1;
	}
	png_init_io(png_ptr, fp.get());
	png_set_IHDR(png_ptr, info_ptr, width, height, bitdepth, colortype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	std::unique_ptr<png_byte[]> block(new png_byte[height*width*pixelsize]);

	/* Edit from here, */

	Image image(block.get());
	for(int row=0; row<height; ++row){
		for(int column=0; column<width; ++column){
			image[row][column] = {column*0xffff/width, 0x00, 0x00};
		}
	}

	/* to here */

	std::unique_ptr<png_bytep[]> row_ptrs(new png_bytep[height]);
	for(int i=0; i<height; ++i){
		row_ptrs[i] = block.get() + i*width*pixelsize;
	}
	png_set_rows(png_ptr, info_ptr, row_ptrs.get());
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return 0;
}