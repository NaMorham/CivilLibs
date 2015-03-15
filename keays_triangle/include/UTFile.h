
#ifndef _UT_FILE
#define _UT_FILE

#pragma once // redundant with the above defines

#include <string>

#include ".\triangle.h"

namespace keays
{
namespace triangle
{

struct KEAYS_TRIANGLE_API V2UTFileHeaderRecord
{
    V2UTFileHeaderRecord(unsigned int offset = 0, unsigned int size = 0,
                          unsigned int recordSize = 0, unsigned int count = 0)
        : m_offset(offset)
        , m_size(size)
        , m_recordSize(recordSize)
        , m_count(count) {}

    V2UTFileHeaderRecord(const V2UTFileHeaderRecord &original)
        : m_offset(original.Offset())
        , m_size(original.Size())
        , m_recordSize(original.RecordSize())
        , m_count(original.Count()) {}

    const V2UTFileHeaderRecord &operator=(const V2UTFileHeaderRecord &rhs)
    {
        Offset() = rhs.Offset();
        Size() = rhs.Offset();
        RecordSize() = rhs.RecordSize();
        Count() = rhs.Count();
        return *this;
    }

    unsigned int &Size() { return m_size; }
    const unsigned int &Size() const { return m_size; }

    unsigned int &Offset() { return m_offset; }
    const unsigned int &Offset() const { return m_offset; }

    unsigned int &RecordSize() { return m_recordSize; }
    const unsigned int &RecordSize() const { return m_recordSize; }

    unsigned int &Count() { return m_count; }
    const unsigned int &Count() const { return m_count; }

    // used to skip/locate a section
    unsigned int m_offset;
    unsigned int m_size;
    // used to parse a section
    unsigned int m_recordSize;
    unsigned int m_count;
};

typedef const unsigned long tUTFileVersion;

// previous versions
const tUTFileVersion G_V2_KUT_FILE_VERSION = 0x020000a;
// current version - default for new files
const tUTFileVersion G_CURRENT_KUT_FILE_VERSION = G_V2_KUT_FILE_VERSION;

struct KEAYS_TRIANGLE_API V2UTFileHeader
{
    V2UTFileHeader()
        : m_fileVersion(G_CURRENT_KUT_FILE_VERSION)
        , m_triangles(0, 0, 0, 0)
        , m_points(0, 0, 0, 0)
        , m_vertexNormals(0, 0, 0, 0){}

    tUTFileVersion &Version() { return m_fileVersion; }
    const tUTFileVersion &Version() const { return m_fileVersion; }

    V2UTFileHeaderRecord &Triangles() { return m_triangles; }
    const V2UTFileHeaderRecord &Triangles() const { return m_triangles; }

    V2UTFileHeaderRecord &Points() { return m_points; }
    const V2UTFileHeaderRecord &Points() const { return m_points; }

    V2UTFileHeaderRecord &VertexNormals() { return m_vertexNormals; }
    const V2UTFileHeaderRecord &VertexNormals() const { return m_vertexNormals; }

    tUTFileVersion            m_fileVersion;
    V2UTFileHeaderRecord    m_triangles;
    V2UTFileHeaderRecord    m_points;
    V2UTFileHeaderRecord    m_vertexNormals;
};

class KEAYS_TRIANGLE_API UTFile
{
public:
    //! \brief Loads a Keays UT File
    /*! Returns true on success and false on failure.
        \param filename [in] Pointer to a string representing the name of the UT file to load.
                             Cannot be NULL.
     */
    static bool Read(LPCTSTR filename, Triangles &triangles, pFnProgressUpdate pfnProgressUpdate = NULL);
    static bool ReadV2(LPCTSTR filename, Triangles &triangles, pFnProgressUpdate pfnProgressUpdate = NULL);

    static bool Save(LPCTSTR filename, const Triangles *pTriangles, const unsigned char version);
    static bool SaveV1(LPCTSTR filename, const Triangles *pTriangles);
    static bool SaveV2(LPCTSTR filename, const Triangles *pTriangles);

    static bool ReadV2Text(LPCTSTR filename, Triangles &triangles, pFnProgressUpdate pfnProgressUpdate = NULL);

    static bool SaveText(LPCTSTR filename, const Triangles *pTriangles, const unsigned char version);
    static bool SaveV2Text(LPCTSTR filename, const Triangles *pTriangles);

    static bool CanLoadExt(const std::string & extension);
};

};
};

#endif // #ifndef _UT_FILE

