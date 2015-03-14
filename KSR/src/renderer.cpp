#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
	Renderer::Renderer(PRESOURCEMANAGER pResourceManager)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Renderer), "Renderer::Renderer()");

		resourceManager = pResourceManager;

		D3D = NULL;
		D3DDevice = NULL;
		colourFormat = D3DFMT_UNKNOWN;
		depthStencilFormat = D3DFMT_UNKNOWN;
		bytesPerPixel = 0;
		multiSampleAvailable = D3DMULTISAMPLE_NONE;
		vertexShaderVersion = 0;
		numMultiSampleQualityLevels = 0;

		ZeroMemory(&presentParameters, sizeof(D3DPRESENT_PARAMETERS));

		currentViewport = NULL;
		currentBackBuffer = NULL;
		currentDeviceBackBuffer = NULL;

		pFullscreenViewport = NULL;

		viewports.clear();

		processingMode = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		precisionMode = 0;

		deviceLost = false;

		textureTargetID = -1;
	}


//-----------------------------------------------------------------------
	Renderer::~Renderer()
	//-------------------------------------------------------------------
	{
		Logf("Destroying Renderer...");

		FreeUsedMemory(sizeof(Renderer), "Renderer::~Renderer()");

		if (D3DDevice)
			D3DDevice->Release();

		if (D3D)
			D3D->Release();
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::CreateViewport(HWND hWnd, PVIEWPORTSETTINGS pSettings, PVIEWPORT *pViewport)
	//-------------------------------------------------------------------
	{
		Logf("Creating Viewport...");

		*pViewport = new Viewport;

		if (pSettings)
			(*pViewport)->SetSettings(pSettings);

		(*pViewport)->SetHWND(hWnd);

		SwapChain *swapChain = new SwapChain;

		ViewportSettings vps;
		(*pViewport)->GetSettings(&vps);

		D3DMULTISAMPLE_TYPE multiSampleType = D3DMULTISAMPLE_NONE;
		DWORD multiSamplingQuality = 0;

		if (vps.multiSampling)
		{
			multiSampleType = multiSampleAvailable;
			multiSamplingQuality = numMultiSampleQualityLevels - 1;
		}

		swapChain->GetPresentParameters()->Windowed					= TRUE;
		swapChain->GetPresentParameters()->BackBufferWidth			= vps.width;
		swapChain->GetPresentParameters()->BackBufferHeight			= vps.height;
		swapChain->GetPresentParameters()->BackBufferFormat			= colourFormat;
		swapChain->GetPresentParameters()->BackBufferCount			= 1;
		swapChain->GetPresentParameters()->SwapEffect				= D3DSWAPEFFECT_DISCARD;
		swapChain->GetPresentParameters()->hDeviceWindow			= (*pViewport)->GetHWND();
		swapChain->GetPresentParameters()->EnableAutoDepthStencil	= FALSE;
		swapChain->GetPresentParameters()->Flags					= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		swapChain->GetPresentParameters()->MultiSampleType			= multiSampleType;
		swapChain->GetPresentParameters()->MultiSampleQuality		= multiSamplingQuality;
		swapChain->GetPresentParameters()->PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;

		LPDIRECT3DSWAPCHAIN9 pSwapChain = NULL;
		LPDIRECT3DSURFACE9	 pZBuffer = NULL;

		if (FAILED(D3DDevice->CreateAdditionalSwapChain(swapChain->GetPresentParameters(), &pSwapChain)))
		{
			Logf("Couldn't create additional swap chain");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->CreateDepthStencilSurface(swapChain->GetPresentParameters()->BackBufferWidth,
														swapChain->GetPresentParameters()->BackBufferHeight,
														depthStencilFormat,
														swapChain->GetPresentParameters()->MultiSampleType,
														0,
														FALSE,
														&pZBuffer,
														NULL)))
		{
			Logf("Error: Couldn't create depth stencil");
			return E_FAIL;
		}

		swapChain->SetSwapChain(pSwapChain);
		swapChain->SetZBuffer(pZBuffer);

		(*pViewport)->SetSwapChain(swapChain);

		PrepareViewport(*pViewport);

		viewports.push_back(*pViewport);

		Logf("Viewport created successfully.\n");

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::DestroyViewport(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		std::list<PVIEWPORT>::iterator i = viewports.begin();
		for (; i != viewports.end();)
		{
			if (*i == pViewport)
			{
				pViewport->Release();

				i = viewports.erase(i);
			}
			else
			{
				i++;
			}
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::ResetDevice()
	//-------------------------------------------------------------------
	{
		if (FAILED(D3DDevice->Reset(&presentParameters)))
		{
			Logf("Failed Reseting Direct3D Device");
			return E_FAIL;
		}

		std::list<PVIEWPORT>::iterator i = viewports.begin();
		for (; i != viewports.end(); i++)
		{
			if (FAILED(ResizeViewport(*i)))
			{
				Logf("Failed Resizing Viewport");
				return E_FAIL;
			}
		}

		if (FAILED(InitDeviceStates()))
			return E_FAIL;

		deviceLost = false;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::CatchLostDevice()
	//-------------------------------------------------------------------
	{
		if (!D3DDevice)
			return E_FAIL;

		deviceLost = true;

		HRESULT hr = D3DDevice->TestCooperativeLevel();

		if (hr == D3DERR_DEVICELOST)
		{
			Logf("Couldn't reset device at this time.");

			return E_FAIL;
		}

		if (hr == D3DERR_DEVICENOTRESET)
		{
			Logf("Reseting Direct3D Device.");

			if (FAILED(ResetDevice()))
				return E_FAIL;
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::ResizeViewport(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!pViewport)
			return E_FAIL;

		if (pFullscreenViewport && pFullscreenViewport != pViewport)
			return S_OK;

		PSWAPCHAIN swapChain = pViewport->GetSwapChain();

		if (!swapChain)
			return E_FAIL;

		bool paused = swapChain->GetPaused();

		swapChain->GetSwapChain()->Release();
		swapChain->SetSwapChain(NULL);

		swapChain->GetZBuffer()->Release();
		swapChain->SetZBuffer(NULL);

		swapChain->SetPaused(paused);

		ViewportSettings vps;

		if (FAILED(pViewport->GetSettings(&vps)))
			return E_FAIL;

		swapChain->GetPresentParameters()->BackBufferWidth	= vps.width;
		swapChain->GetPresentParameters()->BackBufferHeight = vps.height;

		LPDIRECT3DSWAPCHAIN9 pSwapChain = NULL;
		LPDIRECT3DSURFACE9	 pZBuffer = NULL;

		if (FAILED(D3DDevice->CreateAdditionalSwapChain(swapChain->GetPresentParameters(), &pSwapChain)))
		{
			Logf("Error: Couldn't create additional swap chain");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->CreateDepthStencilSurface(swapChain->GetPresentParameters()->BackBufferWidth,
														swapChain->GetPresentParameters()->BackBufferHeight,
														depthStencilFormat,
														swapChain->GetPresentParameters()->MultiSampleType,
														0,
														FALSE,
														&pZBuffer,
														NULL)))
		{
			Logf("Error: Couldn't create depth stencil");
			return E_FAIL;
		}

		swapChain->SetSwapChain(pSwapChain);
		swapChain->SetZBuffer(pZBuffer);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::ClearViewport(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!pViewport)
			return E_FAIL;

		if (pFullscreenViewport && pFullscreenViewport != pViewport)
			return S_OK;

		PSWAPCHAIN swapChain = pViewport->GetSwapChain();

		if (!swapChain)
			return E_FAIL;

		LPDIRECT3DSWAPCHAIN9 pSwapChain = swapChain->GetSwapChain();
		LPDIRECT3DSURFACE9	 pZBuffer = swapChain->GetZBuffer();
		LPDIRECT3DSURFACE9	 pBackBuffer = NULL;
		LPDIRECT3DSURFACE9	 pDeviceBackBuffer = NULL;

		ViewportSettings vps;

		if (FAILED(pViewport->GetSettings(&vps)))
			return E_FAIL;

		if (FAILED(pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
		{
			Logf("Error: Render() failed: GetBackBuffer()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->GetRenderTarget(0, &pDeviceBackBuffer)))
		{
			Logf("Error: Render() failed: GetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetRenderTarget(0, pBackBuffer)))
		{
			Logf("Error: Render() failed: SetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetDepthStencilSurface(pZBuffer)))
		{
			Logf("Error: Render() failed: SetDepthStencilSurface()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->BeginScene()))
			return E_FAIL;

		if (FAILED(D3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, vps.bgColour, 1.0f, 0)))
			return E_FAIL;

		if (FAILED(D3DDevice->EndScene()))
		{
			Logf("Error: Render() failed: EndScene()");
			return E_FAIL;
		}

		Present(pSwapChain, pViewport);

		if (FAILED(D3DDevice->SetRenderTarget(0, pDeviceBackBuffer)))
		{
			Logf("Error: Render() failed: SetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetDepthStencilSurface(NULL)))
		{
			Logf("Error: Render() failed: SetDepthStencilSurface()");
			return E_FAIL;
		}

		if (pBackBuffer)
			pBackBuffer->Release();

		if (pDeviceBackBuffer)
			pDeviceBackBuffer->Release();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::PauseViewport(PVIEWPORT pViewport, bool paused)
	//-------------------------------------------------------------------
	{
		if (!pViewport)
			return E_FAIL;

		if (pFullscreenViewport && pFullscreenViewport != pViewport)
			return S_OK;

		PSWAPCHAIN swapChain = pViewport->GetSwapChain();

		if (!swapChain)
			return E_FAIL;

		swapChain->SetPaused(paused);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::Create(HWND hWnd, CoreSettings *pSettings)
	//-------------------------------------------------------------------
	{
		Logf("Creating Renderer...");

		hMainWnd = hWnd;

		if (!(D3D = Direct3DCreate9(D3D_SDK_VERSION)))
			return E_FAIL;

		if (FAILED(D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &D3DDisplayMode)))
			return E_FAIL;

		if (FAILED(D3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCaps)))
			return E_FAIL;

		colourFormat = D3DDisplayMode.Format;

		switch (colourFormat)
		{
		case D3DFMT_X8R8G8B8: case D3DFMT_A8R8G8B8: case D3DFMT_A2B10G10R10: case D3DFMT_G16R16:
			Logf("32bit Colour Selected.");
			bytesPerPixel = 4;
			break;

		case D3DFMT_R8G8B8:
			Logf("24bit Colour Selected.");
			bytesPerPixel = 3;
			break;

		case D3DFMT_R5G6B5: case D3DFMT_X1R5G5B5: case D3DFMT_A1R5G5B5: case D3DFMT_A4R4G4B4: case D3DFMT_A8R3G3B2:
		case D3DFMT_X4R4G4B4: case D3DFMT_A8L8:
			Logf("16bit Colour Selected.");
			bytesPerPixel = 2;
			break;

		default:
			Logf("8bit Colour Selected.");
			bytesPerPixel = 1;
			break;
		}

		if (D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT && pSettings->deviceType == CS_HAL &&
			!pSettings->forceSoftwareVertexProcessing)
		{
			Logf("Hardware Vertex Processing supported.");
			processingMode = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
		else
			Logf("Hardware Vertex Processing not supported. Software Processing selected.");

		if (pSettings->floatPrecision == CS_DOUBLE)
		{
			precisionMode = D3DCREATE_FPU_PRESERVE;
			Logf("64bit Double-Precision Floating-Point Unit selected.");
		}
		else
			Logf("32bit Single-Precision Floating-Point Unit selected.");

		if (D3DCaps.DevCaps & D3DDEVCAPS_NPATCHES)
			Logf("N-Patches supported.");

		if (D3DCaps.DevCaps & D3DDEVCAPS_QUINTICRTPATCHES)
			Logf("Quintic Bézier curves and B-splines supported.");

		if (D3DCaps.DevCaps & D3DDEVCAPS_RTPATCHES)
			Logf("RT-patches supported.");

		if (D3DCaps.DevCaps & D3DDEVCAPS_RTPATCHHANDLEZERO)
			Logf("RT-patch handle zero supported.");

		if (D3DCaps.MaxVertexIndex <= 2 << 15)
			Logf("16bit Index Buffer supported.");
		else
			Logf("32bit Index Buffer supported.");

		if (pSettings->deviceType == CS_HAL)
			D3DDeviceType = D3DDEVTYPE_HAL;
		else if (pSettings->deviceType == CS_REF)
			D3DDeviceType = D3DDEVTYPE_REF;

		if (SUCCEEDED(D3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
													  D3DDeviceType,
													  colourFormat,
													  TRUE,
													  D3DMULTISAMPLE_4_SAMPLES,
													  &numMultiSampleQualityLevels)))
		{
			Logf("4x Multisampling supported.");
			multiSampleAvailable = D3DMULTISAMPLE_4_SAMPLES;
		}
		else
			Logf("Multisampling not supported.");

		if (SUCCEEDED(D3D->CheckDeviceFormat(D3DADAPTER_DEFAULT,
											 D3DDeviceType,
											 colourFormat,
											 D3DUSAGE_DEPTHSTENCIL,
											 D3DRTYPE_SURFACE,
											 D3DFMT_D32)))
		{
			Logf("32bit Depth Stencil Supported.");
			depthStencilFormat = D3DFMT_D32;
		}
		else if (SUCCEEDED(D3D->CheckDeviceFormat(D3DADAPTER_DEFAULT,
												  D3DDeviceType,
												  colourFormat,
												  D3DUSAGE_DEPTHSTENCIL,
												  D3DRTYPE_SURFACE,
												  D3DFMT_D24S8)))
		{
			Logf("24bit Depth Stencil Supported.");
			depthStencilFormat = D3DFMT_D24S8;
		}
		else if (SUCCEEDED(D3D->CheckDeviceFormat(D3DADAPTER_DEFAULT,
												  D3DDeviceType,
												  colourFormat,
												  D3DUSAGE_DEPTHSTENCIL,
												  D3DRTYPE_SURFACE,
												  D3DFMT_D16)))
		{
			Logf("16bit Depth Stencil Supported.");
			depthStencilFormat = D3DFMT_D16;
		}

		// Permanent Present Parameters
			presentParameters.BackBufferFormat = colourFormat;
			presentParameters.BackBufferCount = 1;
			presentParameters.hDeviceWindow = hMainWnd;
			presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentParameters.EnableAutoDepthStencil = TRUE;
			presentParameters.AutoDepthStencilFormat = depthStencilFormat;
			presentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
			presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		if (FAILED(CreateDeviceAndSwapChains()))
			return E_FAIL;

		if (FAILED(InitDeviceStates()))
			return E_FAIL;

		Logf("Renderer created successfully.\n");

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::CreateDeviceAndSwapChains()
	//-------------------------------------------------------------------
	{
		presentParameters.Windowed = pFullscreenViewport ? FALSE : TRUE;
		presentParameters.BackBufferWidth = pFullscreenViewport ? D3DDisplayMode.Width : 1;
		presentParameters.BackBufferHeight = pFullscreenViewport ? D3DDisplayMode.Height : 1;
		presentParameters.Flags = pFullscreenViewport ? 0 : D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		presentParameters.FullScreen_RefreshRateInHz = pFullscreenViewport ? D3DDisplayMode.RefreshRate : 0;

		if (FAILED(D3D->CreateDevice(D3DADAPTER_DEFAULT,
					    			 D3DDeviceType,
									 hMainWnd,
									 processingMode | precisionMode,
									 &presentParameters,
									 &D3DDevice)))
			return E_FAIL;

		if (pFullscreenViewport)
		{
			LPDIRECT3DSWAPCHAIN9 pSwapChain = NULL;
			LPDIRECT3DSURFACE9 pZBuffer = NULL;

			if (FAILED(D3DDevice->GetSwapChain(0, &pSwapChain)))
				return E_FAIL;

			if (FAILED(D3DDevice->GetDepthStencilSurface(&pZBuffer)))
				return E_FAIL;

			SwapChain *swapChain = pFullscreenViewport->GetSwapChain();

			if (!swapChain)
				return E_FAIL;

			swapChain->SetSwapChain(pSwapChain);
			swapChain->SetZBuffer(pZBuffer);
		}

		if (!pFullscreenViewport)
		{
			std::list< PVIEWPORT >::iterator i = viewports.begin();
			for (; i != viewports.end(); i++)
			{
				SwapChain *swapChain = (*i)->GetSwapChain();

				if (!swapChain)
					return E_FAIL;

				LPDIRECT3DSWAPCHAIN9 pSwapChain = NULL;
				LPDIRECT3DSURFACE9	 pZBuffer = NULL;

				if (FAILED(D3DDevice->CreateAdditionalSwapChain(swapChain->GetPresentParameters(), &pSwapChain)))
				{
					Logf("Couldn't create additional swap chain");
					return E_FAIL;
				}

				if (FAILED(D3DDevice->CreateDepthStencilSurface(swapChain->GetPresentParameters()->BackBufferWidth,
																swapChain->GetPresentParameters()->BackBufferHeight,
																depthStencilFormat,
																swapChain->GetPresentParameters()->MultiSampleType,
																swapChain->GetPresentParameters()->MultiSampleQuality,
																FALSE,
																&pZBuffer,
																NULL)))
				{
					Logf("Error: Couldn't create depth stencil");
					return E_FAIL;
				}

				swapChain->SetSwapChain(pSwapChain);
				swapChain->SetZBuffer(pZBuffer);
			}
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::InitDeviceStates()
	//-------------------------------------------------------------------
	{
		if (!D3DDevice)
			return E_FAIL;

		D3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
		D3DDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
		D3DDevice->SetRenderState(D3DRS_POINTSIZE_MIN, (DWORD)0.0f);

		float scaleA = 0.0f;
		float scaleB = 0.0f;
		float scaleC = 1.0f;

		D3DDevice->SetRenderState(D3DRS_POINTSCALE_A, *((DWORD*)(&scaleA)));
		D3DDevice->SetRenderState(D3DRS_POINTSCALE_B, *((DWORD*)(&scaleB)));
		D3DDevice->SetRenderState(D3DRS_POINTSCALE_C, *((DWORD*)(&scaleC)));

		D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::SetRenderTarget(int textureID)
	//-------------------------------------------------------------------
	{
		if (textureID == -1)
		{
			textureTargetID = -1;
			return S_OK;
		}

		LPDIRECT3DBASETEXTURE9 pBaseTexture = resourceManager->GetTexture(textureID);

		if (pBaseTexture->GetType() != D3DRTYPE_TEXTURE)
			return E_FAIL;

		LPDIRECT3DTEXTURE9 targetTexture = (LPDIRECT3DTEXTURE9)pBaseTexture;
		LPDIRECT3DSURFACE9 targetSurface;

		if (!targetTexture)
			return E_FAIL;

		if (FAILED(targetTexture->GetSurfaceLevel(0, &targetSurface)))
			return E_FAIL;

		D3DSURFACE_DESC desc;

		if (FAILED(targetSurface->GetDesc(&desc)))
			return E_FAIL;

		if (!(desc.Usage & D3DUSAGE_RENDERTARGET))
			return E_FAIL;

		textureTargetID = textureID;

		if (targetSurface)
			targetSurface->Release();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::SelectSourceViewport(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		pViewport->GetSettings(&currentViewportSettings);

		ViewportSettings baseSettings;
		currentViewport->GetSettings(&baseSettings);

		currentViewportSettings.width = baseSettings.width;
		currentViewportSettings.height = baseSettings.height;
		currentViewportSettings.bgColour = baseSettings.bgColour;

		if (FAILED(PrepareViewport(pViewport)))
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::Begin(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!pViewport)
			return E_FAIL;

		if (!pViewport->GetRedraw())
			return S_OK;

		if (pFullscreenViewport && pViewport != pFullscreenViewport)
			return S_OK;

		PSWAPCHAIN swapChain = pViewport->GetSwapChain();

		if (!swapChain)
			return E_FAIL;

		ViewportSettings vps;

		if (FAILED(pViewport->GetSettings(&vps)))
			return E_FAIL;

		LPDIRECT3DSWAPCHAIN9 pSwapChain = NULL;
		LPDIRECT3DSURFACE9   pZBuffer = NULL;
		LPDIRECT3DSURFACE9   pBackBuffer = NULL;
		LPDIRECT3DSURFACE9   pDeviceBackBuffer = NULL;

		pSwapChain = swapChain->GetSwapChain();
		pZBuffer = swapChain->GetZBuffer();

		if (textureTargetID > -1)
		{
			LPDIRECT3DBASETEXTURE9 baseTexture = resourceManager->GetTexture(textureTargetID);

			if (baseTexture->GetType() != D3DRTYPE_TEXTURE)
				return E_FAIL;

			LPDIRECT3DTEXTURE9 textureTarget = (LPDIRECT3DTEXTURE9)baseTexture;

			if (FAILED(textureTarget->GetSurfaceLevel(0, &pBackBuffer)))
			{
				Logf("Error: Render() failed: GetSurfaceLevel()");
				return E_FAIL;
			}
		}
		else
		{
			if (FAILED(pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
			{
				Logf("Error: Render() failed: GetBackBuffer()");
				return E_FAIL;
			}
		}

		if (FAILED(D3DDevice->GetRenderTarget(0, &pDeviceBackBuffer)))
		{
			Logf("Error: Render() failed: GetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetRenderTarget(0, pBackBuffer)))
		{
			Logf("Error: Render() failed: SetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetDepthStencilSurface(pZBuffer)))
		{
			Logf("Error: Render() failed: SetDepthStencilSurface()");
			return E_FAIL;
		}

		if (FAILED(PrepareViewport(pViewport)))
			Logf("Error: Render() failed: PrepareViewport()");

		if (FAILED(D3DDevice->BeginScene()))
		{
			Logf("Error: Render() failed: BeginScene()");
			return E_FAIL;
		}

		if (!pViewport->GetSwapChain()->GetPaused())
		{
			if (FAILED(PreRender(pSwapChain, pViewport)))
				return E_FAIL;
		}

		currentViewport = pViewport;
		currentBackBuffer = pBackBuffer;
		currentDeviceBackBuffer = pDeviceBackBuffer;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::End()
	//-------------------------------------------------------------------
	{
		if (!currentViewport)
			return E_FAIL;

		if (!currentViewport->GetRedraw())
			return S_OK;

		if (pFullscreenViewport && currentViewport != pFullscreenViewport)
			return S_OK;

		if (currentViewport->GetRedraw() == RT_ONCE)
			currentViewport->SetRedraw(RT_NONE);

		currentViewport->FlushText();

		PSWAPCHAIN swapChain = currentViewport->GetSwapChain();

		if (!swapChain)
			return E_FAIL;

		currentViewport = NULL;

		if (FAILED(D3DDevice->EndScene()))
		{
			Logf("Error: Render() failed: EndScene()");
			return E_FAIL;
		}

		if (textureTargetID > -1)
		{
			Present(NULL, currentViewport);
		}
		else
		{
			Present(swapChain->GetSwapChain(), currentViewport);
		}

		if (FAILED(D3DDevice->SetRenderTarget(0, currentDeviceBackBuffer)))
			Logf("Error: Render() failed: SetRenderTarget()");
		if (FAILED(D3DDevice->SetDepthStencilSurface(NULL)))
			Logf("Error: Render() failed: SetDepthStencilSurface()");

		if (currentBackBuffer)
			currentBackBuffer->Release();

		if (currentDeviceBackBuffer)
			currentDeviceBackBuffer->Release();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::Render(SceneList &scenesToRender, PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!pViewport->GetRedraw())
			return S_OK;

		if (pViewport->GetRedraw() == RT_ONCE)
			pViewport->SetRedraw(RT_NONE);

		if (pFullscreenViewport && pViewport != pFullscreenViewport)
			return S_OK;

		PSWAPCHAIN swapChain = pViewport->GetSwapChain();

		if (!swapChain)
			return E_FAIL;

		ViewportSettings vps;

		if (FAILED(pViewport->GetSettings(&vps)))
			return E_FAIL;

		LPDIRECT3DSWAPCHAIN9 pSwapChain = NULL;
		LPDIRECT3DSURFACE9   pZBuffer = NULL;
		LPDIRECT3DSURFACE9   pBackBuffer = NULL;
		LPDIRECT3DSURFACE9   pDeviceBackBuffer = NULL;

		pSwapChain = swapChain->GetSwapChain();
		pZBuffer = swapChain->GetZBuffer();

		if (textureTargetID > -1)
		{
			LPDIRECT3DBASETEXTURE9 baseTexture = resourceManager->GetTexture(textureTargetID);

			if (baseTexture->GetType() != D3DRTYPE_TEXTURE)
				return E_FAIL;

			LPDIRECT3DTEXTURE9 textureTarget = (LPDIRECT3DTEXTURE9)baseTexture;

			if (FAILED(textureTarget->GetSurfaceLevel(0, &pBackBuffer)))
			{
				Logf("Error: Render() failed: GetSurfaceLevel()");
				return E_FAIL;
			}
		}
		else
		{
			if (FAILED(pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
			{
				Logf("Error: Render() failed: pSwapChain->GetBackBuffer()");
				return E_FAIL;
			}
		}

		if (FAILED(D3DDevice->GetRenderTarget(0, &pDeviceBackBuffer)))
		{
			Logf("Error: Render() failed: GetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetRenderTarget(0, pBackBuffer)))
		{
			Logf("Error: Render() failed: SetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetDepthStencilSurface(pZBuffer)))
		{
			Logf("Error: Render() failed: SetDepthStencilSurface()");
			return E_FAIL;
		}

		if (FAILED(PrepareViewport(pViewport)))
			Logf("Error: Render() failed: PrepareViewport()");

		if (FAILED(D3DDevice->BeginScene()))
		{
			Logf("Error: Render() failed: BeginScene()");
			return E_FAIL;
		}

		if (!swapChain->GetPaused())
		{
			if (FAILED(PreRender(pSwapChain, pViewport)))
				return E_FAIL;

			std::vector<RenderGroup> renderList;
			renderList.clear();

			SceneList::iterator s = scenesToRender.begin();
			for (; s != scenesToRender.end(); s++)
			{
#ifdef _DEBUG
				PSCENE pScene = *s;
#endif
				if (!(*s)->GetRedraw()) 
					continue;

				if ((*s)->GetRedraw() == RT_ONCE)
					(*s)->SetRedraw(RT_NONE);

				if (FAILED((*s)->CreateRenderList(renderList, pViewport)))
				{
					Logf("Error: Scene::CreateRenderList()");
					return E_FAIL;
				}
			}

			if (FAILED(RenderGeometry(renderList, pViewport)))
			{
				Logf("Error: Scene::RenderList()");
				return E_FAIL;
			}

			pViewport->FlushText();
		}

		if (FAILED(D3DDevice->EndScene()))
		{
			Logf("Error: Render() failed: EndScene()");
			return E_FAIL;
		}

		HRESULT result = S_OK;
		if (textureTargetID == -1)
		{
			if (FAILED(result = Present(pSwapChain, pViewport)))
				Logf("Error: Render failed: Present(NULL, pViewport)");
		}

		if (FAILED(result = D3DDevice->SetRenderTarget(0, pDeviceBackBuffer)))
			Logf("Error: Render() failed: SetRenderTarget()");

		if (FAILED(result = D3DDevice->SetDepthStencilSurface(NULL)))
			Logf("Error: Render() failed: SetDepthStencilSurface()");

		if (pBackBuffer)
			pBackBuffer->Release();

		if (pDeviceBackBuffer)
			pDeviceBackBuffer->Release();

		return result;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::Render(PSCENE pScene)
	//-------------------------------------------------------------------
	{
		if (!pScene)
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::Render(PSCENE pScene, PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!pViewport->GetRedraw())
			return S_OK;

		if (pViewport->GetRedraw() == RT_ONCE)
			pViewport->SetRedraw(RT_NONE);

		if (pFullscreenViewport && pViewport != pFullscreenViewport)
			return S_OK;

		PSWAPCHAIN swapChain = pViewport->GetSwapChain();

		if (!swapChain)
			return E_FAIL;

		ViewportSettings vps;

		if (FAILED(pViewport->GetSettings(&vps)))
			return E_FAIL;

		LPDIRECT3DSWAPCHAIN9 pSwapChain = NULL;
		LPDIRECT3DSURFACE9   pZBuffer = NULL;
		LPDIRECT3DSURFACE9   pBackBuffer = NULL;
		LPDIRECT3DSURFACE9   pDeviceBackBuffer = NULL;

		pSwapChain = swapChain->GetSwapChain();
		pZBuffer = swapChain->GetZBuffer();

		if (textureTargetID > -1)
		{
			LPDIRECT3DBASETEXTURE9 baseTexture = resourceManager->GetTexture(textureTargetID);

			if (baseTexture->GetType() != D3DRTYPE_TEXTURE)
				return E_FAIL;

			LPDIRECT3DTEXTURE9 textureTarget = (LPDIRECT3DTEXTURE9)baseTexture;

			if (FAILED(textureTarget->GetSurfaceLevel(0, &pBackBuffer)))
			{
				Logf("Error: Render() failed: GetSurfaceLevel()");
				return E_FAIL;
			}
		}
		else
		{
			if (FAILED(pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
			{
				Logf("Error: Render() failed: GetBackBuffer()");
				return E_FAIL;
			}
		}

		if (FAILED(D3DDevice->GetRenderTarget(0, &pDeviceBackBuffer)))
		{
			Logf("Error: Render() failed: GetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetRenderTarget(0, pBackBuffer)))
		{
			Logf("Error: Render() failed: SetRenderTarget()");
			return E_FAIL;
		}

		if (FAILED(D3DDevice->SetDepthStencilSurface(pZBuffer)))
		{
			Logf("Error: Render() failed: SetDepthStencilSurface()");
			return E_FAIL;
		}

		if (FAILED(PrepareViewport(pViewport)))
			Logf("Error: Render() failed: PrepareViewport()");

		if (FAILED(D3DDevice->BeginScene()))
		{
			Logf("Error: Render() failed: BeginScene()");
			return E_FAIL;
		}

		if (!swapChain->GetPaused())
		{
			if (FAILED(PreRender(pSwapChain, pViewport)))
			{
				return E_FAIL;
			}

			if (pScene->GetRedraw())
			{
				if (pScene->GetRedraw() == RT_ONCE)
					pScene->SetRedraw(RT_NONE);

				std::vector<RenderGroup> renderList;
				renderList.clear();

				if (FAILED(pScene->CreateRenderList(renderList, pViewport)))
				{
					Logf("Error: Scene::CreateRenderList()");
					return E_FAIL;
				}

				if (FAILED(RenderGeometry(renderList, pViewport)))
				{
					Logf("Error: Scene::RenderList()");
					return E_FAIL;
				}
			}

			pViewport->FlushText();
		}

		if (FAILED(D3DDevice->EndScene()))
		{
			Logf("Error: Render() failed: EndScene()");
			return E_FAIL;
		}

		if (textureTargetID == -1)
		{
			Present(pSwapChain, pViewport);
		}

		if (FAILED(D3DDevice->SetRenderTarget(0, pDeviceBackBuffer)))
			Logf("Error: Render() failed: SetRenderTarget()");

		if (FAILED(D3DDevice->SetDepthStencilSurface(NULL)))
			Logf("Error: Render() failed: SetDepthStencilSurface()");

		if (pBackBuffer)
			pBackBuffer->Release();

		if (pDeviceBackBuffer)
			pDeviceBackBuffer->Release();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::Render(SceneList &scenesToRender)
	//-------------------------------------------------------------------
	{
		std::list<PVIEWPORT>::iterator i = viewports.begin();
		for (; i != viewports.end(); i++)
			Render(scenesToRender, *i);

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::Render(std::vector<RenderGroup> &renderList)
	//-------------------------------------------------------------------
	{
		if (!currentViewport)
			return E_FAIL;

		if (renderList.size() < 1)
			return S_OK;

		if (pFullscreenViewport && currentViewport != pFullscreenViewport)
			return S_OK;

		if (!currentViewport->GetSwapChain())
			return E_FAIL;

		if (FAILED(RenderGeometry(renderList, currentViewport)))
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::PrepareViewport(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		pViewport->GetSettings(&currentViewportSettings);

		D3DXMATRIX world, view;

		D3DXMatrixIdentity(&world);
		view = pViewport->GetCamera()->GetViewMatrix();

		for (int i = 0; i < currentViewportSettings.numDepthLevels; i++)
		{
			Matrix projection;

			float nearDisplacement = i * currentViewportSettings.depthLevelDistance;
			float farDisplacement = i * currentViewportSettings.depthLevelDistance;

			if (currentViewportSettings.coordSystemMode == VS_LEFTHANDED)
			{
				if (currentViewportSettings.projectionMode == VS_ORTHO)
				{
					D3DXMatrixOrthoLH(&projection, currentViewportSettings.viewWidth,
												   currentViewportSettings.viewHeight,
												   currentViewportSettings.nearClipPlane + nearDisplacement,
												   currentViewportSettings.farClipPlane + farDisplacement);
				}
				else
					D3DXMatrixPerspectiveFovLH(&projection, D3DXToRadian(currentViewportSettings.fov), 
															currentViewportSettings.aspect,
															currentViewportSettings.nearClipPlane + nearDisplacement,
															currentViewportSettings.farClipPlane + farDisplacement);
			}
			else if (currentViewportSettings.coordSystemMode == VS_RIGHTHANDED)
			{
				if (currentViewportSettings.projectionMode == VS_ORTHO)
				{
					D3DXMatrixOrthoRH(&projection, currentViewportSettings.viewWidth,
												   currentViewportSettings.viewHeight,
												   currentViewportSettings.nearClipPlane + nearDisplacement,
												   currentViewportSettings.farClipPlane + farDisplacement);
				}
				else
					D3DXMatrixPerspectiveFovRH(&projection, D3DXToRadian(currentViewportSettings.fov), 
															currentViewportSettings.aspect,
															currentViewportSettings.nearClipPlane + nearDisplacement,
															currentViewportSettings.farClipPlane + farDisplacement);
			}

			pViewport->SetProjectionMatrix(i, projection);
		}

		D3DDevice->SetTransform(D3DTS_WORLD, &world);
		D3DDevice->SetTransform(D3DTS_VIEW, &view);
		D3DDevice->SetTransform(D3DTS_PROJECTION, &pViewport->GetProjectionMatrix(0));

		pViewport->SetCameraFrustum(view * pViewport->GetProjectionMatrix(0));

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::PrepareViewportRender(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::RenderGeometry(std::vector<RenderGroup> &renderList, PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (renderList.size() < 1)
			return S_OK;

		std::sort(renderList.begin(), renderList.end(), RenderGroup::LessOrder);

		ViewportSettings &vps = currentViewportSettings;

		// Create effect parameters
			Matrix world, view, projection, worldView, worldViewProjection;
			Vector4 lightDirection0, lightDiffuse0, lightAmbient0;
			Vector4 backgroundColour;

			D3DDevice->GetTransform(D3DTS_WORLD,	  &world);
			D3DDevice->GetTransform(D3DTS_VIEW,		  &view);
			D3DDevice->GetTransform(D3DTS_PROJECTION, &projection);

			worldView = world * view;
			worldViewProjection = worldView * projection;

			backgroundColour = Math::ARGBToV4RGBA(vps.bgColour);

		PGEOMETRY currentGeometry = NULL;
		POPTIMISEDGEOMETRY currentOptimisedGeometry = NULL;
		int currentEffect = -1;
		int currentMaterial = -1;

		int numEffectGroups = resourceManager->GetNumEffects();

		for (int i = 0; i < numEffectGroups; i++)
		{
			PEFFECT effect = resourceManager->GetEffect(i);
			D3DXHANDLE currentTechnique = effect->m_currentTechnique.c_str();

			if (effect->m_hasWorld)
				effect->SetValue(_T("world"), &world, sizeof(Matrix));

			if (effect->m_hasView)
				effect->SetValue(_T("view"), &view, sizeof(Matrix));

			if (effect->m_hasProjection)
				effect->SetValue(_T("projection"), &projection, sizeof(Matrix));

			if (effect->m_hasWorldView)
				effect->SetValue(_T("worldView"), &worldView, sizeof(Matrix));

			if (effect->m_hasWorldViewProjection)
				effect->SetValue(_T("worldViewProjection"), &worldViewProjection, sizeof(Matrix));

			if (effect->m_hasBackgroundColour)
				effect->SetValue(_T("backgroundColour"), &backgroundColour, sizeof(Vector4));

			for (int n = 0; n < vps.numDepthLevels && n < effect->m_numProjectionLevels; n++)
			{
				if (effect->m_hasProjectionLevels[n])
				{
					TCHAR buf[64];
					_stprintf(buf, _T("projectionLevel%d"), n);

					Matrix projectionLevel = pViewport->GetProjectionMatrix(n);

					effect->SetValue(buf, &projectionLevel, sizeof(Matrix));
				}

				if (effect->m_hasWorldViewProjectionLevels[n])
				{
					TCHAR buf[64];
					_stprintf(buf, _T("worldViewProjectionLevel%d"), n);

					Matrix projectionLevel = pViewport->GetProjectionMatrix(n);
					Matrix worldViewProjectionLevel = worldView * projectionLevel;

					effect->SetValue(buf, &worldViewProjectionLevel, sizeof(Matrix));
				}
			}
		}

		bool *facesDrawn = NULL;

#ifdef DEBUG_DUMP
		OutputDebugString("Rendering RenderGroups\n==========================================================\n\n");
#endif
		std::vector<RenderGroup>::iterator r = renderList.begin();
		for (; r != renderList.end(); r++)
		{
#ifdef DEBUG_DUMP
			char buf[256];
			sprintf(buf, "RenderGroup:\n\torder = %d\n", r->order);
			OutputDebugString(buf);
#endif
			RenderGroup &rr = *r;

#ifdef _DEBUG
			PGEOMETRY pGeometry = r->m_pGeometry;
			POPTIMISEDGEOMETRY pOptimisedGeometry = r->m_pOptimisedGeometry;
#endif
			if (r->m_numFaces < 1)
				continue;

			// Setup currentGeometry
			bool currentChanged = false;

			if ((r->m_pGeometry && (r->m_pGeometry != currentGeometry)) || 
				(r->m_pOptimisedGeometry && (r->m_pOptimisedGeometry != currentOptimisedGeometry)))
			{
				if (r->m_pGeometry)
				{
					currentGeometry = r->m_pGeometry;
					currentOptimisedGeometry = NULL;

					currentChanged = true;
				}
				else if (r->m_pOptimisedGeometry)
				{
					currentOptimisedGeometry = r->m_pOptimisedGeometry;
					currentGeometry = currentOptimisedGeometry->m_pSource;

					currentChanged = true;
				}
			}

			if (!currentGeometry)
				continue;

			if (currentChanged)
			{
				if (currentOptimisedGeometry)
				{
					if (currentOptimisedGeometry->GetNumChunks() < 1 || currentGeometry->GetNumVertices() < 1 ||
						currentOptimisedGeometry->GetNumChunkIndices() < 1 || currentOptimisedGeometry->GetNumVertexIndices() < 1)
						continue;

					D3DDevice->SetStreamSource(0, currentGeometry->GetVertexBuffer(), 0, currentGeometry->GetVertexSize());
					D3DDevice->SetIndices(currentOptimisedGeometry->GetIndexBuffer());

					if (currentGeometry->GetUseVertexDeclarationFormat())
					{
						if (!currentGeometry->GetVertexDeclaration())//>m_pVertexDeclaration)
							continue;

						D3DDevice->SetVertexDeclaration(currentGeometry->GetVertexDeclaration());
					}
					else
						D3DDevice->SetFVF(currentGeometry->GetVertexFormat());

					if (facesDrawn)
					{
						delete[] facesDrawn;
						facesDrawn = NULL;
					}

					facesDrawn = new bool[currentOptimisedGeometry->GetNumChunks()];
					ZeroMemory(facesDrawn, sizeof(bool) * currentOptimisedGeometry->GetNumChunks());
				}
				else
				{
					if (currentGeometry->GetNumChunks() < 1 || currentGeometry->GetNumVertices() < 1 ||
						(currentGeometry->GetNumChunkIndices() < 1 && !(currentGeometry->GetIndexGenerationFlags() & GF_ALLOWZEROCHUNKINDICES)) || 
						(currentGeometry->GetNumVertexIndices() < 1 && !(currentGeometry->GetIndexGenerationFlags() & GF_ALLOWZEROVERTEXINDICES)))
						continue;

					D3DDevice->SetStreamSource(0, currentGeometry->GetVertexBuffer(), 0, currentGeometry->GetVertexSize());
					D3DDevice->SetIndices(currentGeometry->GetIndexBuffer());

					if (currentGeometry->GetUseVertexDeclarationFormat())
					{
						if (!currentGeometry->GetVertexDeclaration())//>m_pVertexDeclaration)
							continue;

						D3DDevice->SetVertexDeclaration(currentGeometry->GetVertexDeclaration());
					}
					else
						D3DDevice->SetFVF(currentGeometry->GetVertexFormat());

					if (facesDrawn)
					{
						delete[] facesDrawn;
						facesDrawn = NULL;
					}

					facesDrawn = new bool[currentGeometry->GetNumChunks()];
					ZeroMemory(facesDrawn, sizeof(bool) * currentGeometry->GetNumChunks());
				}
			}

			// Setup currentEffect
			currentEffect = r->m_effectID;

			if (currentEffect == -1)
				currentEffect = vps.effectID;

			UINT numPasses = 1;

			PEFFECT effect = NULL;

			effect = resourceManager->GetEffect(currentEffect);

			if (r->m_hasTransform)
			{
				if (world != r->m_transform)
				{
					D3DDevice->SetTransform(D3DTS_WORLD, &r->m_transform);
				}

				if (effect && effect->m_hasWorld)
					effect->SetValue(_T("world"), &r->m_transform, sizeof(Matrix));
			}

			if (r->m_pScene)
			{
				for (int lightID = 0; lightID < 8; lightID++)
				{
					int id = r->m_pScene->GetLightID(lightID);

					if (id > -1)
					{
						const D3DLIGHT9 *pLight = resourceManager->GetLight(id);
						D3DDevice->SetLight(lightID, pLight);
						D3DDevice->LightEnable(lightID, TRUE);

						char buf[64];

						if (effect->HasLightPosition(lightID))
						{
							sprintf(buf, "lightPosition%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Position, sizeof(Vector3));
						}

						if (effect->HasLightDirection(lightID))
						{
							sprintf(buf, "lightDirection%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Direction, sizeof(Vector3));
						}

						if (effect->HasLightDiffuse(lightID))
						{
							sprintf(buf, "lightDiffuse%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Diffuse, sizeof(Vector4));
						}

						if (effect->HasLightAmbient(lightID))
						{
							sprintf(buf, "lightAmbient%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Ambient, sizeof(Vector4));
						}

						if (effect->HasLightSpecular(lightID))
						{
							sprintf(buf, "lightSpecular%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Specular, sizeof(Vector4));
						}

						if (effect->HasLightRange(lightID))
						{
							sprintf(buf, "lightRange%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Range, sizeof(float));
						}

						if (effect->HasLightFalloff(lightID))
						{
							sprintf(buf, "lightFalloff%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Falloff, sizeof(float));
						}

						if (effect->HasLightAttenuation(lightID))
						{
							Vector3 attenuation(pLight->Attenuation0, pLight->Attenuation1, pLight->Attenuation2);
							sprintf(buf, "lightAttenuation%d", lightID);
							effect->SetValue(buf, (LPVOID)&attenuation, sizeof(Vector3));
						}

						if (effect->HasLightTheta(lightID))
						{
							sprintf(buf, "lightTheta%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Theta, sizeof(float));
						}

						if (effect->HasLightPhi(lightID))
						{
							sprintf(buf, "lightPhi%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Phi, sizeof(float));
						}

						if (effect->HasLightType(lightID))
						{
							sprintf(buf, "lightType%d", lightID);
							effect->SetValue(buf, (LPVOID)&pLight->Type, sizeof(DWORD));
						}
					}
					else
						D3DDevice->LightEnable(lightID, FALSE);
				}
			}

			effect->GetD3DEffect()->Begin(&numPasses, 0);

			for (UINT p = 0; p < numPasses; p++)
			{
				if (currentOptimisedGeometry)
				{
					ZeroMemory(facesDrawn, sizeof(bool) * currentOptimisedGeometry->GetNumChunks());
				}
				else
				{
					ZeroMemory(facesDrawn, sizeof(bool) * currentGeometry->GetNumChunks());
				}

				effect->GetD3DEffect()->BeginPass(p);

				for (unsigned int f = r->m_startFaceIndex; f < r->m_startFaceIndex + r->m_numFaces; f++)
				{
					int chunkIndex = 0;

					if (currentOptimisedGeometry)
					{
						chunkIndex = currentOptimisedGeometry->GetChunkIndex(f);

						//DebugPrintf("currentOptimisedGeometry->chunkIndex = %d\n", chunkIndex);

						/*if (chunkIndex < 0)
						{
							DebugPrintf("A kaput optimisedGeometry is found for f = %d, chunkIndex = %d,\n"
							"where r->m_startFaceIndex = %d and r->m_numFaces = %d\n"
							"m_chunks.size() = %d\n"
							"m_pChunkIndices = 0x%8.8x\n"
							"m_pIndexBuffer = 0x%8.8x\n"
							"m_numChunkIndices = %d\n"
							"m_numVertexIndices = %d\n"
							"m_indexStride = %d\n\n",
							f, chunkIndex, r->m_startFaceIndex, r->m_numFaces,
							(int)currentOptimisedGeometry->m_chunks.size(), (DWORD)currentOptimisedGeometry->m_pChunkIndices, 
							(DWORD)currentOptimisedGeometry->m_pIndexBuffer, (int)currentOptimisedGeometry->m_numChunkIndices, 
							(int)currentOptimisedGeometry->m_numVertexIndices, (int)currentOptimisedGeometry->m_indexStride);
						}*/
					}
					else
					{
						chunkIndex = currentGeometry->GetChunkIndex(f);

						//DebugPrintf("currentGeometry->chunkIndex = %d\n", chunkIndex);
					}

					if (facesDrawn[chunkIndex])
						continue;

					/*if (chunkIndex < 0)
					{
						DebugPrintf("About to defy logic...\n");
					}*/

					facesDrawn[chunkIndex] = true;

					const Chunk &chunk = currentOptimisedGeometry ? currentOptimisedGeometry->GetChunk(chunkIndex) : 
										 currentGeometry->GetChunk(chunkIndex);

					if (currentEffect != chunk.effect || effect->m_currentTexture0 != chunk.idTexture0)
					{
						if (effect->m_hasTexture0)
						{
							if (chunk.idTexture0 > -1)
								effect->GetD3DEffect()->SetTexture(_T("texture0"), resourceManager->GetTexture(chunk.idTexture0));
							else
								effect->GetD3DEffect()->SetTexture(_T("texture0"), NULL);
						}

						effect->m_currentTexture0 = chunk.idTexture0;

						effect->GetD3DEffect()->CommitChanges();
					}

					if (currentEffect != chunk.effect || effect->m_currentTexture1 != chunk.idTexture1)
					{
						if (effect->m_hasTexture1)
						{
							if (chunk.idTexture1 > -1 && effect->m_hasTexture1)
								effect->GetD3DEffect()->SetTexture(_T("texture1"), resourceManager->GetTexture(chunk.idTexture1));
							else
								effect->GetD3DEffect()->SetTexture(_T("texture1"), NULL);
						}

						effect->m_currentTexture1 = chunk.idTexture1;

						effect->GetD3DEffect()->CommitChanges();
					}

					if (currentEffect != chunk.effect || currentMaterial != chunk.material)
					{
						const D3DMATERIAL9 *pMat = resourceManager->GetMaterial(chunk.material);

						D3DDevice->SetMaterial(pMat);

						currentMaterial = chunk.material;
					}

					if (currentOptimisedGeometry)
					{
						if (FAILED(RenderChunk(true, currentGeometry->GetNumVertices(), &chunk)))
							return E_FAIL;
					}
					else
					{
						if (FAILED(RenderChunk(currentGeometry->GetNumVertexIndices() > 0, currentGeometry->GetNumVertices(), &chunk)))
							return E_FAIL;
					}
				}

				effect->GetD3DEffect()->EndPass();
			}

			if (r->m_hasTransform)
			{
				D3DDevice->SetTransform(D3DTS_WORLD, &world);
			}

			if (effect->m_hasWorld)
				effect->SetValue(_T("world"), &world, sizeof(Matrix));

			effect->GetD3DEffect()->End();
		}

		if (facesDrawn)
		{
			delete[] facesDrawn;
			facesDrawn = NULL;
		}

		return S_OK;
	}

/*
//-----------------------------------------------------------------------
	HRESULT Renderer::RenderChunk(Geometry *geometry, Chunk *chunk)
	//-------------------------------------------------------------------
	{
		if (!geometry || !chunk)
			return E_FAIL;

		if (!chunk->render)
			return S_OK;

		unsigned int numVerts = geometry->GetNumVertices();

		if (geometry->GetIndexGenerationFlags() & GF_ALLOWZEROVERTEXINDICES && geometry->GetNumVertexIndices() < 1)
		{
			switch (chunk->type)
			{
			case CT_POLYGON:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, chunk->startIndex, chunk->numIndices - 2)))
					return E_FAIL;
			break;

			case CT_TRIANGLELIST:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, chunk->startIndex, chunk->numIndices / 3)))
					return E_FAIL;
			break;

			case CT_TRIANGLESTRIP:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, chunk->startIndex, chunk->numIndices - 2)))
					return E_FAIL;
			break;

			case CT_LINELIST:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_LINELIST, chunk->startIndex, chunk->numIndices / 2)))
					return E_FAIL;
			break;

			case CT_LINESTRIP:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_LINESTRIP, chunk->startIndex, chunk->numIndices - 1)))
					return E_FAIL;
			break;

			case CT_POINTLIST:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_POINTLIST, chunk->startIndex, chunk->numVerts)))
					return E_FAIL;
			break;
			}
		}
		else
		{
			switch (chunk->type)
			{
			case CT_POLYGON:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, numVerts, chunk->startIndex, chunk->numIndices - 2)))
					return E_FAIL;
			break;

			case CT_TRIANGLELIST:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVerts, chunk->startIndex, chunk->numIndices / 3)))
					return E_FAIL;
			break;

			case CT_TRIANGLESTRIP:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, numVerts, chunk->startIndex, chunk->numIndices - 2)))
					return E_FAIL;
			break;

			case CT_LINELIST:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, numVerts, chunk->startIndex, chunk->numIndices / 2)))
					return E_FAIL;
			break;

			case CT_LINESTRIP:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, numVerts, chunk->startIndex, chunk->numIndices - 1)))
					return E_FAIL;
			break;

			case CT_POINTLIST:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_POINTLIST, chunk->startIndex, chunk->numVerts)))
					return E_FAIL;
			break;
			}
		}

		return S_OK;
	}*/


//-----------------------------------------------------------------------
	HRESULT Renderer::RenderChunk(bool useIndices, unsigned int numVerts, const Chunk *chunk)
	//-------------------------------------------------------------------
	{
		if (!chunk)
			return E_FAIL;

		if (!chunk->render)
			return S_OK;

		if (useIndices)
		{
			switch (chunk->type)
			{
			case CT_POLYGON:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, numVerts, chunk->startIndex, chunk->numIndices - 2)))
					return E_FAIL;
			break;

			case CT_TRIANGLELIST:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, numVerts, chunk->startIndex, chunk->numIndices / 3)))
					return E_FAIL;
			break;

			case CT_TRIANGLESTRIP:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, numVerts, chunk->startIndex, chunk->numIndices - 2)))
					return E_FAIL;
			break;

			case CT_LINELIST:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, numVerts, chunk->startIndex, chunk->numIndices / 2)))
					return E_FAIL;
			break;

			case CT_LINESTRIP:
				if (FAILED(D3DDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, numVerts, chunk->startIndex, chunk->numIndices - 1)))
					return E_FAIL;
			break;

			case CT_POINTLIST:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_POINTLIST, chunk->startIndex, chunk->numVerts)))
					return E_FAIL;
			break;
			}
		}
		else
		{
			switch (chunk->type)
			{
			case CT_POLYGON:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, chunk->startIndex, chunk->numIndices - 2)))
					return E_FAIL;
			break;

			case CT_TRIANGLELIST:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, chunk->startIndex, chunk->numIndices / 3)))
					return E_FAIL;
			break;

			case CT_TRIANGLESTRIP:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, chunk->startIndex, chunk->numIndices - 2)))
					return E_FAIL;
			break;

			case CT_LINELIST:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_LINELIST, chunk->startIndex, chunk->numIndices / 2)))
					return E_FAIL;
			break;

			case CT_LINESTRIP:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_LINESTRIP, chunk->startIndex, chunk->numIndices - 1)))
					return E_FAIL;
			break;

			case CT_POINTLIST:
				if (FAILED(D3DDevice->DrawPrimitive(D3DPT_POINTLIST, chunk->startIndex, chunk->numVerts)))
					return E_FAIL;
			break;
			}
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::PreRender(LPDIRECT3DSWAPCHAIN9 pSwapChain, PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!pViewport)
			return E_FAIL;

		ViewportSettings vps;
		pViewport->GetSettings(&vps);

		HRESULT hr = D3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, vps.bgColour, 1.0f, 0);

		if (FAILED(hr))
		{
			Logf("Error: PreRender() failed.");

			if (hr == D3DERR_DEVICELOST)
				CatchLostDevice();
			else if (hr == D3DERR_DRIVERINTERNALERROR)
				Logf("Error: Internal Driver Error.");
			else if (hr == D3DERR_OUTOFVIDEOMEMORY)
				Logf("Error: Out of Video Memory.");

			return E_FAIL;
		}

		Viewport::DrawDCCallback pFnPreDCCallback = pViewport->GetPreDrawCallback();

		if (pFnPreDCCallback)
		{
			LPDIRECT3DSURFACE9 pBackBuffer = NULL;

			if (pSwapChain)
			{
				if (FAILED(pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
					return E_FAIL;
			}
			else
			{
				if (FAILED(D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
					return E_FAIL;
			}

			if (pBackBuffer)
			{
				HDC dc;
				D3DSURFACE_DESC desc;

				if (FAILED(pBackBuffer->GetDesc(&desc)))
				{
					pBackBuffer->Release();

					return E_FAIL;
				}

				if (FAILED(pBackBuffer->GetDC(&dc)))
				{
					pBackBuffer->Release();

					return E_FAIL;
				}

				RECT rect;
				rect.left = 0;
				rect.top = 0;
				rect.right = desc.Width;
				rect.bottom = desc.Height;

				pFnPreDCCallback(dc, &rect, pViewport->GetPreDrawData());

				pBackBuffer->ReleaseDC(dc);
				pBackBuffer->Release();
			}
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::Present(LPDIRECT3DSWAPCHAIN9 pSwapChain, PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		//Logf("Present()");

		if (pViewport)
		{
			Viewport::DrawDCCallback pFnPostDCCallback = pViewport->GetPostDrawCallback();

			if (pFnPostDCCallback)
			{
				LPDIRECT3DSURFACE9 pBackBuffer = NULL;

				if (pSwapChain)
				{
					if (FAILED(pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
						return E_FAIL;
				}
				else
				{
					if (FAILED(D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
						return E_FAIL;
				}
				
				if (pBackBuffer)
				{
					HDC dc;
					D3DSURFACE_DESC desc;

					if (FAILED(pBackBuffer->GetDesc(&desc)))
					{
						pBackBuffer->Release();

						return E_FAIL;
					}

					if (FAILED(pBackBuffer->GetDC(&dc)))
					{
						pBackBuffer->Release();

						return E_FAIL;
					}

					RECT rect;
					rect.left = 0;
					rect.top = 0;
					rect.right = desc.Width;
					rect.bottom = desc.Height;

					pFnPostDCCallback(dc, &rect, pViewport->GetPostDrawData());

					pBackBuffer->ReleaseDC(dc);
					pBackBuffer->Release();
				}
			}
		}

		HRESULT hr;

		if (pSwapChain)
		{
			hr = pSwapChain->Present(0, 0, 0, 0, 0);
		}
		else if (D3DDevice)
		{
			hr = D3DDevice->Present(0, 0, 0, 0);
		}

		if (FAILED(hr))
		{
			Logf("Error: Present() failed.");

			if (hr == D3DERR_DEVICELOST)
				CatchLostDevice();
			else if (hr == D3DERR_DRIVERINTERNALERROR)
				Logf("Error: Internal Driver Error.");
			else if (hr == D3DERR_OUTOFVIDEOMEMORY)
				Logf("Error: Out of Video Memory.");

			return E_FAIL;
		}

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Renderer::SetFullscreen(PVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		pFullscreenViewport = pViewport;

		/*std::vector< SwapChainSettings > swapChainSettings;

		std::list<PVIEWPORT>::iterator i = viewports.begin();
		for (; i != viewports.end(); i++)
		{
			if (!(*i))
				continue;

			D3DPRESENT_PARAMETERS d3dpp;

			(*i)->GetSwapChain()->GetSwapChain()->GetPresentParameters(&d3dpp);

			swapChainSettings.push_back(std::make_pair< PVIEWPORT, D3DPRESENT_PARAMETERS >(*i, d3dpp));

			(*i)->GetSwapChain()->Release();
			(*i)->SetSwapChain(NULL);
		}*/

		/*if (D3DDevice)
		{
			D3DDevice->Release();
			D3DDevice = NULL;
		}*/

		/*if (FAILED(CreateDeviceAndSwapChains(&swapChainSettings)))
			return E_FAIL;*/

		if (FAILED(CreateDeviceAndSwapChains()))
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	DWORD Renderer::TestDevice()
	//-------------------------------------------------------------------
	{
		if (!D3DDevice)
			return 0;

		HRESULT hr = D3DDevice->TestCooperativeLevel();

		if (hr == D3DERR_DEVICELOST)
		{
			return DS_LOST;
		}
		if (hr == D3DERR_DEVICENOTRESET)
		{
			return DS_NOTRESET;
		}

		return DS_READY;
	}

// EOF