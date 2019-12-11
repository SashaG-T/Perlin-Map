#pragma once
template<typename T> class GenericMap;

template<typename T>
class GenericProxy {

private:
	GenericMap<T>* parent;
	int xOffset;

public:
	GenericProxy(GenericMap<T>* parent, int xOffset) : parent(parent), xOffset(xOffset) {};

	T& operator[](int yOffset) {
		if (yOffset < 0) {
			yOffset = parent->height + (yOffset % (int)parent->height);
		}
		else if (yOffset >= (int)parent->height) {
			yOffset = yOffset % (int)parent->height;
		}
		return parent->data[xOffset + yOffset * parent->width];
	}

};

template<typename T>
class GenericMap {

	friend GenericProxy<T>;

private:
	unsigned width, height;
	T* data;

public:
	GenericMap(unsigned width, unsigned height) : width(width), height(height), data(new T[width*height]) {};
	~GenericMap() { delete[] data; };

	GenericProxy<T> operator[] (int xOffset) {
		if (xOffset < 0) {
			xOffset = width + (xOffset % (int)width);
		}
		else if (xOffset >= (int)width) {
			xOffset = xOffset % (int)width;
		}
		return GenericProxy<T>(this, xOffset);
	}

	unsigned getWidth() { return width; };
	unsigned getHeight() { return height; };

	T* ptr() { return data; };

};