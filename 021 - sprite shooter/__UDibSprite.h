//---------------------------------------------------------------------------

#ifndef UDibSpriteH
#define UDibSpriteH

#include <vcl.h>
#include <vector.h>

// ----------------------------------------------------------------------------
// --- Набор классов для работы со спрайтами в формате DIB --------------------
// ----------------------------------------------------------------------------
/*
 Перенос спрайтов на фон осуществляется попиксельно, при этом определенный
 предустановленный цвет спрайта считается прозрачным и не перекрывает собой фон
*/ 
// ----------------------------------------------------------------------------

// how to rotate DIB-bitmap
// http://www.ucancode.net/Visual_C_MFC_Samples/Rotate-bitmap-dib-image-file-vc-example.htm
// http://www.drdobbs.com/architecture-and-design/fast-bitmap-rotation-and-scaling/184416337

// 'Прозрачный' цвет спрайта - RGB(255, 100, 255) - пикселы этого цвета не выводятся на конечное изображение
#define TransparentColor 231



// ----------------------------------------------------------------------------
// -------------- базовый класс для спрайтов и заднего фона -------------------
// ----------------------------------------------------------------------------

class DibSpriteBase {

 public:
    __fastcall  DibSpriteBase(HDC, AnsiString);          // инициализация bmp-файлом
    __fastcall  DibSpriteBase(HDC, Graphics::TBitmap *); // инициализация TBitmap*
    __fastcall ~DibSpriteBase();

 private:
    void __fastcall init(HDC);
    virtual void __fastcall virt() = 0; // делаем класс абстрактным

 protected:
    Graphics::TBitmap *_Bitmap;         // исходное изображение
    unsigned char     *_BitmapBits;     // битовый буфер изображения в формате BID
    unsigned int       _InfoSize,
                       _ImageSize;
    BITMAPINFO        *_BitmapInfo;
};



// ----------------------------------------------------------------------------
// --- класс для заднего фона -------------------------------------------------
// ----------------------------------------------------------------------------

class DibBack : public DibSpriteBase {

 // хотим, чтобы спрайт мог отрисовать сам себя на фон через указатель на этот фон
 friend class DibSprite;

 public:
    __fastcall  DibBack(HDC, AnsiString);           // инициализация bmp-файлом
    __fastcall  DibBack(HDC, Graphics::TBitmap *);  // инициализация TBitmap*
    __fastcall ~DibBack();

    // отрисовываем заний фон
    // передаем сюда хэндл, т.к. он почему-то меняется во время выполнения
    void       __fastcall drawBackground(HDC, bool = true);
    inline int __fastcall getWidth();
    inline int __fastcall getHeight();

 private:
    void __fastcall init();
    virtual void __fastcall virt() {}

 private:
    unsigned char *_BitmapBitsClean;    // чистая копия заднего фона
};



// ----------------------------------------------------------------------------
// --- класс для спрайтов -----------------------------------------------------
// ----------------------------------------------------------------------------

class DibSprite : public DibSpriteBase {

 public:
    __fastcall DibSprite(HDC, AnsiString);
    __fastcall DibSprite(HDC, Graphics::TBitmap *);

    // геттер и сеттер для координат x/y
    __property int X = { read = _x, write = _x };
    __property int Y = { read = _y, write = _y };

    // Отрисовываем текущий спрайт на DIB-спрайте BackBits, пропуская невидимые пиксели.
    // При желании можем рисовать не на копию буфера, а на его оригинал:
    // при этом все внесенные изменения сохранятся в дальнейших итерациях
    void __fastcall drawSprite(DibBack *, int, int, bool = false);

 private:
    virtual void __fastcall virt() {} 

 private:
    int _x, _y;     // координаты спрайта на фоновом изображении
};



// ----------------------------------------------------------------------------
// --- класс для коллекции кадров ---------------------------------------------
// ----------------------------------------------------------------------------

class DibCollection {

 typedef unsigned int uint;

 public:

    enum AnimationStyle { FORWARD = 0, BACKWARD, FORWARD_ONCE, BACKWARD_ONCE, THERE_AND_BACK };

    __fastcall  DibCollection(HDC, AnsiString);
    __fastcall ~DibCollection();

    // отрисовываем фрейм с заданным номером на указанный фон по указанным координатам
    inline void __fastcall Draw(DibBack*, int, int, int);
    // получить число загруженных фреймов    
    inline uint __fastcall getFramesQty();

 protected:
    vector<DibSprite *> _vec;           // вектор с фреймами
    unsigned short      _FramesQty;     // общее число загруженных фреймов
};

// инлайновые методы определяем в этом же файле!
// отрисовываем фрейм с заданным номером на указанный фон по указанным координатам
inline void __fastcall DibCollection::Draw(DibBack* Back, int frameNo, int X, int Y) {
    if( frameNo >= 0 && frameNo < _FramesQty )
        _vec[frameNo]->drawSprite(Back, X, Y);
}

inline unsigned int __fastcall DibCollection::getFramesQty() {
    return _FramesQty;
}



// ----------------------------------------------------------------------------
// --- класс для анимации -----------------------------------------------------
// ----------------------------------------------------------------------------

class DibAnimation : public DibCollection {

#define defaultStep 1

 public:

    __fastcall  DibAnimation(HDC, AnsiString, int x, int y,
                                unsigned short = FORWARD, bool = true, bool = false);
    __fastcall ~DibAnimation();

    // встаем на следующий фрейм
           void __fastcall Animate();
    inline void __fastcall Draw(DibBack*);          // отрисовываем текущий фрейм на фон

    // геттер/сеттер для свойства Active
    __property bool Active = { read = _Active, write = _Active };

    __property int X = { read = _X, write = _X };
    __property int Y = { read = _Y, write = _Y };

 private:
    bool             _Active;        // нужно ли отрисовывать анимацию дальше
    int              _X, _Y;         // координаты для вывода фрейма
             short   _CurrentFrame;  // номер текущего фрейма
    unsigned short   _Step;          // шаг анимации
    unsigned short   _aniStyle;      // стиль анимации (вперед, назад, по кругу, туда-сюда)
};

// отрисовываем текущий фрейм на фон
inline void __fastcall DibAnimation::Draw(DibBack* Back) {
    _vec[_CurrentFrame]->drawSprite(Back, _X, _Y);
}

//---------------------------------------------------------------------------
#endif
