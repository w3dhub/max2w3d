#ifndef TT_INCLUDE__CRITICALSECTIONCLASS_H
#define TT_INCLUDE__CRITICALSECTIONCLASS_H


class SCRIPTS_API CriticalSectionClass
{
	CRITICAL_SECTION *handle;
	unsigned int locked;

public:

	class LockClass
	{
		CriticalSectionClass& CriticalSection;

	public:

		LockClass(CriticalSectionClass& section);
		~LockClass();
		LockClass operator=(LockClass&) = delete;

	};

	CriticalSectionClass();
	~CriticalSectionClass();
	void Enter();
	void Exit();

    CriticalSectionClass(const CriticalSectionClass&) = delete;
    CriticalSectionClass& operator = (const CriticalSectionClass&) = delete;


};


class FastCriticalSectionClass
{
	friend class LockClass;

	volatile long Flag; // 0000


	void Enter()
	{
		TT_ASSERT((size_t)&Flag % 4 == 0); // aligned to 4 bytes please
		for (;;)
		{
			if (_interlockedbittestandset(&Flag, 0) == 0) return;
			_mm_pause();
		};
	}


	void Leave()
	{
		Flag = 0;
	}


public:

	class LockClass
	{
		FastCriticalSectionClass& criticalSection;

		LockClass& operator=(const LockClass&)
		{
			return *this;
		}


	public:

		LockClass(FastCriticalSectionClass& _criticalSection) :
			criticalSection(_criticalSection)
		{
			criticalSection.Enter();
		}

		~LockClass()
		{
			criticalSection.Leave();
		}

	};

	FastCriticalSectionClass() :
		Flag(0)
	{
	}

}; // 0004



#endif
