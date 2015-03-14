#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
	Effect::Effect(LPDIRECT3DDEVICE9 pDevice, bool debugVs, bool debugPs)
		:	MemObject(),
			m_vsDebugging(debugVs), m_psDebugging(debugPs),
			m_hasTexture0(false), m_hasTexture1(false), m_hasBackgroundColour(false), m_hasWorld(false), m_hasView(false), 
			m_hasProjection(false), m_hasWorldView(false), m_hasWorldViewProjection(false), 
			m_hasProjectionLevels(NULL), m_hasWorldViewProjectionLevels(NULL),
			m_currentTexture0(-1), m_currentTexture1(-1),
			m_lightPosition(0), m_lightDirection(0), m_lightDiffuse(0), m_lightAmbient(0), m_lightSpecular(0),
			m_lightRange(0), m_lightFalloff(0), m_lightAttenuation(0), m_lightPhi(0), m_lightTheta(0), m_lightType(0),
			m_numProjectionLevels(0),
			m_pD3DDevice(pDevice), m_pD3DEffect(NULL)
	//-------------------------------------------------------------------
	{
		assert(pDevice != NULL);
		AddUsedMemory(sizeof(Effect), _T("Effect::Effect()"));
	}


//-----------------------------------------------------------------------
	Effect::~Effect()
	//-------------------------------------------------------------------
	{
		Clear();
	}


//-----------------------------------------------------------------------
	HRESULT Effect::Clear()
	//-------------------------------------------------------------------
	{
		if (m_pD3DEffect)
		{
			m_pD3DEffect->Release();
			m_pD3DEffect = NULL;
		}

		if (m_hasProjectionLevels)
		{
			delete[] m_hasProjectionLevels;
			m_hasProjectionLevels = NULL;
			FreeUsedMemory(sizeof(bool) * m_numProjectionLevels, _T("Effect::Clear() - m_hasProjectionLevels"));
		}

		if (m_hasWorldViewProjectionLevels)
		{
			delete[] m_hasWorldViewProjectionLevels;
			m_hasWorldViewProjectionLevels = NULL;
			FreeUsedMemory(sizeof(bool) * m_numProjectionLevels, _T("Effect::Clear() - m_hasWorldViewProjectionLevels"));
		}

		m_hasTexture0 = m_hasTexture1 = false;
		m_hasWorld = m_hasView = m_hasProjection = m_hasWorldView = m_hasWorldViewProjection = false;

		m_numProjectionLevels = 0;

		m_lightPosition = m_lightDirection = m_lightDiffuse = m_lightAmbient = m_lightSpecular =
		m_lightRange = m_lightFalloff = m_lightAttenuation = m_lightPhi = m_lightTheta = m_lightType = 0;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Effect::CompileFromFile(LPCTSTR filename, LPCTSTR pDefaultTechnique, String *pErrorBuf)
	//-------------------------------------------------------------------
	{
		if (_tcslen(filename) < 1)
			return E_FAIL;

		if (m_pD3DEffect)
		{
			if (FAILED(Clear()))
				return E_FAIL;
		}

		LPD3DXBUFFER error = NULL;

		DWORD flags = 0;

#ifdef _DEBUG
		if (m_vsDebugging || m_psDebugging)
		{
			if (m_vsDebugging)
				flags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;

			if (m_psDebugging)
				flags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
		}
#endif

		D3DXCreateEffectFromFile(m_pD3DDevice, filename, NULL, NULL, flags, NULL, &m_pD3DEffect, &error);

		if (!m_pD3DEffect)
		{
			if (error)
			{
				Logf(_T("%s"), error->GetBufferPointer());
				if (pErrorBuf)
					(*pErrorBuf) = (LPCTSTR)error->GetBufferPointer();
				error->Release();
			}

			return E_FAIL;
		}

		PopulateTechniqueHandles(pDefaultTechnique);

		HRESULT result(S_OK);
		if (pDefaultTechnique)
		{
			if (FAILED(result = SetTechnique(pDefaultTechnique)))
				return result;
		}
		else
		{
			if (m_techniqueHandles.empty())
				return E_FAIL;

			if (FAILED(result = SetTechnique(m_techniqueHandles[0])))
				return result;
		}

		if (FAILED(DetermineUsage()))
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Effect::CompileFromString(LPCTSTR buffer, LPCTSTR pDefaultTechnique, String *pErrorBuf)
	//-------------------------------------------------------------------
	{
		if (_tcslen(buffer) < 1)
			return E_FAIL;

		if (m_pD3DEffect)
		{
			if (FAILED(Clear()))
				return E_FAIL;
		}

		LPD3DXBUFFER error;

		DWORD flags = 0;

#ifdef _DEBUG
		if (m_vsDebugging || m_psDebugging)
		{
			if (m_vsDebugging)
				flags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;

			if (m_psDebugging)
				flags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
		}
#endif

		D3DXCreateEffect(m_pD3DDevice, buffer, _tcslen(buffer), NULL, NULL, flags, NULL, &m_pD3DEffect, &error);

		if (!m_pD3DEffect)
		{
			if (error)
			{
				Logf(_T("%s"), error->GetBufferPointer());

				error->Release();
			}

			return E_FAIL;
		}

		if (FAILED(DetermineUsage()))
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Effect::ValidateTechnique(LPCTSTR handle)
	//-------------------------------------------------------------------
	{
		if (!m_pD3DEffect)
			return E_FAIL;

		HRESULT result;
		if (FAILED(result = m_pD3DEffect->ValidateTechnique(handle)))
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Effect::SetTechnique(LPCTSTR handle)
	//-------------------------------------------------------------------
	{
		if (!m_pD3DEffect)
			return E_FAIL;

		HRESULT result(S_OK);

		if (FAILED(result = m_pD3DEffect->SetTechnique(handle)))
			return result;

		m_currentTechnique = handle;

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT Effect::SetValue(const String &handle, LPVOID pData, int size)
	//-------------------------------------------------------------------
	{
		if (!m_pD3DEffect)
			return E_FAIL;

		if (FAILED(m_pD3DEffect->SetValue(handle.c_str(), pData, size)))
			return E_FAIL;

		return S_OK;
	}


//-----------------------------------------------------------------------
	LPD3DXEFFECT Effect::GetD3DEffect()
	//-------------------------------------------------------------------
	{
		return m_pD3DEffect;
	}

#ifdef UNICODE
#error "UNICODE DEFINED"
#endif

//-----------------------------------------------------------------------
	HRESULT Effect::DetermineUsage()
	//-------------------------------------------------------------------
	{
		D3DXEFFECT_DESC desc;
		m_pD3DEffect->GetDesc(&desc);

		StringArray projectionLevelHandles;
		StringArray worldViewProjectionLevelHandles;

		// Find the indexed handles
			for (UINT p = 0; p < desc.Parameters; p++)
			{
				D3DXHANDLE param = m_pD3DEffect->GetParameter(NULL, p);
				D3DXPARAMETER_DESC paramDesc;
				m_pD3DEffect->GetParameterDesc(param, &paramDesc);

				String paramDescStr(paramDesc.Name);
				String paramDescTruncated(_T(""));

				size_t length = paramDescStr.find_last_not_of("0123456789");

				if (length == String::npos)
					continue;

				length++;

				paramDescTruncated = paramDescStr.substr(0, length);

				if (_tcscmp(paramDescTruncated.c_str(), _T("projectionLevel")) == 0)
				{
					projectionLevelHandles.push_back(paramDesc.Name);
				}
				else if (_tcscmp(paramDescTruncated.c_str(), _T("worldViewProjectionLevel")) == 0)
				{
					worldViewProjectionLevelHandles.push_back(paramDesc.Name);
				}
				else
				{
					TCHAR indexValue = _ttoi(paramDescStr.substr(length, 1).c_str());

					if (indexValue >= 0 && indexValue < 8 && _istdigit(*paramDescStr.rbegin()))
					{
						char flag = (1 << indexValue);

						if (_tcscmp(paramDescTruncated.c_str(), _T("lightPosition")) == 0)
						{
							m_lightPosition |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightDirection")) == 0)
						{
							m_lightDirection |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightDiffuse")) == 0)
						{
							m_lightDiffuse |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightAmbient")) == 0)
						{
							m_lightAmbient |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightSpecular")) == 0)
						{
							m_lightSpecular |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightRange")) == 0)
						{
							m_lightRange |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightFalloff")) == 0)
						{
							m_lightFalloff |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightAttenuation")) == 0)
						{
							m_lightAttenuation |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightTheta")) == 0)
						{
							m_lightTheta |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightPhi")) == 0)
						{
							m_lightPhi |= flag;
						}
						else if (_tcscmp(paramDescTruncated.c_str(), _T("lightType")) == 0)
						{
							m_lightType |= flag;
						}
					}
				}
			}

		// Determine number of level handles
			std::vector<int> projectionLevelIndices;
			std::vector<int> worldViewProjectionLevelIndices;

			StringArray::iterator i = projectionLevelHandles.begin();
			for (; i != projectionLevelHandles.end(); i++)
			{
				TCHAR *pString = (TCHAR *)i->c_str();

				pString += 15;

				int index = _ttoi(pString);

				int maxIndexRange = index + 1;

				if (maxIndexRange > m_numProjectionLevels)
					m_numProjectionLevels = maxIndexRange;

				projectionLevelIndices.push_back(index);
			}

			i = worldViewProjectionLevelHandles.begin();
			for (; i != worldViewProjectionLevelHandles.end(); i++)
			{
				TCHAR *pString = (TCHAR *)i->c_str();

				pString += 24;

				int index = _ttoi(pString);

				int maxIndexRange = index + 1;

				if (maxIndexRange > m_numProjectionLevels)
					m_numProjectionLevels = maxIndexRange;

				worldViewProjectionLevelIndices.push_back(index);
			}

		// Determine projection levels from handles
			if (m_numProjectionLevels > 0)
			{
				m_hasProjectionLevels = new bool[m_numProjectionLevels];
				ZeroMemory(m_hasProjectionLevels, sizeof(bool) * m_numProjectionLevels);
				AddUsedMemory(sizeof(bool) * m_numProjectionLevels, _T("Effect::DetermineUsage() - m_hasProjectionLevels"));

				m_hasWorldViewProjectionLevels = new bool[m_numProjectionLevels];
				ZeroMemory(m_hasWorldViewProjectionLevels, sizeof(bool) * m_numProjectionLevels);
				AddUsedMemory(sizeof(bool) * m_numProjectionLevels, _T("Effect::DetermineUsage() - worldViewProjectionLevels"));

				StringArray::iterator i = projectionLevelHandles.begin();
				for (size_t n = 0; n < projectionLevelIndices.size(); n++)
				{
					m_hasProjectionLevels[projectionLevelIndices[n]] = true;
				}

				i = worldViewProjectionLevelHandles.begin();
				for (n = 0; n < worldViewProjectionLevelIndices.size(); n++)
				{
					m_hasWorldViewProjectionLevels[worldViewProjectionLevelIndices[n]] = true;
				}
			}

		// Determine usage values from handles
			for (p = 0; p < desc.Parameters; p++)
			{
				D3DXHANDLE param = m_pD3DEffect->GetParameter(NULL, p);
				D3DXPARAMETER_DESC paramDesc;
				m_pD3DEffect->GetParameterDesc(param, &paramDesc);

				if (_tcscmp(paramDesc.Name, _T("texture0")) == 0)
					m_hasTexture0 = true;
				else if (_tcscmp(paramDesc.Name, _T("texture1")) == 0)
					m_hasTexture1 = true;
				else if (_tcscmp(paramDesc.Name, _T("world")) == 0)
					m_hasWorld = true;
				else if (_tcscmp(paramDesc.Name, _T("view")) == 0)
					m_hasView = true;
				else if (_tcscmp(paramDesc.Name, _T("projection")) == 0)
					m_hasProjection = true;
				else if (_tcscmp(paramDesc.Name, _T("worldView")) == 0)
					m_hasWorldView = true;
				else if (_tcscmp(paramDesc.Name, _T("worldViewProjection")) == 0)
					m_hasWorldViewProjection = true;
				else if (_tcscmp(paramDesc.Name, _T("backgroundColour")) == 0)
					m_hasBackgroundColour = true;
			}

		return S_OK;
	}


//-----------------------------------------------------------------------
	void Effect::PopulateTechniqueHandles(LPCTSTR pDefaultTechnique)
	//-------------------------------------------------------------------
	{
		if (!m_pD3DDevice || !m_pD3DEffect)
			return;

		D3DXHANDLE hCurrentTechnique = NULL;
		D3DXHANDLE hLastTechnique = NULL;

		m_defaultTechniqueID = -1;

		for (int n = 0; ; n++)
		{
			if (FAILED(m_pD3DEffect->FindNextValidTechnique(hLastTechnique, &hCurrentTechnique)))
				break;

			if (!hCurrentTechnique)
				break;

			D3DXTECHNIQUE_DESC desc;

			if (FAILED(m_pD3DEffect->GetTechniqueDesc(hCurrentTechnique, &desc)))
				continue;

			hLastTechnique = hCurrentTechnique;

			m_techniqueHandles.push_back(desc.Name);

			if (pDefaultTechnique && m_defaultTechniqueID < 0)
			{
				if (_tcsicmp(pDefaultTechnique, desc.Name) == 0)
				{
					m_defaultTechniqueID = n;
				}
			}
		}

		if (!pDefaultTechnique || m_defaultTechniqueID < 0 || m_defaultTechniqueID >= (int)m_techniqueHandles.size())
		{
			m_defaultTechniqueID = 0;
		}
	}

// EOF