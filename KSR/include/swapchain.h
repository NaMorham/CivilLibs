/*! \file */
/*-----------------------------------------------------------------------
	swapchain.h

	Description: Swapchain class definition
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

struct SwapChain : public MemObject
{
public:
	SwapChain();
	~SwapChain();

	void SetSwapChain(LPDIRECT3DSWAPCHAIN9 s) { m_pSwapChain = s; }
	void SetZBuffer(LPDIRECT3DSURFACE9 s) { m_pZBuffer = s; }
	void SetPaused(bool paused) { m_renderPaused = paused; }

	D3DPRESENT_PARAMETERS *GetPresentParameters() { return &m_presentParameters; }

	LPDIRECT3DSWAPCHAIN9 GetSwapChain() { return m_pSwapChain; }
	LPDIRECT3DSURFACE9	 GetZBuffer() { return m_pZBuffer; }
	bool GetPaused() { return m_renderPaused; }

private:
	D3DPRESENT_PARAMETERS m_presentParameters;
	LPDIRECT3DSWAPCHAIN9  m_pSwapChain;
	LPDIRECT3DSURFACE9	  m_pZBuffer;
	bool m_renderPaused;
};

typedef SwapChain *PSWAPCHAIN;

// EOF