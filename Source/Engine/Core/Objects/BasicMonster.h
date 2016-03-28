#ifndef BasicMonsterH
#define BasicMonsterH

namespace Core {
namespace Objects {

/*
class cBasicMonster : public cObject{
protected:
	virtual void DoMove(float LagFactor);
	virtual void DoDraw();
	virtual void DoDead();
public:
	cBasicMonster();
	virtual ~cBasicMonster();
};
*/
/*
  TFPSBasicMonster = class(TFPSBasicObject)
  protected
    FAttack, FSpeed, FSeeRange: Single;
    FAtackSpeed, LastAttack: Cardinal;
    procedure DoMove; override;
  public
    procedure SaveToStream(Stream: TStream; SaveAll: bool = false); override;
    procedure LoadFromStream(Stream: TStream; LoadAll: bool = false); override;
    procedure Assign(Item: TFPSBasicSprite); override;
  published
    property Attack: Single read FAttack write FAttack;
    property Speed: Single read FSpeed write FSpeed;
    property SeeRange: Single read FSeeRange write FSeeRange;
    property AtackSpeed: Cardinal read FAtackSpeed write FAtackSpeed;
  end;
*/

} //namespace Objects
} //namespace Core

#endif
