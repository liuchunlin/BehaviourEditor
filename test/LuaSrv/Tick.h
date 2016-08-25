#pragma once

class RanaSpace;
class CTick 
{
public:
	virtual void Init();
	virtual void Tick( );
	virtual void Exit();

	RanaSpace	*m_BattleSpace;
};
