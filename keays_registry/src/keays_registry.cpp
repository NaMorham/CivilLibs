// keays_registry.cpp : Defines the entry point for the DLL application.
//

#include "..\include\keays_registry.h"
#include <stdio.h>

namespace keays
{
namespace registry
{

//-------------------------------------------------------------------------
//#region Constants
/* REGISTRY KEY NAMES */
const TCHAR *G_KEAYS_SOFTWARE = _T("Software\\Keays Software");
const TCHAR *G_KEAYS_OLD_SOFTWARE = _T("Software\\Keays");

const TCHAR *G_KEAYS_INSTALL_REG_PATH = _T("\\Install\\Home");
const TCHAR *G_KEAYS_JOBS_REG_PATH = _T("\\Install\\Jobs");
const TCHAR *G_KEAYS_SYM_REG_PATH = _T("\\Install\\Symbol Library");
const TCHAR *G_KEAYS_CAD_REG_PATH = _T("\\Install\\Cad");
const TCHAR *G_KEAYS_NAME_REG_PATH = _T("\\Install\\Name");
const TCHAR *G_KEAYS_COMPANY_REG_PATH = _T("\\Install\\Company");

const TCHAR *G_KEAYS_CUR_JOB = _T("Current Job");    // REG_SZ - the path to the current job
const TCHAR *G_KEAYS_INSTALL_DIR = _T("Home");        // REG_SZ - the path to the keays install dir
//#endregion

//-------------------------------------------------------------------------
//#region Basic Registry Read Write Functions
/*!
    \name Basic Registry Read/Write Functions
    @{
 */
const int G_BUF_SIZE = 1024;

//#region Reading
/*
    Read a string value to an open registry key. Reads up to numChars, and returns the number of
    characters read from the registry.
 */
KEAYS_REGISTRY_API const int GetString(HKEY key, const TCHAR *valueName, TCHAR *data, const int numChars)
{    //#region
    if (data == NULL)
    {
        OutputDebugString("keays::registry::GetString: NULL pointer passed for output\n");
        return false;
    }
    if (numChars < 1)
    {
        OutputDebugString("keays::registry::GetString: invalid size for output passed\n");
        return false;
    }
    if (key == NULL)
    {
        OutputDebugString("keays::registry::GetString: NULL pointer passed for key handle\n");
        return false;
    }

    bool result = true;
    DWORD type, rdSize = (DWORD)(numChars * sizeof(TCHAR));
    //TCHAR *temp = new TCHAR[numChars];
    TCHAR temp[G_BUF_SIZE];

    if ((RegQueryValueEx(key, valueName, 0, &type, ((LPBYTE)temp), &rdSize)!= ERROR_SUCCESS)|| (type != REG_SZ))
    {
        result = false;
    } else
    {
        if (rdSize)
        {
            _tcsncpy(data, temp, (numChars * sizeof(TCHAR)));
        } else
        {
            (*data)= 0;
        }
    }
    //delete[] temp;
    return (result ? _tcsclen(data): 0);
    //#endregion
}

/*
    Read a string value to an open registry key, and returns the number of characters read from the registry.
 */
KEAYS_REGISTRY_API const int GetString(HKEY key, const TCHAR *valueName, String *pData)
{    //#region
    if (pData == NULL)
        return 0;
    TCHAR buf[G_BUF_SIZE];
    memset(buf, 0, G_BUF_SIZE*sizeof(TCHAR));
    int rc = GetString(key, valueName, buf, G_BUF_SIZE-1);
    if (rc)
        pData->assign(buf);
    return rc;
    //#endregion
}

/*
    Read a double precision value from an open registry key.  Attempt to read the value as if it is a binary
    value, if that fails fall through to read as if it is a string value.  Returns true if the read was
    successful.
 */
KEAYS_REGISTRY_API bool GetDouble(HKEY key, const TCHAR *valueName, double *pData)
{    //#region
    if (GetDoubleAsBinary(key, valueName, pData))
        return true;
    else if (GetDoubleAsString(key, valueName, pData))
        return true;
    else
        return false;
    //#endregion
}

/*
    Read a double precision value from an open registry key.  Reads the double as if it was a binary value and
    returns true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetDoubleAsBinary(HKEY key, const TCHAR *valueName, double *pData)
{    //#region
    if (pData == NULL)
    {
        OutputDebugString("keays::registry::GetDouble: NULL pointer passed for output\n");
        return false;
    }
    if (key == NULL)
    {
        OutputDebugString("keays::registry::GetDouble: NULL pointer passed for key handle\n");
        return false;
    }

    DWORD type, size;    // prep to read a string
    double temp;

    size = sizeof(double);

    if ((RegQueryValueEx(key, valueName, 0, &type, ((LPBYTE)&temp), &size)!= ERROR_SUCCESS)|| (type != REG_BINARY))
    {
        return false;
    } else
    {
        // we read a string but is it any good
        (*pData)= temp;
    }
    return true;
    //#endregion
}

/*
    Read a double precision value from an open registry key.  Reads the double as if it was a string value and
    returns true if the read was successful.
 */

KEAYS_REGISTRY_API bool GetDoubleAsString(HKEY key, const TCHAR *valueName, double *pData)
{    //#region
    if (pData == NULL)
    {
        OutputDebugString("keays::registry::GetDoubleAsString: NULL pointer passed for output\n");
        return false;
    }
    if (key == NULL)
    {
        OutputDebugString("keays::registry::GetDoubleAsString: NULL pointer passed for key handle\n");
        return false;
    }

    DWORD type, size;    // prep to read a string
    TCHAR temp[G_BUF_SIZE];

    size = (G_BUF_SIZE * sizeof(TCHAR));

    if ((RegQueryValueEx(key, valueName, 0, &type, ((LPBYTE)temp), &size)!= ERROR_SUCCESS)|| (type != REG_SZ))
    {
        return false;
    } else
    {
        // we read a string but is it any good
        (*pData)= atof(temp);
    }

    return true;
    //#endregion
}

/*
    Read a DWORD value from an open registry key and return true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetDWord(HKEY key, const TCHAR *valueName, DWORD *pData)
{    //#region
    if (pData == NULL)
    {
        OutputDebugString("keays::registry::GetDWord: NULL pointer passed for output\n");
        return false;
    }
    if (key == NULL)
    {
        OutputDebugString("keays::registry::GetDWord: NULL pointer passed for key handle\n");
        return false;
    }

    DWORD dw, type, size = sizeof(DWORD);    // prep to read a DWORD
    if ((RegQueryValueEx(key, valueName, 0, &type, ((LPBYTE)&dw), &size)!= ERROR_SUCCESS)|| (type != REG_DWORD))
    {
        return false;
    }
    (*pData)= dw;
    return true;
    //#endregion
}

/*
    Read a boolean value from an open registry key, return true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetBool(HKEY key, const TCHAR *valueName, bool *pData)
{    //#region
    if (pData == NULL)
    {
        OutputDebugString("keays::registry::GetBool: NULL pointer passed for output\n");
        return false;
    }
    if (key == NULL)
    {
        OutputDebugString("keays::registry::GetBool: NULL pointer passed for key handle\n");
        return false;
    }

    DWORD dw, type, size = sizeof(DWORD);    // prep to read a DWORD
    if ((RegQueryValueEx(key, valueName, 0, &type, ((LPBYTE)&dw), &size)!= ERROR_SUCCESS)|| (type != REG_DWORD))
    {
        return false;
    }
    (*pData)= (dw != 0);
    return true;
    //#endregion
}

/*
    Read a binary (void)value from an open registry key. fills the buffer in pBinData with up to size bytes.
    Returns true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetBinary(HKEY key, const TCHAR *valueName, void *pBinData, const int size)
{    //#region
    if (pBinData == NULL)
    {
        OutputDebugString("keays::registry::GetBinary: NULL pointer passed for output\n");
        return false;
    }
    if (size < 1)
    {
        OutputDebugString("keays::registry::GetBinary: invalid size for output passed\n");
        return false;
    }
    if (key == NULL)
    {
        OutputDebugString("keays::registry::GetBinary: NULL pointer passed for key handle\n");
        return false;
    }

    bool result = true;
    DWORD type, sz = size;
    //LPBYTE data = new BYTE[size];
    BYTE data[G_BUF_SIZE];

    if ((RegQueryValueEx(key, valueName, 0, &type, data, &sz)!= ERROR_SUCCESS)|| (type != REG_BINARY))
    {
        result = false;
        memset(pBinData, 0, size);
    } else
    {
        memcpy(pBinData, (void *)data, size);
    }

    //delete[] data;
    return result;
    //#endregion
}
// End region Reading //#endregion

//-----------------------------------------------------------------------------
//#region Writing
/*!
    \brief Create/Write a string value to an open registry key.

    \param       key [In]  - a handle to an open registry key for writing.
    \param valueName [In]  - a constant TCHAR *string representing the name to assign the value.
    \param      data [In]  - a constant TCHAR *string containing the data to be written.
    \return true if the data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetString(HKEY key, const TCHAR *valueName, const TCHAR *data)
{    //#region
    if (key == NULL)
    {
        OutputDebugString("keays::registry::GetBinary: NULL pointer passed for key handle\n");
        return false;
    }

    return (RegSetValueEx(key, valueName, 0, REG_SZ, ((LPBYTE)data), ((DWORD)_tcsclen(data)+1))== ERROR_SUCCESS);
    //#endregion
}

/*
    Create/Write a double precision value to an open registry key.  Writes the data as a binary value, if the
    value has to be editable from within the registry use SetDoubleAsString(...)instead.  Return true if the
    data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetDouble(HKEY key, const TCHAR *valueName, const double &data)
{    //#region
    if (key == NULL)
    {
        OutputDebugString("keays::registry::SetDouble: NULL pointer passed for key handle\n");
        return false;
    }

    bool result = true;
    if (RegSetValueEx(key, valueName, 0, REG_BINARY, ((LPBYTE)&data), sizeof(double))!= ERROR_SUCCESS)
    {
        result = false;
    }
    return result;
    //#endregion
}

/*
    Create/Write a double precision value to an open registry key.  Writes the data as a string value, if the
    value does not need to be editable from within the registry use SetDouble(...)instead, for efficency.
    Return true if the data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetDoubleAsString(HKEY key, const TCHAR *valueName, const double &data)
{    //#region
    if (key == NULL)
    {
        OutputDebugString("keays::registry::SetDoubleAsString: NULL pointer passed for key handle\n");
        return false;
    }

    bool result = true;
    TCHAR temp[G_BUF_SIZE];
    _sntprintf(temp, G_BUF_SIZE*sizeof(TCHAR), "%.5f", data);
    if (RegSetValueEx(key, valueName, 0, REG_SZ, ((LPBYTE)temp), ((DWORD)_tcsclen(temp)+1))!= ERROR_SUCCESS)
    {
        result = false;
    }
    return result;
    //#endregion
}

KEAYS_REGISTRY_API bool SetDWord(HKEY key, const TCHAR *valueName, const DWORD data)
{    //#region
    if (key == NULL)
    {
        OutputDebugString("keays::registry::SetDWord: NULL pointer passed for key handle\n");
        return false;
    }

    return (RegSetValueEx(key, valueName, 0, REG_DWORD, ((LPBYTE)&data), sizeof(DWORD))== ERROR_SUCCESS);
    //#endregion
}

KEAYS_REGISTRY_API bool SetBool(HKEY key, const TCHAR *valueName, const bool data)
{    //#region
    if (key == NULL)
    {
        OutputDebugString("keays::registry::SetBool: NULL pointer passed for key handle\n");
        return false;
    }

    DWORD dw = (data ? 1 : 0);
    return (RegSetValueEx(key, valueName, 0, REG_DWORD, ((LPBYTE)&dw), sizeof(DWORD))== ERROR_SUCCESS);
    //#endregion
}

KEAYS_REGISTRY_API bool SetBinary(HKEY key, const TCHAR *valueName, const void *pBinData, const int size)
{    //#region
    if (pBinData == NULL)
    {
        OutputDebugString("keays::registry::SetBinary: NULL pointer passed for input\n");
        return false;
    }
    if (size < 1)
    {
        OutputDebugString("keays::registry::SetBinary: invalid size for input passed\n");
        return false;
    }
    if (key == NULL)
    {
        OutputDebugString("keays::registry::SetBinary: NULL pointer passed for key handle\n");
        return false;
    }

    return (RegSetValueEx(key, valueName, 0, REG_BINARY, ((LPBYTE)pBinData), (DWORD)size)== ERROR_SUCCESS);
    //#endregion
}
// End Region Writing //#endregion
//! @}
// End Region Generic Read/Write Routines //#endregion

//-------------------------------------------------------------------------
//#region Keays Specific Functions
//#region Reading
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, TCHAR *data, const int numChars,
             bool readOldKeyOnFail /*= true*/)
{    //#region
    if (!rootKey)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., TCHAR *): NULL rootKey passed in\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }
    // we allow NULL key names, current job is a root KEAYS value for instance
    // we also allow NULL value names, since the registry does
    if (!data || numChars < 1)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., TCHAR *): NULL passed in for output data or output size to small\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }


    // calc the largest size we'll need in numChars
    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // and set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // now attempt to read the value from the new key
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
    {
        if (GetString(rKey, valueName, data, numChars))
            success = true;
        else if (!readOldKeyOnFail)
            result = E_REV_COULDNOT_RETRIEVE_REG_VALUE;

        RegCloseKey(rKey);
    }

    if (!success && readOldKeyOnFail)
    {
        // try the old key
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
        {
            if (GetString(rKey, valueName, data, numChars))
            {
                result = S_REV_SUCCESS_OLDREG;
            } else if (!readOldKeyOnFail)
            {
                result = E_REV_COULDNOT_RETRIEVE_OLD_REG_VALUE;
            }
            RegCloseKey(rKey);
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    //if (kName)
    //    delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, String *pData,
             bool readOldKeyOnFail /*= true*/)
{    //#region
    if (!rootKey)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., String *): NULL rootKey passed in\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }
    // we allow NULL key names, current job is a root KEAYS value for instance
    // we also allow NULL value names, since the registry does
    if (!pData)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., String *): NULL passed in for output data\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }


    // calc the largest size we'll need in numChars
    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));//kNameSize*sizeof(TCHAR));

    // and set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // now attempt to read the value from the new key
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
    {
        if (GetString(rKey, valueName, pData))
            success = true;
        else if (!readOldKeyOnFail)
            result = E_REV_COULDNOT_RETRIEVE_REG_VALUE;

        RegCloseKey(rKey);
    }

    if (!success && readOldKeyOnFail)
    {
        // try the old key
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
        {
            if (GetString(rKey, valueName, pData))
            {
                result = S_REV_SUCCESS_OLDREG;
            } else if (!readOldKeyOnFail)
            {
                result = E_REV_COULDNOT_RETRIEVE_OLD_REG_VALUE;
            }
            RegCloseKey(rKey);
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    } else if (!success)
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    //if (kName)
    //    delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, DWORD *pData,
             bool readOldKeyOnFail /*= true*/)
{    //#region
    if (!rootKey)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., DWORD *): NULL rootKey passed in\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }
    // we allow NULL key names, current job is a root KEAYS value for instance
    // we also allow NULL value names, since the registry does
    if (!pData)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., DWORD *): NULL passed in for output data\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }


    // calc the largest size we'll need in numChars
    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // and set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // now attempt to read the value from the new key
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
    {
        if (GetDWord(rKey, valueName, pData))
            success = true;
        else if (!readOldKeyOnFail)
            result = E_REV_COULDNOT_RETRIEVE_REG_VALUE;

        RegCloseKey(rKey);
    }

    if (!success && readOldKeyOnFail)
    {
        // try the old key
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
        {
            if (GetDWord(rKey, valueName, pData))
            {
                result = S_REV_SUCCESS_OLDREG;
            } else if (!readOldKeyOnFail)
            {
                result = E_REV_COULDNOT_RETRIEVE_OLD_REG_VALUE;
            }
            RegCloseKey(rKey);
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    //if (kName)
    //    delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, double *pData,
             bool readOldKeyOnFail /*= true*/)
{    //#region
    if (!rootKey)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., double *): NULL rootKey passed in\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }
    // we allow NULL key names, current job is a root KEAYS value for instance
    // we also allow NULL value names, since the registry does
    if (!pData)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., double *): NULL passed in for output data\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }


    // calc the largest size we'll need in numChars
    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // and set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // now attempt to read the value from the new key
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
    {
        if (GetDouble(rKey, valueName, pData))
            success = true;
        else if (!readOldKeyOnFail)
            result = E_REV_COULDNOT_RETRIEVE_REG_VALUE;

        RegCloseKey(rKey);
    }

    if (!success && readOldKeyOnFail)
    {
        // try the old key
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
        {
            if (GetDouble(rKey, valueName, pData))
            {
                result = S_REV_SUCCESS_OLDREG;
            } else if (!readOldKeyOnFail)
            {
                result = E_REV_COULDNOT_RETRIEVE_OLD_REG_VALUE;
            }
            RegCloseKey(rKey);
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    //if (kName)
    //    delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, bool *pData,
             bool readOldKeyOnFail /*= true*/)
{    //#region
    if (!rootKey)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., bool *): NULL rootKey passed in\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }
    // we allow NULL key names, current job is a root KEAYS value for instance
    // we also allow NULL value names, since the registry does
    if (!pData)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., bool *): NULL passed in for output data\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }


    // calc the largest size we'll need in numChars
    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // and set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // now attempt to read the value from the new key
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
    {
        if (GetBool(rKey, valueName, pData))
            success = true;
        else if (!readOldKeyOnFail)
            result = E_REV_COULDNOT_RETRIEVE_REG_VALUE;

        RegCloseKey(rKey);
    }

    if (!success && readOldKeyOnFail)
    {
        // try the old key
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
        {
            if (GetBool(rKey, valueName, pData))
            {
                result = S_REV_SUCCESS_OLDREG;
            } else if (!readOldKeyOnFail)
            {
                result = E_REV_COULDNOT_RETRIEVE_OLD_REG_VALUE;
            }
            RegCloseKey(rKey);
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    //if (kName)
    //    delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, void *pData, const int numBytes,
             bool readOldKeyOnFail /*= true*/)
{    //#region
    if (!rootKey)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., void *): NULL rootKey passed in\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }
    // we allow NULL key names, current job is a root KEAYS value for instance
    // we also allow NULL value names, since the registry does
    if (!pData || numBytes < 1)
    {
        OutputDebugString("keays::registry::GetKeaysVar(..., void *): NULL passed in for output data or output size to small\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }


    // calc the largest size we'll need in numChars
    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // and set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // now attempt to read the value from the new key
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
    {
        if (GetBinary(rKey, valueName, pData, numBytes))
            success = true;
        else if (!readOldKeyOnFail)
            result = E_REV_COULDNOT_RETRIEVE_REG_VALUE;

        RegCloseKey(rKey);
    }

    if (!success && readOldKeyOnFail)
    {
        // try the old key
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        if (RegOpenKey(rootKey, kName, &rKey)== ERROR_SUCCESS)
        {
            if (GetBinary(rKey, valueName, pData, numBytes))
            {
                result = S_REV_SUCCESS_OLDREG;
            } else if (!readOldKeyOnFail)
            {
                result = E_REV_COULDNOT_RETRIEVE_OLD_REG_VALUE;
            }
            RegCloseKey(rKey);
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    //if (kName)
    //    delete[] kName;

    return result;
    //#endregion
}
//#endregion

//#region Writing
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const TCHAR *data,
             bool setOldKey /*= true*/)
{    //#region
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    DWORD disposition;

    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // write to the main keays key
    if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
    {
        if (SetString(rKey, valueName, data))
            success = true;
        else
            result = E_REV_FAILED_WRITE;

    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    // if required write to the old key
    if (setOldKey)
    {
        // set up the old full key name
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        // write to the main keays key
        if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
        {
            if (SetString(rKey, valueName, data))
            {
                if (!success)
                {
                    result = S_REV_SUCCESS_OLDREG;
                } else
                {
                    result = E_REV_FAILED_OLD_WRITE;
                }
            }
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    }
    //delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const DWORD data,
             bool setOldKey /*= true*/)
{    //#region
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    DWORD disposition;

    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // write to the main keays key
    if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
    {
        if (SetDWord(rKey, valueName, data))
            success = true;
        else
            result = E_REV_FAILED_WRITE;
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    // if required write to the old key
    if (setOldKey)
    {
        // set up the old full key name
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        // write to the main keays key
        if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
        {
            if (SetDWord(rKey, valueName, data))
            {
                if (!success)
                {
                    result = S_REV_SUCCESS_OLDREG;
                } else
                {
                    result = E_REV_FAILED_OLD_WRITE;
                }
            }
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    }
    //delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const double &data,
             bool setOldKey /*= true*/, bool saveAsString /*= false*/)
{    //#region
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    DWORD disposition;

    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // write to the main keays key
    if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
    {
        if (SetDouble(rKey, valueName, data))
            success = true;
        else
            result = E_REV_FAILED_WRITE;
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    // if required write to the old key
    if (setOldKey)
    {
        // set up the old full key name
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        // write to the main keays key
        if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
        {
            if (SetDouble(rKey, valueName, data))
            {
                if (!success)
                {
                    result = S_REV_SUCCESS_OLDREG;
                } else
                {
                    result = E_REV_FAILED_OLD_WRITE;
                }
            }
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    }
    //delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const bool data,
             bool setOldKey /*= true*/)
{    //#region
    eRegErrorVals result = S_REV_SUCCESS;
    bool success = false;
    HKEY rKey;
    DWORD disposition;

    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // write to the main keays key
    if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
    {
        if (SetBool(rKey, valueName, data))
            success = true;
        else
            result = E_REV_FAILED_WRITE;
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    // if required write to the old key
    if (setOldKey)
    {
        // set up the old full key name
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        // write to the main keays key
        if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
        {
            if (SetBool(rKey, valueName, data))
            {
                if (!success)
                {
                    result = S_REV_SUCCESS_OLDREG;
                } else
                {
                    result = E_REV_FAILED_OLD_WRITE;
                }
            }
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    }
    //delete[] kName;

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const void *pData, const int numBytes,
             bool setOldKey /*= true*/)
{    //#region
    if (!pData || (numBytes < 1))// nothing to write
        return E_REV_NULL_POINTER_PASSED_IN;

    eRegErrorVals result = S_REV_SUCCESS;
    HKEY rKey;
    bool success = false;
    DWORD disposition;

    int kNameSize = _tcsclen(G_KEAYS_OLD_SOFTWARE)+ _tcsclen(keyName)+ 2;
    //TCHAR *kName = new TCHAR[kNameSize];
    TCHAR kName[G_BUF_SIZE];
    //memset(kName, 0, kNameSize*sizeof(TCHAR));
    memset(kName, 0, G_BUF_SIZE*sizeof(TCHAR));

    // set up the full key name
    _tcsncpy(kName, G_KEAYS_SOFTWARE, kNameSize);
    if (keyName)
    {
        if ((*keyName)!= _T('\\'))
            _tcscat(kName, _T("\\"));
        _tcscat(kName, keyName);
    }

    // write to the main keays key
    if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
    {
        if (SetBinary(rKey, valueName, pData, numBytes))
            success = true;
        else
            result = E_REV_FAILED_WRITE;
    } else
    {
        result = E_REV_COULDNOT_OPEN_REG_KEY;
    }

    // if required write to the old key
    if (setOldKey)
    {
        // set up the old full key name
        _tcsncpy(kName, G_KEAYS_OLD_SOFTWARE, kNameSize);
        if (keyName)
        {
            if ((*keyName)!= _T('\\'))
                _tcscat(kName, _T("\\"));
            _tcscat(kName, keyName);
        }

        // write to the main keays key
        if (RegCreateKeyEx(rootKey, kName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &rKey, &disposition))
        {
            if (SetBinary(rKey, valueName, pData, numBytes))
            {
                if (!success)
                {
                    result = S_REV_SUCCESS_OLDREG;
                } else
                {
                    result = E_REV_FAILED_OLD_WRITE;
                }
            }
        } else
        {
            result = E_REV_COULDNOT_OPEN_OLD_REG_KEY;
        }
    }

    //delete[] kName;

    return result;
    //#endregion
}

//#endregion

//-------------------------------------------------------------------------
//#region Current Job Functions
//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJob(TCHAR *curJob, const int curJobNumChars, bool readOldKeyOnFail /*= true*/)
{    //#region
    if ((curJob == NULL)|| (curJobNumChars < 3))
    {
        OutputDebugString("keays::registry::GetCurrentJob: NULL Pointer or insufficent size passed for path\n");
        return E_REV_NULL_POINTER_PASSED_IN;
    }

    memset(curJob, 0, curJobNumChars * sizeof(TCHAR));

    // try the proper key
    HKEY key;
    if (RegOpenKey(HKEY_CURRENT_USER, G_KEAYS_SOFTWARE, &key)== ERROR_SUCCESS)
    {
        if (!GetString(key, G_KEAYS_CUR_JOB, curJob, curJobNumChars))
        {
            if (!readOldKeyOnFail)
                return  E_REV_COULDNOT_RETRIEVE_REG_VALUE;
        } else
        {
            return S_REV_SUCCESS;
        }
    } else if (!readOldKeyOnFail)
    {
        return E_REV_COULDNOT_OPEN_REG_KEY;
    }

    // if we get here we may need to try the old key
    if (readOldKeyOnFail)
    {
        if (RegOpenKey(HKEY_CURRENT_USER, G_KEAYS_OLD_SOFTWARE, &key)== ERROR_SUCCESS)
        {
            if (GetString(key, G_KEAYS_CUR_JOB, curJob, curJobNumChars))
                return S_REV_SUCCESS_OLDREG;
        }
    } else if (!readOldKeyOnFail)
    {
        return E_REV_COULDNOT_OPEN_OLD_REG_KEY;
    }

    // TODO: Add environment variable checking
    return E_REV_COULDNOT_RETRIEVE_OLD_REG_VALUE;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJob(String *pCurJob, bool readOldKeyOnFail /*= true*/)
{    //#region
    if (!pCurJob)
        return E_REV_NULL_POINTER_PASSED_IN;

    TCHAR _curJob[MAX_PATH];
    memset(_curJob, 0, MAX_PATH * sizeof(TCHAR));
    eRegErrorVals result = GetCurrentJob(_curJob, MAX_PATH-1, readOldKeyOnFail);
    if (Succeeded(result))
        (*pCurJob)= _curJob;
    else
        pCurJob->erase();
    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJob(TCHAR *path, const int pathNumChars, TCHAR *jobName, const int jobNameNumChars,
               TCHAR *pRun, bool readOldKeyOnFail /*= true*/)
{    //#region
    TCHAR curJob[MAX_PATH];
    memset(curJob, 0, MAX_PATH * sizeof(TCHAR));

    eRegErrorVals result = GetCurrentJob(curJob, MAX_PATH-1, readOldKeyOnFail);
    if (!Succeeded(result))
        return result;

    TCHAR _drive[MAX_PATH];
    TCHAR _path[_MAX_PATH];
    TCHAR _name[_MAX_FNAME];
    TCHAR _ext[_MAX_EXT];
    _tsplitpath(curJob, _drive, _path, _name, _ext);
    if (path && (pathNumChars > 0))
    {
        _tcsncat(_drive, _path, pathNumChars-_tcsclen(path));
        _tcsncpy(path, _drive, pathNumChars);
    }
    if (jobName && (jobNameNumChars > 0))
    {
        _tcsncpy(jobName, _name, jobNameNumChars);
    }
    if (pRun)
    {
        *pRun = _ext[_tcsclen(_ext)-1];
    }

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJob(String *pPath, String *pJobName, TCHAR *pRun, bool readOldKeyOnFail /*= true*/)
{    //#region
    TCHAR _path[MAX_PATH];
    TCHAR _name[_MAX_FNAME];
    TCHAR *_pPath = (pPath ? _path : NULL);
    TCHAR *_pName = (pJobName ? _name : NULL);

    // empty them as required
    if (pPath)
    {
        pPath->erase();
        memset(_path, 0, MAX_PATH*sizeof(TCHAR));
    }
    if (pJobName)
    {
        pJobName->erase();
        memset(_name, 0, _MAX_FNAME*sizeof(TCHAR));
    }
    if (pRun)
        *pRun = _T('\0');

    // now get the values
    eRegErrorVals result = GetCurrentJob(_pPath, MAX_PATH-1, _pName, _MAX_FNAME-1, pRun, readOldKeyOnFail);
    if (Succeeded(result))
    {
        if (pPath)
            (*pPath)= _path;
        if (pJobName)
            (*pJobName)= _name;
        // NOTE: no need to assign pRun as it is done earlier if required
    }

    return result;
    //#endregion
}

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals GetCurrentJobPath(TCHAR *path, const int pathSize, bool readOldKeyOnFail /*= true*/)
{ return GetCurrentJob(path, pathSize, NULL, -1, NULL, readOldKeyOnFail); }

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals GetCurrentJobPath(String &path, bool readOldKeyOnFail /*= true*/)
{ return GetCurrentJob(&path, NULL, NULL, readOldKeyOnFail); }

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals GetCurrentJobName(TCHAR *jobName, const int jobNameSize, bool readOldKeyOnFail /*= true*/)
{ return GetCurrentJob(NULL, -1, jobName, jobNameSize, NULL, readOldKeyOnFail); }

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals GetCurrentJobName(String &jobName, bool readOldKeyOnFail /*= true*/)
{ return GetCurrentJob(NULL, &jobName, NULL, readOldKeyOnFail); }

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals GetCurrentRun(TCHAR &run, bool readOldKeyOnFail /*= true*/)
{ return GetCurrentJob(NULL, -1, NULL, -1, &run, readOldKeyOnFail); }
//#endregion
//#endregion

} // namespace registry
} // namespace keays