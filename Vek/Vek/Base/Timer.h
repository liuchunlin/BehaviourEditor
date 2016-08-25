#pragma once

#include "Types.h"

namespace Vek
{
	class Timer
	{
		typedef void		(*TIMBER_CALLBACK)( const vector<void*>& uds, Timer* pSelf );
	public:
		Timer( double delay, double interval, TIMBER_CALLBACK fnInterval, TIMBER_CALLBACK fnFinish, double lift, const vector<void*>& uds );
		virtual ~Timer();
		virtual int			Tick( float AppTime );
		bool				Alive() const { return bAlive; }
	public:
		//In
		double				Delay;			//�ӳٵȴ�ʱ��
		double				Interval;		//�������ʱ��
		TIMBER_CALLBACK		cbInterval;		//�����������
		TIMBER_CALLBACK		cbFinish;		//������������
		vector<void*>		UserDatas;		//����
		//In\Update
		double				Lift;			//����ʣ��ʱ��
		//Update
		double				LastTick;		//�ϴ����ʱ��
		double				Start;			//��ʼʱ��
		double				LastInterval;	//�ϴδ������
		bool				bAlive;			//�Ƿ񽡴�
	};
}
