#include "ClipboardHandler.h"
#include <QMimeData>
#include <QBuffer>
#include <QClipboard>
#include <QApplication>
#include <QImageReader>
#include <QtWinExtras/QtWin>

class ClipboardHandlerImage : public ClipboardHandler
{
	ClipboardFormat cbfmtDIB;
	ClipboardFormat cbfmtDIBV5;
	ClipboardFormat cbfmtPNG;
	ClipboardFormat cbfmtGIF;
	ClipboardFormat cbfmtJFIF;
	QStringList     mimeImageFormats;
public:
	ClipboardHandlerImage()
		: ClipboardHandler()
		, cbfmtDIB("CF_DIB")
		, cbfmtDIBV5("CF_DIBV5")
		, cbfmtPNG("PNG")
		, cbfmtGIF("GIF")
		, cbfmtJFIF("JFIF")
	{
		mimeImageFormats << "image/png";
		mimeImageFormats << "image/bmp";
		mimeImageFormats << "image/ico";
		mimeImageFormats << "image/jpg";
		mimeImageFormats << "image/jpeg";
		mimeImageFormats << "image/ppm";
		mimeImageFormats << "image/tif";
		mimeImageFormats << "image/tiff";
		mimeImageFormats << "image/xbm";
		mimeImageFormats << "image/xpm";
	}
	~ClipboardHandlerImage()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName);
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName);
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName);
};


////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CF_DIBV5
#define CF_DIBV5 17
#endif

static ClipboardHandlerImage s_ClipboardHandlerImage;

////////////////////////////////////////////////////////////////////////////////////////////////////


/* The MSVC compilers allows multi-byte characters, that has the behavior of
 * that each character gets shifted into position. 0x73524742 below is for MSVC
 * equivalent to doing 'sRGB', but this does of course not work
 * on conformant C++ compilers. */
#define BMP_LCS_sRGB  0x73524742
#define BMP_LCS_GM_IMAGES  0x00000004L

struct _CIEXYZ {
	long ciexyzX, ciexyzY, ciexyzZ;
};

struct _CIEXYZTRIPLE {
	_CIEXYZ  ciexyzRed, ciexyzGreen, ciexyzBlue;
};

struct BMP_BITMAPV5HEADER {
	DWORD  bV5Size;
	LONG   bV5Width;
	LONG   bV5Height;
	WORD   bV5Planes;
	WORD   bV5BitCount;
	DWORD  bV5Compression;
	DWORD  bV5SizeImage;
	LONG   bV5XPelsPerMeter;
	LONG   bV5YPelsPerMeter;
	DWORD  bV5ClrUsed;
	DWORD  bV5ClrImportant;
	DWORD  bV5RedMask;
	DWORD  bV5GreenMask;
	DWORD  bV5BlueMask;
	DWORD  bV5AlphaMask;
	DWORD  bV5CSType;
	_CIEXYZTRIPLE bV5Endpoints;
	DWORD  bV5GammaRed;
	DWORD  bV5GammaGreen;
	DWORD  bV5GammaBlue;
	DWORD  bV5Intent;
	DWORD  bV5ProfileData;
	DWORD  bV5ProfileSize;
	DWORD  bV5Reserved;
};

struct BMP_FILEHDR {                     // BMP file header
	char   bfType[2];                    // "BM"
	qint32  bfSize;                      // size of file
	qint16  bfReserved1;
	qint16  bfReserved2;
	qint32  bfOffBits;                   // pointer to the pixmap bits
};

struct BMP_INFOHDR {                     // BMP information header
	qint32  biSize;                      // size of this struct
	qint32  biWidth;                     // pixmap width
	qint32  biHeight;                    // pixmap height
	qint16  biPlanes;                    // should be 1
	qint16  biBitCount;                  // number of bits per pixel
	qint32  biCompression;               // compression method
	qint32  biSizeImage;                 // size of image
	qint32  biXPelsPerMeter;             // horizontal resolution
	qint32  biYPelsPerMeter;             // vertical resolution
	qint32  biClrUsed;                   // number of colors used
	qint32  biClrImportant;              // number of important colors
};


const int BMP_FILEHDR_SIZE = 14;                // size of BMP_FILEHDR data


static void swapPixel01(QImage *image)        // 1-bpp: swap 0 and 1 pixels
{
	int i;
	if (image->depth() == 1 && image->colorCount() == 2) {
		register uint *p = (uint *)image->bits();
		int nbytes = image->byteCount();
		for (i=0; i<nbytes/4; i++) {
			*p = ~*p;
			p++;
		}
		uchar *p2 = (uchar *)p;
		for (i=0; i<(nbytes&3); i++) {
			*p2 = ~*p2;
			p2++;
		}
		QRgb t = image->color(0);                // swap color 0 and 1
		image->setColor(0, image->color(1));
		image->setColor(1, t);
	}
}

/*
static QDataStream &operator>>(QDataStream &s, BMP_FILEHDR &bf)
{                                                // read file header
	s.readRawData(bf.bfType, 2);
	s >> bf.bfSize >> bf.bfReserved1 >> bf.bfReserved2 >> bf.bfOffBits;
	return s;
}

static QDataStream &operator<<(QDataStream &s, const BMP_FILEHDR &bf)
{                                                // write file header
	s.writeRawData(bf.bfType, 2);
	s << bf.bfSize << bf.bfReserved1 << bf.bfReserved2 << bf.bfOffBits;
	return s;
}
*/

static const int BMP_OLD  = 12;					// old Windows/OS2 BMP size
static const int BMP_WIN  = 40;					// new Windows BMP size
static const int BMP_OS2  = 64;					// new OS/2 BMP size

static const int BMP_RGB  = 0;					// no compression
static const int BMP_RLE8 = 1;					// run-length encoded, 8 bits
static const int BMP_RLE4 = 2;					// run-length encoded, 4 bits
static const int BMP_BITFIELDS = 3;				// RGB values encoded in data as bit-fields


static QDataStream &operator>>(QDataStream &s, BMP_INFOHDR &bi)
{
	s >> bi.biSize;
	if(bi.biSize == BMP_WIN || bi.biSize == BMP_OS2)
	{
		s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
		s >> bi.biCompression >> bi.biSizeImage;
		s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
		s >> bi.biClrUsed >> bi.biClrImportant;
	}
	else // probably old Windows format
	{
		qint16 w, h;
		s >> w >> h >> bi.biPlanes >> bi.biBitCount;
		bi.biWidth  = w;
		bi.biHeight = h;
		bi.biCompression = BMP_RGB;                // no compression
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = bi.biYPelsPerMeter = 0;
		bi.biClrUsed = bi.biClrImportant = 0;
	}
	return s;
}

static QDataStream &operator<<(QDataStream &s, const BMP_INFOHDR &bi)
{
	s << bi.biSize;
	s << bi.biWidth << bi.biHeight;
	s << bi.biPlanes;
	s << bi.biBitCount;
	s << bi.biCompression;
	s << bi.biSizeImage;
	s << bi.biXPelsPerMeter << bi.biYPelsPerMeter;
	s << bi.biClrUsed << bi.biClrImportant;
	return s;
}

static int calc_shift(int mask)
{
	int result = 0;
	while (mask && !(mask & 1))
	{
		result++;
		mask >>= 1;
	}
	return result;
}

static bool read_dib_infoheader(QDataStream &s, BMP_INFOHDR &bi)
{
	s >> bi;                                        // read BMP info header
	if (s.status() != QDataStream::Ok)
		return false;

	int nbits = bi.biBitCount;
	int comp = bi.biCompression;
	if (!(nbits == 1 || nbits == 4 || nbits == 8 || nbits == 16 || nbits == 24 || nbits == 32) || bi.biPlanes != 1 || comp > BMP_BITFIELDS)
	{
		return false; // weird BMP image
	}
	if (!(comp == BMP_RGB || (nbits == 4 && comp == BMP_RLE4) || (nbits == 8 && comp == BMP_RLE8) || ((nbits == 16 || nbits == 32) && comp == BMP_BITFIELDS)))
	{
		 return false; // weird compression type
	}
	return true;
}

static bool read_dib_body(QDataStream &s, const BMP_INFOHDR &bi, int offset, int startpos, QImage &image)
{
	QIODevice* d = s.device();
	if (d->atEnd())                                // end of stream/file
		return false;
#if 0
	qDebug("offset...........%d", offset);
	qDebug("startpos.........%d", startpos);
	qDebug("biSize...........%d", bi.biSize);
	qDebug("biWidth..........%d", bi.biWidth);
	qDebug("biHeight.........%d", bi.biHeight);
	qDebug("biPlanes.........%d", bi.biPlanes);
	qDebug("biBitCount.......%d", bi.biBitCount);
	qDebug("biCompression....%d", bi.biCompression);
	qDebug("biSizeImage......%d", bi.biSizeImage);
	qDebug("biXPelsPerMeter..%d", bi.biXPelsPerMeter);
	qDebug("biYPelsPerMeter..%d", bi.biYPelsPerMeter);
	qDebug("biClrUsed........%d", bi.biClrUsed);
	qDebug("biClrImportant...%d", bi.biClrImportant);
#endif
	int w = bi.biWidth,         h = bi.biHeight,  nbits = bi.biBitCount;
	int t = bi.biSize,         comp = bi.biCompression;
	int red_mask = 0;
	int green_mask = 0;
	int blue_mask = 0;
	int red_shift = 0;
	int green_shift = 0;
	int blue_shift = 0;
	int red_scale = 0;
	int green_scale = 0;
	int blue_scale = 0;

	int ncols = 0;
	int depth = 0;
	QImage::Format format;
	switch(nbits)
	{
		case 32:
		case 24:
		case 16:
			depth = 32;
			format = QImage::Format_RGB32;
			break;
		case 8:
		case 4:
			depth = 8;
			format = QImage::Format_Indexed8;
			break;
		default:
			depth = 1;
			format = QImage::Format_Mono;
	}

	if(bi.biHeight < 0)
		h = -h;                  // support images with negative height

	if(image.size() != QSize(w, h) || image.format() != format)
	{
		image = QImage(w, h, format);
		if(image.isNull())                        // could not create image
			return false;
	}

	if (depth != 32) {
		ncols = bi.biClrUsed ? bi.biClrUsed : 1 << nbits;
		image.setColorCount(ncols);
	}

	image.setDotsPerMeterX(bi.biXPelsPerMeter);
	image.setDotsPerMeterY(bi.biYPelsPerMeter);

	if (!d->isSequential())
		d->seek(startpos + BMP_FILEHDR_SIZE + bi.biSize); // goto start of colormap

	if (ncols > 0) {                                // read color table
		uchar rgb[4];
		int   rgb_len = t == BMP_OLD ? 3 : 4;
		for (int i=0; i<ncols; i++) {
			if (d->read((char *)rgb, rgb_len) != rgb_len)
				return false;
			image.setColor(i, qRgb(rgb[2],rgb[1],rgb[0]));
			if (d->atEnd())                        // truncated file
				return false;
		}
	} else if (comp == BMP_BITFIELDS && (nbits == 16 || nbits == 32)) {
		if (d->read((char *)&red_mask, sizeof(red_mask)) != sizeof(red_mask))
			return false;
		if (d->read((char *)&green_mask, sizeof(green_mask)) != sizeof(green_mask))
			return false;
		if (d->read((char *)&blue_mask, sizeof(blue_mask)) != sizeof(blue_mask))
			return false;
		red_shift = calc_shift(red_mask);
		red_scale = 256 / ((red_mask >> red_shift) + 1);
		green_shift = calc_shift(green_mask);
		green_scale = 256 / ((green_mask >> green_shift) + 1);
		blue_shift = calc_shift(blue_mask);
		blue_scale = 256 / ((blue_mask >> blue_shift) + 1);
	} else if (comp == BMP_RGB && (nbits == 24 || nbits == 32)) {
		blue_mask = 0x000000ff;
		green_mask = 0x0000ff00;
		red_mask = 0x00ff0000;
		blue_shift = 0;
		green_shift = 8;
		red_shift = 16;
		blue_scale = green_scale = red_scale = 1;
	} else if (comp == BMP_RGB && nbits == 16) {
		blue_mask = 0x001f;
		green_mask = 0x03e0;
		red_mask = 0x7c00;
		blue_shift = 0;
		green_shift = 2;
		red_shift = 7;
		red_scale = 1;
		green_scale = 1;
		blue_scale = 8;
	}

	// offset can be bogus, be careful
	if (offset>=0 && startpos + offset > d->pos()) {
		if (!d->isSequential())
			d->seek(startpos + offset);                // start of image data
	}

	int             bpl = image.bytesPerLine();
	uchar *data = image.bits();

	if (nbits == 1) {                                // 1 bit BMP image
		while (--h >= 0) {
			if (d->read((char*)(data + h*bpl), bpl) != bpl)
				break;
		}
		if (ncols == 2 && qGray(image.color(0)) < qGray(image.color(1)))
			swapPixel01(&image);                // pixel 0 is white!
	}

	else if (nbits == 4) {                        // 4 bit BMP image
		int    buflen = ((w+7)/8)*4;
		uchar *buf    = new uchar[buflen];
		if (comp == BMP_RLE4) {                // run length compression
			int x=0, y=0, c, i;
			quint8 b;
			register uchar *p = data + (h-1)*bpl;
			const uchar *endp = p + w;
			while (y < h) {
				if (!d->getChar((char *)&b))
					break;
				if (b == 0) {                        // escape code
					if (!d->getChar((char *)&b) || b == 1) {
						y = h;                // exit loop
					} else switch (b) {
						case 0:                        // end of line
							x = 0;
							y++;
							p = data + (h-y-1)*bpl;
							break;
						case 2:                        // delta (jump)
						{
							quint8 tmp;
							d->getChar((char *)&tmp);
							x += tmp;
							d->getChar((char *)&tmp);
							y += tmp;
						}

							// Protection
							if ((uint)x >= (uint)w)
								x = w-1;
							if ((uint)y >= (uint)h)
								y = h-1;

							p = data + (h-y-1)*bpl + x;
							break;
						default:                // absolute mode
							// Protection
							if (p + b > endp)
								b = endp-p;

							i = (c = b)/2;
							while (i--) {
								d->getChar((char *)&b);
								*p++ = b >> 4;
								*p++ = b & 0x0f;
							}
							if (c & 1) {
								unsigned char tmp;
								d->getChar((char *)&tmp);
								*p++ = tmp >> 4;
							}
							if ((((c & 3) + 1) & 2) == 2)
								d->getChar(0);        // align on word boundary
							x += c;
					}
				} else {                        // encoded mode
					// Protection
					if (p + b > endp)
						b = endp-p;

					i = (c = b)/2;
					d->getChar((char *)&b);                // 2 pixels to be repeated
					while (i--) {
						*p++ = b >> 4;
						*p++ = b & 0x0f;
					}
					if (c & 1)
						*p++ = b >> 4;
					x += c;
				}
			}
		} else if (comp == BMP_RGB) {                // no compression
			memset(data, 0, h*bpl);
			while (--h >= 0) {
				if (d->read((char*)buf,buflen) != buflen)
					break;
				register uchar *p = data + h*bpl;
				uchar *b = buf;
				for (int i=0; i<w/2; i++) {        // convert nibbles to bytes
					*p++ = *b >> 4;
					*p++ = *b++ & 0x0f;
				}
				if (w & 1)                        // the last nibble
					*p = *b >> 4;
			}
		}
		delete [] buf;
	}

	else if (nbits == 8) {                        // 8 bit BMP image
		if (comp == BMP_RLE8) {                // run length compression
			int x=0, y=0;
			quint8 b;
			register uchar *p = data + (h-1)*bpl;
			const uchar *endp = p + w;
			while (y < h) {
				if (!d->getChar((char *)&b))
					break;
				if (b == 0) {                        // escape code
					if (!d->getChar((char *)&b) || b == 1) {
							y = h;                // exit loop
					} else switch (b) {
						case 0:                        // end of line
							x = 0;
							y++;
							p = data + (h-y-1)*bpl;
							break;
						case 2:                        // delta (jump)
							// Protection
							if ((uint)x >= (uint)w)
								x = w-1;
							if ((uint)y >= (uint)h)
								y = h-1;

							{
								quint8 tmp;
								d->getChar((char *)&tmp);
								x += tmp;
								d->getChar((char *)&tmp);
								y += tmp;
							}
							p = data + (h-y-1)*bpl + x;
							break;
						default:                // absolute mode
							// Protection
							if (p + b > endp)
								b = endp-p;

							if (d->read((char *)p, b) != b)
								return false;
							if ((b & 1) == 1)
								d->getChar(0);        // align on word boundary
							x += b;
							p += b;
					}
				} else {                        // encoded mode
					// Protection
					if (p + b > endp)
						b = endp-p;

					char tmp;
					d->getChar(&tmp);
					memset(p, tmp, b); // repeat pixel
					x += b;
					p += b;
				}
			}
		} else if (comp == BMP_RGB) {                // uncompressed
			while (--h >= 0) {
				if (d->read((char *)data + h*bpl, bpl) != bpl)
					break;
			}
		}
	}

	else if (nbits == 16 || nbits == 24 || nbits == 32) { // 16,24,32 bit BMP image
		register QRgb *p;
		QRgb  *end;
		uchar *buf24 = new uchar[bpl];
		int    bpl24 = ((w*nbits+31)/32)*4;
		uchar *b;
		int c;

		while (--h >= 0) {
			p = (QRgb *)(data + h*bpl);
			end = p + w;
			if (d->read((char *)buf24,bpl24) != bpl24)
				break;
			b = buf24;
			while (p < end) {
				c = *(uchar*)b | (*(uchar*)(b+1)<<8);
				if (nbits != 16)
					c |= *(uchar*)(b+2)<<16;
				*p++ = qRgb(((c & red_mask) >> red_shift) * red_scale,
										((c & green_mask) >> green_shift) * green_scale,
										((c & blue_mask) >> blue_shift) * blue_scale);
				b += nbits/8;
			}
		}
		delete[] buf24;
	}

	if (bi.biHeight < 0) {
		// Flip the image
		uchar *buf = new uchar[bpl];
		h = -bi.biHeight;
		for (int y = 0; y < h/2; ++y) {
			memcpy(buf, data + y*bpl, bpl);
			memcpy(data + y*bpl, data + (h-y-1)*bpl, bpl);
			memcpy(data + (h-y-1)*bpl, buf, bpl);
		}
		delete [] buf;
	}

	return true;
}
bool qt_read_dib(QDataStream &s, QImage &image)
{
	BMP_INFOHDR bi;
	if (!read_dib_infoheader(s, bi))
		return false;
	return read_dib_body(s, bi, -1, -BMP_FILEHDR_SIZE, image);
}
#if 0
//Supports only 32 bit DIBV5
static bool qt_read_dibv5(QDataStream &s, QImage &image)
{
	BMP_BITMAPV5HEADER bi;
	QIODevice* d = s.device();
	if (d->atEnd())
		return false;

	d->read((char *)&bi, sizeof(bi));   // read BITMAPV5HEADER header
	if (s.status() != QDataStream::Ok)
		return false;

	int nbits = bi.bV5BitCount;
	int comp = bi.bV5Compression;
	if (nbits != 32 || bi.bV5Planes != 1 || comp != BMP_BITFIELDS)
		return false; //Unsupported DIBV5 format

	int w = bi.bV5Width, h = bi.bV5Height;
	int red_mask = bi.bV5RedMask;
	int green_mask = bi.bV5GreenMask;
	int blue_mask = bi.bV5BlueMask;
	int alpha_mask = bi.bV5AlphaMask;
	int red_shift = 0;
	int green_shift = 0;
	int blue_shift = 0;
	int alpha_shift = 0;
	QImage::Format format = QImage::Format_ARGB32;

	if (bi.bV5Height < 0)
		h = -h;     // support images with negative height
	if (image.size() != QSize(w, h) || image.format() != format) {
		image = QImage(w, h, format);
		if (image.isNull())     // could not create image
			return false;
	}
	image.setDotsPerMeterX(bi.bV5XPelsPerMeter);
	image.setDotsPerMeterY(bi.bV5YPelsPerMeter);
	// read color table
	DWORD colorSpace[3];
	if (d->read((char *)colorSpace, sizeof(colorSpace)) != sizeof(colorSpace))
		return false;

	red_shift = calc_shift(red_mask);
	green_shift = calc_shift(green_mask);
	blue_shift = calc_shift(blue_mask);
	if (alpha_mask) {
		alpha_shift = calc_shift(alpha_mask);
	}

	int  bpl = image.bytesPerLine();
	uchar *data = image.bits();
	register QRgb *p;
	QRgb  *end;
	uchar *buf24 = new uchar[bpl];
	int    bpl24 = ((w*nbits+31)/32)*4;
	uchar *b;
	unsigned int c;

	while (--h >= 0) {
		p = (QRgb *)(data + h*bpl);
		end = p + w;
		if (d->read((char *)buf24,bpl24) != bpl24)
			break;
		b = buf24;
		while (p < end) {
			c = *b | (*(b+1))<<8 | (*(b+2))<<16 | (*(b+3))<<24;
			*p++ = qRgba(((c & red_mask) >> red_shift) ,
									((c & green_mask) >> green_shift),
									((c & blue_mask) >> blue_shift),
									((c & alpha_mask) >> alpha_shift));
			b += 4;
		}
	}
	delete[] buf24;

	if (bi.bV5Height < 0) {
		// Flip the image
		uchar *buf = new uchar[bpl];
		h = -bi.bV5Height;
		for (int y = 0; y < h/2; ++y) {
			memcpy(buf, data + y*bpl, bpl);
			memcpy(data + y*bpl, data + (h-y-1)*bpl, bpl);
			memcpy(data + (h-y-1)*bpl, buf, bpl);
		}
		delete [] buf;
	}

	return true;
}

#else

//Supports only 32 bit DIBV5
static bool qt_read_dibv5(QDataStream &s, QImage &image)
{
	BMP_BITMAPV5HEADER bi;
	QIODevice* d = s.device();
	if (d->atEnd())
		return false;

	d->read((char *)&bi, sizeof(bi));   // read BITMAPV5HEADER header
	if (s.status() != QDataStream::Ok)
		return false;

	int nbits = bi.bV5BitCount;
	int comp = bi.bV5Compression;
	if (nbits != 32 || bi.bV5Planes != 1 || !(comp == BMP_BITFIELDS || comp == BMP_RGB))
		return false; //Unsupported DIBV5 format

	int w = bi.bV5Width, h = bi.bV5Height;
	int red_mask = bi.bV5RedMask;
	int green_mask = bi.bV5GreenMask;
	int blue_mask = bi.bV5BlueMask;
	int alpha_mask = bi.bV5AlphaMask;
	int red_shift = 0;
	int green_shift = 0;
	int blue_shift = 0;
	int alpha_shift = 0;
	QImage::Format format = QImage::Format_ARGB32;

	if (bi.bV5Height < 0)
		h = -h;     // support images with negative height
	if (image.size() != QSize(w, h) || image.format() != format) {
		image = QImage(w, h, format);
		if (image.isNull())     // could not create image
			return false;
	}
	image.setDotsPerMeterX(bi.bV5XPelsPerMeter);
	image.setDotsPerMeterY(bi.bV5YPelsPerMeter);
	// read color table
	DWORD colorSpace[3];
	if (d->read((char *)colorSpace, sizeof(colorSpace)) != sizeof(colorSpace))
		return false;

	red_shift = calc_shift(red_mask);
	green_shift = calc_shift(green_mask);
	blue_shift = calc_shift(blue_mask);
	if (alpha_mask) {
		alpha_shift = calc_shift(alpha_mask);
	}
	if(comp == BMP_RGB)
	{
		QRgb *data = (QRgb*)image.bits();
		for(int y = h-1; y>=0; y--)
		{
			for(int x=0; x<w; x++)
			{
				int b,g,r,a;
				d->read((char*)&b, 1);
				d->read((char*)&g, 1);
				d->read((char*)&r, 1);
				d->read((char*)&a, 1);
				data[y*w+x]=qRgba(r,g,b,a);
			}
		}
		return true;
	}

	int  bpl = image.bytesPerLine();
	uchar *data = image.bits();
	register QRgb *p;
	QRgb  *end;
	uchar *buf24 = new uchar[bpl];
	int    bpl24 = ((w*nbits+31)/32)*4;
	uchar *b;
	unsigned int c;

	while (--h >= 0) {
		p = (QRgb *)(data + h*bpl);
		end = p + w;
		if (d->read((char *)buf24,bpl24) != bpl24)
			break;
		b = buf24;
		while (p < end) {
			c = *b | (*(b+1))<<8 | (*(b+2))<<16 | (*(b+3))<<24;
			*p++ = qRgba(((c & red_mask) >> red_shift) ,
									((c & green_mask) >> green_shift),
									((c & blue_mask) >> blue_shift),
									((c & alpha_mask) >> alpha_shift));
			b += 4;
		}
	}
	delete[] buf24;

	if (bi.bV5Height < 0) {
		// Flip the image
		uchar *buf = new uchar[bpl];
		h = -bi.bV5Height;
		for (int y = 0; y < h/2; ++y) {
			memcpy(buf, data + y*bpl, bpl);
			memcpy(data + y*bpl, data + (h-y-1)*bpl, bpl);
			memcpy(data + (h-y-1)*bpl, buf, bpl);
		}
		delete [] buf;
	}

	return true;
}


#endif

bool hasOriginalDIBV5(IDataObject *pDataObj)
{
	bool isSynthesized = true;
	IEnumFORMATETC *pEnum =NULL;
	HRESULT res = pDataObj->EnumFormatEtc(1, &pEnum);
	if (res == S_OK && pEnum) {
		FORMATETC fc;
		while ((res = pEnum->Next(1, &fc, 0)) == S_OK) {
			if (fc.ptd)
				CoTaskMemFree(fc.ptd);
			if (fc.cfFormat == CF_DIB)
				break;
			else if (fc.cfFormat == CF_DIBV5) {
				isSynthesized  = false;
				break;
			}
		}
		pEnum->Release();
	}
	return !isSynthesized;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
HBITMAP CreateBitmapEx(HDC hDC, ULONG nWidth, ULONG nHeight, ULONG nColors)
{
	if(nColors == 2)
	{
		return CreateBitmap(nWidth, nHeight, 1, 1, NULL);
	}
	else
	{
		BITMAPINFOHEADER bmih;

		bmih.biSize				 = sizeof(BITMAPINFOHEADER);
		bmih.biWidth			 = nWidth;
		bmih.biHeight			 = nHeight;
		bmih.biPlanes			 = 1;
		bmih.biBitCount			 = 4;

		bmih.biCompression		 =
		bmih.biSizeImage		 =
		bmih.biXPelsPerMeter	 =
		bmih.biYPelsPerMeter	 =
		bmih.biClrUsed			 =
		bmih.biClrImportant		 = 0;

		return CreateDIBitmap(hDC, &bmih, 0L, NULL, NULL, 0);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant	ClipboardHandlerImage::read(IDataObject* pDataObj, const QString& formatName)
{
	if(cbfmtDIB.canGetData(formatName, pDataObj))
	{
		QByteArray data = cbfmtDIB.getData(pDataObj);
		QImage image;
		QDataStream Stream(&data, QIODevice::ReadOnly);
		Stream.setByteOrder(QDataStream::LittleEndian);// Intel byte order ####
		if(qt_read_dib(Stream, image))			// ##### encaps "-14"
		{
			return image;
		}
	}
	else if(cbfmtDIBV5.canGetData(formatName, pDataObj))
	{
		// Try to convert from a format which has more data
		// DIBV5, use only if its is not synthesized
		//if(hasOriginalDIBV5(pDataObj))
		{
			QByteArray data = cbfmtDIBV5.getData(pDataObj);
			QImage image;
			QDataStream stream(&data, QIODevice::ReadOnly);
			stream.setByteOrder(QDataStream::LittleEndian);
			if(qt_read_dibv5(stream, image))	// #### supports only 32bit DIBV5
			{
				return image;
			}
		}
	}
	else if(cbfmtPNG.canGetData(formatName, pDataObj))
	{
		//PNG, MS Office place this (undocumented)
		QImage image;
		QByteArray data = cbfmtPNG.getData(pDataObj);
		if(image.loadFromData(data, "PNG"))
		{
			return image;
		}
	}
	else if(cbfmtGIF.canGetData(formatName, pDataObj))
	{
		QImage image;
		QByteArray data = cbfmtGIF.getData(pDataObj);
		if(image.loadFromData(data, "GIF"))
		{
			return image;
		}
	}
	else if(cbfmtJFIF.canGetData(formatName, pDataObj))
	{
		QImage image;
		QByteArray data = cbfmtJFIF.getData(pDataObj);
		if(image.loadFromData(data, "JPG"))
		{
			return image;
		}
	}
	else if(formatName == "CF_BITMAP" && CanGetData(CF_BITMAP, pDataObj, TYMED_GDI))
	{
		if(OpenClipboard(0))
		{
			BITMAP bm;
			HBITMAP hBmClip;
			if(NULL != (hBmClip = (HBITMAP)GetClipboardData(CF_BITMAP)))
			{
				GetObject(hBmClip, sizeof(BITMAP), &bm);

				HDC hDC = GetDC(NULL);
				HDC hDCClip = CreateCompatibleDC(hDC);
				HBITMAP hBmClipOld = (HBITMAP)SelectObject(hDCClip, hBmClip);

				HDC hDCTarget = CreateCompatibleDC(hDC);
				HBITMAP hBmTarget = CreateBitmapEx(hDC, bm.bmWidth, bm.bmHeight, 16);
				HBITMAP hBmTargetOld = (HBITMAP)SelectObject(hDCTarget, hBmTarget);

				BitBlt(hDCTarget, 0, 0, bm.bmWidth, bm.bmHeight, hDCClip, 0, 0, SRCCOPY);

				SelectObject(hDCClip, hBmClipOld);
				SelectObject(hDCTarget, hBmTargetOld);
				DeleteDC(hDCClip);
				DeleteDC(hDCTarget);

				ReleaseDC(NULL, hDC);

				CloseClipboard();
#if (QT_VERSION_MAJOR == 4)
				return QPixmap::fromWinHBITMAP(hBmTarget).toImage();
#else
				return QtWin::fromHBITMAP(hBmTarget).toImage();
#endif
			}
			CloseClipboard();
		}
	}
	else
	{
		for(int i = 0; i < mimeImageFormats.size(); i++)
		{
			if(mimeImageFormats.at(i) == formatName)
			{
				QClipboard* cb = QApplication::clipboard();
				const QMimeData*  md = cb->mimeData();
				if(md->hasFormat(formatName))
				{
					QImage image;
					QByteArray ba(md->data(formatName));
					QBuffer buf(&ba);
					QImageReader(&buf).read(&image);
					if(!image.isNull())
					{
						return image;
					}
				}
			}
		}


	}
	return QVariant();
}
bool		ClipboardHandlerImage::canRead(IDataObject* pDataObj, const QString& formatName)
{
	return (   cbfmtDIB.canGetData(formatName, pDataObj)
			|| cbfmtDIBV5.canGetData(formatName, pDataObj)
			|| cbfmtPNG.canGetData(formatName, pDataObj)
			|| cbfmtGIF.canGetData(formatName, pDataObj)
			|| cbfmtJFIF.canGetData(formatName, pDataObj)
			);
	for(int i = 0; i < mimeImageFormats.size(); i++)
	{
		if(mimeImageFormats.at(i) == formatName)
		{
			QClipboard* cb = QApplication::clipboard();
			const QMimeData*  md = cb->mimeData();
			return md->hasFormat(formatName);
		}
	}
	return false;
}
#include "panels/ImagePanel.h"

QWidget*	ClipboardHandlerImage::getWidget(IDataObject* pDataObj, const QString& formatName)
{
	QVariant data = read(pDataObj, formatName);
	if(data.isValid())
	{
#if 0
		QLabel* label = new QLabel(0);
		label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
		label->setPixmap(QPixmap::fromImage(data.value<QImage>()));
		return label;
#else
		ImagePanel* panel = new ImagePanel();
		panel->load(data.value<QImage>());
		return panel;
#endif
	}
	return NULL;
}


