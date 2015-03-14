/*! \file */
/*-----------------------------------------------------------------------
	memobject.h

	Description: Memory Managed Object Base Class
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

struct MemObject
{
public:
	MemObject();
	virtual ~MemObject();

	void AddUsedMemory(const DWORD numBytes, const String &name = "Unnamed");
	void FreeUsedMemory(const DWORD numBytes, const String &name = "Unnamed");

	void AddRef();
	virtual void Release();

	const DWORD GetRefCount() const { return m_refCount; };
	const DWORD GetUsedMemory() const { return m_usedMemory; };

private:
	DWORD m_refCount,
		  m_usedMemory;
};

// EOF