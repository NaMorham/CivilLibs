#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX1 &VERTEX_DIFFUSE_TEX1::operator =(const VERTEX_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX1 &VERTEX_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX1 &VERTEX_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX1 &VERTEX_DIFFUSE_TEX1::operator =(const VERTEX_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv = rhs.uv[0];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX1 &VERTEX_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        uv = rhs.uv[0];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX1 &VERTEX_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv = rhs.uv[0];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX1 &VERTEX_DIFFUSE_TEX1::operator =(const VERTEX_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX1 &VERTEX_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX1 &VERTEX_NORMAL_TEX1::operator =(const VERTEX_NORMAL_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX1 &VERTEX_NORMAL_TEX1::operator =(const VERTEX_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX1 &VERTEX_NORMAL_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX1 &VERTEX_NORMAL_TEX1::operator =(const VERTEX_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        uv = rhs.uv[0];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX1 &VERTEX_NORMAL_TEX1::operator =(const VERTEX_NORMAL_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv = rhs.uv[0];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX1 &VERTEX_NORMAL_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv = rhs.uv[0];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX1 &VERTEX_NORMAL_TEX1::operator =(const VERTEX_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX1 &VERTEX_NORMAL_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX1 &VERTEX_NORMAL_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX1 &VERTEX_NORMAL_DIFFUSE_TEX1::operator =(const VERTEX_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX1 &VERTEX_NORMAL_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX1 &VERTEX_NORMAL_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv = rhs.uv[0];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX1 &VERTEX_NORMAL_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;
        uv = rhs.uv[0];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX1 &VERTEX_NORMAL_DIFFUSE_TEX1::operator =(const VERTEX_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX1 &VERTEX_NORMAL_DIFFUSE_TEX1::operator =(const VERTEX_NORMAL_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX2 &VERTEX_DIFFUSE_TEX2::operator =(const VERTEX_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX2 &VERTEX_DIFFUSE_TEX2::operator =(const VERTEX_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX2 &VERTEX_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX2 &VERTEX_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX2 &VERTEX_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX2 &VERTEX_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX2 &VERTEX_DIFFUSE_TEX2::operator =(const VERTEX_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE_TEX2 &VERTEX_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX2 &VERTEX_NORMAL_TEX2::operator =(const VERTEX_NORMAL_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX2 &VERTEX_NORMAL_TEX2::operator =(const VERTEX_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX2 &VERTEX_NORMAL_TEX2::operator =(const VERTEX_NORMAL_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX2 &VERTEX_NORMAL_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX2 &VERTEX_NORMAL_TEX2::operator =(const VERTEX_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX2 &VERTEX_NORMAL_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX2 &VERTEX_NORMAL_TEX2::operator =(const VERTEX_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_TEX2 &VERTEX_NORMAL_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX2 &VERTEX_NORMAL_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX2 &VERTEX_NORMAL_DIFFUSE_TEX2::operator =(const VERTEX_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX2 &VERTEX_NORMAL_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX2 &VERTEX_NORMAL_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;
        uv[0] = rhs.uv;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX2 &VERTEX_NORMAL_DIFFUSE_TEX2::operator =(const VERTEX_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX2 &VERTEX_NORMAL_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        uv[0] = rhs.uv[0];
        uv[1] = rhs.uv[1];

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX2 &VERTEX_NORMAL_DIFFUSE_TEX2::operator =(const VERTEX_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE_TEX2 &VERTEX_NORMAL_DIFFUSE_TEX2::operator =(const VERTEX_NORMAL_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE &VERTEX_DIFFUSE::operator =(const VERTEX_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE &VERTEX_DIFFUSE::operator =(const VERTEX_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE &VERTEX_DIFFUSE::operator =(const VERTEX_NORMAL_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE &VERTEX_DIFFUSE::operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE &VERTEX_DIFFUSE::operator =(const VERTEX_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE &VERTEX_DIFFUSE::operator =(const VERTEX_NORMAL_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE &VERTEX_DIFFUSE::operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_DIFFUSE &VERTEX_DIFFUSE::operator =(const VERTEX_NORMAL_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE &VERTEX_NORMAL_DIFFUSE::operator =(const VERTEX_NORMAL_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE &VERTEX_NORMAL_DIFFUSE::operator =(const VERTEX_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE &VERTEX_NORMAL_DIFFUSE::operator =(const VERTEX_NORMAL_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE &VERTEX_NORMAL_DIFFUSE::operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE &VERTEX_NORMAL_DIFFUSE::operator =(const VERTEX_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE &VERTEX_NORMAL_DIFFUSE::operator =(const VERTEX_NORMAL_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE &VERTEX_NORMAL_DIFFUSE::operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        normal = rhs.normal;
        color = rhs.color;

        return *this;
    }


//-----------------------------------------------------------------------
    const VERTEX_NORMAL_DIFFUSE &VERTEX_NORMAL_DIFFUSE::operator =(const VERTEX_DIFFUSE &rhs)
    //-------------------------------------------------------------------
    {
        position = rhs.position;
        color = rhs.color;

        return *this;
    }



#ifdef _DEBUG
//-----------------------------------------------------------------------
    template <class IDTYPE>
    LWIDServer<IDTYPE>::LWIDServer()
    //-------------------------------------------------------------------
    {
        m_ID = 0;
        m_pFreedIDs = new std::set<IDTYPE>;    OutputDebugString(_T("LWIDServer::Allocate m_pFreedIDs\n"));

        memset(dbgDump, 0, DBG_DUMP_SIZE);
    }


#define DBG_DUMP_SIZE 1024
char g_dbgDump[1024];

//-----------------------------------------------------------------------
    template <class IDTYPE>
    LWIDServer<IDTYPE>::~LWIDServer()
    //-------------------------------------------------------------------
    {
        m_ID = 0;
        if (m_pFreedIDs)
        {
            m_pFreedIDs->clear();
            delete m_pFreedIDs;
            m_pFreedIDs = NULL;    OutputDebugString(_T("LWIDServer::Delete m_pFreedIDs\n"));
        }
    }


//-----------------------------------------------------------------------
    template <class IDTYPE>
    const IDTYPE LWIDServer<IDTYPE>::GetID()
    //-------------------------------------------------------------------
    {
        IDTYPE id = 0;
        if (m_pFreedIDs->size() >= 1)
        {
            id = *(m_pFreedIDs->begin());
            m_pFreedIDs->erase(m_pFreedIDs->begin());
        } else
        {
            id = m_ID;
            ++m_ID;
        }
        return id;
    }


//-----------------------------------------------------------------------
    template <class IDTYPE>
    bool LWIDServer<IDTYPE>::ReleaseID(const IDTYPE id)
    //-------------------------------------------------------------------
    {
        if (id >= m_ID)
        {
            // out of range
            return false;
        }
        if (m_pFreedIDs->find(id) != m_pFreedIDs->end())
        {
            // already freed
            return false;
        }

        m_pFreedIDs->insert(id);
        Fix();
        return true;
    }


//-----------------------------------------------------------------------
    template <class IDTYPE>
    void LWIDServer<IDTYPE>::Fix()
    //-------------------------------------------------------------------
    {
        // go through the set from the top to the bottm, and reduce the id if required
        std::set<IDTYPE>::iterator itr = m_pFreedIDs->end();
        itr--;
        while (1)
        {
            if (itr == m_pFreedIDs->end() || (m_ID <= 0))
            {
                break;
            }

            IDTYPE id = (*itr);
            if (id == (m_ID - 1))
            {
                m_ID--;
                itr = m_pFreedIDs->erase(itr);
                itr--;
            } else
            {
                break;
            }

        }
    }


//-----------------------------------------------------------------------
    template <class IDTYPE>
    LPCTSTR LWIDServer<IDTYPE>::DumpData() const
    //-------------------------------------------------------------------
    {
        memset(g_dbgDump, 0, DBG_DUMP_SIZE * sizeof(TCHAR));
        int len = _sntprintf(g_dbgDump, DBG_DUMP_SIZE-1, _T("\n---------- DUMP ----------\nm_ID == %d\n"), m_ID);
        if (m_pFreedIDs->size() > 0)
        {
            len += _sntprintf(g_dbgDump + len, DBG_DUMP_SIZE-(1+len), _T("---------- FREE ----------\n"));
            std::set<IDTYPE>::const_iterator itr;
            int count = 0;
            for (itr = m_pFreedIDs->begin(); itr != m_pFreedIDs->end(); itr++)
            {
                len += _sntprintf(g_dbgDump + len, DBG_DUMP_SIZE-(1+len), _T("%d%s"), (*itr), (count >= 5 ? _T("\n") : _T(", ")));
                ++count;
                if (count >= 10)
                {
                    count = 0;
                }
            }
            len += _sntprintf(g_dbgDump + len, DBG_DUMP_SIZE-(1+len), _T("\n"));
        }

        len += _sntprintf(g_dbgDump + len, DBG_DUMP_SIZE-(1+len), _T("--------------------------\n"));
        return dbgDump;
    }
#endif

// EOF