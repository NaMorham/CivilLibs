/*! \file */
/*-----------------------------------------------------------------------
    ksrxfile.h

    Description: Keays Simulation & Rendering Extensions API file & I/O header
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

/*-----------------------------------------------------------------------
    Enumerations
    ---------------------------------------------------------------------*/
    enum
    {
        UT_EF_BREAKLINE  = 0x01,
        UT_EF_XBREAKLINE = 0x02,
        UT_EF_ACTIVE     = 0x04,
        UT_EF_BOUNDARY   = 0x08,
        UT_EF_INTERNAL   = 0x10,
        UT_EF_POLYGON    = 0x20,
        UT_EF_CATCH      = 0x40,
        UT_EF_STREAM     = 0x80,
        UT_EF_CAT_STREAM_BREAK = 0xC1,
    };

    enum
    {
        UT_TF_ACTIVE = 0x01,
    };

    enum
    {
        UT_TRIANGLES = 0x01,
        UT_POINTS    = 0x02,
    };

    enum KRGLUMPS
    {
        KRG_GEOMETRY = 0,
        KRG_SCENE,
        KRG_ENTITY,
    };


/*-----------------------------------------------------------------------
    Structures
    ---------------------------------------------------------------------*/
    struct UTPoint
    {
        double x, y, z;

        const KSR::Vector3 V3() const { return KSR::Vector3( (FLOAT)x, (FLOAT)y, (FLOAT)z ); }
        const KSR::Vector2 V2XY() const { return KSR::Vector2( (FLOAT)x, (FLOAT)y ); }
        const KSR::Vector2 V2XZ() const { return KSR::Vector2( (FLOAT)x, (FLOAT)z ); }
        const KSR::Vector2 V2YZ() const { return KSR::Vector2( (FLOAT)y, (FLOAT)z ); }
    };

    struct UTTriangle
    {
        long links[3];
        char back[3];
        unsigned char layer;
        long vertices[3];
        unsigned char eflags[3];
        unsigned char tflags;
    };


/*-----------------------------------------------------------------------
    Constants
    ---------------------------------------------------------------------*/
    const DWORD UTPalette[256] =
    {
        0xFF000000, 0xFF7EC0EE, 0xFFEFEFEF, 0xFFC0C0C0, 0xFFCD0000, 0xFF008B00, 0xFF0000CD, 0xFF7FC0CD,
        0xFFFF8B00, 0xFFCD8B3F, 0xFFCD8B8B, 0xFFCD3F8B, 0xFFFF008B, 0xFFFFA9A9, 0xFFFF0000, 0xFFC00000,
        0xFF8B0000, 0xFFFFFFA9, 0xFFFFFF00, 0xFFC0C000, 0xFF8B8B00, 0xFFA9FFA9, 0xFF00FF00, 0xFF00C000,
        0xFF008B00, 0xFF00FF8B, 0xFF3FCD8B, 0xFF8BCD8B, 0xFF8BCD3F, 0xFF8BFF00, 0xFFA9FFFF, 0xFF00FFFF,
        0xFF00C0C0, 0xFF008B8B, 0xFFA9A9FF, 0xFF0000FF, 0xFF0000C0, 0xFF00008B, 0xFF8B00FF, 0xFF8B3FCD,
        0xFF8B8BCD, 0xFF3F8BCD, 0xFF008BFF, 0xFFFFA9FF, 0xFFFF00FF, 0xFFC000C0, 0xFF8B008B, 0xFF000000,
        0xFFFFFFFF, 0xFFF3F3F3, 0xFFE6E6E6, 0xFFD8D8D8, 0xFFC9C9C9, 0xFFB9B9B9, 0xFFA9A9A9, 0xFF989898,
        0xFF868686, 0xFF737373, 0xFF5F5F5F, 0xFF4B4B4B, 0xFF373737, 0xFF282828, 0xFF191919, 0xFF0A0A0A,
        0xFFBFBFFF, 0xFFB4B4F0, 0xFFA8A8E1, 0xFF9D9DD2, 0xFF9292C3, 0xFF8787B4, 0xFF7B7BA5, 0xFF707096,
        0xFF656587, 0xFF5A5A78, 0xFF4E4E69, 0xFF43435A, 0xFF38384B, 0xFF2D2D3C, 0xFF21212D, 0xFF16161E,
        0xFFBFDFFF, 0xFFB4D2F0, 0xFFA8C4E1, 0xFF9DB7D2, 0xFF92AAC3, 0xFF879DB4, 0xFF7B90A5, 0xFF708396,
        0xFF657687, 0xFF5A6978, 0xFF4E5B69, 0xFF434E5A, 0xFF38414B, 0xFF2D343C, 0xFF21272D, 0xFF161A1E,
        0xFFBFFFFF, 0xFFB4F0F0, 0xFFA8E1E1, 0xFF9DD2D2, 0xFF92C3C3, 0xFF87B4B4, 0xFF7BA5A5, 0xFF709696,
        0xFF658787, 0xFF5A7878, 0xFF4E6969, 0xFF435A5A, 0xFF384B4B, 0xFF2D3C3C, 0xFF212D2D, 0xFF161E1E,
        0xFFBFFFDF, 0xFFB4F0D2, 0xFFA8E1C4, 0xFF9DD2B7, 0xFF92C3AA, 0xFF87B49D, 0xFF7BA590, 0xFF709683,
        0xFF658776, 0xFF5A7869, 0xFF4E695B, 0xFF435A4E, 0xFF384B41, 0xFF2D3C34, 0xFF212D27, 0xFF161E1A,
        0xFFBFFFBF, 0xFFB4F0B4, 0xFFA8E1A8, 0xFF9DD29D, 0xFF92C392, 0xFF87B487, 0xFF7BA57B, 0xFF709670,
        0xFF658765, 0xFF5A785A, 0xFF4E694E, 0xFF435A43, 0xFF384B38, 0xFF2D3C2D, 0xFF212D21, 0xFF161E16,
        0xFFDFFFBF, 0xFFD2F0B4, 0xFFC4E1A8, 0xFFB7D29D, 0xFFAAC392, 0xFF9DB487, 0xFF90A57B, 0xFF839670,
        0xFF768765, 0xFF69785A, 0xFF5B694E, 0xFF4E5A43, 0xFF414B38, 0xFF343C2D, 0xFF272D21, 0xFF1A1E16,
        0xFFFFFFBF, 0xFFF0F0B4, 0xFFE1E1A8, 0xFFD2D29D, 0xFFC3C392, 0xFFB4B487, 0xFFA5A57B, 0xFF969670,
        0xFF878765, 0xFF78785A, 0xFF69694E, 0xFF5A5A43, 0xFF4B4B38, 0xFF3C3C2D, 0xFF2D2D21, 0xFF1E1E16,
        0xFFFFDFBF, 0xFFF0D2B4, 0xFFE1C4A8, 0xFFD2B79D, 0xFFC3AA92, 0xFFB49D87, 0xFFA5907B, 0xFF968370,
        0xFF877665, 0xFF78695A, 0xFF695B4E, 0xFF5A4E43, 0xFF4B4138, 0xFF3C342D, 0xFF2D2721, 0xFF1E1A16,
        0xFFFFBFBF, 0xFFF0B4B4, 0xFFE1A8A8, 0xFFD29D9D, 0xFFC39292, 0xFFB48787, 0xFFA57B7B, 0xFF967070,
        0xFF876565, 0xFF785A5A, 0xFF694E4E, 0xFF5A4343, 0xFF4B3838, 0xFF3C2D2D, 0xFF2D2121, 0xFF1E1616,
        0xFFFFBFDF, 0xFFF0B4D2, 0xFFE1A8C4, 0xFFD29DB7, 0xFFC392AA, 0xFFB4879D, 0xFFA57B90, 0xFF967083,
        0xFF876576, 0xFF785A69, 0xFF694E5B, 0xFF5A434E, 0xFF4B3841, 0xFF3C2D34, 0xFF2D2127, 0xFF1E161A,
        0xFFFFBFFF, 0xFFF0B4F0, 0xFFE1A8E1, 0xFFD29DD2, 0xFFC392C3, 0xFFB487B4, 0xFFA57BA5, 0xFF967096,
        0xFF876587, 0xFF785A78, 0xFF694E69, 0xFF5A435A, 0xFF4B384B, 0xFF3C2D3C, 0xFF2D212D, 0xFF1E161E,
        0xFFDFBFFF, 0xFFD2B4F0, 0xFFC4A8E1, 0xFFB79DD2, 0xFFAA92C3, 0xFF9D87B4, 0xFF907BA5, 0xFF837096,
        0xFF766587, 0xFF695A78, 0xFF5B4E69, 0xFF4E435A, 0xFF41384B, 0xFF342D3C, 0xFF27212D, 0xFF1A161E,
    };

    /*static int palette[256] =
    {
        0xFF000000, 0xFF7EC0EE, 0xFFEFEFEF, 0xFFC0C0C0, 0xFFCD0000, 0xFF008B00, 0xFF0000CD, 0xFF7FC0CD,
        0xFFFF8B00, 0xFFCD8B3F, 0xFFCD8B8B, 0xFFCD3F8B, 0xFFFF008B, 0xFFFFA9A9, 0xFFFF0000, 0xFFC00000,
        0xFF8B0000, 0xFFFFFFA9, 0xFFFFFF00, 0xFFC0C000, 0xFF8B8B00, 0xFFA9FFA9, 0xFF00FF00, 0xFF00C000,
        0xFF008B00, 0xFF00FF8B, 0xFF3FCD8B, 0xFF8BCD8B, 0xFF8BCD3F, 0xFF8BFF00, 0xFFA9FFFF, 0xFF00FFFF,
        0xFF00C0C0, 0xFF008B8B, 0xFFA9A9FF, 0xFF0000FF, 0xFF0000C0, 0xFF00008B, 0xFF8B00FF, 0xFF8B3FCD,
        0xFF8B8BCD, 0xFF3F8BCD, 0xFF008BFF, 0xFFFFA9FF, 0xFFFF00FF, 0xFFC000C0, 0xFF8B008B, 0xFF000000,

        0xFFFFFFFF, 0xFFF3F3F3, 0xFFE6E6E6, 0xFFD8D8D8, 0xFFC9C9C9, 0xFFB9B9B9, 0xFFA9A9A9, 0xFF989898,
        0xFF868686, 0xFF737373, 0xFF5F5F5F, 0xFF4B4B4B, 0xFF373737, 0xFF282828, 0xFF191919, 0xFF0A0A0A,

        0xFFBFBFFF, 0xFFB4B4F0, 0xFFA8A8E1, 0xFF9D9DD2, 0xFF9292C3, 0xFF8787B4, 0xFF7B7BA5, 0xFF707096,
        0xFF656587, 0xFF5A5A78, 0xFF4E4E69, 0xFF43435A, 0xFF38384B, 0xFF2D2D3C, 0xFF21212D, 0xFF16161E,
        0xFFBFDFFF, 0xFFB4D2F0, 0xFFA8C4E1, 0xFF9DB7D2, 0xFF92AAC3, 0xFF879DB4, 0xFF7B90A5, 0xFF708396,
        0xFF657687, 0xFF5A6978, 0xFF4E5B69, 0xFF434E5A, 0xFF38414B, 0xFF2D343C, 0xFF21272D, 0xFF161A1E,
        0xFFBFFFFF, 0xFFB4F0F0, 0xFFA8E1E1, 0xFF9DD2D2, 0xFF92C3C3, 0xFF87B4B4, 0xFF7BA5A5, 0xFF709696,
        0xFF658787, 0xFF5A7878, 0xFF4E6969, 0xFF435A5A, 0xFF384B4B, 0xFF2D3C3C, 0xFF212D2D, 0xFF161E1E,
        0xFFBFFFDF, 0xFFB4F0D2, 0xFFA8E1C4, 0xFF9DD2B7, 0xFF92C3AA, 0xFF87B49D, 0xFF7BA590, 0xFF709683,
        0xFF658776, 0xFF5A7869, 0xFF4E695B, 0xFF435A4E, 0xFF384B41, 0xFF2D3C34, 0xFF212D27, 0xFF161E1A,
        0xFFBFFFBF, 0xFFB4F0B4, 0xFFA8E1A8, 0xFF9DD29D, 0xFF92C392, 0xFF87B487, 0xFF7BA57B, 0xFF709670,
        0xFF658765, 0xFF5A785A, 0xFF4E694E, 0xFF435A43, 0xFF384B38, 0xFF2D3C2D, 0xFF212D21, 0xFF161E16,
        0xFFDFFFBF, 0xFFD2F0B4, 0xFFC4E1A8, 0xFFB7D29D, 0xFFAAC392, 0xFF9DB487, 0xFF90A57B, 0xFF839670,
        0xFF768765, 0xFF69785A, 0xFF5B694E, 0xFF4E5A43, 0xFF414B38, 0xFF343C2D, 0xFF272D21, 0xFF1A1E16,
        0xFFFFFFBF, 0xFFF0F0B4, 0xFFE1E1A8, 0xFFD2D29D, 0xFFC3C392, 0xFFB4B487, 0xFFA5A57B, 0xFF969670,
        0xFF878765, 0xFF78785A, 0xFF69694E, 0xFF5A5A43, 0xFF4B4B38, 0xFF3C3C2D, 0xFF2D2D21, 0xFF1E1E16,
        0xFFFFDFBF, 0xFFF0D2B4, 0xFFE1C4A8, 0xFFD2B79D, 0xFFC3AA92, 0xFFB49D87, 0xFFA5907B, 0xFF968370,
        0xFF877665, 0xFF78695A, 0xFF695B4E, 0xFF5A4E43, 0xFF4B4138, 0xFF3C342D, 0xFF2D2721, 0xFF1E1A16,
        0xFFFFBFBF, 0xFFF0B4B4, 0xFFE1A8A8, 0xFFD29D9D, 0xFFC39292, 0xFFB48787, 0xFFA57B7B, 0xFF967070,
        0xFF876565, 0xFF785A5A, 0xFF694E4E, 0xFF5A4343, 0xFF4B3838, 0xFF3C2D2D, 0xFF2D2121, 0xFF1E1616,
        0xFFFFBFDF, 0xFFF0B4D2, 0xFFE1A8C4, 0xFFD29DB7, 0xFFC392AA, 0xFFB4879D, 0xFFA57B90, 0xFF967083,
        0xFF876576, 0xFF785A69, 0xFF694E5B, 0xFF5A434E, 0xFF4B3841, 0xFF3C2D34, 0xFF2D2127, 0xFF1E161A,
        0xFFFFBFFF, 0xFFF0B4F0, 0xFFE1A8E1, 0xFFD29DD2, 0xFFC392C3, 0xFFB487B4, 0xFFA57BA5, 0xFF967096,
        0xFF876587, 0xFF785A78, 0xFF694E69, 0xFF5A435A, 0xFF4B384B, 0xFF3C2D3C, 0xFF2D212D, 0xFF1E161E,
        0xFFDFBFFF, 0xFFD2B4F0, 0xFFC4A8E1, 0xFFB79DD2, 0xFFAA92C3, 0xFF9D87B4, 0xFF907BA5, 0xFF837096,
        0xFF766587, 0xFF695A78, 0xFF5B4E69, 0xFF4E435A, 0xFF41384B, 0xFF342D3C, 0xFF27212D, 0xFF1A161E,
    };*/


/*-----------------------------------------------------------------------
    Function Declarations
    ---------------------------------------------------------------------*/
    //! \brief Loads a Keays UT File
    /*! Returns S_OK on success and E_FAIL on failure.
        \param filename [in] Pointer to a string representing the name of the UT file to load. Cannot be NULL.
        \param p [in] Pointer to a valid Interface. Can be NULL if pGeometry is not NULL.
        \param pGeometry [out] Pointer to a Geometry structure to be filled with the loaded geometry. Can be NULL. */
    HRESULT LoadUTFromFile(LPCTSTR filename, const KSR::Chunk &baseChunk, bool centerOnOrigin,
                           DWORD flags, KSR::PGEOMETRY pGeometry, const KSR::Vector3 &offset = KSR::Vector3(0, 0, 0));

    //! \brief Loads a Keays UT File
    /*! Returns S_OK on success and E_FAIL on failure.
        \param filename [in] Pointer to a string representing the name of the UT file to load. Cannot be NULL.
        \param p [in] Pointer to a valid Interface. Can be NULL if pGeometry is not NULL.
        \param pGeometry [out] Pointer to a Geometry structure to be filled with the loaded geometry. Can be NULL. */
    HRESULT LoadLayeredUTFromFile(LPCTSTR filename, const KSR::Chunk &baseChunk, const int *pTextureIDs, const int *pTextureScales, int numTextureIDs,
                                  const DWORD *pMaterialColours, int numMaterialColours,
                                  bool centerOnOrigin, DWORD flags, KSR::PGEOMETRY pGeometry, KSR::PINTERFACE pKSR,
                                  const KSR::Vector3 &offset, const unsigned char ambientDivisor = 3,
                                  KSR::pProgressUpdateFn ProgressCallback = NULL, void *pProgressPayload = NULL, const unsigned int numProgrssUpdates = 20,
                                  const unsigned char specularDivisor = 0, const unsigned char emmisiveDivisor = 0);

    HRESULT LoadLayeredUTFromFile(LPCTSTR filename, const KSR::Chunk &baseChunk, bool centerOnOrigin, DWORD flags,
                                  KSR::PGEOMETRY pGeometry, const unsigned int materialIDs[256], const KSR::Vector3 &offset = KSR::Vector3(0, 0, 0));

    //! \brief Loads an X Mesh File
    /*! Returns S_OK on success and E_FAIL on failure.
        \param filename [in] Pointer to a string representing the name of the X file to load. Cannot be NULL.
        \param p [in] Pointer to a valid Interface. Cannot be NULL.
        \param pGeometry [out] Pointer to a Geometry structure to be filled with the loaded geometry. Cannot be NULL. */
    HRESULT LoadXMeshFromFile(LPCTSTR filename, int effectID, KSR::PGEOMETRY pGeometry, KSR::PINTERFACE pKSRInterface);

    //! \brief Saves Geometry to a Keays UT File
    /*! Returns S_OK on success and E_FAIL on failure.
        \param geometry [in] Pointer to a valid Geometry structure containing the geometry to save to file. Cannot be NULL.
        \param filename [in] Pointer to a string representing the name of the UT file to save. Cannot be NULL. */
    HRESULT SaveUTToFile(KSR::PGEOMETRY geometry, LPCTSTR filename);

// EOF