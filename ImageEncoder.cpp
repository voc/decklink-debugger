#include "ImageEncoder.h"

#include <sstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <assert.h>
#include <png.h>

#include "MutableVideoFrame.h"

ImageEncoder::ImageEncoder(DeviceProber* deviceProber) :
	m_refCount(1),
	m_deviceProber(deviceProber)
{
	m_frameConverter = CreateVideoConversionInstance();
}

ULONG ImageEncoder::AddRef(void)
{
	return __sync_add_and_fetch(&m_refCount, 1);
}

ULONG ImageEncoder::Release(void)
{
	int32_t newRefValue = __sync_sub_and_fetch(&m_refCount, 1);
	if (newRefValue == 0)
	{
		m_frameConverter->Release(); // does not assert to 0

		delete this;
		return 0;
	}
	return newRefValue;
}

std::string ImageEncoder::EncodeImage() {
	IDeckLinkVideoFrame* frame = m_deviceProber->GetLastFrame();
	if(frame == NULL) {
		return "";
	}

	frame->AddRef();

	frame = convertFrameIfReqired(frame);
	std::string encodedImage = encodeToPng(frame);

	frame->Release();

	return encodedImage;
}

void pngWriteCallback(png_structp  png_ptr, png_bytep data, png_size_t length)
{
	std::stringstream* stream = (std::stringstream*)png_get_io_ptr(png_ptr);
	std::string str = std::string((const char*)data, length);
	(*stream) << str;
}

std::string ImageEncoder::encodeToPng(IDeckLinkVideoFrame* frame)
{
	png_structp png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,
		/* user_error_ptr */  NULL,
		/* user_error_fn */   NULL,
		/* user_warning_fn */ NULL);

	if (!png_ptr) {
		std::cerr << "Unable to allocate png_structp" << std::endl;
		exit(1);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		std::cerr << "Unable to allocate png_infop" << std::endl;
		exit(1);
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		std::cerr << "Unable to setup png_jmpbuf" << std::endl;
		exit(1);
	}

	png_set_IHDR(
		png_ptr,
		info_ptr,
		frame->GetWidth(),
		frame->GetHeight(),
		/* bit_depth */ 8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);


	png_byte* frameBytes;
	frame->GetBytes((void**) &frameBytes);
	
	png_bytepp row_pointers = new png_bytep[frame->GetHeight()];
	for(long row = 0; row < frame->GetHeight(); row++)
	{
		row_pointers[row] = frameBytes + (row * frame->GetRowBytes());
	}

	png_set_rows(
		png_ptr,
		info_ptr,
		row_pointers);

	std::stringstream pngBody;

	png_set_write_fn(
		png_ptr,
		&pngBody,
		pngWriteCallback,
		NULL);

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, NULL);

	if (info_ptr != NULL)
	{
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	}

	if (png_ptr != NULL)
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	}

	delete row_pointers;

	return pngBody.str();
}

IDeckLinkVideoFrame* ImageEncoder::convertFrameIfReqired(IDeckLinkVideoFrame* frame)
{
	HRESULT result;

	if(frame->GetPixelFormat() == bmdFormat8BitBGRA)
	{
		return frame;
	}

	IDeckLinkVideoFrame* convertedFrame = new MutableVideoFrame(
		frame->GetWidth(),
		frame->GetHeight(),
		bmdFormat8BitBGRA);

	result = m_frameConverter->ConvertFrame(frame, convertedFrame);
	if (result != S_OK)
	{
		fprintf(stderr, "Failed to convert frame\n");
		exit(1);
	}

	frame->Release();
	return convertedFrame;
}
