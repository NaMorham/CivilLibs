/*! \file */
/*-----------------------------------------------------------------------
	renderer.h

	Description: Graphics Management, Drawing/Geometry and Direct3D9 Wrapping
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

struct Renderer : public MemObject
{
public:
	Renderer(PRESOURCEMANAGER pResourceManager);
	~Renderer();

	HRESULT Create(HWND hWnd, CoreSettings *pSettings);
	HRESULT InitDeviceStates();

	HRESULT CreateDeviceAndSwapChains();

	HRESULT CreateViewport(HWND hWnd, PVIEWPORTSETTINGS pSettings, PVIEWPORT *pViewport);
	HRESULT DestroyViewport(PVIEWPORT pViewport);
	HRESULT ResizeViewport(PVIEWPORT pViewport);
	HRESULT ClearViewport(PVIEWPORT pViewport);
	HRESULT PauseViewport(PVIEWPORT pViewport, bool paused);
	HRESULT PrepareViewport(PVIEWPORT pViewport);
	HRESULT SelectSourceViewport(PVIEWPORT pViewport);

	HRESULT SetRenderTarget(int textureID);

	HRESULT Begin(PVIEWPORT pViewport);
	HRESULT End();

	HRESULT Render(SceneList &scenesToRender);
	HRESULT Render(SceneList &scenesToRender, PVIEWPORT pViewport);
	HRESULT Render(PSCENE pScene, PVIEWPORT pViewport);
	HRESULT Render(PSCENE pScene);
	HRESULT Render(std::vector<RenderGroup> &renderList);

	HRESULT SetFullscreen(PVIEWPORT pViewport);

	LPDIRECT3DDEVICE9 GetD3DDevice() { return D3DDevice; };
	D3DCAPS9 GetD3DCaps() { return D3DCaps; };

	DWORD TestDevice();
	HRESULT ResetDevice();

private:
	HRESULT CatchLostDevice();
	HRESULT PrepareViewportRender(PVIEWPORT pViewport);
	HRESULT RenderGeometry(std::vector<RenderGroup> &renderList, PVIEWPORT pViewport);
	//HRESULT RenderChunk(Geometry *geometry, Chunk *chunk);
	HRESULT RenderChunk(bool useIndices, unsigned int numVerts, const Chunk *chunk);
	HRESULT PreRender(LPDIRECT3DSWAPCHAIN9 pSwapChain, PVIEWPORT pViewport);
	HRESULT Present(LPDIRECT3DSWAPCHAIN9 pSwapChain, PVIEWPORT pViewport);

	LPDIRECT3D9		  D3D;
	LPDIRECT3DDEVICE9 D3DDevice;
	D3DDISPLAYMODE	  D3DDisplayMode;
	D3DDEVTYPE		  D3DDeviceType;
	D3DCAPS9		  D3DCaps;

	D3DPRESENT_PARAMETERS presentParameters;

	DWORD processingMode;
	DWORD precisionMode;

	HWND hMainWnd;

	PRESOURCEMANAGER resourceManager;

	D3DFORMAT			colourFormat;
	D3DFORMAT			depthStencilFormat;

	int					bytesPerPixel;
	D3DMULTISAMPLE_TYPE multiSampleAvailable;
	DWORD				numMultiSampleQualityLevels;
	DWORD				vertexShaderVersion;

	int textureTargetID;

	PVIEWPORT		   currentViewport;
	ViewportSettings   currentViewportSettings;
	LPDIRECT3DSURFACE9 currentBackBuffer;
	LPDIRECT3DSURFACE9 currentDeviceBackBuffer;

	bool deviceLost;

	PVIEWPORT pFullscreenViewport;

	std::list<PVIEWPORT> viewports;
};

typedef struct Renderer *PRENDERER;

// EOF