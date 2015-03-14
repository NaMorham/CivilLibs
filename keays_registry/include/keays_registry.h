/*!
	\file keays_registry.h
	\brief	Registry routines for use with Keays Software.
	Part of the keays::registry namespace.
	\note TCHAR is a macro that depends on the state of the UNICODE and MBCS defines, it allows
	      simple substitutions as follows,
			<table>
			<tr><td></td><td>UNICODE && MBCS Not defined</td><td>MBCS Defined</td><td>UNICODE Defined</td></tr>
			<tr><td>TCHAR</td><td>char</td><td>char</td><td>wchar_t</td></tr>
			<tr><td>_T()or _TEXT()Macros</td><td>No effect (removed by preprocessor)</td>
			    <td>No effect (removed by preprocessor)</td>
				 <td>L (converts following character or string to its Unicode counterpart)</td></tr>
			</table>.

	\date May 2005.
	\author Andrew Hickey, Keays Software.
 */

#if !defined(_KEAYS_REGISTRY_H_)
#define _KEAYS_REGISTRY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#region Import/Export define
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the KEAYS_REGISTRY_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// KEAYS_REGISTRY_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef KEAYS_REGISTRY_EXPORTS
#define KEAYS_REGISTRY_API __declspec(dllexport)
#else
#define KEAYS_REGISTRY_API __declspec(dllimport)
#endif
//#endregion

#include <windows.h>
#include <tchar.h>
#include <string>

/*!
	\typedef Define up a String type based on the state of UNICODE define
 */
#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

/*!
	\brief General Keays Software Namespace
	Used to prevent conflicts with existing functions/ variables
 */
namespace keays
{
/*!
	\brief Registry and keays environment routines for use in Keays Software applications
 */
namespace registry
{

// CONSTANTS
//#region -----------------------------------------------------------------
/* REGISTRY KEY NAMES */
//! The Main Keays Software registry path. "Software/Keays Software"
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_SOFTWARE;
//! The Main Keays Software registry path. "Software/Keays"
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_OLD_SOFTWARE;

/*!
	\brief The Keays Software install path. ".../Install/Home".
	\warning ONLY USE WITH HKEY_LOCAL_MACHINE
 */
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_INSTALL_REG_PATH;
/*!
	\brief The Keays Software jobs path. ".../Install/Jobs".
	\warning ONLY USE WITH HKEY_LOCAL_MACHINE
 */
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_JOBS_REG_PATH;
/*!
	\brief The Keays Software symbol library path. ".../Install/Symbol Library".
	\warning ONLY USE WITH HKEY_LOCAL_MACHINE
 */
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_SYM_REG_PATH;
/*!
	\brief The Keays Software cad path. ".../Install/Cad".
	\warning ONLY USE WITH HKEY_LOCAL_MACHINE
 */
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_CAD_REG_PATH;
/*!
	\brief The Keays Software registerned name path. ".../Install/Name".
	Who the software is registered to.
	\warning ONLY USE WITH HKEY_LOCAL_MACHINE
 */
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_NAME_REG_PATH;
/*!
	\brief The Keays Software registerned company name path. ".../Install/Company".
	What company the software is registered to.
	\warning ONLY USE WITH HKEY_LOCAL_MACHINE
 */
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_COMPANY_REG_PATH;

extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_CUR_JOB;
extern KEAYS_REGISTRY_API const TCHAR *G_KEAYS_INSTALL_DIR;

//#region Error Values
//-------------------------------------------------------------------------
enum KEAYS_REGISTRY_API eRegErrorVals
{
	S_REV_SUCCESS	= 0,					//!< Success - Successfully read the data from the main Keays Software Key.
	S_REV_SUCCESS_OLDREG,					//!< Success - Successfully read the data from the old Keays Software Key.
	S_REV_SUCCESS_ENV,						//!< Success - Successfully read the data from the System environment variables.

	E_REV_UNKNOWN_ERROR,					//!< Failure - An unknown error occured when retrieving values.

	E_REV_COULDNOT_OPEN_REG_KEY,			//!< Failure - Could not opn the registry key for reading/writing.
	E_REV_COULDNOT_OPEN_OLD_REG_KEY,		//!< Failure - Could not open the old registry key for reading/writing. (For this to occur it means the main registry key has already failed but we are falling through)

	E_REV_FAILED_WRITE,						//!< Failure - Could not write the data to the key.
	E_REV_FAILED_OLD_WRITE,					//!< Failure - Could not write the data to the old key.

	E_REV_COULDNOT_RETRIEVE_REG_VALUE,		//!< Failure - Could not retrieve the value from the registry key.
	E_REV_COULDNOT_RETRIEVE_OLD_REG_VALUE,	//!< Failure - Could not retrieve the value from the old registry key. (For this to occur it means the main registry path has already failed but we are falling through)
	E_REV_COULDNOT_RETRIEVE_ENV_VAR,		//!< Failure - Could not retrieve the value from the system environment. (For this to occur it means the old registry path has already failed but we are falling through)

	E_REV_RETURN_BUFFER_SIZE_TO_SMALL,		//!< Failure - Not enough room was provided in the return buffer.
	E_REV_NULL_POINTER_PASSED_IN,			//!< Failure - A NULL pointer was passed in for a return value.

// ----------- THESE SHOULD NOT BE USED -----------
//	E_REV_COULDNOT_RETRIEVE_CUR_JOB,
//	E_REV_COULDNOT_RETRIEVE_LIB_DIR,
//	E_REV_COULDNOT_RETRIEVE_INST_DIR,
//	E_REV_INVALID_CUR_JOB_VALUE,
//	E_REV_COULDNOT_WRITE_CUR_JOB,
//	E_REV_COULDNOT_WRITE_OLD_CUR_JOB,
// ----------- THESE SHOULD NOT BE USED -----------
};

inline bool Succeeded(const int returnedValue)
{
	return ((returnedValue == S_REV_SUCCESS)||
			 (returnedValue == S_REV_SUCCESS_OLDREG)||
			 (returnedValue == S_REV_SUCCESS_ENV));
}
//#endregion

//#endregion --------------------------------------------------------------

#if 0 //-----------------------------------------------------------------------
//#region Registry Key Class - DO NOT USE
//! \brief RKey class return values
enum KEAYS_REGISTRY_API eRKeyReturnVals
{
	E_FAILED = 0,	//!< The operation failed, the key could not be opened or created.
	S_OPENED,		//!< The key was successfully opened.
	S_CREATED,		//!< The key was successfully created.
};

/*!
	\brief Simple Registry Key manager class.
	This is a simple version of the more faetured MFC CRegKey class.
 */
class KEAYS_REGISTRY_API RKey
{
public:

	/*!
		\brief Default Constructor
		Creates an uninitialised (NULL)RKey
	 */
	RKey();

	/*!
		\brief Constructor
		Create the RKey.
		This will NOT open the Registry Key
		and Create instead.

		\param base [In]  - the base Registry Key to use (HKLM, HKCU, HK_CLASSES...)
		\param path [In]  - the registry path the RKey uses
	 */
	RKey(HKEY base, const TCHAR *path);

	/*!
		\brief Copy Constructor
		Duplicate an existing RKey
	 */
	RKey(const RKey &orig);

	/*!
		\brief Destructor.
		Close and destroy the RKey
	 */
	~RKey();

	/*!
		\brief Create and open a new Registry Key.

		\param       base [In]  - a handle to the base Registry Key to use (HKLM, HKCU, HK_CLASSES...)
		\param       path [In]  - a constant TCHAR string specifying the registry path the RKey uses
		\param accessMask [In]  - an access mask specifying the access rights, a combination of
		<table>
		<tr><td>KEY_CREATE_LINK</td><td>Permission to create a symbolic link.</td></tr>
		<tr><td>KEY_CREATE_SUB_KEY</td><td>Permission to create subkeys.</td></tr>
		<tr><td>KEY_ENUMERATE_SUB_KEYS</td><td>Permission to enumerate subkeys.</td></tr>
		<tr><td>KEY_EXECUTE</td><td>Permission for read access.</td></tr>
		<tr><td>KEY_NOTIFY</td><td>Permission for change notification.</td></tr>
		<tr><td>KEY_QUERY_VALUE</td><td>Permission to query subkey data.</td></tr>
		<tr><td>KEY_SET_VALUE</td><td>Permission to set subkey data.</td></tr>
		<tr><td>KEY_ALL_ACCESS</td><td>Combines the KEY_QUERY_VALUE, KEY_ENUMERATE_SUB_KEYS, KEY_NOTIFY,
		KEY_CREATE_SUB_KEY, KEY_CREATE_LINK, and KEY_SET_VALUE access rights, plus all the standard
		access rights except SYNCHRONIZE.</td></tr>
		<tr><td>KEY_READ</td><td>Combines the STANDARD_RIGHTS_READ, KEY_QUERY_VALUE, KEY_ENUMERATE_SUB_KEYS,
		and KEY_NOTIFY access rights.</td></tr>
		<tr><td>KEY_WRITE</td><td>Combines the STANDARD_RIGHTS_WRITE, KEY_SET_VALUE, and KEY_CREATE_SUB_KEY access rights.</td></tr>
		</table>

		\return an int indicating if the key was created, opened or failed, one of (E_FAILED, S_OPENED, S_CREATED).
	 */
	const int Create(HKEY base, const TCHAR *path, DWORD accessMask = KEY_ALL_ACCESS);

	/*!
		\brief Open the Registry Key.
		\note This will NOT Create a key if it does not exist.

		\param     base [In]  - the base Registry Key to use (HKLM, HKCU, HK_CLASSES...)
		\param     path [In]  - the registry path the RKey uses
		\param accessMask [In]  - an access mask specifying the access rights, a combination of
		<table>
		<tr><td>KEY_CREATE_LINK</td><td>Permission to create a symbolic link.</td></tr>
		<tr><td>KEY_CREATE_SUB_KEY</td><td>Permission to create subkeys.</td></tr>
		<tr><td>KEY_ENUMERATE_SUB_KEYS</td><td>Permission to enumerate subkeys.</td></tr>
		<tr><td>KEY_EXECUTE</td><td>Permission for read access.</td></tr>
		<tr><td>KEY_NOTIFY</td><td>Permission for change notification.</td></tr>
		<tr><td>KEY_QUERY_VALUE</td><td>Permission to query subkey data.</td></tr>
		<tr><td>KEY_SET_VALUE</td><td>Permission to set subkey data.</td></tr>
		<tr><td>KEY_ALL_ACCESS</td><td>Combines the KEY_QUERY_VALUE, KEY_ENUMERATE_SUB_KEYS, KEY_NOTIFY,
		KEY_CREATE_SUB_KEY, KEY_CREATE_LINK, and KEY_SET_VALUE access rights, plus all the standard
		access rights except SYNCHRONIZE.</td></tr>
		<tr><td>KEY_READ</td><td>Combines the STANDARD_RIGHTS_READ, KEY_QUERY_VALUE, KEY_ENUMERATE_SUB_KEYS,
		and KEY_NOTIFY access rights.</td></tr>
		<tr><td>KEY_WRITE</td><td>Combines the STANDARD_RIGHTS_WRITE, KEY_SET_VALUE, and KEY_CREATE_SUB_KEY access rights.</td></tr>
		</table>

		\return an int indicating if the key was opened or failed, one of (E_FAILED, S_OPENED).
	 */
	const int Open(HKEY base, const TCHAR *path, DWORD accessMask = KEY_ALL_ACCESS);

	/*!
		\brief Close the Registry Key
		\return true if it was closed successfully
	 */
	bool Close();

	/*!
		\brief Delete the Key.
		Deletes the Key and removes it from the registry, closes the RKey object's HANDLES as well
		\return true if successful.
	 */
	bool Delete();

	/*!
		\brief Reset the RKey.
		Close the Key and empty the members.
		\return true if successful.
	 */
	bool Reset();

	/*!
		\brief Test if the Key is open.
		\return true if the key is opened
	 */
	bool IsOpen();

	/*!
		\brief Write a double precision value to the registry.
		This stores the double as a binary value, if the value must be editable in the registry write it as a string value instead.

		\param valueName [In]  -
	 */
	bool Write(const TCHAR *valueName, const double &val);
	bool Write(const TCHAR *valueName, const float &val);
	bool Write(const TCHAR *valueName, const int val);
	bool Write(const TCHAR *valueName, const DWORD val);
	bool Write(const TCHAR *valueName, const TCHAR *val);
	bool Write(const TCHAR *valueName, const String &val);
	bool WriteBinary(const TCHAR *valueName, const void *val, const unsigned int numBytes);

	bool Read(const TCHAR *valueName, double &val);
	bool Read(const TCHAR *valueName, float &val);
	bool Read(const TCHAR *valueName, int &val);
	bool Read(const TCHAR *valueName, DWORD &val);
	bool Read(const TCHAR *valueName, TCHAR *val, const int numChars);
	bool Read(const TCHAR *valueName, String &val);
	bool ReadBinary(const TCHAR *valueName, void *val, const unsigned int numBytes);

private:
	HKEY m_baseKey;			//!< the base key to use. i.e. HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE
	HKEY m_theKey;				//!< the key to open.
	std::string *m_pPath;	//!< the path from the baseKey to the key.
};
//#endregion
#endif //----------------------------------------------------------------------

/*!
	\name Basic Registry Read Write Functions
	@{
 */
//#region -----------------------------------------------------------------
//#region Reading
/*!
	\brief Read a string value from an open registry key.

	\param       key [In]  - a handle to an open registry key for reading.
	\param valueName [In]  - a constant TCHAR *string representing the name of the value.
	\param      data [In]  - a TCHAR *buffer for the data to be read into.
	\param	numChars [In]  - a constant integer specifying the maximum number of character in the data buffer.
	\return the number of characters read from the registry.
 */
KEAYS_REGISTRY_API const int GetString(HKEY key, const TCHAR *valueName, TCHAR *data, const int numChars);

/*!
	\brief Read a string value from an open registry key.

	\param       key [In]  - a handle to an open registry key for reading.
	\param valueName [In]  - a constant TCHAR *string representing the name of the value.
	\param      data [In]  - a pointer to a String for the data to be read into.
	\return the number of characters read from the registry.
 */
KEAYS_REGISTRY_API const int GetString(HKEY key, const TCHAR *valueName, String *pData);

/*!
	\brief Read a double precision value from an open registry key.

	\param       key [In]  - a handle to an open registry key for reading.
	\param valueName [In]  - a constant TCHAR *string representing the name of the value.
	\param      data [In]  - a pointer to a double for the data to be read into.
	\return true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetDouble(HKEY key, const TCHAR *valueName, double *pData);

/*!
	\brief Read a double precision value from an open registry key.
	Reads the double as if it was a binary value.

	\param       key [In]  - a handle to an open registry key for reading.
	\param valueName [In]  - a constant TCHAR *string representing the name of the value.
	\param      data [In]  - a pointer to a double for the data to be read into.
	\return true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetDoubleAsBinary(HKEY key, const TCHAR *valueName, double *pData);

/*!
	\brief Read a double precision value from an open registry key.
	Reads the double as if it was a string value.

	\param       key [In]  - a handle to an open registry key for reading.
	\param valueName [In]  - a constant TCHAR *string representing the name of the value.
	\param      data [In]  - a pointer to a double for the data to be read into.
	\return true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetDoubleAsString(HKEY key, const TCHAR *valueName, double *pData);

/*!
	\brief Read a DWORD value from an open registry key.

	\param       key [In]  - a handle to an open registry key for reading.
	\param valueName [In]  - a constant TCHAR *string representing the name of the value.
	\param      data [In]  - a pointer to a DWORD for the data to be read into.
	\return true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetDWord(HKEY key, const TCHAR *valueName, DWORD *pData);

/*!
	\brief Read a boolean value from an open registry key.

	\param       key [In]  - a handle to an open registry key for reading.
	\param valueName [In]  - a constant TCHAR *string representing the name of the value.
	\param      data [In]  - a pointer to a bool for the data to be read into.
	\return true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetBool(HKEY key, const TCHAR *valueName, bool *pData);

/*!
	\brief Read a binary (void)value from an open registry key.

	\param       key [In]  - a handle to an open registry key for reading.
	\param valueName [In]  - a constant TCHAR *string representing the name of the value.
	\param      data [In]  - a void pointer to a buffer for the data to be read into.
	\param      size [In]  - a constant int specifying the size in bytes fo the data read.
	\return true if the read was successful.
 */
KEAYS_REGISTRY_API bool GetBinary(HKEY key, const TCHAR *valueName, void *pBinData, const int size);
//#endregion

//#region Writing
/*!
	\brief Create/Write a string value to an open registry key.

	\param       key [In]  - a handle to an open registry key for writing.
	\param valueName [In]  - a constant TCHAR *string representing the name to assign the value.
	\param      data [In]  - a constant TCHAR *string containing the data to be written.
	\return true if the data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetString(HKEY key, const TCHAR *valueName, const TCHAR *data);

/*!
	\brief Create/Write a double precision value to an open registry key.
	Writes the data as a binary value, if the value has to be editable from within the registry use
	SetDoubleAsString(...)instead.

	\param       key [In]  - a handle to an open registry key for writing.
	\param valueName [In]  - a constant TCHAR *string representing the name to assign the value.
	\param      data [In]  - a constant reference to a double with the value to be written.
	\return true if the data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetDouble(HKEY key, const TCHAR *valueName, const double &data);

/*!
	\brief Create/Write a double precision value to an open registry key.
	Writes the data as a string value, if the value does not need to be editable from within the registry use
	SetDouble(...)instead, for efficency.

	\param       key [In]  - a handle to an open registry key for writing.
	\param valueName [In]  - a constant TCHAR *string representing the name to assign the value.
	\param      data [In]  - a constant reference to a double with the value to be written.
	\return true if the data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetDoubleAsString(HKEY key, const TCHAR *valueName, const double &data);

/*!
	\brief Create/Write a DWord value to an open registry key.

	\param       key [In]  - a handle to an open registry key for writing.
	\param valueName [In]  - a constant TCHAR *string representing the name to assign the value.
	\param      data [In]  - a constant DWORD to be written.
	\return true if the data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetDWord(HKEY key, const TCHAR *valueName, const DWORD data);

/*!
	\brief Create/Write a boolean value to an open registry key.
	Writes the data as a DWORD value, 0 for false, 1 for true.

	\param       key [In]  - a handle to an open registry key for writing.
	\param valueName [In]  - a constant TCHAR *string representing the name to assign the value.
	\param      data [In]  - a constant boolean value to be written.
	\return true if the data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetBool(HKEY key, const TCHAR *valueName, const bool data);

/*!
	\brief Create/Write a generic binary value to an open registry key.
	Because this is raw binary data, it can be used to write arrays, structures and classes.  It
	is recommended to also write the size as a DWORD at the same time in the case of an array.

	\param       key [In]  - a handle to an open registry key for writing.
	\param valueName [In]  - a constant TCHAR *string representing the name to assign the value.
	\param      data [In]  - a constant void pointer to the memory buffer to write.
	\param      size [In]  - a constant integer value indicating the size of the buffer passed in.
	\return true if the data was successfully written to the key under the tag valuename.
 */
KEAYS_REGISTRY_API bool SetBinary(HKEY key, const TCHAR *valueName, const void *pBinData, const int size);
//#endregion
//#endregion --------------------------------------------------------------
//! @} // end group

/*!
	\name Keays Specific Registry Functions
	@{
 */
//#region -----------------------------------------------------------------
//#region Reading
//#region Generic reading functions
/*!
	\brief Read a string from a Keays registry key
	Reads up to numChars characters from the keyName specified under the keays software key
	(\SOFTWARE\Keays Software\<keyName>), given the root specified.  If readOldKeyOnFail is set,
	the function will attempt to read from the old Keays registry path (\Software\Keays\<keyName>).

	\param          rootKey [In]  - the base key to use, one of HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER, ...
	\param          keyName [In]  - the sub key to use. May be NULL if reading from the base Keays key.
	\param        valueName [In]  - the name of the value to retrieve. May be NULL if reading the default value.
	\param             data [Out] - a pointer to a TCHAR buffer of at least numChars size.
	\param         numChars [In]  - the size of the output buffer (data), the number of TCHAR elements, not the size in bytes.
	\param readOldKeyOnFail [In]  - a boolean flag indicating if the function should attemp to read the
	                                old key if the main key fails.

	\return one of the value from the eRegErrorVals enumeration. It is very important to check for success using
	the Succeeded()function provided as there are multiple success and failure values.
 */
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, TCHAR *data, const int numChars,
			 bool readOldKeyOnFail = true);

/*!
	\overload
	\param pDdata [Out] - a pointer to a String (std::string or std::wstring)to receive output.
 */
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, String *pData,
			 bool readOldKeyOnFail = true);

/*!
	\brief Read a DWORD from a Keays registry key
	Reads a single DWORD from the keyName specified under the keays software key
	(\SOFTWARE\Keays Software\<keyName>), given the root specified.  If readOldKeyOnFail is set,
	the function will attempt to read from the old Keays registry path (\Software\Keays\<keyName>).

	\param          rootKey [In]  - the base key to use, one of HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER, ...
	\param          keyName [In]  - the sub key to use. May be NULL if reading from the base Keays key.
	\param        valueName [In]  - the name of the value to retrieve. May be NULL if reading the default value.
	\param            pData [Out] - a pointer to a DWORD to receive the output.
	\param readOldKeyOnFail [In]  - a boolean flag indicating if the function should attemp to read the
	                                old key if the main key fails.

	\return one of the value from the eRegErrorVals enumeration. It is very important to check for success using
	the Succeeded()function provided as there are multiple success and failure values.
 */
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, DWORD *pData,
			 bool readOldKeyOnFail = true);

/*!
	\brief Read a double from a Keays registry key
	Reads a single double precision value from the keyName specified under the keays software key
	(\SOFTWARE\Keays Software\<keyName>), given the root specified.  If readOldKeyOnFail is set,
	the function will attempt to read from the old Keays registry path (\Software\Keays\<keyName>).

	\param          rootKey [In]  - the base key to use, one of HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER, ...
	\param          keyName [In]  - the sub key to use. May be NULL if reading from the base Keays key.
	\param        valueName [In]  - the name of the value to retrieve. May be NULL if reading the default value.
	\param            pData [Out] - a pointer to a double to receive the output.
	\param readOldKeyOnFail [In]  - a boolean flag indicating if the function should attemp to read the
	                                old key if the main key fails.

	\return one of the value from the eRegErrorVals enumeration. It is very important to check for success using
	the Succeeded()function provided as there are multiple success and failure values.
 */
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, double *pData,
			 bool readOldKeyOnFail = true);

/*!
	\brief Read a boolean value from a Keays registry key
	Reads a single boolean from the keyName specified under the keays software key
	(\SOFTWARE\Keays Software\<keyName>), given the root specified.  If readOldKeyOnFail is set,
	the function will attempt to read from the old Keays registry path (\Software\Keays\<keyName>).

	\param          rootKey [In]  - the base key to use, one of HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER, ...
	\param          keyName [In]  - the sub key to use. May be NULL if reading from the base Keays key.
	\param        valueName [In]  - the name of the value to retrieve. May be NULL if reading the default value.
	\param            pData [Out] - a pointer to a double to receive the output.
	\param readOldKeyOnFail [In]  - a boolean flag indicating if the function should attemp to read the
	                                old key if the main key fails.

	\return one of the value from the eRegErrorVals enumeration. It is very important to check for success using
	the Succeeded()function provided as there are multiple success and failure values.
 */
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, bool *pData,
			 bool readOldKeyOnFail = true);

/*!
	\brief Read binary data from a Keays registry key
	Reads up to numBytes from the keyName specified under the keays software key
	(\SOFTWARE\Keays Software\<keyName>), given the root specified.  If readOldKeyOnFail is set,
	the function will attempt to read from the old Keays registry path (\Software\Keays\<keyName>).

	\param          rootKey [In]  - the base key to use, one of HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER, ...
	\param          keyName [In]  - the sub key to use. May be NULL if reading from the base Keays key.
	\param        valueName [In]  - the name of the value to retrieve. May be NULL if reading the default value.
	\param            pData [Out] - a void pointer to a buffer of at least numBytes size.
	\param         numBytes [In]  - the size of the output buffer (data), the size in bytes.
	\param readOldKeyOnFail [In]  - a boolean flag indicating if the function should attemp to read the
	                                old key if the main key fails.

	\return one of the value from the eRegErrorVals enumeration. It is very important to check for success using
	the Succeeded()function provided as there are multiple success and failure values.
 */
KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, void *pData, const int numBytes,
			 bool readOldKeyOnFail = true);
//#endregion

//#region Read System Variables
//-------------------------------------------------------------------------
/*!
	\brief Read a string from a Keays System registry key
	Reads up to numChars characters from the keyName specified under the keays software key
	(HKEY_LOCAL_MACHINE\SOFTWARE\Keays Software\<keyName>), given the root specified.  If readOldKeyOnFail
	is set, the function will attempt to read from the old Keays registry path
	(HKEY_LOCAL_MACHINE\Software\Keays\<keyName>).

	\param          keyName [In]  - the sub key to use. May be NULL if reading from the base Keays key.
	\param        valueName [In]  - the name of the value to retrieve. May be NULL if reading the default value.
	\param             data [Out] - a pointer to a TCHAR buffer of at least numChars size.
	\param         numChars [In]  - the size of the output buffer (data), the number of TCHAR elements, not the size in bytes.
	\param readOldKeyOnFail [In]  - a boolean flag indicating if the function should attemp to read the
	                                old key if the main key fails.

	\return one of the value from the eRegErrorVals enumeration. It is very important to check for success using
	the Succeeded()function provided as there are multiple success and failure values.
 */
inline KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, TCHAR *data, const int numChars,
				   bool readOldKeyOnFail = true)
{ return GetKeaysVar(HKEY_LOCAL_MACHINE, keyName, valueName, data, readOldKeyOnFail); }

/*!
	\overload
	\param pData [Out] - a pointer to a TCHAR buffer of at least numChars size.
 */
inline KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, String *pData, bool readOldKeyOnFail = true)
{ return GetKeaysVar(HKEY_LOCAL_MACHINE, keyName, valueName, pData, readOldKeyOnFail); }

/*!
	\brief Read a DWORD from a Keays System registry key
	Reads a single DWORD from the keyName specified under the keays software key
	(HKEY_LOCAL_MACHINE\SOFTWARE\Keays Software\<keyName>), given the root specified.  If readOldKeyOnFail
	is set, the function will attempt to read from the old Keays registry path
	(HKEY_LOCAL_MACHINE\Software\Keays\<keyName>).

	\param          keyName [In]  - the sub key to use. May be NULL if reading from the base Keays key.
	\param        valueName [In]  - the name of the value to retrieve. May be NULL if reading the default value.
	\param            pData [Out] - a pointer to a DWORD to receive the data.
	\param readOldKeyOnFail [In]  - a boolean flag indicating if the function should attemp to read the
	                                old key if the main key fails.

	\return one of the value from the eRegErrorVals enumeration. It is very important to check for success using
	the Succeeded()function provided as there are multiple success and failure values.
 */
inline KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, DWORD *pData, bool readOldKeyOnFail = true)
{ return GetKeaysVar(HKEY_LOCAL_MACHINE, keyName, valueName, pData, readOldKeyOnFail); }

/*!
	\brief Read a DWORD from a Keays System registry key
	Reads a single DWORD from the keyName specified under the keays software key
	(HKEY_LOCAL_MACHINE\SOFTWARE\Keays Software\<keyName>), given the root specified.  If readOldKeyOnFail
	is set, the function will attempt to read from the old Keays registry path
	(HKEY_LOCAL_MACHINE\Software\Keays\<keyName>).

	\param          keyName [In]  - the sub key to use. May be NULL if reading from the base Keays key.
	\param        valueName [In]  - the name of the value to retrieve. May be NULL if reading the default value.
	\param            pData [Out] - a pointer to a DWORD to receive the data.
	\param readOldKeyOnFail [In]  - a boolean flag indicating if the function should attemp to read the
	                                old key if the main key fails.

	\return one of the value from the eRegErrorVals enumeration. It is very important to check for success using
	the Succeeded()function provided as there are multiple success and failure values.
 */
inline KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, double *pData, bool readOldKeyOnFail = true)
{ return GetKeaysVar(HKEY_LOCAL_MACHINE, keyName, valueName, pData, readOldKeyOnFail); }

inline KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, bool *pData, bool readOldKeyOnFail = true)
{ return GetKeaysVar(HKEY_LOCAL_MACHINE, keyName, valueName, pData, readOldKeyOnFail); }

inline KEAYS_REGISTRY_API const eRegErrorVals
GetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, void *pData, const int numBytes,
				   bool readOldKeyOnFail = true)
{ return GetKeaysVar(HKEY_LOCAL_MACHINE, keyName, valueName, pData, readOldKeyOnFail); }
//#endregion

//#region Read User Variables
//-------------------------------------------------------------------------
inline KEAYS_REGISTRY_API const eRegErrorVals GetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, TCHAR *data, const int numChars, bool readOldKeyOnFail = true)
{
	return GetKeaysVar(HKEY_CURRENT_USER, keyName, valueName, data, readOldKeyOnFail);
}

inline KEAYS_REGISTRY_API const eRegErrorVals GetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, String *pData, const int numChars, bool readOldKeyOnFail = true)
{
	return GetKeaysVar(HKEY_CURRENT_USER, keyName, valueName, pData, readOldKeyOnFail);
}

inline KEAYS_REGISTRY_API const eRegErrorVals GetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, DWORD *pData, bool readOldKeyOnFail = true)
{
	return GetKeaysVar(HKEY_CURRENT_USER, keyName, valueName, pData, readOldKeyOnFail);
}

inline KEAYS_REGISTRY_API const eRegErrorVals GetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, double *pData, bool readOldKeyOnFail = true)
{
	return GetKeaysVar(HKEY_CURRENT_USER, keyName, valueName, pData, readOldKeyOnFail);
}

inline KEAYS_REGISTRY_API const eRegErrorVals GetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, bool *pData, bool readOldKeyOnFail = true)
{
	return GetKeaysVar(HKEY_CURRENT_USER, keyName, valueName, pData, readOldKeyOnFail);
}

inline KEAYS_REGISTRY_API const eRegErrorVals GetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, void *pData, const int numBytes, bool readOldKeyOnFail = true)
{
	return GetKeaysVar(HKEY_CURRENT_USER, keyName, valueName, pData, numBytes, readOldKeyOnFail);
}
//#endregion

//#region Read the Current Job
//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJob(TCHAR *curJob, const int curJobNumChars, bool readOldKeyOnFail = true);
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJob(String &curJob, bool readOldKeyOnFail = true);
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJob(TCHAR *path, const int pathNumChars, TCHAR *jobName, const int jobNameNumChars,
			   TCHAR *pRun, bool readOldKeyOnFail = true);
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJob(String *pPath, String *pJobName, TCHAR *pRun, bool readOldKeyOnFail = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJobPath(TCHAR *path, const int pathSize, bool readOldKeyOnFail = true);
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJobPath(String &path, bool readOldKeyOnFail = true);
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJobName(TCHAR *jobName, const int jobNameSize, bool readOldKeyOnFail = true);
 KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentJobName(String &jobName, bool readOldKeyOnFail = true);
KEAYS_REGISTRY_API const eRegErrorVals
GetCurrentRun(TCHAR &run, bool readOldKeyOnFail = true);
//#endregion
// end region - Reading //#endregion
// ------------------------------------------------------------------------
//#region Writing
//-------------------------------------------------------------------------
//#region Generic writing functions
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const TCHAR *data,
			 bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const DWORD data,
			 bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const double &data,
			 bool setOldKey = true, bool saveAsString = false);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const bool data,
			 bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysVar(HKEY rootKey, const TCHAR *keyName, const TCHAR *valueName, const void *pData, const int numBytes,
			 bool setOldKey = true);
//#endregion

//-------------------------------------------------------------------------
//#region Write System Variables
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, const TCHAR *data, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, const DWORD data, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, const double &data, bool setOldKey = true,
				   bool saveAsString = false);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, const bool data, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysSystemVar(const TCHAR *keyName, const TCHAR *valueName, const void *data, const int numBytes,
				   bool setOldKey = true);
//#endregion

//-------------------------------------------------------------------------
//#region Write User Variables
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, const TCHAR *data, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, const DWORD data, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, const double &data, bool setOldKey = true,
				 bool saveAsString = false);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, const bool data, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetKeaysUserVar(const TCHAR *keyName, const TCHAR *valueName, const void *data, const int numBytes,
				 bool setOldKey = true);
//#endregion

//-------------------------------------------------------------------------
//#region Write the Current Job
KEAYS_REGISTRY_API const eRegErrorVals
SetCurrentJob(const TCHAR *curJob, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetCurrentJob(const String &curJob, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetCurrentJob(const TCHAR *path, const TCHAR *jobName, const TCHAR run, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetCurrentJobPath(const TCHAR *path, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetCurrentJobName(const TCHAR *jobName, bool setOldKey = true);

//-------------------------------------------------------------------------
KEAYS_REGISTRY_API const eRegErrorVals
SetCurrentRun(const TCHAR run, bool setOldKey = true);
//#endregion

// end region Writing //#endregion
//#endregion --------------------------------------------------------------
//! @} // end group Keays Specific Registry Functions

} // namespace registry
} // namespace keays


#endif // !defined(_KEAYS_REGISTRY_H_)
