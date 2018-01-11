/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2018/01/11.23:05:00 */

/*
 * Copyright (c) 2013-2018 yaruopooner [https://github.com/yaruopooner]
 *
 * This file is part of ac-clang.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#ifndef __COMMON_HPP__
#define __COMMON_HPP__




/*================================================================================================*/
/*  Include Files                                                                                 */
/*================================================================================================*/

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <algorithm>
#include <tuple>
#include <string>
#include <vector>

#include "clang-c/Index.h"



/*================================================================================================*/
/*  Class                                                                                         */
/*================================================================================================*/


template< int _Size >
struct Alignment
{
    template< typename T >
    static T Down( T _Value )
    {
        return ( ( static_cast< uintptr_t >( _Value ) & ~( _Size - 1 ) ) );
    }

    template< typename T >
    static T Up( T _Value )
    {
        return ( ( ( static_cast< uintptr_t >( _Value ) + ( _Size - 1 ) ) & ~( _Size - 1 ) ) );
    }

    static  constexpr   size_t  Size = _Size;
};




class Buffer
{
public:
    Buffer( void ) = default;
    Buffer( size_t _Size, bool _IsFill = false, int _Value = 0 );
    virtual ~Buffer( void );

    void Allocate( size_t _Size, bool _IsFill = false, int _Value = 0 );
    void Deallocate( void );

    void Fill( const int _Value = 0 )
    {
        if ( m_Address )
        {
            std::fill( m_Address, m_Address + m_Size, _Value );
        }
    }

    bool IsAllocated( void ) const
    {
        return ( m_Address != nullptr );
    }

    size_t GetSize( void ) const
    {
        return m_Size;
    }

    uint8_t* GetAddress( void ) const
    {
        return m_Address;
    }

    template< typename T >
    T GetAddress( void ) const
    {
        return reinterpret_cast< T >( m_Address );
    }

private:
    enum
    {
        kInitialSize = 4096,
    };

    size_t      m_Size     = 0;
    size_t      m_Capacity = 0;
    uint8_t*    m_Address  = nullptr;
};


class StreamReader
{
public:
    StreamReader( void );
    virtual ~StreamReader( void ) = default;
    
    template< typename T >
    void ReadToken( const char* _Format, T& _Value, bool _IsStepNextLine = true, bool _IsPolling = true )
    {
        ClearLine();

        while ( ( std::fscanf( m_File, _Format, &_Value ) < 0 ) && _IsPolling )
        {
            // polling
            // const int   error_no = std::ferror( m_File );
            // const int   eof      = std::feof( m_File );
        }

        if ( _IsStepNextLine )
        {
            StepNextLine();
        }
    }

    const char* ReadToken( const char* _Format, bool _IsStepNextLine = true, bool _IsPolling = true );

    void Read( char* _Buffer, size_t _ReadSize );
    
private:
    void ClearLine( void );
    void StepNextLine( void );

private:
    enum
    {
        kLineMax = 2048,
    };
    
    FILE*   m_File = stdin;
    char    m_Line[ kLineMax ];
};


class StreamWriter
{
public:
    StreamWriter( void ) = default;
    virtual ~StreamWriter( void ) = default;

    void Write( const char* _Format, ... );
    void Flush( void );
    
private:
    FILE*   m_File = stdout;
};


class PacketManager
{
public:
    PacketManager( void );
    virtual ~PacketManager( void ) = default;

    void Receive( void );
    void Send( void );


    const Buffer& GetReceiveBuffer( void ) const
    {
        return m_ReceiveBuffer;
    }
    Buffer& GetReceiveBuffer( void )
    {
        return m_ReceiveBuffer;
    }

    const Buffer& GetSendBuffer( void ) const
    {
        return m_SendBuffer;
    }
    Buffer& GetSendBuffer( void )
    {
        return m_SendBuffer;
    }


private:
    StreamReader    m_Reader;
    size_t          m_ReceivedSize = 0;
    Buffer          m_ReceiveBuffer;

    StreamWriter    m_Writer;
    size_t          m_SentSize = 0;
    Buffer          m_SendBuffer;
};



class CFlagsBuffer
{
public:
    CFlagsBuffer( void ) = default;
    virtual ~CFlagsBuffer( void );
        
    void Allocate( const std::vector< std::string >& _CFlags );
    void Deallocate( void );

    int32_t GetNumberOfCFlags( void ) const
    {
        return m_NumberOfCFlags;
    }
    const char* const * GetCFlags( void ) const
    {
        return m_CFlags;
    }

private:
    int32_t     m_NumberOfCFlags = 0;
    char**      m_CFlags         = nullptr;
};


class CSourceCodeBuffer
{
public:
    CSourceCodeBuffer( void ) = default;
    virtual ~CSourceCodeBuffer( void );
    
    void Allocate( int32_t _Size );
    void Deallocate( void );

    int32_t GetSize( void ) const
    {
        return m_Size;
    }
    char* GetBuffer( void ) const
    {
        return m_Buffer;
    }

private:
    enum
    {
        kInitialSrcBufferSize = 4096, 
    };

    int32_t m_Size           = 0;
    int32_t m_BufferCapacity = 0;
    char*   m_Buffer         = nullptr;
};



class ClangContext
{
public:
    ClangContext( bool _IsExcludeDeclarationsFromPCH = false );
    virtual ~ClangContext( void );

    void Allocate( void );
    void Deallocate( void );

    const CXIndex GetCXIndex( void ) const
    {
        return m_CxIndex;
    }
    CXIndex GetCXIndex( void )
    {
        return m_CxIndex;
    }

    void SetTranslationUnitFlags( uint32_t _Flags )
    {
        m_TranslationUnitFlags = _Flags;
    }
    uint32_t GetTranslationUnitFlags( void ) const
    {
        return m_TranslationUnitFlags;
    }

    void SetCompleteAtFlags( uint32_t _Flags )
    {
        m_CompleteAtFlags = _Flags;
    }
    uint32_t GetCompleteAtFlags( void ) const
    {
        return m_CompleteAtFlags;
    }
    
    void SetCompleteResultsLimit( uint32_t _NumberOfLimit )
    {
        m_CompleteResultsLimit = _NumberOfLimit;
    }
    uint32_t GetCompleteResultsLimit( void ) const
    {
        return m_CompleteResultsLimit;
    }
    
    
private:
    CXIndex             m_CxIndex;

    bool                m_ExcludeDeclarationsFromPCH;
    uint32_t            m_TranslationUnitFlags;
    uint32_t            m_CompleteAtFlags;
    uint32_t            m_CompleteResultsLimit;
};




template< uint32_t Value >
struct BitField
{
    enum
    {
        kValue = Value, 
        kIndex = BitField< (Value >> 1) >::kIndex + 1,
    };
};

template<>
struct BitField< 0 >
{
    enum
    {
        kValue = 0, 
        kIndex = -1, 
    };
};



class FlagConverter
{
public:
    using Details = std::tuple< const char*, uint32_t >;
    

    enum
    {
        kMaxValues = 32,
    };


    FlagConverter( void ) = default;
    virtual ~FlagConverter( void ) = default;

    void Clear( void )
    {
        m_MaxValue = 0;
    }


    void Add( const Details& _Values )
    {
        Add( std::get< 0 >( _Values ), std::get< 1 >( _Values ) );
    }
    
    void Add( const char* _Name, uint32_t _BitIndex )
    {
        assert( _Name );
        assert( _BitIndex < kMaxValues );

        m_FlagNames[ _BitIndex ] = _Name;
        m_MaxValue               = std::max( m_MaxValue, (_BitIndex + 1) );
    }

    uint32_t GetValue( const std::string& _Names ) const
    {
        return GetValue( _Names.c_str() );
    }

    uint32_t GetValue( const char* _Names ) const
    {
        if ( !_Names )
        {
            return 0;
        }

        std::string     names( _Names );
        const char*     delimit = "|";

        if ( *(names.rbegin()) != *delimit )
        {
            names += delimit;
        }

        uint32_t    value = 0;
        size_t      begin = 0;
        size_t      end   = names.find_first_of( delimit );
        
        while ( end != std::string::npos )
        {
            const size_t        length = end - begin;
            const std::string   name   = names.substr( begin, length );

            for ( size_t i = 0; i < m_MaxValue; ++i )
            {
                if ( m_FlagNames[ i ] == name )
                {
                    value |= (1 << i);
                    break;
                }
            }

            begin = end + 1;
            end   = names.find_first_of( delimit, begin );
        }

        return value;
    }

private:    
    std::string     m_FlagNames[ kMaxValues ];
    uint32_t        m_MaxValue = 0;
};


#define FLAG_DETAILS( _FLAG_NAME )          FlagConverter::Details( #_FLAG_NAME, BitField< _FLAG_NAME >::kIndex )




class ClangFlagConverters
{
public:
    ClangFlagConverters( void );


    static const FlagConverter& GetCXTranslationUnitFlags( void )
    {
        return sm_CXTranslationUnitFlags;
    }
    static const FlagConverter& GetCXCodeCompleteFlags( void )
    {
        return sm_CXCodeCompleteFlags;
    }

private:
    static FlagConverter       sm_CXTranslationUnitFlags;
    static FlagConverter       sm_CXCodeCompleteFlags;
};






#endif  // __COMMON_HPP__
/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
