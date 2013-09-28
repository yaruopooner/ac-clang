/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*	last updated : 2013/09/23.19:38:47 */

/*
 * Copyright (c) 2013 yaruopooner [https://github.com/yaruopooner]
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


/*================================================================================================*/
/*  Comment                                                                                       */
/*================================================================================================*/


/*================================================================================================*/
/*  Include Files                                                                                 */
/*================================================================================================*/

#include <string.h>
#include <stdarg.h>

#include "Common.hpp"


using	namespace	std;
// using	namespace	tr1;
using	namespace	std::tr1;



FlagConverter	ClangFlagConverters::sm_CXTranslationUnitFlags;
FlagConverter	ClangFlagConverters::sm_CXCodeCompleteFlags;



/*================================================================================================*/
/*  Global Class Method Definitions Section                                                       */
/*================================================================================================*/



StreamReader::StreamReader( void )
	:
	m_File( stdin )
{
	ClearLine();
}

StreamReader::~StreamReader( void )
{
}


void	StreamReader::ClearLine( void )
{
	::memset( m_Line, 0, kLineMax );
}


void	StreamReader::StepNextLine( void )
{
	char	crlf[ kLineMax ];
    ::fgets( crlf, kLineMax, m_File );
}

const char*	StreamReader::ReadToken( const char* Format, bool bStepNextLine )
{
	ClearLine();
	::fscanf( m_File, Format, m_Line );
	if ( bStepNextLine )
	{
		StepNextLine();
	}

	return ( m_Line );
}

void	StreamReader::Read( char* Buffer, size_t ReadSize )
{
	for ( size_t i = 0; i < ReadSize; ++i )
	{
		Buffer[ i ] = (char) ::fgetc( m_File );
	}
}


StreamWriter::StreamWriter( void )
	:
	m_File( stdout )
{
}

StreamWriter::~StreamWriter( void )
{
}


void	StreamWriter::Write( const char* Format, ... )
{
	va_list    args;
	va_start( args, Format );
	
	::vfprintf( m_File, Format, args );

	va_end( args );
}

void	StreamWriter::Flush( void )
{
	::fprintf( m_File, "$" );
	::fflush( m_File );
}




CFlagsBuffer::CFlagsBuffer( void )
	:
	m_NumberOfCFlags( 0 )
	, m_CFlags( nullptr )
{
}


CFlagsBuffer::~CFlagsBuffer( void )
{
	Deallocate();
}


void	CFlagsBuffer::Allocate( const std::vector< std::string >& Args )
{
	Deallocate();

	m_NumberOfCFlags = static_cast< int32_t >( Args.size() );
	m_CFlags		 = reinterpret_cast< char** >( ::calloc( sizeof( char* ), m_NumberOfCFlags ) );

	for ( int32_t i = 0; i < m_NumberOfCFlags; ++i )
	{
		m_CFlags[ i ] = reinterpret_cast< char* >( ::calloc( sizeof( char ), Args[ i ].length() + 1 ) );

		::strcpy( m_CFlags[ i ], Args[ i ].c_str() );
	}
}

void	CFlagsBuffer::Deallocate( void )
{
	if ( !m_CFlags )
	{
		return;
	}

	for ( int32_t i = 0; i < m_NumberOfCFlags; ++i )
	{
		::free( m_CFlags[ i ] );
	}
	::free( m_CFlags );

	m_CFlags		 = nullptr;
	m_NumberOfCFlags = 0;
}


CSourceCodeBuffer::CSourceCodeBuffer( void )
	:
	m_Size( 0 )
	, m_BufferCapacity( 0 )
	, m_Buffer( nullptr )
{
}

CSourceCodeBuffer::~CSourceCodeBuffer( void )
{
	Deallocate();
}


void	CSourceCodeBuffer::Allocate( int32_t Size )
{
	m_Size = Size;

	if ( m_Size >= m_BufferCapacity )
	{
		m_BufferCapacity = std::max( m_Size * 2, static_cast< int32_t >( kInitialSrcBufferSize ) );
		m_Buffer		 = reinterpret_cast< char* >( ::realloc( m_Buffer, m_BufferCapacity ) );
	}
}

void	CSourceCodeBuffer::Deallocate( void )
{
	if ( m_Buffer )
	{
		::free( m_Buffer );
		m_Buffer = nullptr;
	}

	m_Size = 0;
}



ClangContext::ClangContext( bool excludeDeclarationsFromPCH )
	:
	m_CxIndex( nullptr )
	, m_TranslationUnitFlags( CXTranslationUnit_PrecompiledPreamble )
	, m_CompleteAtFlags( CXCodeComplete_IncludeMacros )
{
	m_CxIndex = clang_createIndex( excludeDeclarationsFromPCH, 0 );
}

ClangContext::~ClangContext( void )
{
	if ( m_CxIndex )
	{
		clang_disposeIndex( m_CxIndex );
		m_CxIndex = nullptr;
	}
}



ClangFlagConverters::ClangFlagConverters( void )
{
	sm_CXTranslationUnitFlags.Add( FLAG_DETAILS( CXTranslationUnit_DetailedPreprocessingRecord ) );
	sm_CXTranslationUnitFlags.Add( FLAG_DETAILS( CXTranslationUnit_Incomplete ) );
	sm_CXTranslationUnitFlags.Add( FLAG_DETAILS( CXTranslationUnit_PrecompiledPreamble ) );
	sm_CXTranslationUnitFlags.Add( FLAG_DETAILS( CXTranslationUnit_CacheCompletionResults ) );
	sm_CXTranslationUnitFlags.Add( FLAG_DETAILS( CXTranslationUnit_ForSerialization ) );
	sm_CXTranslationUnitFlags.Add( FLAG_DETAILS( CXTranslationUnit_CXXChainedPCH ) );
	sm_CXTranslationUnitFlags.Add( FLAG_DETAILS( CXTranslationUnit_SkipFunctionBodies ) );
	sm_CXTranslationUnitFlags.Add( FLAG_DETAILS( CXTranslationUnit_IncludeBriefCommentsInCodeCompletion ) );

	sm_CXCodeCompleteFlags.Add( FLAG_DETAILS( CXCodeComplete_IncludeMacros ) );
	sm_CXCodeCompleteFlags.Add( FLAG_DETAILS( CXCodeComplete_IncludeCodePatterns ) );
	sm_CXCodeCompleteFlags.Add( FLAG_DETAILS( CXCodeComplete_IncludeBriefComments ) );
}





/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
