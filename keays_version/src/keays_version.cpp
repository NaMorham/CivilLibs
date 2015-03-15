#include "..\include\keays_version.h"
#include <ctype.h>

namespace keays
{

const char *Version::ALLOWABLE_MODS = "ab";

// constructors
Version::Version( const Version &orig )
{
    m_major = orig.Major();
    m_minor = orig.Minor();
    m_patch = orig.Patch();
    m_mod = orig.Mod();
}

// mutators
// these 4 mutators are here in preperation for later pre processing if required
const unsigned int Version::Major( const unsigned major )
{
    m_major = major;
    return m_major;
}

const unsigned int Version::Minor( const unsigned minor )
{
    m_minor = minor;
    return m_minor;
}

const unsigned int Version::Patch( const unsigned int patch )
{
    m_patch = patch;
    return m_patch;
}

const char Version::Mod( const char mod )
{
    if( isalpha( mod ) )
    {
        char m = tolower( mod );
        if( strchr( ALLOWABLE_MODS, m ) )
        {
            m_mod = m;
        }
    }
    return m_mod;
}

// operators
const Version &Version::operator=( const Version &rhs )
{
    m_major = rhs.Major();
    m_minor = rhs.Minor();
    m_patch = rhs.Patch();
    m_mod = rhs.Mod();

    return *this;
}

const Version &Version::operator=( const char *str )
{
    unsigned int major = 0,
                 minor = 0,
                 patch = 0;
    char mod = 0;
    if( str )
    {
        if( sscanf( str, "v%d.%d.%d%c", &major, &minor, &patch, &mod ) < 3 )
        {
            if( sscanf( str, "v%d.%d%c", &major, &minor, &mod ) < 2 )
            {
                if(!sscanf( str, "v%d%c", &major, &mod ) )
                {
                    major = minor = patch = 0;
                    mod = 0;
                }
            }
        }
    }

    Major( major );
    Minor( minor );
    Patch( patch );
    Mod( mod );

    return *this;
}

const bool Version::operator<( const Version &rhs )
{
    if( m_major < rhs.Major() )
    {
        return true;
    } else if( m_major > rhs.Major() )
    {
        return false;
    } else
    {
        // they are equal, test the minor
        if( m_minor < rhs.Minor() )
        {
            return true;
        } else if( m_minor > rhs.Minor() )
        {
            return false;
        } else
        {
            // they are equal, test the patch level
            if( m_patch < rhs.Patch() )
            {
                return true;
            } else if( m_patch > rhs.Patch() )
            {
                return false;
            } else
            {
                // they are equal, test the mod
                if( m_mod && !rhs.Mod() )
                {
                    return true;
                } else if( m_mod == rhs.Mod() )
                {
                    return false;
                } else if( ( m_mod == 'a' ) && ( rhs.Mod() == 'b' )  ) // beta is more mature than alpha
                {
                    return true;
                } else
                {
                    return false;
                }
            }
        }
    }
}

const bool Version::operator>( const Version &rhs )
{
    if( m_major > rhs.Major() )
    {
        return true;
    } else if( m_major < rhs.Major() )
    {
        return false;
    } else
    {
        // they are equal, test the minor
        if( m_minor > rhs.Minor() )
        {
            return true;
        } else if( m_minor < rhs.Minor() )
        {
            return false;
        } else
        {
            // they are equal, test the patch level
            if( m_patch > rhs.Patch() )
            {
                return true;
            } else if( m_patch < rhs.Patch() )
            {
                return false;
            } else
            {
                // they are equal, test the modification
                if( m_mod && !rhs.Mod() )
                {
                    return true;
                } else if( m_mod == rhs.Mod() )
                {
                    return false;
                } else if( ( m_mod == 'a' ) && ( rhs.Mod() == 'b' )  ) // beta is more mature than alpha
                {
                    return true;
                } else
                {
                    return false;
                }
            }
        }
    }
}

const bool Version::operator==( const Version &rhs )
{
    return ( ( m_major == rhs.Major() ) &&
               ( m_minor == rhs.Minor() ) &&
                ( m_patch == rhs.Patch() ) &&
                ( m_mod == rhs.Mod() ) );
}

Version::operator const std::string () const
{
    char buf[256];
    int len;
    std::string str;

    memset( buf, 0, 256 );
    len = _snprintf( buf, 255, "v%d.%d", m_major, m_minor );

    if( m_patch )
        len += _snprintf( buf+len, 255-len, ".%d", m_patch );
    if( m_mod )
        len += _snprintf( buf+len, 255-len, "%c", m_mod );

    str = buf;
    return str;
}

}; // namespace keays