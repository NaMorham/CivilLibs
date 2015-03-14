#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
	SwapChain::SwapChain()
	//-------------------------------------------------------------------
	:	m_pSwapChain(NULL), m_pZBuffer(NULL), m_renderPaused(false)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(SwapChain), "SwapChain::SwapChain()");

		ZeroMemory(&m_presentParameters, sizeof(D3DPRESENT_PARAMETERS));
	}


//-----------------------------------------------------------------------
	SwapChain::~SwapChain()
	//-------------------------------------------------------------------
	{
		FreeUsedMemory(sizeof(SwapChain), "SwapChain::~SwapChain()");

		Logf("Destroying Swap Chain...");

		if (m_pSwapChain)
			m_pSwapChain->Release();

		if (m_pZBuffer)
			m_pZBuffer->Release();
	}

// EOF