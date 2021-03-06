#ifndef BPCGEN_IMAGE_HPP_
#define BPCGEN_IMAGE_HPP_

#include <cstdarg>
#include <cstdio>
#include "Pixel.hpp"
class ImageProcess;
class PatternGenerator;
class PixelConverter;

extern const byte_t bitdepth;
#ifdef ENABLE_TIFF
struct tiff;
#endif
#ifdef ENABLE_PNG
extern const int colortype;
struct png_struct_def;
struct png_info_def;
struct png_text_struct;
#endif
extern const byte_t pixelsize;

#ifdef __GNUC__
#define ATTRIBUTE_FORMAT(archetype, strindex, first_to_check) __attribute__((format(archetype, strindex, first_to_check)))
#else
#define ATTRIBUTE_FORMAT(archetype, strindex, first_to_check)
#endif

class Row{
public:
	typedef Pixel<> pixel_type;
	Row(byte_t* row, const column_t& a_width): row_(row), width_(a_width){}
	const column_t& width()const{return width_;}
	pixel_type& operator[](column_t column)const{return *reinterpret_cast<pixel_type*>(const_cast<byte_t*>(row_) + column*pixelsize);}
	Row& operator++(){row_ += width()*pixelsize; return *this;}
	bool operator!=(const Row& rhs)const{return this->row_ != rhs.row_;}
	static void fill(Row first, Row last, const Row& row);
private:
	const byte_t* row_;
	const column_t& width_;
};

class Image{
public:
	typedef Row::pixel_type pixel_type;
	enum Orientation{
		ORI_HORI = 0x01,
		ORI_VERT = 0x02,
		ORI_AUTO = 0xff
	};
	enum FileFormat{
		FMT_NONE = 0x00,
		FMT_TIFF = 0x01,
		FMT_PNG  = 0x02
	};
	Image(const column_t& a_width, const row_t& a_height):
		head_(new byte_t[a_height*a_width*pixelsize]), width_(a_width), height_(a_height){}
	Image(const std::string& filename): head_(NULL), width_(0), height_(0){read(filename);}
	Image(const Image& image);
	Image& operator=(const Image& image);
	~Image(){delete[] head_;}
	Row operator[](row_t row)const{return Row(head_ + row*width()*pixelsize, width());}
	Image  operator<< (const PatternGenerator& generator)const;
	Image& operator<<=(const PatternGenerator& generator);
	Image& operator<<=(std::istream& is);
	Image  operator>> (const ImageProcess& process)const;
	Image& operator>>=(const ImageProcess& process);
	Image  operator>> (const PixelConverter& converter)const;
	Image& operator>>=(const PixelConverter& converter);
	Image& operator<<(const std::string& filename){return read(filename);}
	Image& operator>>(const std::string& filename)const{return write(filename);}
	Image  operator<< (byte_t shift)const;
	Image& operator<<=(byte_t shift);
	Image  operator>> (byte_t shift)const;
	Image& operator>>=(byte_t shift);
	Image  operator& (const Image& image)const;
	Image  operator& (const pixel_type& pixel)const;
	Image& operator&=(const pixel_type& pixel);
	Image  operator| (const Image& image)const;
	Image  operator| (const pixel_type& pixel)const;
	Image& operator|=(const pixel_type& pixel);
	Image  operator()(const Image& image, byte_t orientation = ORI_AUTO)const;
	Image& read(const std::string& filename);
	Image& write(const std::string& filename, FileFormat fmt = FMT_NONE)const;
	byte_t* head()const{return head_;}
	byte_t* tail()const{return head_ + data_size();}
	const column_t& width()const{return width_;}
	const row_t& height()const{return height_;}
	std::size_t data_size()const{return height_*width_*pixelsize;}
	Image& swap(Image& rhs);
private:
	class lshifter{
	public:
		lshifter(byte_t shift): shift_(shift){}
		void       operator()(      pixel_type& pixel)const;
		pixel_type operator()(const pixel_type& pixel)const;
	private:
		byte_t shift_;
	};
	class rshifter{
	public:
		rshifter(byte_t shift): shift_(shift){}
		void       operator()(      pixel_type& pixel)const;
		pixel_type operator()(const pixel_type& pixel)const;
	private:
		byte_t shift_;
	};
	class bit_and{
	public:
		bit_and(const pixel_type& pixel): pixel_(pixel){}
		void       operator()(      pixel_type& pixel)const;
		pixel_type operator()(const pixel_type& pixel)const;
	private:
		const pixel_type& pixel_;
	};
	class bit_or{
	public:
		bit_or(const pixel_type& pixel): pixel_(pixel){}
		void       operator()(      pixel_type& pixel)const;
		pixel_type operator()(const pixel_type& pixel)const;
	private:
		const pixel_type& pixel_;
	};
#ifdef ENABLE_TIFF
	class Tiff{
	public:
		Tiff(const std::string& filename, const char* mode);
		~Tiff();
		operator tiff*()const{return tif_;}
		static void error(const char* module, const char* fmt, std::va_list ap)ATTRIBUTE_FORMAT(printf, 2, 0);
	private:
		Tiff(const Tiff&);
		Tiff& operator=(const Tiff&);
		tiff* tif_;
	};
#endif
#ifdef ENABLE_PNG
	class File{
	public:
		File(const std::string& filename, const char* mode);
		~File(){std::fclose(fp_);}
		operator std::FILE*()const{return fp_;}
	private:
		File(const File& file);
		File& operator=(const File& file);
		std::FILE* fp_;
	};
	class Png{
	public:
		enum RW{
			IO_READ,
			IO_WRITE
		};
		Png(RW rw);
		~Png();
		operator png_struct_def*()const{return png_ptr_;}
		operator png_info_def*()const{return info_ptr_;}
	private:
		Png(const Png& png);
		Png& operator=(const Png& png);
		RW rw_;
		png_struct_def* png_ptr_;
		png_info_def* info_ptr_;
	};
#endif

#ifdef ENABLE_TIFF
	Image& read_tiff(const std::string& filename);
	Image& write_tiff(const std::string& filename)const;
#endif
#ifdef ENABLE_PNG
	Image& read_png(const std::string& filename);
	Image& write_png(const std::string& filename)const;
	static void construct_tEXt_chunk(png_text_struct* text_ptr);
#endif
#ifdef ENABLE_JPEG
	Image& read_jpeg(const std::string& filename);
#endif
	byte_t* head_;
	column_t width_;
	row_t height_;
};

inline std::istream& operator>>(std::istream& is, Image& image){image <<= is; return is;}
inline bool has_ext(const std::string& filename, const std::string& ext){const std::string::size_type idx = filename.find(ext); return !(idx == std::string::npos || idx + ext.size() != filename.size());}
inline std::string append_ext(const std::string& filename, const std::string& ext){return has_ext(filename, ext) ? filename : filename + ext;}
void get_current_time(char* buf);
#ifdef ENABLE_PNG
const char* get_current_time_rfc1123();
#endif

#endif
