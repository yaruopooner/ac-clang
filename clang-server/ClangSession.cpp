/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2018/01/11.07:17:13 */

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


/*================================================================================================*/
/*  Comment                                                                                       */
/*================================================================================================*/


/*================================================================================================*/
/*  Include Files                                                                                 */
/*================================================================================================*/

#include <regex>

#include "Profiler.hpp"
#include "ClangSession.hpp"
#include "DataObject.hpp"




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

    ScopedClangResource( Resource _Resource ) : ScopedClangResource( _Resource, nullptr ) {};
    // ScopedClangResource( CXString _Resource ) : ScopedClangResource( _Resource, clang_disposeString ) {};
    // ScopedClangResource( CXCodeCompleteResults* _Resource ) : ScopedClangResource( _Resource, clang_disposeCodeCompleteResults ) {};
    // ScopedClangResource( CXDiagnostic _Resource ) : ScopedClangResource( _Resource, clang_disposeDiagnostic ) {};

    ScopedClangResource& operator =( Resource _Resource )
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

    Resource GetResource( void ) const
    {
        return m_Resource;
    }

    Resource operator ()( void ) const
    {
        return m_Resource;
    }

private:
    Resource    m_Resource;
    Disposer    m_Disposer;
};


// template<>
// ScopedClangResource< CXString >::ScopedClangResource( CXString _Resource ) : 
//     ScopedClangResource( _Resource, clang_disposeString ) {};

template<>
ScopedClangResource< CXCodeCompleteResults* >::ScopedClangResource( CXCodeCompleteResults* _Resource ) : 
    ScopedClangResource( _Resource, clang_disposeCodeCompleteResults ) {};

template<>
ScopedClangResource< CXDiagnostic >::ScopedClangResource( CXDiagnostic _Resource ) :
    ScopedClangResource( _Resource, clang_disposeDiagnostic ) {};





std::string CXStringToString( CXString _String )
{
    const char*         c_text = clang_getCString( _String );
    const std::string   text   = c_text ? c_text : "";

    clang_disposeString( _String );

    return text;
}


std::string GetNormalizePath( CXFile _File )
{
    const std::string   path( ::CXStringToString( clang_getFileName( _File ) ) );
    const std::regex    expression( "[\\\\]+" );
    const std::string   replace( "/" );
    const std::string   normalize_path = regex_replace( path, expression, replace );

    return normalize_path;
}




} // namespace



class ClangSession::Command::ReadCFlags : public IMultiSerializable
{
public:
    ReadCFlags( ClangSession& _Session ) : 
        m_Session( _Session )
    {
    }

    virtual void Read( const Lisp::Text::Object& _InData ) override
    {
        Lisp::SAS::DetectHandler    handler;
        Lisp::SAS::Parser           parser;
        uint32_t                    read_count       = 0;
        bool                        is_detect_cflags = false;
        std::vector< std::string >  cflags;

        cflags.reserve( 128 );

        handler.m_OnEnterSequence = [&is_detect_cflags]( Lisp::SAS::DetectHandler::SequenceContext& _Context ) -> bool
            {
                is_detect_cflags = ( (*_Context.m_ParentSymbol) == ":CFLAGS" );

                if ( !is_detect_cflags )
                {
                    _Context.m_Mode = Lisp::SAS::DetectHandler::SequenceContext::kPropertyList;
                }

                return true;
            };
        handler.m_OnAtom = [&is_detect_cflags, &cflags]( const size_t _Index, const Lisp::SAS::SExpression& _SExpression ) -> bool
            {
                if ( is_detect_cflags )
                {
                    cflags.emplace_back( _SExpression.GetValueString() );
                }

                return true;
            };
        handler.m_OnProperty = [this, &read_count, &cflags]( const size_t _Index, const std::string& _Symbol, const Lisp::SAS::SExpression& _SExpression ) -> bool
            {
                if ( _Symbol == ":CFLAGS" )
                {
                    m_Session.m_CFlagsBuffer.Allocate( cflags );
                    ++read_count;
                }

                if ( read_count == 1 )
                {
                    return false;
                }

                return true;
            };

        parser.Parse( _InData, handler );
    }

    virtual void Read( const Lisp::Node::Object& _InData ) override
    {
        Lisp::Node::PropertyListIterator    iterator = _InData.GetRootPropertyListIterator();
        std::vector< std::string >          cflags;

        cflags.reserve( 128 );

        for ( ; !iterator.IsEnd(); iterator.Next() )
        {
            if ( iterator.IsSameKey( ":CFLAGS" ) )
            {
                Lisp::Node::Iterator    cflags_iterator = iterator.GetValueElementIterator();

                for ( ; !cflags_iterator.IsEnd(); cflags_iterator.Next() )
                {
                    cflags.emplace_back( cflags_iterator.RefValue< std::string >() );
                }
                break;
            }
        }

        m_Session.m_CFlagsBuffer.Allocate( cflags );
    }

    virtual void Read( const Json& _InData ) override
    {
        const std::vector< std::string >    cflags = _InData[ "CFLAGS" ];

        m_Session.m_CFlagsBuffer.Allocate( cflags );
    }

private:
    // input
    ClangSession&               m_Session;
};

class ClangSession::Command::ReadSourceCode : public IMultiSerializable
{
public:
    ReadSourceCode( ClangSession& _Session ) :
        m_Session( _Session )
    {
    }

    virtual void Read( const Lisp::Text::Object& _InData ) override
    {
    }

    virtual void Read( const Lisp::Node::Object& _InData ) override
    {
        Lisp::Node::PropertyListIterator    iterator = _InData.GetRootPropertyListIterator();

        for ( ; !iterator.IsEnd(); iterator.Next() )
        {
            if ( iterator.IsSameKey( ":SourceCode" ) )
            {
                const std::string&   source_code = iterator.RefValue< std::string >();

                // m_Session.m_CSourceCodeBuffer.Allocate( source_code.size() + 1, true );
                m_Session.m_CSourceCodeBuffer.Allocate( source_code.size() + 1 );
                // source_code.copy( m_Session.m_CSourceCodeBuffer.GetAddress< char* >(), source_code.size() );
                std::strcpy( m_Session.m_CSourceCodeBuffer.GetAddress< char* >(), source_code.c_str() );
                break;
            }
        }

    }

    virtual void Read( const Json& _InData ) override
    {
        const std::string   source_code = _InData[ "SourceCode" ];

        // m_Session.m_CSourceCodeBuffer.Allocate( source_code.size() + 1, true );
        m_Session.m_CSourceCodeBuffer.Allocate( source_code.size() + 1 );
        // source_code.copy( m_Session.m_CSourceCodeBuffer.GetAddress< char* >(), source_code.size() );
        std::strcpy( m_Session.m_CSourceCodeBuffer.GetAddress< char* >(), source_code.c_str() );
    }

private:
    // input
    ClangSession&               m_Session;
};

class ClangSession::Command::ReadLineColumn : public IMultiSerializable
{
public:
    ReadLineColumn( ClangSession& _Session ) : 
        m_Session( _Session )
    {
    }

    virtual void Read( const Lisp::Text::Object& _InData ) override
    {
    }

    virtual void Read( const Lisp::Node::Object& _InData ) override
    {
        Lisp::Node::PropertyListIterator    iterator = _InData.GetRootPropertyListIterator();

        for ( ; !iterator.IsEnd(); iterator.Next() )
        {
            if ( iterator.IsSameKey( ":Line" ) )
            {
                m_Session.m_Line = iterator.GetValue< int32_t >();
            }
            else if ( iterator.IsSameKey( ":Column" ) )
            {
                m_Session.m_Column = iterator.GetValue< int32_t >();
            }
        }
    }

    virtual void Read( const Json& _InData ) override
    {
        m_Session.m_Line   = _InData[ "Line" ];
        m_Session.m_Column = _InData[ "Column" ];
    }

private:
    // input
    ClangSession&               m_Session;
};




class Completion_ChunkIterator
{
public:
    Completion_ChunkIterator( CXCompletionString _CompletionString ) :
        m_CompletionString( _CompletionString )
    {
        m_MaxIndex = clang_getNumCompletionChunks( m_CompletionString );
    }

    // ~Completion_ChunkIterator( void );


    uint32_t GetMaxIndex( void ) const
    {
        return m_MaxIndex;
    }

    bool HasNext( void ) const
    {
        return ( m_Index < m_MaxIndex );
    }

    void Next( void )
    {
        if ( HasNext() )
        {
            m_Index++;
        }
    }

    void Rewind( void )
    {
        m_Index = 0;
    }

    CXCompletionChunkKind GetChunkKind( void ) const
    {
        return clang_getCompletionChunkKind( m_CompletionString, m_Index );
    }

    Completion_ChunkIterator GetOptionalChunkIterator( void ) const
    {
        return Completion_ChunkIterator( clang_getCompletionChunkCompletionString( m_CompletionString, m_Index ) );
    }
    
    std::string GetString( void ) const
    {
        CXString    cx_chunk_text = clang_getCompletionChunkText( m_CompletionString, m_Index );
        std::string chunk_text    = clang_getCString( cx_chunk_text );

        clang_disposeString( cx_chunk_text );

        return std::move( chunk_text );
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


    uint32_t GetMaxIndex( void ) const
    {
        return m_MaxIndex;
    }

    bool HasNext( void ) const
    {
        return ( m_Index < m_MaxIndex );
    }

    void Next( void )
    {
        if ( HasNext() )
        {
            m_Index++;
        }
    }

    void Rewind( void )
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


    CXAvailabilityKind GetAvailabilityKind( void ) const
    {
        return clang_getCompletionAvailability( m_CompletionString );
    }

    Completion_ChunkIterator GetChunkIterator( void ) const
    {
        return Completion_ChunkIterator( m_CompletionString );
    }

    Completion_AnnotationIterator GetAnnotationIterator( void ) const
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




class ClangSession::Command::Completion : public ICommand
{
public:
    struct Candidate
    {
        enum
        {
            kInitialSize = 128,
        };

        Candidate( void ) = default;
        Candidate( CXCompletionString _CompletionString );

        bool Parse( Completion_Holder& _Holder );
        bool ParseChunk( Completion_ChunkIterator& _Iterator );

        bool                        m_IsValid              = false;
        std::string                 m_Name;
        // std::ostringstream          m_Prototype;
        std::string                 m_Prototype;
        // std::string                 m_ResultType;
        // std::ostringstream          m_Snippet;
        // std::ostringstream          m_DisplayText;
        std::string                 m_BriefComment;
        std::vector< std::string >  m_Annotations;
        uint32_t                    m_NumberOfPlaceHolders = 0;
    };


    Completion( ClangSession& _Session ) :
        m_Session( _Session )
    {
    }

    virtual bool Evaluate( void ) override;

    virtual void Read( const Lisp::Text::Object& _InData ) override;
    virtual void Write( Lisp::Text::Object& _OutData ) const override;

    virtual void Read( const Lisp::Node::Object& _InData ) override;

    virtual void Read( const Json& _InData ) override;
    virtual void Write( Json& _OutData ) const override;

    // void GenerateCandidate( CXCompletionString CompletionString );

private:
    // input
    ClangSession&               m_Session;
    // output
    std::vector< Candidate >    m_Candidates;
};


class ClangSession::Command::Diagnostics : public ICommand
{
public:
    // struct Diagnostic
    // {
    //     Diagnostic( void ) = default;
    //     std::string      m_message;
    // };

    Diagnostics( ClangSession& _Session ) :
        m_Session( _Session )
    {
    }

    virtual bool Evaluate( void ) override;

    virtual void Read( const Lisp::Text::Object& _InData ) override;
    virtual void Write( Lisp::Text::Object& _OutData ) const override;

    virtual void Read( const Lisp::Node::Object& _InData ) override;

    virtual void Read( const Json& _InData ) override;
    virtual void Write( Json& _OutData ) const override;

private:
    // input
    ClangSession&               m_Session;
    // output
    // std::vector< Diagnostic >   m_Diagnostics;
    std::vector< std::string >  m_Diagnostics;
};


class ClangSession::Command::Jump : public ICommand
{
public:
    struct Location
    {
        Location( void ) = default;

        std::string m_NormalizePath;
        uint32_t    m_Line   = 0;
        uint32_t    m_Column = 0;
    };


    Jump( ClangSession& _Session ) :
        m_Session( _Session )
    {
    }
        
    virtual bool Evaluate( void ) override;

    virtual void Read( const Lisp::Text::Object& _InData ) override;
    virtual void Write( Lisp::Text::Object& _OutData ) const override;

    virtual void Read( const Lisp::Node::Object& _InData ) override;

    virtual void Read( const Json& _InData ) override;
    virtual void Write( Json& _OutData ) const override;

private:
    CXCursor GetCursor( void ) const;
    bool EvaluateCursorLocation( const CXCursor& _Cursor );

public:
    bool EvaluateInclusionFileLocation( void );
    bool EvaluateDefinitionLocation( void );
    bool EvaluateDeclarationLocation( void );
    bool EvaluateSmartJumpLocation( void );

private:
    // input
    ClangSession&       m_Session;
    // output
    Location            m_Location;
};





ClangSession::Command::Completion::Candidate::Candidate( CXCompletionString _CompletionString )
{
    // m_Name.reserve( kInitialSize );
    // m_Prototype.reserve( kInitialSize );
    // m_BriefComment.reserve( kInitialSize );

    Completion_Holder   holder( _CompletionString );

    Parse( holder );
}


bool ClangSession::Command::Completion::Candidate::Parse( Completion_Holder& _Holder )
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

bool ClangSession::Command::Completion::Candidate::ParseChunk( Completion_ChunkIterator& _Iterator )
{
    for ( ; _Iterator.HasNext(); _Iterator.Next() )
    {
        switch ( _Iterator.GetChunkKind() )
        {
            case CXCompletionChunk_TypedText:
                // m_Prototype << _Iterator.GetString();
                m_Prototype.append( _Iterator.GetString() );
                m_Name = _Iterator.GetString();
                break;
            case CXCompletionChunk_ResultType:
                // m_Prototype << "[#" << _Iterator.GetString() << "#]";
                m_Prototype.append( "[#" );
                m_Prototype.append( _Iterator.GetString() );
                m_Prototype.append( "#]" );
                break;
            case CXCompletionChunk_Placeholder:
                m_NumberOfPlaceHolders++;
                // m_Prototype << "<#" << _Iterator.GetString() << "#>";
                m_Prototype.append( "<#" );
                m_Prototype.append( _Iterator.GetString() );
                m_Prototype.append( "#>" );
                break;
            case CXCompletionChunk_Optional:
                m_NumberOfPlaceHolders++;
                // m_Prototype << "{#";
                m_Prototype.append( "{#" );
                {
                    Completion_ChunkIterator    optional_iterator = _Iterator.GetOptionalChunkIterator();

                    ParseChunk( optional_iterator );
                }
                // m_Prototype << "#}";
                m_Prototype.append( "#}" );
                break;
            default:
                // m_Prototype << _Iterator.GetString();
                m_Prototype.append( _Iterator.GetString() );
                break;
        }
    }

    return true;
}




bool ClangSession::Command::Completion::Evaluate( void )
{
    CXUnsavedFile                                   unsaved_file = m_Session.GetCXUnsavedFile();
    // necessary call?
    // clang_reparseTranslationUnit( m_Session.m_CxTU, 1, &unsaved_file, m_Session.m_TranslationUnitFlags );
    ScopedClangResource< CXCodeCompleteResults* >   complete_results( clang_codeCompleteAt( m_Session.m_CxTU, m_Session.m_SessionName.c_str(), m_Session.m_Line, m_Session.m_Column, &unsaved_file, 1, m_Session.m_CompleteAtFlags ) );


    if ( !complete_results() )
    {
        // logic failed?
        m_Error << " /[ClangSession::Completion] CXCodeCompleteResults is null pointer!!";

        return false;
    }

    // clang_codeCompleteAt returns the entity address even if the completion candidate is 0 ( CXCodeCompleteResults::NumResults == 0 )

    // limit check
    {
        const uint32_t  results_limit = m_Session.m_ClangContext.GetCompleteResultsLimit();
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




void ClangSession::Command::Completion::Read( const Lisp::Text::Object& _InData )
{
    PROFILER_SCOPED_SAMPLE_FUNCTION();
    ClangSession::Command::ReadLineColumn( m_Session ).Read( _InData );
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}

void ClangSession::Command::Completion::Write( Lisp::Text::Object& _OutData ) const
{
    PROFILER_SCOPED_SAMPLE_FUNCTION();
    Lisp::Text::NewList plist( _OutData );

    plist.AddProperty( ":RequestId", m_Session.m_CommandContext.GetRequestId() );

    plist.AddSymbol( ":Results" );

    {
        Lisp::Text::NewVector   results_vector( plist );

        for ( const auto& candidate : m_Candidates )
        {
            if ( !candidate.m_IsValid )
            {
                continue;
            }

            {
                Lisp::Text::NewList candidate_plist( results_vector );

                candidate_plist.AddProperty( ":Name", candidate.m_Name );
                // candidate_plist.AddProperty( ":Prototype", candidate.m_Prototype.str() );
                candidate_plist.AddQuotedProperty( ":Prototype", candidate.m_Prototype );
                if ( !candidate.m_BriefComment.empty() )
                {
                    candidate_plist.AddQuotedProperty( ":BriefComment", candidate.m_BriefComment );
                }
            }
        }
    }

    if ( !m_Error.str().empty() )
    {
        plist.AddProperty( ":Error", m_Error.str() );
    }
}


void ClangSession::Command::Completion::Read( const Lisp::Node::Object& _InData )
{
    PROFILER_SCOPED_SAMPLE_FUNCTION();
    ClangSession::Command::ReadLineColumn( m_Session ).Read( _InData );
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}


void ClangSession::Command::Completion::Read( const Json& _InData )
{
    PROFILER_SCOPED_SAMPLE_FUNCTION();
    ClangSession::Command::ReadLineColumn( m_Session ).Read( _InData );
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}

void ClangSession::Command::Completion::Write( Json& _OutData ) const
{
    PROFILER_SCOPED_SAMPLE_FUNCTION();
    _OutData[ "RequestId" ] = m_Session.m_CommandContext.GetRequestId();

    for ( const auto& candidate : m_Candidates )
    {
        if ( !candidate.m_IsValid )
        {
            continue;
        }

        if ( candidate.m_BriefComment.empty() )
        {
            _OutData[ "Results" ].push_back( 
                                            {
                                                { "Name", candidate.m_Name }, 
                                                // { "Prototype", candidate.m_Prototype.str() }, 
                                                { "Prototype", candidate.m_Prototype }, 
                                            }
                                             );
        }
        else
        {
            _OutData[ "Results" ].push_back( 
                                            {
                                                { "Name", candidate.m_Name }, 
                                                // { "Prototype", candidate.m_Prototype.str() }, 
                                                { "Prototype", candidate.m_Prototype }, 
                                                { "BriefComment", candidate.m_BriefComment }, 
                                            }
                                             );
        }
    }

    if ( !m_Error.str().empty() )
    {
        _OutData[ "Error" ] = m_Error.str();
    }
}




#if 0
void ClangSession::Command::Completion::GenerateCandidate( CXCompletionString CompletionString )
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




bool ClangSession::Command::Diagnostics::Evaluate( void )
{
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



void ClangSession::Command::Diagnostics::Read( const Lisp::Text::Object& _InData )
{
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}

void ClangSession::Command::Diagnostics::Write( Lisp::Text::Object& _OutData ) const
{
    std::ostringstream  diagnostics;

    for ( const auto& message : m_Diagnostics )
    {
        diagnostics << message << std::endl;
    }

    Lisp::Text::NewList plist( _OutData );

    plist.AddProperty( ":RequestId", m_Session.m_CommandContext.GetRequestId() );
    plist.AddSymbol( ":Results" );

    {
        Lisp::Text::NewList results_plist( plist );

        results_plist.AddQuotedProperty( ":Diagnostics", diagnostics.str() );
    }

    if ( !m_Error.str().empty() )
    {
        plist.AddProperty( ":Error", m_Error.str() );
    }
}


void ClangSession::Command::Diagnostics::Read( const Lisp::Node::Object& _InData )
{
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}


void ClangSession::Command::Diagnostics::Read( const Json& _InData )
{
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}

void ClangSession::Command::Diagnostics::Write( Json& _OutData ) const
{
    std::ostringstream  diagnostics;

    for ( const auto& message : m_Diagnostics )
    {
        diagnostics << message << std::endl;
    }

    _OutData[ "RequestId" ] = m_Session.m_CommandContext.GetRequestId();
    _OutData[ "Results" ]   = { { "Diagnostics", diagnostics.str() } };

    if ( !m_Error.str().empty() )
    {
        _OutData[ "Error" ] = m_Error.str();
    }
}



CXCursor ClangSession::Command::Jump::GetCursor( void ) const
{
    const CXFile            file     = clang_getFile( m_Session.m_CxTU, m_Session.m_SessionName.c_str() );
    const CXSourceLocation  location = clang_getLocation( m_Session.m_CxTU, file, m_Session.m_Line, m_Session.m_Column );
    const CXCursor          cursor   = clang_getCursor( m_Session.m_CxTU, location );

    return cursor;
}


bool ClangSession::Command::Jump::EvaluateCursorLocation( const CXCursor& _Cursor )
{
    if ( clang_isInvalid( _Cursor.kind ) )
    {
        m_Error << " /[ClangSession::Command::Jump::EvaluateCursorLocation] cursor is invalid.";

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
        m_Error << " /[ClangSession::Command::Jump::EvaluateCursorLocation] CXFile is null pointer.";

        return false;
    }

    const std::string        normalize_path = ::GetNormalizePath( dest_file );
    
    m_Location.m_NormalizePath = normalize_path;
    m_Location.m_Line          = dest_line;
    m_Location.m_Column        = dest_column;

    return true;
}


bool ClangSession::Command::Jump::EvaluateInclusionFileLocation( void )
{
    CXUnsavedFile       unsaved_file = m_Session.GetCXUnsavedFile();

    clang_reparseTranslationUnit( m_Session.m_CxTU, 1, &unsaved_file, m_Session.m_TranslationUnitFlags );

    const CXCursor      source_cursor = GetCursor();

    if ( !clang_isInvalid( source_cursor.kind ) )
    {
        if ( source_cursor.kind == CXCursor_InclusionDirective )
        {
            const CXFile  file = clang_getIncludedFile( source_cursor );

            if ( file )
            {
                // file top location
                const uint32_t      file_line      = 1;
                const uint32_t      file_column    = 1;
                const std::string   normalize_path = ::GetNormalizePath( file );

                m_Location.m_NormalizePath = normalize_path;
                m_Location.m_Line          = file_line;
                m_Location.m_Column        = file_column;

                return true;
            }
        }
    }

    m_Error << " /[ClangSession::Command::Jump::EvaluateInclusionFileLocation] cursor kind is not CXCursor_InclusionDirective.";

    return false;
}

bool ClangSession::Command::Jump::EvaluateDefinitionLocation( void )
{
    CXUnsavedFile       unsaved_file = m_Session.GetCXUnsavedFile();

    clang_reparseTranslationUnit( m_Session.m_CxTU, 1, &unsaved_file, m_Session.m_TranslationUnitFlags );

    const CXCursor      source_cursor = GetCursor();

    if ( !clang_isInvalid( source_cursor.kind ) )
    {
        return EvaluateCursorLocation( clang_getCursorDefinition( source_cursor ) );
    }

    m_Error << " /[ClangSession::Command::Jump::EvaluateDefinitionLocation] cursor is invalid.";

    return false;
}

bool ClangSession::Command::Jump::EvaluateDeclarationLocation( void )
{
    CXUnsavedFile       unsaved_file = m_Session.GetCXUnsavedFile();

    clang_reparseTranslationUnit( m_Session.m_CxTU, 1, &unsaved_file, m_Session.m_TranslationUnitFlags );

    const CXCursor      source_cursor = GetCursor();

    if ( !clang_isInvalid( source_cursor.kind ) )
    {
        return EvaluateCursorLocation( clang_getCursorReferenced( source_cursor ) );
    }

    m_Error << " /[ClangSession::Command::Jump::EvaluateDeclarationLocation] cursor is invalid.";

    return false;
}

bool ClangSession::Command::Jump::EvaluateSmartJumpLocation( void )
{
    CXUnsavedFile       unsaved_file = m_Session.GetCXUnsavedFile();

    clang_reparseTranslationUnit( m_Session.m_CxTU, 1, &unsaved_file, m_Session.m_TranslationUnitFlags );

    const CXCursor      source_cursor = GetCursor();

    if ( !clang_isInvalid( source_cursor.kind ) )
    {
        if ( source_cursor.kind == CXCursor_InclusionDirective )
        {
            const CXFile  file = clang_getIncludedFile( source_cursor );

            if ( file )
            {
                // file top location
                const uint32_t      file_line      = 1;
                const uint32_t      file_column    = 1;
                const std::string   normalize_path = ::GetNormalizePath( file );

                m_Location.m_NormalizePath = normalize_path;
                m_Location.m_Line          = file_line;
                m_Location.m_Column        = file_column;

                return true;
            }
        }
        else
        {
            const bool    is_success = ( EvaluateCursorLocation( clang_getCursorDefinition( source_cursor ) ) 
                                         || EvaluateCursorLocation( clang_getCursorReferenced( source_cursor ) ) );
            if ( is_success )
            {
                m_Error.str( "" );
                m_Error.clear();
            }

            return is_success;
        }
    }

    m_Error << " /[ClangSession::Command::Jump::EvaluateSmartJumpLocation] cursor is invalid.";

    return false;
}


bool ClangSession::Command::Jump::Evaluate( void )
{
    const std::string&  command_name = m_Session.m_CommandContext.GetCommandName();

    if ( command_name == "INCLUSION" )
    {
        return EvaluateInclusionFileLocation();
    }
    else if ( command_name == "DEFINITION" )
    {
        return EvaluateDefinitionLocation();
    }
    else if ( command_name == "DECLARATION" )
    {
        return EvaluateDeclarationLocation();
    }
    else if ( command_name == "SMARTJUMP" )
    {
        return EvaluateSmartJumpLocation();
    }

    return false;
}



void ClangSession::Command::Jump::Read( const Lisp::Text::Object& _InData )
{
    ClangSession::Command::ReadLineColumn( m_Session ).Read( _InData );
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}

void ClangSession::Command::Jump::Write( Lisp::Text::Object& _OutData ) const
{
    Lisp::Text::NewList plist( _OutData );

    plist.AddProperty( ":RequestId", m_Session.m_CommandContext.GetRequestId() );
    plist.AddSymbol( ":Results" );

    {
        Lisp::Text::NewList results_plist( plist );

        results_plist.AddProperty( ":Path", m_Location.m_NormalizePath );
        results_plist.AddProperty( ":Line", m_Location.m_Line );
        results_plist.AddProperty( ":Column", m_Location.m_Column );
    }

    if ( !m_Error.str().empty() )
    {
        plist.AddProperty( ":Error", m_Error.str() );
    }
}


void ClangSession::Command::Jump::Read( const Lisp::Node::Object& _InData )
{
    ClangSession::Command::ReadLineColumn( m_Session ).Read( _InData );
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}


void ClangSession::Command::Jump::Read( const Json& _InData )
{
    ClangSession::Command::ReadLineColumn( m_Session ).Read( _InData );
    ClangSession::Command::ReadSourceCode( m_Session ).Read( _InData );
}

void ClangSession::Command::Jump::Write( Json& _OutData ) const
{
    _OutData[ "RequestId" ] = m_Session.m_CommandContext.GetRequestId();
    _OutData[ "Results" ]   = 
    {
        { "Path", m_Location.m_NormalizePath }, 
        { "Line", m_Location.m_Line }, 
        { "Column", m_Location.m_Column }, 
    };

    if ( !m_Error.str().empty() )
    {
        _OutData[ "Error" ] = m_Error.str();
    }
}



/*================================================================================================*/
/*  Global Class Method Definitions Section                                                       */
/*================================================================================================*/


ClangSession::ClangSession( const std::string& _SessionName, const ClangContext& _ClangContext, CommandContext& _CommandContext ) : 
    m_SessionName( _SessionName )
    , m_ClangContext( _ClangContext )
    , m_CommandContext( _CommandContext )
    , m_CxTU( nullptr )
    , m_TranslationUnitFlags( _ClangContext.GetTranslationUnitFlags() )
    , m_CompleteAtFlags( _ClangContext.GetCompleteAtFlags() )
    , m_Line( 0 )
    , m_Column( 0 )
{
}



ClangSession::~ClangSession( void )
{
    Deallocate();
}


void ClangSession::CreateTranslationUnit( void )
{
    if ( m_CxTU )
    {
        // clang parser already exist
        return;
    }

    CXUnsavedFile       unsaved_file   = GetCXUnsavedFile();
    const CXErrorCode   parse_result   = clang_parseTranslationUnit2( m_ClangContext.GetCXIndex(), m_SessionName.c_str(), 
                                                                      m_CFlagsBuffer.GetCFlags(), m_CFlagsBuffer.GetNumberOfCFlags(), 
                                                                      &unsaved_file, 1, m_TranslationUnitFlags, &m_CxTU );
    const int           reparse_result = clang_reparseTranslationUnit( m_CxTU, 1, &unsaved_file, m_TranslationUnitFlags );

    if ( ( parse_result != CXError_Success ) || ( reparse_result != 0 ) )
    {
        // error
    }

    // success
}

void ClangSession::DeleteTranslationUnit( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    clang_disposeTranslationUnit( m_CxTU );
    m_CxTU = nullptr;
}



void ClangSession::Allocate( void )
{
    Deserializer< Command::ReadCFlags >( *this, m_CommandContext );
    Deserializer< Command::ReadSourceCode >( *this, m_CommandContext );
    CreateTranslationUnit();
}

void ClangSession::Deallocate( void )
{
    DeleteTranslationUnit();
}


void ClangSession::commandSuspend( void )
{
    DeleteTranslationUnit();
}

void ClangSession::commandResume( void )
{
    CreateTranslationUnit();
}


void ClangSession::commandSetCFlags( void )
{
    DeleteTranslationUnit();
    Deserializer< Command::ReadCFlags >( *this, m_CommandContext );
    Deserializer< Command::ReadSourceCode >( *this, m_CommandContext );
    CreateTranslationUnit();
}


void ClangSession::commandSetSourceCode( void )
{
    Deserializer< Command::ReadSourceCode >( *this, m_CommandContext );
}

void ClangSession::commandReparse( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    CXUnsavedFile               unsaved_file = GetCXUnsavedFile();

    clang_reparseTranslationUnit( m_CxTU, 1, &unsaved_file, m_TranslationUnitFlags );
}


void ClangSession::commandCompletion( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    CommandEvaluator< Command::Completion > evaluator( *this, m_CommandContext );
}


void ClangSession::commandDiagnostics( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    CommandEvaluator< Command::Diagnostics >    evaluator( *this, m_CommandContext );
}


void ClangSession::commandInclusion( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    CommandEvaluator< Command::Jump >   evaluator( *this, m_CommandContext );
}


void ClangSession::commandDeclaration( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    CommandEvaluator< Command::Jump >   evaluator( *this, m_CommandContext );
}


void ClangSession::commandDefinition( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    CommandEvaluator< Command::Jump >   evaluator( *this, m_CommandContext );
}


void ClangSession::commandSmartJump( void )
{
    if ( !m_CxTU )
    {
        // clang parser not exist
        return;
    }

    CommandEvaluator< Command::Jump >   evaluator( *this, m_CommandContext );
}





/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
