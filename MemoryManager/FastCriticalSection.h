#ifndef _FASTCRITICALSECTION_H_
#define _FASTCRITICALSECTION_H_
#include "platform.h"
class FastCriticalSection
{
protected:
	volatile long Flag;
public:
	inline FastCriticalSection(): Flag(0) {};
	inline FastCriticalSection(const FastCriticalSection&): Flag(0) { assert(false); }; 
	inline void Enter()
	{
		for (;;)
		{
			if (_interlockedbittestandset(&Flag, 0) == 0) return;
			YieldThread();
		};
	};
	inline void Leave()
	{
		Flag = 0;
	};

	class Lock
	{
	protected:
		FastCriticalSection& cs;
	public:
		Lock& operator=(const Lock&) 
		{ 
			return *this; 
		};
		inline Lock(FastCriticalSection& _cs): cs(_cs)
		{
			cs.Enter();
		};
		inline ~Lock()
		{
			cs.Leave();
		};
	};
};
#endif