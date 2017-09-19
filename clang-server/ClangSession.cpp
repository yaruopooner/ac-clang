/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/09/19.16:33:02 */

/*
 * Copyright (c) 2013-2017 yaruopooner [https://github.com/yaruopooner]
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

#include <regex>

#include "ClangSession.hpp"


using   namespace   std;
using   json = nlohmann::json;



/*================================================================================================*/
/*  Internal Printer Class Definitions Section                                                    */
/*================================================================================================*/


namespace
{

template< typename Resource >
class ScopedClangResource
{
public:
    using Disposer = std::function< void ( Resource ) >;

    ScopedClangResource( Resource _Resource, Disposer _Disposer ) : 
        m_Resource( _Resource )
        , m_Disposer( _Disposer )
    {
    }

    ScopedClangResource( CXString _Resource ) : ScopedClangResource( _Resource, clang_disposeString ) {};
    ScopedClangResource( CXCodeCompleteResults* _Resource ) : ScopedClangResource( _Resource, clang_disposeCodeCompleteResults ) {};
    ScopedClangResource( CXDiagnostic _Resource ) : ScopedClangResource( _Resource, clang_disposeDiagnostic ) {};

    ScopedClangResource&    operator =( Resource _Resource )
    {
        new( this ) ScopedClangResource( _Resource );

        return *this;
    }

    // ScopedClangResource&    operator =( CXString _Resource )
    // {
    //     new( this ) ScopedClangResource( _Resource );

    //     return *this;
    // }
    // ScopedClangResource&    operator =( CXCodeCompleteResults _Resource )
    // {
    //     new( this ) ScopedClangResource( _Resource );

    //     return *this;
    // }
    // ScopedClangResource&    operator =( CXDiagnostic _Resource )
    // {
    //     new( this ) ScopedClangResource( _Resource );

    //     return *this;
    // }


    ~ScopedClangResource( void )
    {
        m_Disposer( m_Resource );
    }

    Resource    GetResource( void ) const
    {
        return m_Resource;
    }

    Resource    operator ()( void ) const
    {
        return m_Resource;
    }

private:
    Resource    m_Resource;
    Disposer    m_Disposer;
};





std::string CXStringToString( CXString _String )
{
    const char*         c_text = clang_getCString( _String );
    const std::string   text   = c_text ? c_text : "";

    clang_disposeString( _String );

    return text;
}


string  GetNormalizePath( CXFile _File )
{
    const std::string   path( ::CXStringToString( clang_getFileName( _File ) ) );
    const std::regex    expression( "[\\\\]+" );
    const std::string   replace( "/" );
    const std::string   normalize_path = regex_replace( path, expression, replace );

    return normalize_path;
}



}




class Completion_ChunkIterator
{
public:
    Completion_ChunkIterator( CXCompletionString _CompletionString ) :
        m_CompletionString( _CompletionString )
    {
        m_MaxIndex = clang_getNumCompletionChunks( m_CompletionString );
    }

    // ~Completion_ChunkIterator( void );


    uint32_t    GetMaxIndex( void ) const
    {
        return m_MaxIndex;
    }

    bool    HasNext( void ) const
    {
        return ( m_Index < m_MaxIndex );
    }

    void    Next( void )
    {
        if ( HasNext() )
        {
            m_Index++;
        }
    }

    void    Rewind( void )
    {
        m_Index = 0;
    }

    CXCompletionChunkKind   GetChunkKind( void ) const
    {
        return clang_getCompletionChunkKind( m_CompletionString, m_Index );
    }

    Completion_ChunkIterator   GetOptionalChunkIterator( void ) const
    {
        return Completion_ChunkIterator( clang_getCompletionChunkCompletionString( m_CompletionString, m_Index ) );
    }
    
    std::string GetString( void ) const
    {
        CXString            cx_chunk_text = clang_getCompletionChunkText( m_CompletionString, m_Index );
        const std::string   chunk_text    = clang_getCString( cx_chunk_text );

        clang_disposeString( cx_chunk_text );

        return chunk_text;
    }

    void GetString( std::string& _Text ) const
    {
        CXString                cx_string = clang_getCompletionChunkText( m_CompletionString, m_Index );

        _Text = clang_getCString( cx_string );

        clang_disposeString( cx_string );
    }

private:
    CXCompletionString  m_CompletionString;
    uint32_t            m_Index = 0;
    uint32_t            m_MaxIndex;
};



class Completion_AnnotationIterator
{
public:
    Completion_AnnotationIterator( CXCompletionString _CompletionString ) :
        m_CompletionString( _CompletionString )
    {
        m_MaxIndex = clang_getCompletionNumAnnotations( m_CompletionString );
    }


    uint32_t    GetMaxIndex( void ) const
    {
        return m_MaxIndex;
    }

    bool    HasNext( void ) const
    {
        return ( m_Index < m_MaxIndex );
    }

    void    Next( void )
    {
        if ( HasNext() )
        {
            m_Index++;
        }
    }

    void    Rewind( void )
    {
        m_Index = 0;
    }

    std::string GetString( void ) const
    {
        return ::CXStringToString( clang_getCompletionAnnotation( m_CompletionString, m_Index ) );
    }

private:
    CXCompletionString  m_CompletionString;
    uint32_t            m_Index = 0;
    uint32_t            m_MaxIndex;
};



class Completion_Holder
{
public:
    Completion_Holder( CXCompletionString _CompletionString ) :
        m_CompletionString( _CompletionString )
    {
    }


    CXAvailabilityKind  GetAvailabilityKind( void ) const
    {
        return clang_getCompletionAvailability( m_CompletionString );
    }

    Completion_ChunkIterator   GetChunkIterator( void ) const
    {
        return Completion_ChunkIterator( m_CompletionString );
    }

    Completion_AnnotationIterator   GetAnnotationIterator( void ) const
    {
        return Completion_AnnotationIterator( m_CompletionString );
    }

    std::string GetBriefComment( void ) const
    {
        return ::CXStringToString( clang_getCompletionBriefComment( m_CompletionString ) );
    }

private:
    CXCompletionString  m_CompletionString;
};




class ClangSession::Completion
{
public:
    struct Candidate
    {
        Candidate( void );
        Candidate( CXCompletionString _CompletionString );

        bool    Parse( Completion_Holder& _Holder );
        bool    ParseChunk( Completion_ChunkIterator& _Iterator );

        bool                        m_IsValid              = false;
        std::string                 m_Name;
        std::ostringstream          m_Prototype;
        std::string                 m_ResultType;
        std::ostringstream          m_Snippet;
        std::ostringstream          m_DisplayText;
        std::string                 m_BriefComment;
        std::vector< std::string >  m_Annotations;
        uint32_t                    m_NumberOfPlaceHolders = 0;
    };


    Completion( ClangSession& _Session ) :
        m_Session( _Session )
    {
    }

    void    PrintCompleteCandidates( void );

private:
    bool    Evaluate( void );

    // void    GenerateCandidate( CXCompletionString CompletionString );

private:
    ClangSession&               m_Session;
    std::vector< Candidate >    m_Candidates;
    std::ostringstream          m_Error;
};


class ClangSession::Diagnostics
{
public:
    // struct Diagnostic
    // {
    //     string      m_message;
    // };

    Diagnostics( ClangSession& _Session ) :
        m_Session( _Session )
    {
    }

    bool    Evaluate( void );
    void    PrintDiagnosticsResults( void );

private:
    ClangSession&               m_Session;
    // std::vector< Diagnostic >   m_Diagnostics;
    std::vector< std::string >  m_Diagnostics;
    std::ostringstream          m_Error;
};


class ClangSession::Jump
{
public:
    struct Location
    {
        string      m_NormalizePath;
        uint32_t    m_Line   = 0;
        uint32_t    m_Column = 0;
    };


    Jump( ClangSession& _Session ) :
        m_Session( _Session )
    {
    }
        
    void    PrintInclusionFileLocation( void );
    void    PrintDefinitionLocation( void );
    void    PrintDeclarationLocation( void );
    void    PrintSmartJumpLocation( void );

private:
    CXCursor    GetCursor( const uint32_t _Line, const uint32_t _Column ) const;
    void    PrepareTransaction( uint32_t& _Line, uint32_t& _Column );
    bool    EvaluateCursorLocation( const CXCursor& _Cursor );

    bool    EvaluateInclusionFileLocation( void );
    bool    EvaluateDefinitionLocation( void );
    bool    EvaluateDeclarationLocation( void );
    bool    EvaluateSmartJumpLocation( void );
    void    PrintLocation( void );

private:
    ClangSession&       m_Session;
    Location            m_Location;
    std::ostringstream  m_Error;
};





ClangSession::Completion::Candidate::Candidate( void )
{
}

ClangSession::Completion::Candidate::Candidate( CXCompletionString _CompletionString )
{
    Completion_Holder   holder( _CompletionString );

    Parse( holder );
}


bool    ClangSession::Completion::Candidate::Parse( Completion_Holder& _Holder )
{
    // check accessibility of candidate. (access specifier of member : public/protected/private)
    if ( _Holder.GetAvailabilityKind() == CXAvailability_NotAccessible )
    {
        return false;
    }

    {
        Completion_ChunkIterator    iterator = _Holder.GetChunkIterator();

        ParseChunk( iterator );
    }

    m_BriefComment = _Holder.GetBriefComment();

#if 0
    // test code
    {
        Completion_AnnotationIterator   iterator = _Holder.GetAnnotationIterator();

        if ( iterator.GetMaxIndex() )
        {
            m_Annotations.reserve( iterator.GetMaxIndex() );

            for ( ; iterator.HasNext(); iterator.Next() )
            {
                m_Annotations.emplace_back( iterator.GetString() );
            }
        }
    }
#endif

    m_IsValid = true;

    return true;
}

bool    ClangSession::Completion::Candidate::ParseChunk( Completion_ChunkIterator& _Iterator )
{
    for ( ; _Iterator.HasNext(); _Iterator.Next() )
    {
        switch ( _Iterator.GetChunkKind() )
        {
            case CXCompletionChunk_TypedText:
                m_Prototype << _Iterator.GetString();
                m_Name = _Iterator.GetString();
                break;
            case CXCompletionChunk_ResultType:
                m_Prototype << "[#" << _Iterator.GetString() << "#]";
                break;
            case CXCompletionChunk_Placeholder:
                m_NumberOfPlaceHolders++;
                m_Prototype << "<#" << _Iterator.GetString() << "#>";
                break;
            case CXCompletionChunk_Optional:
                m_NumberOfPlaceHolders++;
                m_Prototype << "{#";
                ParseChunk( _Iterator.GetOptionalChunkIterator() );
                m_Prototype << "#}";
                break;
            default:
                m_Prototype << _Iterator.GetString();
                break;
        }
    }

    return true;
}




bool    ClangSession::Completion::Evaluate( void )
{
    const uint32_t  line   = m_Session.m_ReceivedCommand[ "Line" ];
    const uint32_t  column = m_Session.m_ReceivedCommand[ "Column" ];

    m_Session.ReadSourceCode();

    CXUnsavedFile                                   unsaved_file = m_Session.GetCXUnsavedFile();
    // necessary call?
    // clang_reparseTranslationUnit( m_Session.m_CxTU, 1, &unsaved_file, m_Session.m_TranslationUnitFlags );
    ScopedClangResource< CXCodeCompleteResults* >   complete_results( clang_codeCompleteAt( m_Session.m_CxTU, m_Session.m_SessionName.c_str(), line, column, &unsaved_file, 1, m_Session.m_CompleteAtFlags ) );


    if ( !complete_results() )
    {
        // logic failed?
        m_Error << " /[ClangSession::Completion] CXCodeCompleteResults is null pointer!!";

        return false;
    }

    // clang_codeCompleteAt returns the entity address even if the completion candidate is 0 ( CXCodeCompleteResults::NumResults == 0 )

    // limit check
    {
        const uint32_t  results_limit = m_Session.m_Context.GetCompleteResultsLimit();
        const bool      is_accept     = results_limit ? ( complete_results()->NumResults < results_limit ) : true;

        if ( !is_accept )
        {
            m_Error << " /[ClangSession::Completion] A number of completion results(" << complete_results()->NumResults << ") is threshold value(" << results_limit << ") over!!";

            return false;
        }
    }


    // candidates sort
    clang_sortCodeCompletionResults( complete_results()->Results, complete_results()->NumResults );

    // memory reserve
    m_Candidates.reserve( complete_results()->NumResults );

    for ( uint32_t i = 0; i < complete_results()->NumResults; ++i )
    {
        m_Candidates.emplace_back( complete_results()->Results[ i ].CompletionString );

        // Candidate&  candidate = *m_Candidates.rbegin();

        // if ( candidate.m_IsValid )
        // {
        // }
    }


    return true;
}

void    ClangSession::Completion::PrintCompleteCandidates( void )
{
    Evaluate();


    json&   results = m_Session.m_CommandResults;

    results[ "RequestId" ] = m_Session.m_ReceivedCommand[ "RequestId" ];

    for ( const auto& candidate : m_Candidates )
    {
        if ( candidate.m_IsValid )
        {
            if ( candidate.m_BriefComment.empty() )
            {
                results[ "Results" ].push_back( 
                                               {
                                                   { "Name", candidate.m_Name }, 
                                                   { "Prototype", candidate.m_Prototype.str() }, 
                                               }
                                                );
            }
            else
            {
                results[ "Results" ].push_back( 
                                               {
                                                   { "Name", candidate.m_Name }, 
                                                   { "Prototype", candidate.m_Prototype.str() }, 
                                                   { "BriefComment", candidate.m_BriefComment }, 
                                               }
                                                );
            }
        }
    }

    if ( !m_Error.str().empty() )
    {
        results[ "Error" ] = m_Error.str();
    }
}




#if 0
void    ClangSession::Completion::GenerateCandidate( CXCompletionString CompletionString )
{
    // check accessibility of candidate. (access specifier of member : public/protected/private)
    if ( clang_getCompletionAvailability( CompletionString ) == CXAvailability_NotAccessible )
    {
        // return false;
        return;
    }

    Candidate       candidate;

    for ( Completion_ChunkIterator it( CompletionString ); it.HasNext(); it.Next() )
    {
        switch ( it.GetChunkKind() )
        {
            case CXCompletionChunk_ResultType:
                candidate.m_ResultType = it.GetString();
                candidate.m_DisplayText << candidate.m_ResultType;
                break;
            case CXCompletionChunk_TypedText:
                candidate.m_Name = it.GetString();
                break;
            case CXCompletionChunk_Placeholder:
                candidate.m_NumberOfPlaceHolders++;
                candidate.m_Prototype << it.GetString();
                // candidate.m_Snippet += "${1:" + it.GetString() + "}";
                // candidate.m_Snippet << "${" << candidate.m_NumberOfPlaceHolders << ":" << it.GetString() << "}";
                candidate.m_Snippet << "${" << it.GetString() << "}";
                // candidate.m_Snippet += "${" + std::to_string( candidate.m_NumberOfPlaceHolders ) + it.GetString() + "}";
                break;
            case CXCompletionChunk_Optional:
                candidate.m_Prototype << it.GetString();
                break;
            case CXCompletionChunk_LeftParen:
                candidate.m_Prototype << '(';
                break;
            case CXCompletionChunk_RightParen:
                candidate.m_Prototype << ')';
                break;
            case CXCompletionChunk_LeftBracket:
                candidate.m_Prototype << '[';
                break;
            case CXCompletionChunk_RightBracket:
                candidate.m_Prototype << ']';
                break;
            case CXCompletionChunk_LeftBrace:
                candidate.m_Prototype << '{';
                break;
            case CXCompletionChunk_RightBrace:
                candidate.m_Prototype << '}';
                break;
            case CXCompletionChunk_LeftAngle:
                candidate.m_Prototype << '<';
                break;
            case CXCompletionChunk_RightAngle:
                candidate.m_Prototype << '>';
                break;
            case CXCompletionChunk_Comma:
                candidate.m_Prototype << ',';
                if ( candidate.m_NumberOfPlaceHolders > 0 )
                {
                    candidate.m_Snippet << ',';
                }
                break;
            case CXCompletionChunk_Colon:
                candidate.m_Prototype << ':';
                break;
            case CXCompletionChunk_SemiColon:
                candidate.m_Prototype << ';';
                break;
            case CXCompletionChunk_Equal:
                candidate.m_Prototype << '=';
                break;
            case CXCompletionChunk_HorizontalSpace:
                candidate.m_Prototype << ' ';
                break;
            case CXCompletionChunk_VerticalSpace:
                candidate.m_Prototype << '\n';
                break;
            default:
                break;
        }
    }

    candidate.m_BriefComment = ::CXStringToString( clang_getCompletionBriefComment( CompletionString ) );

    const uint32_t  n_chunks = clang_getNumCompletionChunks( CompletionString );

    // inspect all chunks only to find the TypedText chunk
    for ( uint32_t i_chunk = 0; i_chunk < n_chunks; ++i_chunk )
    {
        if ( clang_getCompletionChunkKind( CompletionString, i_chunk ) == CXCompletionChunk_TypedText )
        {
            // We got it, just dump it to fp
            CXString    ac_string = clang_getCompletionChunkText( CompletionString, i_chunk );
            std::string text = clang_getCString( ac_string );

            candidate.m_Name = text;

            clang_disposeString( ac_string );

            // care package on the way
            // return true;
        }
    }




    // print completion item head: COMPLETION: typed_string
    if ( PrintCompletionHeadTerm( CompletionString ) )
    {
        // If there's not only one TypedText chunk in this completion string,
        //  * we still have a lot of info to dump:
        //  *
        //  *     COMPLETION: typed_text : ##infos## \n
        m_Session.m_Writer.Write( " : " );

        PrintAllCompletionTerms( CompletionString );

        m_Session.m_Writer.Write( "\n" );
    }
}
#endif




bool    ClangSession::Diagnostics::Evaluate( void )
{
    m_Session.ReadSourceCode();
    
    CXUnsavedFile   unsaved_file = m_Session.GetCXUnsavedFile();

    clang_reparseTranslationUnit( m_Session.m_CxTU, 1, &unsaved_file, m_Session.m_TranslationUnitFlags );

    const uint32_t  n_diagnostics = clang_getNumDiagnostics( m_Session.m_CxTU );

    // memory reserve
    m_Diagnostics.reserve( n_diagnostics );

    for ( uint32_t i = 0; i < n_diagnostics; ++i )
    {
        ScopedClangResource< CXDiagnostic > diagnostic( clang_getDiagnostic( m_Session.m_CxTU, i ) );
        const std::string                   message( ::CXStringToString( clang_formatDiagnostic( diagnostic(), clang_defaultDiagnosticDisplayOptions() ) ) );

        m_Diagnostics.emplace_back( message );
    }

    return true;
}


void    ClangSession::Diagnostics::PrintDiagnosticsResults( void )
{
    Evaluate();

    ostringstream   diagnostics;

    for ( const auto& message : m_Diagnostics )
    {
        diagnostics << message << std::endl;
    }

    json&   results = m_Session.m_CommandResults;

    results[ "RequestId" ] = m_Session.m_ReceivedCommand[ "RequestId" ];
    results[ "Results" ]   = { { "Diagnostics", diagnostics.str() } };

    if ( !m_Error.str().empty() )
    {
        results[ "Error" ] = m_Error.str();
    }
}



CXCursor    ClangSession::Jump::GetCursor( const uint32_t _Line, const uint32_t _Column ) const
{
    const CXFile            file     = clang_getFile( m_Session.m_CxTU, m_Session.m_SessionName.c_str() );
    const CXSourceLocation  location = clang_getLocation( m_Session.m_CxTU, file, _Line, _Column );
    const CXCursor          cursor   = clang_getCursor( m_Session.m_CxTU, location );

    return cursor;
}

void    ClangSession::Jump::PrepareTransaction( uint32_t& _Line, uint32_t& _Column )
{
    // const uint32_t  line   = m_Session.m_ReceivedCommand[ "Line" ];
    // const uint32_t  column = m_Session.m_ReceivedCommand[ "Column" ];

    _Line   = m_Session.m_ReceivedCommand[ "Line" ];
    _Column = m_Session.m_ReceivedCommand[ "Column" ];
    
    m_Session.ReadSourceCode();

    CXUnsavedFile       unsaved_file = m_Session.GetCXUnsavedFile();

    clang_reparseTranslationUnit( m_Session.m_CxTU, 1, &unsaved_file, m_Session.m_TranslationUnitFlags );
}


bool    ClangSession::Jump::EvaluateCursorLocation( const CXCursor& _Cursor )
{
    if ( clang_isInvalid( _Cursor.kind ) )
    {
        m_Error << " /[ClangSession::Jump] cursor is invalid.";

        return false;
    }

    const CXSourceLocation  dest_location = clang_getCursorLocation( _Cursor );
    CXFile                  dest_file;
    uint32_t                dest_line;
    uint32_t                dest_column;
    uint32_t                dest_offset;

    clang_getExpansionLocation( dest_location, &dest_file, &dest_line, &dest_column, &dest_offset );

    if ( !dest_file )
    {
        m_Error << " /[ClangSession::Jump] CXFile is null pointer.";

        return false;
    }

    const string        normalize_path = ::GetNormalizePath( dest_file );
    
    m_Location.m_NormalizePath = normalize_path;
    m_Location.m_Line          = dest_line;
    m_Location.m_Column        = dest_column;

    return true;
}


bool    ClangSession::Jump::EvaluateInclusionFileLocation( void )
{
    uint32_t        line;
    uint32_t        column;

    PrepareTransaction( line, column );

    const CXCursor    source_cursor = GetCursor( line, column );

    if ( !clang_isInvalid( source_cursor.kind ) )
    {
        if ( source_cursor.kind == CXCursor_InclusionDirective )
        {
            const CXFile  file = clang_getIncludedFile( source_cursor );

            if ( file )
            {
                // file top location
                const uint32_t  file_line      = 1;
                const uint32_t  file_column    = 1;
                const string    normalize_path = ::GetNormalizePath( file );

                m_Location.m_NormalizePath = normalize_path;
                m_Location.m_Line          = file_line;
                m_Location.m_Column        = file_column;

                return true;
            }
        }
    }

    m_Error << " /[ClangSession::Jump] cursor kind is not CXCursor_InclusionDirective.";

    return false;
}

bool    ClangSession::Jump::EvaluateDefinitionLocation( void )
{
    uint32_t        line;
    uint32_t        column;

    PrepareTransaction( line, column );

    const CXCursor    source_cursor = GetCursor( line, column );

    if ( !clang_isInvalid( source_cursor.kind ) )
    {
        return EvaluateCursorLocation( clang_getCursorDefinition( source_cursor ) );
    }

    m_Error << " /[ClangSession::Jump] cursor is invalid.";

    return false;
}

bool    ClangSession::Jump::EvaluateDeclarationLocation( void )
{
    uint32_t        line;
    uint32_t        column;

    PrepareTransaction( line, column );

    const CXCursor    source_cursor = GetCursor( line, column );

    if ( !clang_isInvalid( source_cursor.kind ) )
    {
        return EvaluateCursorLocation( clang_getCursorReferenced( source_cursor ) );
    }

    m_Error << " /[ClangSession::Jump] cursor is invalid.";

    return false;
}

bool    ClangSession::Jump::EvaluateSmartJumpLocation( void )
{
    uint32_t        line;
    uint32_t        column;

    PrepareTransaction( line, column );

    const CXCursor    source_cursor = GetCursor( line, column );

    if ( !clang_isInvalid( source_cursor.kind ) )
    {
        if ( source_cursor.kind == CXCursor_InclusionDirective )
        {
            const CXFile  file = clang_getIncludedFile( source_cursor );

            if ( file )
            {
                // file top location
                const uint32_t  file_line      = 1;
                const uint32_t  file_column    = 1;
                const string    normalize_path = ::GetNormalizePath( file );

                m_Location.m_NormalizePath = normalize_path;
                m_Location.m_Line          = file_line;
                m_Location.m_Column        = file_column;

                return true;
            }
        }
        else
        {
            return ( EvaluateCursorLocation( clang_getCursorDefinition( source_cursor ) ) 
                     || EvaluateCursorLocation( clang_getCursorReferenced( source_cursor ) ) );
        }
    }

    m_Error << " /[ClangSession::Jump] cursor is invalid.";

    return false;
}


void    ClangSession::Jump::PrintLocation( void )
{
    json&   results = m_Session.m_CommandResults;

    const uint32_t  request_id = m_Session.m_ReceivedCommand[ "RequestId" ];

    results[ "RequestId" ] = request_id;
    results[ "Results" ] = 
    {
        { "Path", m_Location.m_NormalizePath }, 
        { "Line", m_Location.m_Line }, 
        { "Column", m_Location.m_Column }, 
    };

    if ( !m_Error.str().empty() )
    {
        results[ "Error" ] = m_Error.str();
    }
}


void    ClangSession::Jump::PrintInclusionFileLocation( void )
{
    EvaluateInclusionFileLocation();

    PrintLocation();
}

void    ClangSession::Jump::PrintDefinitionLocation( void )
{
    EvaluateDefinitionLocation();

    PrintLocation();
}

void    ClangSession::Jump::PrintDeclarationLocation( void )
{
    EvaluateDeclarationLocation();

    PrintLocation();
}

void    ClangSession::Jump::PrintSmartJumpLocation( void )
{
    EvaluateSmartJumpLocation();

    PrintLocation();
}




/*================================================================================================*/
/*  Global Class Method Definitions Section                                                       */
/*================================================================================================*/


ClangSession::ClangSession( const std::string& SessionName, const ClangContext& Context, nlohmann::json& ReceivedCommand, nlohmann::json& CommandResults, StreamWriter& Writer )
    :
    m_SessionName( SessionName )
    , m_Context( Context )
    , m_ReceivedCommand( ReceivedCommand )
    , m_CommandResults( CommandResults )
    , m_Reader( *reinterpret_cast< StreamReader* >( nullptr ) )
    , m_Writer( Writer )
    , m_CxTU( nullptr )
    , m_TranslationUnitFlags( Context.GetTranslationUnitFlags() )
    , m_CompleteAtFlags( Context.GetCompleteAtFlags() )
{
}


ClangSession::ClangSession( const std::string& SessionName, const ClangContext& Context, StreamReader& Reader, StreamWriter& Writer )
    :
    m_SessionName( SessionName )
    , m_Context( Context )
    , m_ReceivedCommand( *reinterpret_cast< nlohmann::json* >( nullptr ) )
    , m_CommandResults( *reinterpret_cast< nlohmann::json* >( nullptr ) )
    , m_Reader( Reader )
    , m_Writer( Writer )
    , m_CxTU( nullptr )
    , m_TranslationUnitFlags( Context.GetTranslationUnitFlags() )
    , m_CompleteAtFlags( Context.GetCompleteAtFlags() )
{
}


ClangSession::~ClangSession( void )
{
    Deallocate();
}



void    ClangSession::ReadCFlags( void )
{
    const std::vector< std::string >    cflags = m_ReceivedCommand[ "CFLAGS" ];

    m_CFlagsBuffer.Allocate( cflags );
}

void    ClangSession::ReadSourceCode( void )
{
    const std::string   source_code = m_ReceivedCommand[ "SourceCode" ];
    
    m_CSourceCodeBuffer.Allocate( source_code.size() );
    source_code.copy( m_CSourceCodeBuffer.GetAddress< char* >(), source_code.size() );
}



void    ClangSession::CreateTranslationUnit( void )
{
    if ( m_CxTU )
    {
        // clang parser already exist
        return;
    }

    CXUnsavedFile               unsaved_file = GetCXUnsavedFile();

    m_CxTU = clang_parseTranslationUnit( m_Context.GetCXIndex(), m_SessionName.c_str(), 
                                         static_cast< const char * const *>( m_CFlagsBuffer.GetCFlags() ), m_CFlagsBuffer.GetNumberOfCFlags(), 
                                         &unsaved_file, 1, m_TranslationUnitFlags );
                                         
    clang_reparseTranslationUnit( m_CxTU, 1, &unsaved_file, m_TranslationUnitFlags );
}

void    ClangSession::DeleteTranslationUnit( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    clang_disposeTranslationUnit( m_CxTU );
    m_CxTU = nullptr;
}



void    ClangSession::Allocate( void )
{
    ReadCFlags();
    ReadSourceCode();
    CreateTranslationUnit();
}

void    ClangSession::Deallocate( void )
{
    DeleteTranslationUnit();
}


void    ClangSession::commandSuspend( void )
{
    DeleteTranslationUnit();
}

void    ClangSession::commandResume( void )
{
    CreateTranslationUnit();
}


void    ClangSession::commandSetCFlags( void )
{
    DeleteTranslationUnit();
    ReadCFlags();
    ReadSourceCode();
    CreateTranslationUnit();
}


void    ClangSession::commandSetSourceCode( void )
{
    ReadSourceCode();
}

void    ClangSession::commandReparse( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    CXUnsavedFile               unsaved_file = GetCXUnsavedFile();

    clang_reparseTranslationUnit( m_CxTU, 1, &unsaved_file, m_TranslationUnitFlags );
}


void    ClangSession::commandCompletion( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    Completion                  command( *this );

    command.PrintCompleteCandidates();
}


void    ClangSession::commandDiagnostics( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    Diagnostics                 command( *this );

    command.PrintDiagnosticsResults();
}


void    ClangSession::commandInclusion( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    Jump                        command( *this );

    command.PrintInclusionFileLocation();
}


void    ClangSession::commandDeclaration( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    Jump                        command( *this );

    command.PrintDeclarationLocation();
}


void    ClangSession::commandDefinition( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    Jump                        command( *this );

    command.PrintDefinitionLocation();
}


void    ClangSession::commandSmartJump( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    Jump                        command( *this );

    command.PrintSmartJumpLocation();
}





/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
