#pragma once
//#include <deque>
#include <vector>
#include <cstdint>

//データは上から下へ流れる。BMPの作法の逆。
//this class not use BMP data rule.now.

template<class Color>
class Surface256 {
public:

	typedef std::uint16_t Cood;
	typedef std::uint8_t DataType;
	typedef Color ColorType;
	typedef std::size_t SizeType;

	//typedef std::deque<DataType> DType;
	//typedef std::deque<ColorType> PType;
	typedef std::vector<DataType> DType;
	typedef std::vector<ColorType> PType;

	Surface256() :Palette(1), Data(1), Width_(1), Height_(1) {}
	Surface256(const Cood& W, const Cood& H) :Palette(1), Data(W*H), Width_(W), Height_(H) {}

	SizeType PaletteSize() {
		return Palette.size();
	}
	Cood Width() {
		return Width_;
	}

	Cood Height() {
		return Height_;
	}

	DataType& IndexData(const Cood& X, const Cood& Y) {

		return Data[(Width_ * Y) + X];
	}
	ColorType& IndexColor(const SizeType& In) {
		return Palette[In];
	}

	PType& GetPallete() {//lolololololo....
		return Palette;
	}

	DType& GetData() {//lololololololllll....
		return Data;
	}




protected:
	PType Palette;
	DType Data;
	Cood Width_ = 1;
	Cood Height_ = 1;
};