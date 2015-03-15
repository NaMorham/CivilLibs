/*! \file */
/*-----------------------------------------------------------------------
    interface.h

    Description: Simulation & Rendering User API class definition
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

//typedef std::list<PSCENE> SceneList;
typedef std::vector<RenderGroup> RenderList;

//! \struct Interface
struct Interface
{
private:
    typedef void (*ProgressCallbackFunc)(DWORD, float);

public:
    Interface(HWND hWnd, PCORESETTINGS pCoreSettings, LPCTSTR logFilename, int maxLogSize);
    ~Interface();

    // Creation
        //! \brief Creates a Viewport
        /*! Returns S_OK on success and E_FAIL on failure.
            \param hWnd Handle to the window the Viewport will use to draw to
            \param pSettings [in] Pointer to a ViewportSettings structure used to initialise the Viewport.
            If this value is NULL, a default structure will be used.
            \param pViewport [out] Address of a pointer to a Viewport structure, representing the created viewport.*/
        HRESULT CreateViewport(HWND hWnd, PVIEWPORTSETTINGS pSettings, PVIEWPORT *pViewport);

        //! \brief Creates a Scene
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pSettings [in] Pointer to a SceneSettings structure to initialise the Scene.
            If this value is NULL, a default structure will be used.
            \param pVisual [out] Address of a pointer to a Scene structure, representing the created scene. */
        HRESULT CreateScene(PSCENESETTINGS pSettings, PSCENE *pScene);

        //! \brief Creates a Visual
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pVisual [out] Address of a pointer to a Visual structure, representing the created visual.
            \param pID [out] Pointer to a unique id representing the resulting index to this visual. */
        HRESULT CreateVisual(PVISUAL *pVisual, int *pID);

        //! \brief Creates an Entity
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pEntity [out] Address of a pointer to a Entity structure, representing the created entity.*/
        HRESULT CreateEntity(PENTITY *pEntity);

        //! \brief Creates a Geometry object
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pGeometry [out] Address of a pointer to a Geometry structure, representing the created Geometry object.*/
        HRESULT CreateGeometry(PGEOMETRY *pGeometry);

#if 0
        //! \brief Creates a patch
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pPatch [out] Address of a pointer to a Patch structure, representing the created patch.*/
        HRESULT CreatePatch(PPATCH *pPatch);
#endif

        //! \brief Creates a Serialiser object
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pGeometry [out] Address of a pointer to a Serialiser structure, representing the created Serialiser object.*/
        HRESULT CreateSerialiser(PSERIALISER *pSerialiser);

        //! \brief Creates an effect
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pFXFile [in] Address of a string containing the filename of the FX file to generate the effect from.
                   May be NULL if pFXCode is not NULL.
            \param pFXCode [in] Address of a buffer containing the FX code to generate the effect from.
                   May be NULL if pFXFile is not NULL.
            \param codeLength Length of the buffer referenced by pFXCode, in bytes. May be NULL if pFXCode is also NULL.
            \param pID [out] Pointer to a unique id representing the resulting index to this effect. May be NULL.
            \param pErrorBuf [out] Optional pointer to an STL string to receive the error text if the effect fails to load */
        HRESULT CreateEffect(LPCTSTR pFXFile, LPCTSTR pFXCode, int codeLength, int *pID,
                             LPCTSTR pDefaultTechnique = NULL, String *pErrorBuf = NULL);

        //! \brief Creates a Texture
        /*! Returns S_OK on success and E_FAIL on failure.
            \param width Width, in pixels of the resulting texture.
            \param height Height, in pixels of the resulting texture.
            \param pBits [in] Pointer to the bits used to fill the resulting texture.
            \param flags Flags used to create this texture.
            \param pID [out] Pointer to a unique id representing the resulting index to this texture. */
        HRESULT CreateTexture(int width, int height, int *pBits, DWORD flags, int *pID);

        HRESULT InsertLight(const Light &light, int *pID);

        //! \brief Creates a Cube Texture
        /*! Returns S_OK on success and E_FAIL on failure.
            \param width Width, in pixels of the resulting texture.
            \param height Height, in pixels of the resulting texture.
            \param pBits [in] Pointer to the bits used to fill the resulting texture.
            \param flags Flags used to create this texture.
            \param pID [out] Pointer to a unique id representing the resulting index to this texture. */
        //HRESULT CreateCubeTexture(int width, int *pBits, DWORD flags, int *pID);

        //! \brief Creates a Volume Texture
        /*! Returns S_OK on success and E_FAIL on failure.
            \param width Width, in texels of the resulting texture.
            \param height Height, in pixels of the resulting texture.
            \param depth Depth, in pixels of the resulting texture.
            \param pBits [in] Pointer to the bits used to fill the resulting texture.
            \param flags Flags used to create this texture.
            \param pID [out] Pointer to a unique id representing the resulting index to this texture. */
        //HRESULT CreateVolumeTexture(int width, int height, int depth, int *pBits, DWORD flags, int *pID);

        //! \brief Creates a Material
        /*! Returns S_OK on success and E_FAIL on failure.
            \param diffuse Diffuse Colour, in 32-bit ARGB format.
            \param ambient Ambient Colour, in 32-bit ARGB format.
            \param specular Specular Colour, in 32-bit ARGB format.
            \param emmisive Emissive Colour, in 32-bit ARGB format.
            \param specularPower A floating-point value specifying the sharpness of specular highlights.
            \param pID [out] Pointer to a unique id representing the resulting index to this material. */
        HRESULT CreateMaterial(DWORD diffuse, DWORD ambient, DWORD specular, DWORD emissive, float specularPower, int *pID);

        //! \brief Creates a Font
        /*! Returns S_OK on success and E_FAIL on failure.
            \param height Height of the font, in pixels.
            \param width Width of the font, in pixels.
            \param weight Font weight.
            \param italic Italic option.
            \param charSet Font character set.
            \param pitchAndFamily Pitch and Family.
            \param pFaceName Typeface name.
            \param pFont [out] Address of a pointer to a Font structure, representing the created font.*/
        HRESULT CreateFont(int height, int width, int weight, bool italic,
                           DWORD charSet, DWORD pitchAndFamily, LPCTSTR pFaceName, PFONT *pFont);

        //! \brief Destroys a scene
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pScene Pointer to the scene to be deleted. */
        HRESULT DestroyScene(PSCENE pScene);

        //! \brief Destroys a viewport
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pViewport index of the viewport to be deleted.*/
        HRESULT DestroyViewport(PVIEWPORT pViewport);

        //! \brief Destroys a texture
        /*! Returns S_OK on success and E_FAIL on failure.
            \param textureID Index of the texture to be deleted.*/
        HRESULT DestroyTexture(int textureID);

        //! \brief Destroys a visual
        /*! Returns S_OK on success and E_FAIL on failure.
            \param visualID Index of the visual to be deleted.*/
        HRESULT DestroyVisual(int visualID);

        //! \brief Destroys an effect
        /*! Returns S_OK on success and E_FAIL on failure.
            \param effectID Index of the effectto be deleted.*/
        HRESULT DestroyEffect(int effectID);

        //! \brief Sets the technique of a given effect.
        /*! Returns S_OK on success and E_FAIL on failure.
            \param effectID Index of the effect.
            \param handle Handle of the technique to set.*/
        HRESULT SetEffectTechnique(int effectID, const String &handle);

        //! \brief Validates a given effect technique.
        /*! Returns S_OK on success and E_FAIL on failure.
            \param effectID Index of the effect.
            \param handle Handle of the technique to set.*/
        HRESULT ValidateEffectTechnique(int effectID, const String &handle);

        //! \brief Sets a value on a given effect.
        /*! Returns S_OK on success and E_FAIL on failure.
            \param effectID Index of the effect.
            \param handle Handle of the value to set.
            \param pData Pointer to a buffer containing the data to copy.
            \param size Size of the buffer referenced by pData.*/
        HRESULT SetEffectValue(int effectID, const String &handle, LPVOID pData, int size);

        //! \brief Sets the light parameters for a given light on a given effect.
        /*! Returns S_OK on success and E_FAIL on failure.
            \param effectID Index of the effect.
            \param lightID Index of this light used by this effect.
            \param pLight Pointer to a Light structure, containing the light data to set.*/
        HRESULT SetEffectLight(int effectID, int lightID, PLIGHT pLight);

        //! \brief Sets the material parameters for a given material on a given effect.
        /*! Returns S_OK on success and E_FAIL on failure.
            \param effectID Index of the effect.
            \param materialID Index of this material used by this effect..*/
        HRESULT SetEffectMaterial(int effectID, int materialID);

        const StringArray *GetEffectTechniqueNames(int effectID);

        //! \brief Retrieves the Log
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pBegin Pointer to an iterator representing the first element in the list.
            \param pEnd Pointer to an iterator representing the end of the list.*/
        HRESULT GetLog(StringList::iterator *pBegin, StringList::iterator *pEnd);

        //! \brief Locks and retrieves access to texture data
        /*! Returns S_OK on success and E_FAIL on failure.
            \param id Unique id representing the index to this texture.
            \param pWidth [out] Pointer to the width, in pixels of the texture.
            \param pHeight [out] Pointer to the height, in pixels of the texture.
            \param pPitch [out] Pointer to the pitch, in bytes of the texture.
            \param pBits [out] Pointer to the locked bits of the texture. */
        HRESULT LockTexture(int id, int *pWidth, int *pHeight, int *pPitch, void **pBits);

        //! \brief Unlocks a Texture
        /*! Returns S_OK on success and E_FAIL on failure.
            \param id Unique id representing the index to the texture to unlock.*/
        HRESULT UnlockTexture(int id);

        //! \brief Loads a texture from a file
        /*! Returns S_OK on success and E_FAIL on failure.
            \param filename [in] Pointer to a string, representing the filename of the texture to open.
            \param pID [out] Pointer to a unique id, representing the resulting index to this texture. */
        HRESULT LoadTextureFromFile(LPCTSTR filename, int *pID);

        //! \brief Loads a cube texture from a file
        /*! Returns S_OK on success and E_FAIL on failure.
            \param filename [in] Pointer to a string, representing the filename of the texture to open.
            \param pID [out] Pointer to a unique id, representing the resulting index to this texture. */
        HRESULT LoadCubeTextureFromFile(LPCTSTR filename, int *pID);

        //! \brief Loads a volume texture from a file
        /*! Returns S_OK on success and E_FAIL on failure.
            \param filename [in] Pointer to a string, representing the filename of the texture to open.
            \param pID [out] Pointer to a unique id, representing the resulting index to this texture. */
        HRESULT LoadVolumeTextureFromFile(LPCTSTR filename, int *pID);

    // Manipulation
        //! \brief Sets a texture render target
        /*! Returns S_OK on success and E_FAIL on failure.
            \param textureID index of the texture to render to. -1 to set viewport chain.*/
        HRESULT SetRenderTarget(int textureID);

        //! Resizes a Viewport
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pViewport [in] Pointer to the KSRViewport structure to resize.*/
        HRESULT ResizeViewport(PVIEWPORT pViewport);

        //! Clears a Viewport to its default colour
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pViewport [in] Pointer to the KSRViewport structure to clear.*/
        HRESULT ClearViewport(PVIEWPORT pViewport);

        //! Prepares a Viewport for rendering
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pViewport [in] Pointer to the KSRViewport structure to prepare.*/
        HRESULT PrepareViewport(PVIEWPORT pViewport);

        //! Pauses/Unpauses a Viewport
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pViewport [in] Pointer to the KSRViewport structure to clear.*/
        HRESULT PauseViewport(PVIEWPORT pViewport, bool paused);

        //! \brief Updates Settings
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pSettings [in] Pointer to a KSRCoreSettings structure, representing the new settings.*/
        HRESULT SetSettings(PCORESETTINGS settings);

        //! \brief Starts the simulation timer
        /*! Returns S_OK on success and E_FAIL on failure. */
        HRESULT StartSimulation();

        //! \brief Stops the simulation timer
        /*! Returns S_OK on success and E_FAIL on failure. */
        HRESULT StopSimulation();

        //! \brief Steps the simulation timer by the amount specified in timeDelta
        /*! Returns S_OK on success and E_FAIL on failure. */
        HRESULT StepSimulation(float timeDelta);

        //! \brief Querys the time the simulation has been running
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pTime [out] Pointer to a float to be filled with the time data. */
        HRESULT QueryTime(float *pTime);

        //! \brief Querys the time since the last update
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pTimeDelta [out] Pointer to a float to be filled with the time data. */
        HRESULT QueryTimeDelta(float *pTimeDelta);

        //! \brief Querys the number of times Update() was called last second.
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pFPS [out] Pointer to a float to be filled with the time data. */
        HRESULT QueryFramesPerSecond(float *pFPS);

        //! \brief Performs diagnostics and posts and logs the results.
        /*! Returns S_OK on success and E_FAIL on failure. */
        HRESULT LogDiagnostic();

    // Main
        //! \brief Sets the Progress Bar Callback function.
        /*! Returns S_OK on success and E_FAIL on failure. */
        HRESULT SetProgressCallback(ProgressCallbackFunc progressCallbackFunc);

        //! \brief Updates the scenes
        /*! Returns S_OK on success and E_FAIL on failure. */
        HRESULT Update();

        //! \brief Selects the viewport to render to within BeginRender() and EndRender() calls.
        /*! Returns S_OK on success and E_FAIL on failure. */
        HRESULT SelectSourceViewport(PVIEWPORT pViewport);

        //! \brief Begins rendering to a given viewport.
        /*! Returns S_OK on success and E_FAIL on failure.*/
        HRESULT BeginRender(PVIEWPORT pViewport);

        //! \brief Ends rendering to a given viewport.
        /*! Returns S_OK on success and E_FAIL on failure.*/
        HRESULT EndRender();

        //! \brief Render a list of render groups to the viewport specified to BeginRender(PVIEWPORT).
        /*! Returns S_OK on success and E_FAIL on failure.*/
        HRESULT Render(RenderList &renderList);

        //! \brief Renders all scenes to all viewports
        /*! Returns S_OK on success and E_FAIL on failure.*/
        HRESULT Render();

        //! \brief Renders the specified scene BeginRender() and EndRender() calls.
        /*! Returns S_OK on success and E_FAIL on failure.*/
        HRESULT Render(PSCENE pScene);

        //! \brief Renders a specified scene to a specified viewport..
        /*! Returns S_OK on success and E_FAIL on failure.
            \param pScene [in] Pointer to a Scene structure, representing the Scene to render.
            May not be NULL.
            \param pViewport [in] Pointer to a Viewport structure, representing the Viewport to render to.
            May not be NULL.*/
        HRESULT Render(PSCENE pScene, PVIEWPORT pViewport);

        //! \brief Renders a list of scenes to a viewport
        /*! Returns S_OK on success and E_FAIL on failure.
            \paran numScenes number of Scene structures contained in pScenes. If 0, all scenes are used.
            \param ppScenes [in] Pointer to an array of KSRScene structure, representing the list of scenes to render..
            Must be NULL if numScenes is 0.
            \param pViewport [in] Pointer to a KSRViewport structure, representing the Viewport to render to.*/
        HRESULT Render(int numScenes, PSCENE *ppScenes, PVIEWPORT pViewport);

        HRESULT SetFullscreen(PVIEWPORT pViewport);

        D3DLIGHT9 *GetLight(int index);

        //! \brief Gets the number of Scenes.
        /*! Returns the number of scenes stored by this Interface. */
        DWORD GetNumScenes();

        //! \brief Gets the number of Visuals.
        /*! Returns the number of visuals stored by this Interface. */
        DWORD GetNumVisuals();

        //! \brief Gets the number of Textures.
        /*! Returns the number of textures stored by this Interface. */
        DWORD GetNumTextures();

        //! \brief Gets the number of Materials.
        /*! Returns the number of materials stored by this Interface. */
        DWORD GetNumMaterials();

        //! \brief Gets the number of Effects.
        /*! Returns the number of effects stored by this Interface. */
        DWORD GetNumEffects();

        //! \brief Tests the state of the rendering device.
        /*! Returns true if the device is ready to render, false if it is not. */
        DWORD TestRenderingDevice();

        //! \brief Attempts to reset the rendering device.
        /*! Returns S_OK on success and E_FAIL on failure. */
        HRESULT ResetRenderingDevice();

    // Accesssors
        PRENDERER         GetRenderer()          { return m_pRenderer; }
        PRESOURCEMANAGER GetResourceManager() { return m_pResourceManager; }

/*
#ifdef _DEBUG
#ifdef _DEBUGID
    KSR::LWIDServer<unsigned int> *pServer;
#endif
#endif
//*/

private:
    Logger                m_logger;
    PRENDERER            m_pRenderer;
    PRESOURCEMANAGER    m_pResourceManager;
    PCORESETTINGS        m_pSettings;
    PTIMER                m_pTimer;

    SceneList            m_scenes;

    ProgressCallbackFunc m_pFnProgressCallback;
};

//! \typedef Interface *PINTERFACE
typedef Interface *PINTERFACE;

// EOF