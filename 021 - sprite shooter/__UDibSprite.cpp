//---------------------------------------------------------------------------

#pragma hdrstop
#include "__UDibSprite.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// --- DibSpriteBase ----------------------------------------------------------

// ����������� ��� bmp-�����
__fastcall DibSpriteBase::DibSpriteBase(HDC handle, AnsiString fileName) {
    _Bitmap = new Graphics::TBitmap();
    _Bitmap->LoadFromFile(fileName);

    init(handle);
}

// ����������� ��� TBitmap
__fastcall DibSpriteBase::DibSpriteBase(HDC handle, Graphics::TBitmap *bitmap)
    : _Bitmap(bitmap) {
    init(handle);
}

__fastcall DibSpriteBase::~DibSpriteBase() {
    delete [] _BitmapInfo;
    delete [] _BitmapBits;
    delete _Bitmap;
}

void __fastcall DibSpriteBase::init(HDC handle) {

    _Bitmap->PixelFormat = pf8bit;

    GetDIBSizes(_Bitmap->Handle, _InfoSize, _ImageSize);
    _BitmapInfo = (BITMAPINFO *) new char[_InfoSize];
    memset(_BitmapInfo, 0, _InfoSize);

    _BitmapInfo->bmiHeader.biWidth       =  _Bitmap->Width;
    _BitmapInfo->bmiHeader.biHeight      = -_Bitmap->Height;
    _BitmapInfo->bmiHeader.biPlanes      =  1;
    _BitmapInfo->bmiHeader.biBitCount    =  8;
    _BitmapInfo->bmiHeader.biCompression =  BI_RGB;
//  _BitmapInfo->bmiHeader.biCompression =  BI_BITFIELDS;     // for 16-bit and more
    _BitmapInfo->bmiHeader.biSize        =  sizeof(BITMAPINFOHEADER);

    // ������� ������ ��� DIB-������� � �������� ���������
    _BitmapBits = new unsigned char[_ImageSize];

    // ������������� ����������� � DIB-�������
    GetDIBits(handle,
        _Bitmap->Handle, 0, _Bitmap->Height,
        _BitmapBits, _BitmapInfo, DIB_RGB_COLORS);
}



// --- DibBack ----------------------------------------------------------------

__fastcall DibBack::DibBack(HDC handle, AnsiString fileName)
    : DibSpriteBase(handle, fileName) {
    init();
}

__fastcall DibBack::DibBack(HDC handle, Graphics::TBitmap *bitmap)
    : DibSpriteBase(handle, bitmap) {
    init();
}

__fastcall DibBack::~DibBack() {
    delete[] _BitmapBitsClean;
}

// �������� ���� �����, �.�. �� ������-�� �������� �� ����� ����������
void __fastcall DibBack::drawBackground(HDC Handle, bool doClear) {

    SetDIBitsToDevice(Handle,   // handle of device context
                           0,   // x-coordinate of upper-left corner of dest. rect.
                           0,   // y-coordinate of upper-left corner of dest. rect.
              _Bitmap->Width,   // source rectangle width
             _Bitmap->Height,   // source rectangle height
                           0,   // x-coordinate of lower-left corner of source rect.
                           0,   // y-coordinate of lower-left corner of source rect.
                           0,   // first scan line in array
             _Bitmap->Height,   // number of scan lines
                 _BitmapBits,   // address of array with DIB bits
                 _BitmapInfo,   // address of structure with bitmap info.
              DIB_RGB_COLORS    // RGB or palette indices
    );

    // ����� ����� ��������� ������� ������� ������, ������� ��� ������
    if( doClear )
        memcpy(_BitmapBits, _BitmapBitsClean, _ImageSize);
}

int __fastcall DibBack::getWidth() {
    return _Bitmap->Width;
}

int __fastcall DibBack::getHeight() {
    return _Bitmap->Height;
}

void __fastcall DibBack::init() {
    _BitmapBitsClean = new unsigned char[_ImageSize];

    // �������� ������ ��� � ������� DIB � ��������� ������
    for(unsigned int i = 0; i < _ImageSize; i++)
        _BitmapBitsClean[i] = _BitmapBits[i];
}



// --- DibSprite --------------------------------------------------------------

__fastcall DibSprite::DibSprite(HDC handle, AnsiString fileName)
    : DibSpriteBase(handle, fileName) {
}

__fastcall DibSprite::DibSprite(HDC handle, Graphics::TBitmap *bitmap)
    : DibSpriteBase(handle, bitmap) {
}

void __fastcall DibSprite::drawSprite(DibBack *Back, int X, int Y, bool drawOnCleanCopy) {

    // ��� ������� ��� ��������� ����� �� ������������
    unsigned char spriteBit = 0;
    // ��������, ���� �������� - �� ����� ������ ��� �� ��� ��������
    unsigned char *Bits = drawOnCleanCopy ? Back->_BitmapBitsClean : Back->_BitmapBits;
    int posX, posY, Line,
        spriteWidth = _Bitmap->Width,
        backWidth   = Back->_Bitmap->Width,
        backHeight  = Back->_Bitmap->Height;

    for (int y = 0; y < _Bitmap->Height; y++) {

        Line = y * spriteWidth;

        for (int x = 0; x < spriteWidth; x++) {

            spriteBit = _BitmapBits[Line + x];

            // ���� ���� ������� �� ����� 231 (RGB(255, 100, 255)), �� ������������ ���
            if( (int)spriteBit != TransparentColor ) {
                posX = X + x;
                posY = Y + y;
                if( posX > 0 && posX < backWidth && posY > 0 && posY < backHeight )
                    Bits[posY * backWidth + posX] = spriteBit;
            }
        }
    }
}



// --- DibCollection ----------------------------------------------------------

__fastcall DibCollection::DibCollection(HDC handle, AnsiString fileMask)
    : _FramesQty(0) {

    // ������ ����� �� ��������� ������� �� ��������� �����
    while( FileExists( ExtractFileDir(Application->ExeName) + "\\" + fileMask + IntToStr(_FramesQty) + ".bmp" ) ) {

        DibSprite *ptr = new DibSprite(handle, fileMask + IntToStr(_FramesQty) + ".bmp");
        _vec.push_back(ptr);
        _FramesQty++;
    }
}

__fastcall DibCollection::~DibCollection() {
    if( _vec.size() )
        for(unsigned int i = 0; i < _vec.size(); i++)
            delete _vec[i];
}



// --- DibAnimation -----------------------------------------------------------

__fastcall DibAnimation::DibAnimation(HDC handle, AnsiString fileMask, int x, int y,
    unsigned short aniStyle, bool active, bool randomPhase)
        : DibCollection(handle, fileMask),
         _X(x), _Y(y), _CurrentFrame(0), _Step(defaultStep), _Active(active), _aniStyle(aniStyle) {

    if( aniStyle == BACKWARD || aniStyle == BACKWARD_ONCE )
        _Step *= -1;

    // ������ �������� � ��������� ����
    if( randomPhase ) {
        randomize();
        _CurrentFrame = random(_FramesQty);
    }
}

__fastcall DibAnimation::~DibAnimation() {
    _Active = false;
}

void __fastcall DibAnimation::Animate() {

    if( _Active ) {

        _CurrentFrame += _Step;

        if( _CurrentFrame == _FramesQty || _CurrentFrame == -1 ) {

            switch( _aniStyle ) {

                case FORWARD:
                    _CurrentFrame = 0;
                    break;
                case BACKWARD:
                    _CurrentFrame = _FramesQty - 1;
                    break;
                case FORWARD_ONCE:
                case BACKWARD_ONCE:
                    _Active = false;
                    break;
                case THERE_AND_BACK:
                    _Step *= -1;
                    _CurrentFrame = (_CurrentFrame == _FramesQty) ? _FramesQty - 2 : 1;
                    break;
                default:
                    _Active = false;
            }
        }
    }
}



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
