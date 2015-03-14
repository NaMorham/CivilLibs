/*! \file */
/*-----------------------------------------------------------------------
	effect.h

	Description: Effect object declaration
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

//! \struct Effect
struct Effect : public MemObject
{
public:
	Effect(LPDIRECT3DDEVICE9 pDevice, bool debugVs, bool debugPs);
	~Effect();

	HRESULT Clear();

	HRESULT CompileFromFile(LPCTSTR filename, LPCTSTR pDefaultTechnique = NULL, String *pErrorBuf = NULL);
	HRESULT CompileFromString(LPCTSTR buffer, LPCTSTR pDefaultTechnique = NULL, String *pErrorBuf = NULL);

	HRESULT ValidateTechnique(LPCTSTR handle);
	inline HRESULT ValidateTechnique(const String &handle) { return ValidateTechnique(handle.c_str()); }
	HRESULT SetTechnique(LPCTSTR handle);
	inline HRESULT SetTechnique(const String &handle) { return SetTechnique(handle.c_str()); }

	HRESULT SetValue(const String &handle, LPVOID pData, int size);

	LPD3DXEFFECT GetD3DEffect();

	const StringArray *GetTechniqueNames() const { return &m_techniqueHandles; }
	const String &GetDefaultTechnique() const { return m_techniqueHandles[m_defaultTechniqueID]; }
	int GetDefaultTechniqueID() const { return m_defaultTechniqueID; }

	const bool HasLightPosition(unsigned int index) const		{ return index < 8 ? (m_lightPosition & 1 << index) != 0 : false; }
	const bool HasLightDirection(unsigned int index) const		{ return index < 8 ? (m_lightDirection & 1 << index) != 0 : false; }
	const bool HasLightDiffuse(unsigned int index) const		{ return index < 8 ? (m_lightDiffuse & 1 << index) != 0 : false; }
	const bool HasLightAmbient(unsigned int index) const		{ return index < 8 ? (m_lightAmbient & 1 << index) != 0 : false; }
	const bool HasLightSpecular(unsigned int index) const		{ return index < 8 ? (m_lightSpecular & 1 << index) != 0 : false; }
	const bool HasLightRange(unsigned int index) const			{ return index < 8 ? (m_lightRange & 1 << index) != 0 : false; }
	const bool HasLightFalloff(unsigned int index) const		{ return index < 8 ? (m_lightFalloff & 1 << index) != 0 : false; }
	const bool HasLightAttenuation(unsigned int index) const	{ return index < 8 ? (m_lightAttenuation & 1 << index) != 0 : false; }
	const bool HasLightTheta(unsigned int index) const			{ return index < 8 ? (m_lightTheta & 1 << index) != 0 : false; }
	const bool HasLightPhi(unsigned int index) const			{ return index < 8 ? (m_lightPhi & 1 << index) != 0 : false; }
	const bool HasLightInnerCone(unsigned int index) const		{ return index < 8 ? (m_lightTheta & 1 << index) != 0 : false; }
	const bool HasLightOuterCone(unsigned int index) const		{ return index < 8 ? (m_lightPhi & 1 << index) != 0 : false; }
	const bool HasLightType(unsigned int index) const			{ return index < 8 ? (m_lightType & 1 << index) != 0 : false; }

	String m_currentTechnique;

	bool m_hasTexture0, m_hasTexture1;
	bool m_hasWorld, m_hasView, m_hasProjection, m_hasWorldView, m_hasWorldViewProjection;
	bool m_hasBackgroundColour;
	bool *m_hasProjectionLevels, *m_hasWorldViewProjectionLevels;
	//bool m_lightDirection[8], m_lightDiffuse[8], m_lightAmbient[8], m_lightSpecular[8];

	// Bit-vectors - each char represents 8 boolean values for the existence of the indexed value.
	ubyte m_lightPosition, m_lightDirection, m_lightDiffuse, m_lightAmbient, m_lightSpecular, 
		  m_lightRange, m_lightFalloff, m_lightAttenuation, m_lightPhi, m_lightTheta, m_lightType;

	int m_numProjectionLevels;

	int m_currentTexture0, m_currentTexture1;

private:
	HRESULT DetermineUsage();
	void PopulateTechniqueHandles(LPCTSTR pDefaultTechnique);

	bool m_vsDebugging, m_psDebugging;

	StringArray m_techniqueHandles;

	int m_defaultTechniqueID;

	LPDIRECT3DDEVICE9 m_pD3DDevice;
	LPD3DXEFFECT	  m_pD3DEffect;
};

//! \typedef Effect *PEFFECT
typedef Effect *PEFFECT;

// EOF