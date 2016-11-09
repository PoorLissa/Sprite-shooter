//---------------------------------------------------------------------------

#ifndef UDibSpriteH
#define UDibSpriteH

#include <vcl.h>
#include <vector.h>

// ----------------------------------------------------------------------------
// --- ����� ������� ��� ������ �� ��������� � ������� DIB --------------------
// ----------------------------------------------------------------------------
/*
 ������� �������� �� ��� �������������� �����������, ��� ���� ������������
 ����������������� ���� ������� ��������� ���������� � �� ����������� ����� ���
*/ 
// ----------------------------------------------------------------------------

// how to rotate DIB-bitmap
// http://www.ucancode.net/Visual_C_MFC_Samples/Rotate-bitmap-dib-image-file-vc-example.htm
// http://www.drdobbs.com/architecture-and-design/fast-bitmap-rotation-and-scaling/184416337

// '����������' ���� ������� - RGB(255, 100, 255) - ������� ����� ����� �� ��������� �� �������� �����������
#define TransparentColor 231



// ----------------------------------------------------------------------------
// -------------- ������� ����� ��� �������� � ������� ���� -------------------
// ----------------------------------------------------------------------------

class DibSpriteBase {

 public:
    __fastcall  DibSpriteBase(HDC, AnsiString);          // ������������� bmp-������
    __fastcall  DibSpriteBase(HDC, Graphics::TBitmap *); // ������������� TBitmap*
    __fastcall ~DibSpriteBase();

 private:
    void __fastcall init(HDC);
    virtual void __fastcall virt() = 0; // ������ ����� �����������

 protected:
    Graphics::TBitmap *_Bitmap;         // �������� �����������
    unsigned char     *_BitmapBits;     // ������� ����� ����������� � ������� BID
    unsigned int       _InfoSize,
                       _ImageSize;
    BITMAPINFO        *_BitmapInfo;
};



// ----------------------------------------------------------------------------
// --- ����� ��� ������� ���� -------------------------------------------------
// ----------------------------------------------------------------------------

class DibBack : public DibSpriteBase {

 // �����, ����� ������ ��� ���������� ��� ���� �� ��� ����� ��������� �� ���� ���
 friend class DibSprite;

 public:
    __fastcall  DibBack(HDC, AnsiString);           // ������������� bmp-������
    __fastcall  DibBack(HDC, Graphics::TBitmap *);  // ������������� TBitmap*
    __fastcall ~DibBack();

    // ������������ ����� ���
    // �������� ���� �����, �.�. �� ������-�� �������� �� ����� ����������
    void       __fastcall drawBackground(HDC, bool = true);
    inline int __fastcall getWidth();
    inline int __fastcall getHeight();

 private:
    void __fastcall init();
    virtual void __fastcall virt() {}

 private:
    unsigned char *_BitmapBitsClean;    // ������ ����� ������� ����
};



// ----------------------------------------------------------------------------
// --- ����� ��� �������� -----------------------------------------------------
// ----------------------------------------------------------------------------

class DibSprite : public DibSpriteBase {

 public:
    __fastcall DibSprite(HDC, AnsiString);
    __fastcall DibSprite(HDC, Graphics::TBitmap *);

    // ������ � ������ ��� ��������� x/y
    __property int X = { read = _x, write = _x };
    __property int Y = { read = _y, write = _y };

    // ������������ ������� ������ �� DIB-������� BackBits, ��������� ��������� �������.
    // ��� ������� ����� �������� �� �� ����� ������, � �� ��� ��������:
    // ��� ���� ��� ��������� ��������� ���������� � ���������� ���������
    void __fastcall drawSprite(DibBack *, int, int, bool = false);

 private:
    virtual void __fastcall virt() {} 

 private:
    int _x, _y;     // ���������� ������� �� ������� �����������
};



// ----------------------------------------------------------------------------
// --- ����� ��� ��������� ������ ---------------------------------------------
// ----------------------------------------------------------------------------

class DibCollection {

 typedef unsigned int uint;

 public:

    enum AnimationStyle { FORWARD = 0, BACKWARD, FORWARD_ONCE, BACKWARD_ONCE, THERE_AND_BACK };

    __fastcall  DibCollection(HDC, AnsiString);
    __fastcall ~DibCollection();

    // ������������ ����� � �������� ������� �� ��������� ��� �� ��������� �����������
    inline void __fastcall Draw(DibBack*, int, int, int);
    // �������� ����� ����������� �������    
    inline uint __fastcall getFramesQty();

 protected:
    vector<DibSprite *> _vec;           // ������ � ��������
    unsigned short      _FramesQty;     // ����� ����� ����������� �������
};

// ���������� ������ ���������� � ���� �� �����!
// ������������ ����� � �������� ������� �� ��������� ��� �� ��������� �����������
inline void __fastcall DibCollection::Draw(DibBack* Back, int frameNo, int X, int Y) {
    if( frameNo >= 0 && frameNo < _FramesQty )
        _vec[frameNo]->drawSprite(Back, X, Y);
}

inline unsigned int __fastcall DibCollection::getFramesQty() {
    return _FramesQty;
}



// ----------------------------------------------------------------------------
// --- ����� ��� �������� -----------------------------------------------------
// ----------------------------------------------------------------------------

class DibAnimation : public DibCollection {

#define defaultStep 1

 public:

    __fastcall  DibAnimation(HDC, AnsiString, int x, int y,
                                unsigned short = FORWARD, bool = true, bool = false);
    __fastcall ~DibAnimation();

    // ������ �� ��������� �����
           void __fastcall Animate();
    inline void __fastcall Draw(DibBack*);          // ������������ ������� ����� �� ���

    // ������/������ ��� �������� Active
    __property bool Active = { read = _Active, write = _Active };

    __property int X = { read = _X, write = _X };
    __property int Y = { read = _Y, write = _Y };

 private:
    bool             _Active;        // ����� �� ������������ �������� ������
    int              _X, _Y;         // ���������� ��� ������ ������
             short   _CurrentFrame;  // ����� �������� ������
    unsigned short   _Step;          // ��� ��������
    unsigned short   _aniStyle;      // ����� �������� (������, �����, �� �����, ����-����)
};

// ������������ ������� ����� �� ���
inline void __fastcall DibAnimation::Draw(DibBack* Back) {
    _vec[_CurrentFrame]->drawSprite(Back, _X, _Y);
}

//---------------------------------------------------------------------------
#endif
