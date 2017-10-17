/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/17.16:07:06 */


#pragma once

#ifndef __LISP_PARSER_HPP__
#define __LISP_PARSER_HPP__



/*================================================================================================*/
/*  Comment                                                                                       */
/*================================================================================================*/


/*================================================================================================*/
/*  Include Files                                                                                 */
/*================================================================================================*/

#include <cstdint>
#include <cctype>

#include <string>
#include <sstream>
#include <iomanip>
#include <functional>
#include <stack>


/*================================================================================================*/
/*  Class                                                                                         */
/*================================================================================================*/

namespace   Lisp
{


class TextObject : protected std::ostringstream
{
public:
    TextObject( void ) = default;
    virtual ~TextObject( void ) = default;


    void Add( const char* _String )
    {
        *this << _String;
    }
    void AddSymbol( const char* _Symbol )
    {
        // symbol + delimiter(ws)
        *this << _Symbol << " ";
    }
#if 0
    void AddElement( const char* _Element )
    {
        // element + delimiter(ws)
        *this << _Element << " ";
    }
    void AddStringElement( const char* _Element )
    {
        // double-quote + element + double-quote + delimiter(ws)
        *this << "\"" << _Element << "\" ";
        // *this << std::quoted( _Element ) << " ";
    }
#endif
    void AddDelimiter( void )
    {
        *this << " ";
    }

    template< typename ElementType >
    void AddElement( const ElementType& _Element )
    {
        // element + delimiter(ws)
        *this << _Element << " ";
    }
    // template< typename ElementType >
    // void AddQuoteElement( const ElementType& _Element )
    // {
    //     *this << "\"" << _Element << "\" ";
    // }

    void AddElement( const std::string& _Element )
    {
        AddElement( _Element.c_str() );
    }
    void AddElement( const char* _Element )
    {
        // double-quote + element + double-quote + delimiter(ws)
        *this << "\"" << _Element << "\" ";
        // *this << std::quoted( _Element ) << " ";
    }
    
    const std::string GetString( void ) const
    {
        return this->str();
    }

    void Set( const char* _Address )
    {
        this->str( _Address );
    }

    void Clear( void )
    {
        this->str( "" );
        this->clear();
    }
    
};



class ISequence
{
protected:
    ISequence( TextObject& _Object ) :
        m_Object( _Object )
    {
    }
    virtual ~ISequence( void ) = default;

public:
    operator TextObject&() const
    {
        return m_Object;
    }

    TextObject& GetTextObject( void )
    {
        return m_Object;
    }
    const TextObject& GetTextObject( void ) const
    {
        return m_Object;
    }

    void AddSymbol( const std::string& _Symbol )
    {
        AddSymbol( _Symbol.c_str() );
    }
    void AddSymbol( const char* _Symbol )
    {
        m_Object.AddSymbol( _Symbol );
        m_Size += 1;
    }

    template< typename ValueType >
    void AddProperty( const std::string& _Symbol, const ValueType& _Value )
    {
        AddProperty( _Symbol.c_str(), _Value );
    }
    template< typename ValueType >
    void AddProperty( const char* _Symbol, const ValueType& _Value )
    {
        // m_Object.AddElement( _Symbol );
        m_Object.AddSymbol( _Symbol );
        m_Object.AddElement( _Value );
        m_Size += 2;
    }
    uint32_t GetSize( void ) const
    {
        return m_Size;
    }

protected:
    TextObject&     m_Object;
    uint32_t        m_Size = 0;
};



class AddList : public ISequence
{
public:
    AddList( TextObject& _Object ) :
        ISequence( _Object )
    {
        m_Object.Add( "(" );
    }

    AddList( AddList& _Object ) :
        AddList( _Object.GetTextObject() )
    {
    }

    AddList& operator =( const AddList& ) = delete;

    virtual ~AddList( void ) override
    {
        m_Object.Add( ")" );
    }
};


class AddVector : public ISequence
{
public:
    AddVector( TextObject& _Object ) :
        ISequence( _Object )
    {
        m_Object.Add( "[" );
    }
        
    AddVector( AddVector& _Object ) :
        AddVector( _Object.GetTextObject() )
    {
    }

    AddVector& operator =( const AddVector& ) = delete;

    virtual ~AddVector( void ) override
    {
        m_Object.Add( "]" );
    }
};




// SAS = Simple API for S-Expression
// SAX like
namespace   SAS
{


// character iterator
class Iterator
{
public:
    Iterator( const char* _Begin ) :
        m_Begin( _Begin )
        , m_Current( _Begin )
    {
    }
    virtual ~Iterator( void ) = default;


    Iterator& operator ++()
    {
        ++m_Current;

        return *this;
    }

    Iterator operator ++( int )
    {
        Iterator    prev = *this;

        ++m_Current;

        return prev;
    }

    operator const char*( void ) const
    {
        return m_Current;
    }

    const char operator *( void ) const
    {
        return *m_Current;
    }

    const char* Get( void ) const
    {
        return m_Current;
    }
    void Set( const char* _Current )
    {
        m_Current = _Current;
    }

    const char* GetBegin( void ) const
    {
        return m_Begin;
    }

    void Next( void )
    {
        ++m_Current;
    }
    void Prev( void )
    {
        --m_Current;
    }

    size_t GetPosition( void ) const
    {
        return ( m_Current - m_Begin );
    }

    void Rewind( void )
    {
        m_Current = m_Begin;
    }

    bool IsClosedRange( char _BeginAscii, char _EndAscii ) const
    {
        return ( ( _BeginAscii <= *m_Current ) && ( *m_Current <= _EndAscii ) );
    }

    bool Is( const char _Character ) const
    {
        return ( *m_Current == _Character );
    }

    bool Is( const char _Character, const size_t _Index ) const
    {
        return ( m_Current[ _Index ] == _Character );
    }

    bool IsNext( const char _Character ) const
    {
        return ( *(m_Current + 1) == _Character );
    }
    bool IsPrev( const char _Character ) const
    {
        return ( *(m_Current - 1) == _Character );
    }

    bool IsEOS( void ) const
    {
        return Is( '\0' );
    }

    bool IsSpace( void ) const
    {
        // return Is( ' ' );
        return ( std::isspace( static_cast< uint8_t >( *m_Current ) ) != 0 );
    }

    bool IsBackslash( void ) const
    {
        return Is( '\\' );
    }

    bool IsEscape( void ) const
    {
        return IsClosedRange( 0x07, 0x0D );
    }

    bool IsComment( void ) const
    {
        return Is( ';' );
    }

    bool IsEnterList( void ) const
    {
        return Is( '(' );
    }
    bool IsLeaveList( void ) const
    {
        return Is( ')' );
    }

    bool IsEnterVector( void ) const
    {
        return Is( '[' );
    }
    bool IsLeaveVector( void ) const
    {
        return Is( ']' );
    }

    bool IsEnterSequence( void ) const
    {
        return ( IsEnterList() || IsEnterVector() );
    }
    bool IsLeaveSequence( void ) const
    {
        return ( IsLeaveList() || IsLeaveVector() );
    }

    bool IsSequence( void ) const
    {
        return ( IsEnterSequence() || IsLeaveSequence() );
    }


    bool IsEnterString( void ) const
    {
        return Is( '\"' );
    }
    bool IsLeaveString( void ) const
    {
        return ( !Is( '\\', -1 ) && Is( '\"' ) );
    }
    bool IsNextLeaveString( void ) const
    {
        return ( !Is( '\\' ) && Is( '\"', 1 ) );
    }

    bool IsNumberCharacter( void ) const
    {
        // return IsClosedRange( '0', '9' );
        return ( std::isdigit( static_cast< uint8_t >( *m_Current ) ) != 0 );
    }

    bool IsEnterNumber( void ) const
    {
        return ( IsNumberCharacter() || Is( '+' ) || Is( '-' ) || Is( '.' ) );
    }

    bool IsNumber( void ) const
    {
        return ( IsEnterNumber() || Is( 'e' ) );
    }

    std::string GetTrailedString( void ) const
    {
        // return std::move( std::string( m_Begin, GetPosition() ) );
        return std::string( m_Begin, GetPosition() );
    }

    // operations
    size_t SkipSpace( void )
    {
        const char* skip_begin = m_Current;

        while ( IsSpace() )
        {
            ++m_Current;
        }

        return ( m_Current - skip_begin );
    }

private:
    const char*     m_Begin   = nullptr;
    const char*     m_Current = nullptr;
};



struct SExpression
{
    enum Type
    {
        kInvalid = -1, 
        kSequence, 
        kList, 
        kVector, 
        kSymbol, 
        kString, 
        kInterger, 
        kFloat, 
    };

    SExpression( void ) = default;
    SExpression( Type _Type, const std::string& _Value )
    {
        Set( _Type, _Value );
    }

    void Set( Type _Type, const std::string& _Value )
    {
        m_Type  = _Type;
        // m_Value = _Value;
        m_Value = std::move( _Value );
    }

    void Clear( void )
    {
        m_Type = Type::kInvalid;
        m_Value.clear();
    }

    bool IsType( Type _Type ) const
    {
        return ( m_Type == _Type );
    }
    Type GetType( void ) const
    {
        return m_Type;
    }

    const std::string& GetValueString( void ) const
    {
        return m_Value;
    }

    template< typename ValueType >
    ValueType GetValue( void ) const;

    template<>
    std::string GetValue< std::string >( void ) const
    {
        return m_Value;
    }

    template<>
    int32_t GetValue< int32_t >( void ) const
    {
        const int32_t   value = std::stoi( m_Value );

        return value;
    }

    template<>
    uint32_t GetValue< uint32_t >( void ) const
    {
        const uint32_t   value = std::stoi( m_Value );

        return value;
    }

    template<>
    float GetValue< float >( void ) const
    {
        const float value = std::stof( m_Value );

        return value;
    }

    template<>
    bool GetValue< bool >( void ) const
    {
        return ( m_Value != "nil" );
    }


    Type            m_Type = Type::kInvalid;
    std::string     m_Value;
};



class DetectHandler
{
public:
    struct SequenceContext
    {
        enum ParseMode
        {
            kNormal, 
            kPropertyList, 
        };

        bool IsPropertyListMode( void ) const
        {
            return ( m_Type == ParseMode::kPropertyList );
        }

        SExpression::Type   m_Type         = SExpression::Type::kSequence;
        ParseMode           m_Mode         = ParseMode::kNormal;
        size_t              m_Length       = 0;
        const std::string*  m_ParentSymbol = nullptr;
    };


    DetectHandler( void ) = default;
    virtual ~DetectHandler( void ) = default;

    virtual bool OnEnterSequence( SequenceContext& _Context )
    {
        return m_OnEnterSequence ? m_OnEnterSequence( _Context ) : true;
    };
    virtual bool OnLeaveSequence( const SequenceContext& _Context )
    {
        return m_OnLeaveSequence ? m_OnLeaveSequence( _Context ) : true;
    };

    virtual bool OnAtom( const size_t _Index, const SExpression& _SExpression )
    {
        return m_OnAtom ? m_OnAtom( _Index, _SExpression ) : true;
    };

    virtual bool OnProperty( const size_t _Index, const std::string& _Symbol, const SExpression& _SExpression )
    {
        return m_OnProperty ? m_OnProperty( _Index, _Symbol, _SExpression ) : true;
    }

    void SetSequenceDepth( uint32_t _Depth )
    {
        m_SequenceDepth = _Depth;
    }
    uint32_t GetSequenceDepth( void ) const
    {
        return m_SequenceDepth;
    }

public:
    std::function< bool ( SequenceContext& _Context ) >                                                         m_OnEnterSequence;
    std::function< bool ( const SequenceContext& _Context ) >                                                   m_OnLeaveSequence;
    std::function< bool ( const size_t _Index, const SExpression& _SExpression ) >                              m_OnAtom;
    std::function< bool ( const size_t _Index, const std::string& _Symbol, const SExpression& _SExpression ) >  m_OnProperty;

protected:
    uint32_t    m_SequenceDepth  = 0;
};



class Parser
{
public:
    Parser( void ) = default;
    virtual ~Parser( void ) = default;

    void Parse( const TextObject& _Input, DetectHandler& _Handler )
    {
        Parse( _Input.GetString().c_str(), _Handler );
    }

    void Parse( const char* _Input, DetectHandler& _Handler )
    {
        if ( !_Input )
        {
            return;
        }

        m_DetectHandler = &_Handler;
        m_SequenceDepth = 0;

        Iterator    it( _Input );
        SExpression s_expr;

        it.SkipSpace();

        if ( it.IsSequence() )
        {
            m_SequenceLayer.push( "" );
            ParseSequence( it, s_expr );
            m_SequenceLayer.pop();
        }
    }

private:
    void ParseString( Iterator& _Input, SExpression& _SExpression )
    {
        // skip quote
        _Input.Next();

        Iterator    string_it( _Input.Get() );

        while ( !( string_it.IsEOS() || string_it.IsLeaveString() ) )
        {
            string_it.Next();
        }

        const std::string   value = string_it.GetTrailedString();

        // skip quote
        string_it.Next();

        _Input.Set( string_it.Get() );

        _SExpression.Set( SExpression::Type::kString, value );
    }

    void ParseNumber( Iterator& _Input, SExpression& _SExpression )
    {
        Iterator    number_it( _Input.Get() );
        bool        is_float = false;

        while ( number_it.IsNumber() )
        {
            if ( number_it.Is( '.' ) || number_it.Is( 'e' ) )
            {
                is_float = true;
            }

            number_it.Next();
        }

        const std::string   value = number_it.GetTrailedString();

        _Input.Set( number_it.Get() );

        _SExpression.Set( is_float ? SExpression::Type::kFloat : SExpression::Type::kInterger, value );
    }

    void ParseSymbol( Iterator& _Input, SExpression& _SExpression )
    {
        Iterator    symbol_it( _Input.Get() );

        while ( !( symbol_it.IsSpace() || symbol_it.IsLeaveSequence() ) )
        {
            symbol_it.Next();
        }

        const std::string   value = symbol_it.GetTrailedString();

        _Input.Set( symbol_it.Get() );

        _SExpression.Set( SExpression::Type::kSymbol, value );
    }

    bool ParseSequence( Iterator& _Input, SExpression& _SExpression )
    {
        const bool                      is_list             = _Input.IsEnterList();
        const SExpression::Type         sequence_type       = is_list ? SExpression::Type::kList : SExpression::Type::kVector;
        const char                      sequence_leave_code = is_list ? ')' : ']';
        Iterator                        it( _Input.Get() );
        bool                            is_continue         = true;
        size_t                          index               = 0;
        SExpression                     s_expr;
        SExpression                     prev_s_expr;
        DetectHandler::SequenceContext  context;

        context.m_Type         = sequence_type;
        context.m_ParentSymbol = &( m_SequenceLayer.top() );

        // skip begin bracket
        it.Next();

        // start sequence
        m_DetectHandler->SetSequenceDepth( IncreaseDepth() );
        is_continue &= m_DetectHandler->OnEnterSequence( context );

        while ( !it.Is( sequence_leave_code ) && is_continue )
        {
            it.SkipSpace();

            const bool  is_plist_mode_and_prev_symbol_element = ( context.IsPropertyListMode() && prev_s_expr.IsType( SExpression::Type::kSymbol ) );

            if ( it.IsEnterSequence() )
            {
                // sequence
                m_SequenceLayer.push( is_plist_mode_and_prev_symbol_element ? prev_s_expr.m_Value : "" );
                is_continue &= ParseSequence( it, s_expr );
                m_SequenceLayer.pop();
            }
            else
            {
                // atom element
                // symbol, string, interger, float
                if ( it.IsEnterString() )
                {
                    ParseString( it, s_expr );
                }
                else if ( it.IsEnterNumber() )
                {
                    ParseNumber( it, s_expr );
                }
                else
                {
                    ParseSymbol( it, s_expr );
                }

                is_continue &= m_DetectHandler->OnAtom( index, s_expr );
            }

            // property list element parse
            if ( is_plist_mode_and_prev_symbol_element && ( index % 2 ) )
            {
                is_continue &= m_DetectHandler->OnProperty( index, prev_s_expr.m_Value, s_expr );
            }

            ++index;
            prev_s_expr = std::move( s_expr );
        }

        context.m_Length = index;

        // end sequence
        is_continue &= m_DetectHandler->OnLeaveSequence( context );
        DecreaseDepth();

        // skip end bracket
        it.Next();

        _Input.Set( it.Get() );
        _SExpression.Set( SExpression::Type::kSequence, "" );

        return is_continue;
    }

    uint32_t GetSequenceDepth( void ) const
    {
        return m_SequenceDepth;
    }
    uint32_t IncreaseDepth( void )
    {
        return ++m_SequenceDepth;
    }
    uint32_t DecreaseDepth( void )
    {
        return --m_SequenceDepth;
    }

public:
    DetectHandler*              m_DetectHandler = nullptr;
    uint32_t                    m_SequenceDepth = 0;
    std::stack< std::string >   m_SequenceLayer;
};



}  // namespace SAS





}  // namespace Lisp






#endif
/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
