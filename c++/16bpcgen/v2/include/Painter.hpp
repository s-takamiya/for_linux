#ifndef _16BPCGEN_PAINTER_HPP_
#define _16BPCGEN_PAINTER_HPP_

#if 201103L <= __cplusplus
#	include <random>
#endif
#include "FrameBuffer.hpp"

class Painter{
public:
	virtual ~Painter(){}
	virtual Pixel operator()() = 0;
};

class UniColor: Painter{
public:
	UniColor(const Pixel& pixel): pixel_(pixel){}
	virtual Pixel operator()(){return pixel_;}
private:
	const Pixel pixel_;
};

class Gradator: Painter{
public:
	Gradator(const Pixel& step, const Pixel& initial=black, bool invert=false):
		step_(step), state_(initial), invert_(invert){}
	Pixel operator()()
	{
		Pixel tmp = state_;
		state_ = invert_ ? state_ - step_ : state_ + step_;
		return tmp;
	}
private:
	Pixel step_;
	Pixel state_;
	bool invert_;
};

#if 201103L <= __cplusplus
class RandomColor: Painter{
public:
	RandomColor(): engine_(), distribution_(0x0000, 0xffff){}
	virtual Pixel operator()(){return {distribution_(engine_), distribution_(engine_), distribution_(engine_)};}
private:
	std::mt19937 engine_;
	std::uniform_int_distribution<uint16_t> distribution_;
};
#endif

#endif
