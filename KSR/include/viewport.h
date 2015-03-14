/*! \file */
/*-----------------------------------------------------------------------
	viewport.h

	Description: Viewport class definition
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

struct TextInstance
{
	String m_text;
	RECT m_rect;
	DWORD m_format, m_colour;
	PFONT m_pFont;
};

//! \struct Viewport
struct Viewport : public MemObject
{
public:
	typedef HRESULT (*DrawDCCallback)(HDC, PRECT, void *);

	Viewport();
	~Viewport();

	HRESULT ZoomExtents(const KSR::Math::AABB &bounds, float bufferRatio);

	void SetFont(PFONT pFont);
	void SetFontFormat(LPRECT pRect, DWORD format, DWORD textColour);

	// Okay, this is bad. Due to a conflict with wx names, DrawText is Drawtext here, despite naming conventions.
	void Drawtext(const String &text, LPRECT pRect, DWORD format, DWORD textColour);
	void Drawtext(const String &text);
	void FlushText();

	//! \brief Adjusts the view volume size of this Viewport by the amount specified in viewSize.
		/*! Only affects Viewports while using an orthographic projection.*/
	void AdjustFOV(float fov);
	//! \brief Adjusts the view volume size of this Viewport by the amount specified in viewSize.
		/*! Only affects Viewports while using an orthographic projection.*/
	void AdjustViewVolume(const Vector2 &viewSize);

	//! \brief Adjusts the field of view of the Viewport by the amount specified in fov.
		/*! Only affects Viewports while using an perspective projection.*/
	void SetCameraCoordSystem(const Vector3 &up, const Vector3 &forward);

	//! \brief Sets the camera position of this Viewport.
		/*! More..*/
	void SetCameraPosition(const Vector3 &position);
	//! \brief Sets the camera rotation of this Viewport.
		/*! Pitch, Yaw, Roll. In Degrees. */
	void SetCameraRotation(const Vector3 &rotation);
	//! \brief Sets the orbit target of this Viewport.
		/*! Target to center on while orbiting. */
	void SetCameraTarget(const Vector3 &target);
	//! \brief Adjusts the field of view of the Viewport by the amount specified in fov.
		/*! Only affects Viewports while using an perspective projection.*/
	//void SetCameraTarget(Entity *target);
	//! \brief Adjusts the field of view of the Viewport by the amount specified in fov.
		/*! Only affects Viewports while using an perspective projection.*/
	void SetCameraMode(DWORD transformMode, DWORD translationMode, DWORD rotationMode, DWORD orbitMode);

	void SetProjectionMatrix(int id, const Matrix &matrix);

	void SetPreDrawData(void *pData);
	void SetPostDrawData(void *pData);

	void SetPreDrawCallback(DrawDCCallback pFnDrawDCCallback);
	void SetPostDrawCallback(DrawDCCallback pFnDrawDCCallback);

	void *GetPreDrawData() const;
	void *GetPostDrawData() const;

	DrawDCCallback GetPreDrawCallback() const;
	DrawDCCallback GetPostDrawCallback() const;

	//! \brief Sets the camera rotation of this Viewport to look at a point.
		/*! More .*/
	void CameraLookAt(const Vector3 &target);
	//! \brief Transforms the camera
		/*! More.*/
	void CameraTransform(int type, const Vector3 &transform);

	//! \brief Creates the interface
		/*! More.*/
	void SetRedraw(DWORD r) { m_redraw = r; };
	//! \brief Creates the interface
		/*! More.*/
	void SetSettings(PVIEWPORTSETTINGS settings);
	//! \brief Returns the current camera orbit target
		/*! More.*/
	DWORD GetCameraTransformMode();
	//! \brief Returns the current camera position
		/*! More.*/
	const Vector3 &GetCameraPosition();
	//! \brief Returns the current camera position
		/*! More.*/
	const Vector3 GetCameraDirection();
	//! \brief Returns the current camera orbit target
		/*! More.*/
	const Vector3 &GetCameraTarget();

	//! \brief Returns the current camera orbit target
		/*! More.*/
	Matrix GetCameraOrientation();

	//! \brief Returns the current camera forward vector
		/*! More.*/
	const Vector3 &GetCameraForwardVector();

	//! \brief Returns the current camera up vector
		/*! More.*/
	const Vector3 &GetCameraUpVector();

	//! \brief Gets the KSRViewportSettings structure for this Viewport
		/*! Returns a pointer to the KSRViewportSettings structure for this Viewport.*/
	HRESULT GetSettings(PVIEWPORTSETTINGS pSettings);

	void SetCameraFrustum(const Matrix &cameraFrustum);
	void SetHWND(HWND h) { m_hWnd = h; };
	void SetUpdateStates(bool u) { m_updateStates = u; };
	void SetStateBlock(LPDIRECT3DSTATEBLOCK9 s) { m_pStateBlock = s; };

	void SetSwapChain(PSWAPCHAIN pSwapChain);

	HWND GetHWND() { return m_hWnd; };
	PCAMERA GetCamera() { return m_pCamera; };

	//! \brief Gets the current redraw state for this Viewport.
		/*! A.*/
	DWORD GetRedraw() { return m_redraw; };
	bool  GetUpdateStates() { return m_updateStates; };

	bool CheckPointCameraFrustum(const Vector3 &point);
	bool CheckSphereCameraFrustum(const Vector3 &position, float radius);
	bool CheckBoxCameraFrustum(const Vector3 &min, const Vector3 &max);

	LPDIRECT3DSTATEBLOCK9 GetStateBlock() { return m_pStateBlock; };

	Matrix GetCameraFrustum();
	Matrix GetProjectionMatrix(int id);

	PSWAPCHAIN GetSwapChain() { return m_pSwapChain; };

private:
	HWND m_hWnd;
	PCAMERA m_pCamera;
	PSWAPCHAIN m_pSwapChain;

	bool m_updateStates;
	bool m_renderPaused;

	DWORD m_redraw;

	PVIEWPORTSETTINGS m_pSettings;
	LPDIRECT3DSTATEBLOCK9 m_pStateBlock;

	RECT m_textRect;
	DWORD m_textFormat,
		  m_textColour;

	DrawDCCallback m_pFnPreDrawDCCallback;
	DrawDCCallback m_pFnPostDrawDCCallback;
	void *m_pPreDrawData;
	void *m_pPostDrawData;

	PFONT m_pFont;
	std::vector<TextInstance> m_textInstances;

	std::vector<Matrix> m_projections;
};

//! \typedef ViewportSettings *PVIEWPORTSETTINGS
typedef struct Viewport *PVIEWPORT;

// EOF