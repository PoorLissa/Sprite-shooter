//---------------------------------------------------------------------------

#include <vcl.h>
#include <math.h>
#pragma hdrstop

#include "Unit1.h"
#include "__UDibSprite.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

class DibMonster {

 friend class DibBullet;

 public:
    // генерим монстра в заданной позиции
    DibMonster(DibSprite *sprite, DibBack *back, DibCollection *anim, int x, int y)
        : Sprite(sprite), _Anim(anim), Back(back), _X(x), _Y(y), _Alive(true) {
    }
    // генерим монстра в случайной позиции за пределами кадра
    DibMonster(DibSprite *sprite, DibBack *back, DibCollection *anim, int Qty)
        : Sprite(sprite), Back(back), _Alive(true), _Anim(anim) {

        if( random(2) ) {
            _X = random(Form1->Width + 100) - 50;
            _Y = random(2) ? -50 : Form1->Height + 50;
        }
        else {
            _X = random(2) ? -50 : Form1->Width + 50;
            _Y = random(Form1->Height + 100) - 50;
        }

        // добавляем скорости монстру в зависимости от их общего кол-ва
        _Speed = double(random(20)+1 + double(Qty)) / 100;
        
        _CurrentFrame = 0;
        _Count = 0;
        _Step = 1;
    }

   ~DibMonster() {}

    inline void __fastcall Draw() {
        Sprite->drawSprite(Back, _X, _Y);
    }

    inline void __fastcall Draw(int aaa) {

        if( _CurrentFrame == FramesQty || _CurrentFrame == -1 ) {
            _Step *= -1;
            _CurrentFrame = (_CurrentFrame == FramesQty) ? FramesQty - 2 : 0;
        }

        _Anim->Draw(Back, _CurrentFrame, _X, _Y);
        _Count++;

        if( _Count > 1 ) {
            _CurrentFrame += _Step;
            _Count = 0;
        }
    }

    inline bool __fastcall isAlive() {
        return _Alive;
    }

    inline int __fastcall getX() {
        return _X;
    }

    inline int __fastcall getY() {
        return _Y;
    }

    void __fastcall Move(double x, double y) {

        double dX = x - _X;
        double dY = y - _Y;
        double Speed_Divided_By_Dist = _Speed / sqrt(dX*dX + dY*dY);

        dX = Speed_Divided_By_Dist * dX;
        dY = Speed_Divided_By_Dist * dY;

        _X += dX;
        _Y += dY;
    }

 private:
    DibSprite *Sprite;
    DibBack   *Back;
    double    _X, _Y, _Speed;
    bool      _Alive;

    DibCollection *_Anim;
    int            _Count, _CurrentFrame, _Step;

 public:
    static unsigned int FramesQty;
};


class DibBullet {
 public:
    DibBullet(DibSprite* spr, DibBack *back, int x0, int y0, int X, int Y, float speed = 1.0)
        : Sprite(spr), Back(back), _x(x0), _y(y0), _X(X), _Y(Y), Speed(speed), explosionCounter(0), MonsterIsHit(false) {

        // вычислим dX и dY:
/*
        double Dist = sqrt((_x - X)*(_x - X) + (_y - Y)*(_y - Y));
        dX = Speed * (X - _x) / Dist;
        dY = Speed * (Y - _y) / Dist;
*/
        // немного ускоряем вычисление
        dX = X - _x;
        dY = Y - _y;
        double Speed_Divided_By_Dist = Speed / sqrt(dX*dX + dY*dY);

        // заодно, пользуясь полученными dX и dY, вычислим угол, под которым выпущена пуля по отношению к стрелку
/*
        if( dX == 0.0 ) {
            Angle = dY > 0 ? 180.0 : 0.0;
        }
        else {
            Angle = 180 * atan(dY/dX) / M_PI;
            Angle = dX > 0.0 ? Angle + 90.0 : Angle + 270.0;
        }
*/
        // угол вычислен, вычисляем dX и dY:
        dX = Speed_Divided_By_Dist * dX;
        dY = Speed_Divided_By_Dist * dY;

        Collision = (_X < _x);
    }

   ~DibBullet() {
        // спрайт и фон не удаляем, т.к. они реюзабле, их удалит основная программа
        // delete Sprite;
        // delete Back;
    }

    void Show() {
        Sprite->drawSprite(Back, _x, _y);

        if( explosionCounter == 1 ) {

            Graphics::TBitmap *b = new Graphics::TBitmap();
            b->Width  = 24;
            b->Height = 24;

            b->Canvas->Brush->Color = TColor(RGB(255, 100, 255));
            b->Canvas->FillRect(Rect(0, 0, 24, 24));

            for(int i = 0; i < 5 + random(5); i++)
                b->Canvas->Pixels[random(24)][random(24)] = clBlack;

            if( !MonsterIsHit ) {
                b->Canvas->Brush->Color = clGray;
                b->Canvas->Ellipse(Rect(9, 9, 15, 15));
                b->Canvas->Brush->Color = clLtGray;
                b->Canvas->Ellipse(Rect(10, 10, 14, 14));
            }

            // оставим след взрыва на главной копии фона
            DibSprite s(Form1->Canvas->Handle, b);
            s.drawSprite(Back, _x, _y, true);
        }
    }

    // просчитываем движение пули, столкновение ее с монстром или конец траектории
    // возвращаем ноль, если столкновения не происходит, или счетчик анимации взрыва, если столкновение произошло
    int Move(vector<DibMonster*> &vecMon) {

        if( !explosionCounter ) {
            for(unsigned int i = 0; i < vecMon.size(); i++) {

                if( commonSectionCircle(_x, _y, _x + dX, _y + dY, vecMon[i]->getX() + 24, vecMon[i]->getY() + 24, 24) ) {
                    MonsterIsHit = true;
                    vecMon[i]->_Alive = false;
                    _X = vecMon[i]->getX()+12;
                    _Y = vecMon[i]->getY()+12;                    
                    return ++explosionCounter;
                }
            }
        }

        _x += dX;
        _y += dY;

        // ??? нужно проверять еще и по игреку, а то по вертикали пули уходят в вечность
        // проверяем, достигли ли цели
        if( (_X < _x) != Collision || explosionCounter ) {
            _x = _X;
            _y = _Y;
            dX = dY = 0.0;
            return ++explosionCounter;
        }

        return 0;
    }

    inline int getX() { return _X; }
    inline int getY() { return _Y; }

    void setSprite(DibSprite *spr) {
        Sprite = spr;
    }

 private:
 
    // пересечение отрезка с окружностью ( http://www.cyberforum.ru/cpp-beginners/thread853799.html )
    bool commonSectionCircle (double x1, double y1, double x2, double y2, double xC, double yC, double R) {

        x1 -= xC;
        y1 -= yC;
        x2 -= xC;
        y2 -= yC;

        double dx = x2 - x1;
        double dy = y2 - y1;

        // составляем коэффициенты квадратного уравнения на пересечение прямой и окружности.
        // если на отрезке [0..1] есть отрицательные значения, значит отрезок пересекает окружность
        double a = dx*dx + dy*dy;
        double b = 2.0 * (x1*dx + y1*dy);
        double c = x1*x1 + y1*y1 - R*R;

        // а теперь проверяем, есть ли на отрезке [0..1] решения
        if (-b < 0)
            return (c < 0);
        if (-b < (2.0 * a))
            return ((4.0 * a*c - b*b) < 0);

        return (a+b+c < 0);
    }

 private:
    DibSprite *Sprite;      // спрайт для пули
    DibBack   *Back;
    double    _x, _y,       // текущая координата нашей пули
              _X, _Y,       // конечная точка, в которую пуля летит
               dX, dY;      // смещения по x и по y для нахождения новой позиции пули
    int        explosionCounter;
    float      Speed;
    bool       Collision;   // определяем знак (X < X0) и потом, как только знак этого выражения меняется, понимаем, что достигли цели
    bool       MonsterIsHit;
};

class DibTank {

#define TankSpeed   5

 public:
    DibTank(DibSprite* spr, DibSprite* tower, DibBack *back, int X, int Y)
        : TankSprite(spr), TowerSprite(tower), Back(back), _X(X), _Y(Y),
         _LEFT(false), _RIGHT(false), _UP(false), _DOWN(false) {
    }
   ~DibTank() {
        delete TankSprite;
        delete TowerSprite;
    }

    void __fastcall Show() {
        TankSprite->drawSprite(Back, _X, _Y);
        TowerSprite->drawSprite(Back, _X, _Y);
    }

    void __fastcall Move() {

        Step = TankSpeed;

        // в случае нажатия двух клавиш уменьшаем шаг в sqrt(2) раз, чтобы компенсировать сложение двух векторов движения
        if( (_UP && _LEFT) || (_UP && _RIGHT) || (_DOWN && _LEFT) || (_DOWN && _RIGHT) )
            Step /= 1.414214;

        if( DOWN )  _Y += Step;
        if( LEFT )  _X -= Step;
        if( UP )    _Y -= Step;
        if( RIGHT ) _X += Step;
    }

    // по событию нажатия/отпускания клавиши выставляем эти свойства в true/false
    __property bool LEFT  = { read = _LEFT,  write = _LEFT  };
    __property bool RIGHT = { read = _RIGHT, write = _RIGHT };
    __property bool UP    = { read = _UP,    write = _UP    };
    __property bool DOWN  = { read = _DOWN,  write = _DOWN  };

    inline int __fastcall getX() {
        return _X;
    }
    
    inline int __fastcall getY() {
        return _Y;
    }

 private:
    DibSprite *TankSprite, *TowerSprite;
    DibBack   *Back;
    double    _X, _Y, Step;
    bool      _LEFT, _RIGHT, _UP, _DOWN;
};

class DibExplosion {
 public:
    DibExplosion(DibCollection *frames, int x, int y)
        : _Frames(frames), _X(x), _Y(y), _Active(true), _Counter(0) {
    }
   ~DibExplosion() {
    }

    void Draw(DibBack *Back) {
        if( _Counter < FramesQty ) {
            _Frames->Draw(Back, _Counter, _X, _Y);
            _Cnt++;

            if( _Cnt > 10 ) {
                _Counter++;
                _Cnt = 0;
            }
        }
        else
            _Active = false;
    }

    inline bool isActive() {
        return _Active;
    }

 private:
    DibCollection *_Frames;
    int            _X, _Y;
    unsigned int   _Counter, _Cnt;
    bool           _Active;
    
 public:
    static unsigned int FramesQty;
};



#define BulletSpeed 50.0
DibBack       *Back;
DibSprite     *BulletSprite, *Explosion, *TankSprite, *TankTower, *Monster;
DibTank       *Tank;
DibCollection *AnimateExpl, *AnimateMnstr;
unsigned int DibExplosion::FramesQty = 0;
unsigned int DibMonster::FramesQty = 0;

int     playerPosX, playerPosY;
vector  <DibBullet *>    vec;
vector  <DibExplosion *> vecExpl;
vector  <DibMonster *>   vecMon;
bool    LMB_Pressed = false;
int     LMB_Counter = 0;
int     MouseX, MouseY;

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
    playerPosX = Form1->Width  / 2;
    playerPosY = Form1->Height / 2;

    Back         = new DibBack(Form1->Canvas->Handle, "_Bgr.bmp");
    BulletSprite = new DibSprite(Form1->Canvas->Handle, "_Bullet.bmp");
    Explosion    = new DibSprite(Form1->Canvas->Handle, "_explosion.bmp");
    TankSprite   = new DibSprite(Form1->Canvas->Handle, "_tank.bmp");
    TankTower    = new DibSprite(Form1->Canvas->Handle, "_tankTower.bmp");
    Monster      = new DibSprite(Form1->Canvas->Handle, "_monster.bmp");
    Tank         = new DibTank(TankSprite, TankTower, Back, playerPosX, playerPosY);
    AnimateExpl  = new DibCollection(Form1->Canvas->Handle, "aaa\\");
    AnimateMnstr = new DibCollection(Form1->Canvas->Handle, "bbb\\");
    DibExplosion::FramesQty = AnimateExpl ->getFramesQty();
    DibMonster  ::FramesQty = AnimateMnstr->getFramesQty();

    randomize();
    for(int i = 0; i < 33; i++)
        //vecMon.push_back(new DibMonster(Monster, Back, AnimateMnstr, vecMon.size()));
        vecMon.push_back(new DibMonster(Monster, Back, AnimateMnstr, 200));
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormDestroy(TObject *Sender)
{
    if( vec.size() )
        for(unsigned int i = 0; i < vec.size(); i++)
            if(vec[i] != NULL)
                delete vec[i];

    if( vecExpl.size() )
        for(unsigned int i = 0; i < vecExpl.size(); i++)
            if(vecExpl[i] != NULL)
                delete vecExpl[i];

    if( vecMon.size() )
        for(unsigned int i = 0; i < vecMon.size(); i++)
            if(vecMon[i] != NULL)
                delete vecMon[i];

    delete Back;
    delete BulletSprite;
    delete Explosion;
    delete Tank;
    delete AnimateExpl;
    delete AnimateMnstr;
}
//---------------------------------------------------------------------------

// onTimer Event
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
    Tank->Move();
    Tank->Show();

    playerPosX = Tank->getX();
    playerPosY = Tank->getY();

    int Hit;

    // вектор с пулями
    for(unsigned int i = 0; i < vec.size(); i++) {
        Hit = vec[i]->Move(vecMon);
        vec[i]->Show();

        // дошли до точки, куда стреляли
        if( Hit ) {

            if( Hit == 1) {
                vecExpl.push_back(new DibExplosion(AnimateExpl, vec[i]->getX(), vec[i]->getY()));

                DibBullet *ptr = vec[i];
                vec.erase(&vec[i]);
                delete ptr;
            }
        }
    }

    // вектор со взрывами
    for(unsigned int i = 0; i < vecExpl.size(); i++) {

        if( vecExpl[i]->isActive() )
            vecExpl[i]->Draw(Back);
        else {
            DibExplosion *ptr = vecExpl[i];
            vecExpl.erase(&vecExpl[i]);
            delete ptr;
        }
    }

    // вектор с монстрами
    for(unsigned int i = 0; i < vecMon.size(); i++) {

        if( vecMon[i]->isAlive() ) {
            vecMon[i]->Move(playerPosX, playerPosY);
            vecMon[i]->Draw(0);
        }
        else {

            DibMonster *ptr = vecMon[i];
            vecMon.erase(&vecMon[i]);
            delete ptr;

            vecMon.push_back(new DibMonster(Monster, Back, AnimateMnstr, vecMon.size()));
            vecMon.push_back(new DibMonster(Monster, Back, AnimateMnstr, vecMon.size()));
        }
    }

    if( LMB_Pressed ) {

        LMB_Counter++;

        if( LMB_Counter * Form1->Timer1->Interval > 1 )
        {
            DibBullet *b = new DibBullet(BulletSprite, Back, playerPosX, playerPosY, MouseX + random(47) - 23, MouseY + random(47) - 23, BulletSpeed);
            vec.push_back(b);
            LMB_Counter = 0;
        }
    }

    // отрисовываем всю сцену
    Back->drawBackground(Form1->Canvas->Handle);

    Form1->Caption = "Total Monsters = " + IntToStr(vecMon.size());
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormPaint(TObject *Sender)
{
    Back->drawBackground(Form1->Canvas->Handle);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    LMB_Pressed = true;
    LMB_Counter = 0;
    vec.push_back( new DibBullet(BulletSprite, Back, playerPosX, playerPosY, X + random(23) - 11, Y + random(23) - 11, BulletSpeed) );
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
    LMB_Pressed = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    switch( Key ) {
        case 37:
        case 65:
            Tank->LEFT = true;
        break;

        case 38:
        case 87:
            Tank->UP = true;
        break;

        case 39:
        case 68:
            Tank->RIGHT = true;
        break;

        case 40:
        case 83:
            Tank->DOWN = true;
        break;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    switch( Key ) {
        case 37:
        case 65:
            Tank->LEFT = false;
        break;

        case 38:
        case 87:
            Tank->UP = false;
        break;

        case 39:
        case 68:
            Tank->RIGHT = false;
        break;

        case 40:
        case 83:
            Tank->DOWN = false;
        break;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormMouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
    MouseX = X;
    MouseY = Y;
}
//---------------------------------------------------------------------------


