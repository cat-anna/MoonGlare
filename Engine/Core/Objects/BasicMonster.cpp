#include <pch.h>
//#include <MoonGlare.h>

namespace Core {
namespace Objects {

#if 0
void cBasicMonster::DoMove(float LagFactor){
	const c3DPoint &PlayerPos = m_Engine->GetPlayer()->GetPosition();
	float DistanceToPlayer = PlayerPos.DistanceXZ(m_Position);

	if(DistanceToPlayer > 4) return;
	/*
procedure TFPSBasicMonster.DoMove;
var i: Integer;
    Collisioned: bool;
    Alfa: GLfloat;
    NewPosition: T3Dpoint;
    Obj: TFPSBasicSprite;
begin
//atak
  if IsCollision(DrawPosition, OwnerEngine.Player.Position, 3) then
  begin
    if GetTickCount - LastAttack >= AtackSpeed then
    begin
      LastAttack := GetTickCount;
      OwnerEngine.Player.HitPoints := OwnerEngine.Player.HitPoints - Attack;
    end;
  end else
//ruch
  if Distance2d(Position, OwnerEngine.Player.Position) <= SeeRange then
  begin
    Alfa := ArcSin((OwnerEngine.Player.Position.Z - Position.Z) / Distance2d(OwnerEngine.Player.Position, Position));
    Alfa := Alfa + Pi / 2;    
    NewPosition := Sum3DPoints(Position, Make3DPoint(-Sin(Alfa) * Speed, 0, -Cos(Alfa) * Speed));
    i := 0;
    Collisioned := false;

    while(i < OwnerMap.Objects.Count)and(not Collisioned) do
    begin
      Obj := TFPSBasicSprite(OwnerMap.Objects[i]);
      Inc(i);
      if Obj = Self then Continue;
      if(Obj.Collision)and(IsCollision(Obj.DrawPosition, Position))then
        Collisioned := true;
    end;
    if not Collisioned then
    begin
      Position := NewPosition;
      OwnerEngine.ReObjectPos(Self);
    end;
  end;
end;*/
}

void cBasicMonster::DoDraw(){
	cObject::DoDraw();
}

void cBasicMonster::DoDead(){
	cObject::DoDead();
}

cBasicMonster::cBasicMonster(): cObject(){

}

cBasicMonster::~cBasicMonster(){

}
#endif
/*
procedure TFPSBasicMonster.SaveToStream(Stream: TStream; SaveAll: bool = false);
begin
  inherited;
  if SaveAll then
  begin
    Stream.Write(FAttack, SizeOf(FAttack));
    Stream.Write(FSpeed, SizeOf(FSpeed));
    Stream.Write(FAtackSpeed, SizeOf(FAtackSpeed));
    Stream.Write(FSeeRange, SizeOf(FSeeRange));
  end;
end;*/
/*
procedure TFPSBasicMonster.LoadFromStream(Stream: TStream; LoadAll: bool = false);
begin
  inherited;
  if LoadAll then
  begin
    Stream.Read(FAttack, SizeOf(FAttack));
    Stream.Read(FSpeed, SizeOf(FSpeed));
    Stream.Read(FAtackSpeed, SizeOf(FAtackSpeed));
    Stream.Read(FSeeRange, SizeOf(FSeeRange));
  end;
end;*/
/*
procedure TFPSBasicMonster.Assign(Item: TFPSBasicSprite);
var Obj: TFPSBasicMonster;
begin
  inherited;
  if Item is TFPSBasicMonster then
  begin
    Obj := TFPSBasicMonster(Item);
    FAttack := Obj.Attack;
    FSpeed := Obj.Speed;
    FAtackSpeed := Obj.AtackSpeed;
    FSeeRange := Obj.SeeRange;
    LastAttack := GetTickCount;
  end;
end;
*/

} //namespace Objects
} //namespace Core
