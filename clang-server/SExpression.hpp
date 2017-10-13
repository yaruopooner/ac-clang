/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/13.22:15:48 */


#pragma once

#ifndef __S_EXPRESSION_HPP__
#define __S_EXPRESSION_HPP__



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

namespace   SExpression
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
// like SAX
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


struct Atom
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


    Atom( void ) = default;
    Atom( Type _Type, const std::string& _Value )
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

    // template< typename ValueType >
    // ValueType GetValue( void ) const;

    template< typename ValueType >
    auto GetValue( void ) const -> ValueType;

    Type            m_Type = Type::kInvalid;
    std::string     m_Value;
};


template<>
inline
// std::string Atom::GetValue< std::string >( void ) const
auto Atom::GetValue< std::string >( void ) const -> std::string
{
    return m_Value;
}

template<>
inline
int32_t Atom::GetValue< int32_t >( void ) const
{
    const int32_t   value = std::stoi( m_Value );

    return value;
}

template<>
inline
// uint32_t Atom::GetValue< uint32_t >( void ) const
auto Atom::GetValue< uint32_t >( void ) const -> uint32_t
{
    const uint32_t   value = std::stoi( m_Value );

    return value;
}

template<>
inline
float Atom::GetValue< float >( void ) const
{
    const float value = std::stof( m_Value );

    return value;
}




class IDetectHandler
{
public:
    struct SequenceContext
    {
        enum ParseMode
        {
            kNormal, 
            kPropertyList, 
        };

        Atom::Type          m_Type         = Atom::Type::kSequence;
        ParseMode           m_Mode         = ParseMode::kNormal;
        size_t              m_Length       = 0;
        const std::string*  m_ParentSymbol = nullptr;
    };


    IDetectHandler( void ) = default;
    virtual ~IDetectHandler( void ) = default;

    virtual void OnEnterSequence( SequenceContext& _Context ) {};
    virtual void OnLeaveSequence( const SequenceContext& _Context ) {};

    virtual void OnAtom( const size_t _Index, const Atom& _Atom ) {};

    virtual void OnProperty( const size_t _Index, const std::string& _Symbol, const Atom& _Atom ) {};

    // void SetSymbol( std::function< void () >& _Receiver )
    // {
    // }

    void SetSequenceDepth( uint32_t _Depth )
    {
        m_SequenceDepth = _Depth;
    }
    uint32_t GetSequenceDepth( void ) const
    {
        return m_SequenceDepth;
    }

protected:
    uint32_t    m_SequenceDepth  = 0;
};



class CommandParseHandler : public IDetectHandler
{
public:
    CommandParseHandler( uint32_t& _RequestId, std::string& _CommandType, std::string& _CommandName, std::string& _SessionName, bool& _IsProfile ) : 
        m_RequestId( _RequestId )
        , m_CommandType( _CommandType )
        , m_CommandName( _CommandName )
        , m_SessionName( _SessionName )
        , m_IsProfile( _IsProfile )
    {
    }
    virtual ~CommandParseHandler( void ) = default;

    virtual void OnEnterSequence( SequenceContext& _Context ) override
    {
        // if ( m_SequenceDepth == 1 )
        // {
        //     _Context.m_Mode = SequenceContext::ParseMode::kPropertyList;
        // }
        // else if ( ( m_SequenceDepth == 2 ) && m_IsCflags )
        // {
        //     _Context.m_Mode = SequenceContext::ParseMode::kNormal;
        // }

        if ( *_Context.m_ParentSymbol == ":CFLAGS" )
        {
            _Context.m_Mode = SequenceContext::ParseMode::kNormal;
        }
        else
        {
            _Context.m_Mode = SequenceContext::ParseMode::kPropertyList;
        }
    }
    virtual void OnLeaveSequence( const SequenceContext& _Context ) override
    {
        // if ( ( m_SequenceDepth == 2 ) && m_IsCflags )
        // {
        //     m_IsCflags = false;
        // }
    }
    
    virtual void OnAtom( const size_t _Index, const Atom& _Atom ) override
    {
        // if ( _Atom.IsType( Atom::Type::kSymbol ) && ( _Atom.GetValueString() == ":CFLAGS" ) )
        // {
        //     m_IsCflags = true;
        // }
    }
    
    virtual void OnProperty( const size_t _Index, const std::string& _Symbol, const Atom& _Atom ) override
    {
        if ( _Symbol == ":RequestId" )
        {
            m_RequestId = _Atom.GetValue< uint32_t >();
        }
        else if ( _Symbol == ":CommandType" )
        {
            m_CommandType = _Atom.GetValue< std::string >();
        }
        else if ( _Symbol == ":CommandName" )
        {
            m_CommandName = _Atom.GetValue< std::string >();
        }
        else if ( _Symbol == ":SessionName" )
        {
            m_SessionName = _Atom.GetValue< std::string >();
        }
        else if ( _Symbol == ":IsProfile" )
        {
            m_IsProfile = ( _Atom.GetValue< int32_t >() != 0 );
        }
    }

    uint32_t&                       m_RequestId;
    std::string&                    m_CommandType;
    std::string&                    m_CommandName;
    std::string&                    m_SessionName;
    bool&                           m_IsProfile;

    bool                            m_IsCflags = false;
};



#if 0
void EnterList()
{
    if ( _Prev.IsType( kSymbol ) )
    {
        if ( _Prev.IsSameSymbol( ":CFLAGS" ) )
        {
            // normal list
        }
        else
        {
            // property list
        }

        // if ( _Prev.IsSameSymbol( ":CFLAGS" ) )
        // {
        //     // normal list
        // }
    }
    
}

void LeaveList()
{
}





void    OnPropertyValue( const std::string& _Symbol, const AtomValue& _Current )
{
    if ( _Symbol == ":RequestId" )
    {
        if ( _Current.IsType( kInterger ) )
        {
            m_RequestId = _Current.m_Integer;
        }
    }
    else if ( _Symbol == ":CommandType" )
    {
        if ( _Current.IsType( kString ) )
        {
            m_CommandType = _Current.m_String;
        }
    }
    else if ( _Symbol == ":CommandName" )
    {
        if ( _Current.IsType( kString ) )
        {
            m_CommandName = _Current.m_String;
        }
    }
    else if ( _Symbol == ":SessionName" )
    {
        if ( _Current.IsType( kString ) )
        {
            m_SessionName = _Current.m_String;
        }
    }
    else if ( _Symbol == ":IsProfile" )
    {
        if ( _Current.IsType( kInterger ) )
        {
            m_IsProfile = _Current.m_Integer;
        }
    }
}

class CFLAGS_Hander
{
    void OnPropertyValue();
}



void EnterList2()
{
    if ( _Prev.IsType( kSymbol ) )
    {
        if ( _Prev.IsSameSymbol( ":CFLAGS" ) )
        {
            // normal list
            PushHandler( cflags_handers );
        }
        else
        {
            // property list
            PushHandler( normal_handers );
        }

        // if ( _Prev.IsSameSymbol( ":CFLAGS" ) )
        // {
        //     // normal list
        // }
    }
    
}

void LeaveList2()
{
    PopHandler();
}


void    OnValue( const AtomValue& _Prev, const AtomValue& _Current )
{
    if ( _Prev.IsType( kSymbol ) )
    {
        if ( _Prev.IsSameSymbol( ":RequestId" )  )
        {
            if ( _Current.IsType( kInterger ) )
            {
                m_RequestId = _Current.m_Integer;
            }
        }
    }
    
    m_RequestId   = _InData[ "RequestId" ];
    m_CommandType = _InData[ "CommandType" ];
    m_CommandName = _InData[ "CommandName" ];
    m_SessionName = ( _InData.find( "SessionName" ) != _InData.end() ) ? _InData[ "SessionName" ] : std::string();
    m_IsProfile   = ( _InData.find( "IsProfile" ) != _InData.end() ) ? _InData[ "IsProfile" ] : false;
}

void    test( const std::string& _Value )
{
    if ( m_Prev.IsType( kSymbol ) && m_Prev.IsSameSymbol( ":RequestId" ) )
    {
        if ( m_Current.IsType( kInterger ) )
        {
            m_RequestId = m_Current.m_Integer;
        }
    }
    
    m_RequestId   = _InData[ "RequestId" ];
    m_CommandType = _InData[ "CommandType" ];
    m_CommandName = _InData[ "CommandName" ];
    m_SessionName = ( _InData.find( "SessionName" ) != _InData.end() ) ? _InData[ "SessionName" ] : std::string();
    m_IsProfile   = ( _InData.find( "IsProfile" ) != _InData.end() ) ? _InData[ "IsProfile" ] : false;
}

#endif



class Parser
{
public:
    Parser( void ) = default;
    virtual ~Parser( void ) = default;


    void Parse( const char* _Input, IDetectHandler* _Handler )
    {
        if ( !( _Input && _Handler ) )
        {
            return;
        }

        m_test.str("");
        m_test.clear();

        m_DetectHandler = _Handler;
        m_SequenceDepth = 0;

        Iterator    it( _Input );
        Atom        atom;

        it.SkipSpace();

        if ( it.IsSequence() )
        {
            m_ParseLayer.push( "" );
            ParseSequence( it, atom );
            m_ParseLayer.pop();
        }
    }

private:
    void ParseString( Iterator& _Input, Atom& _Atom )
    {
        // skip quote
        _Input.Next();

        Iterator    string_it( _Input.Get() );

        while ( !( string_it.IsEOS() || string_it.IsLeaveString() ) )
        {
            string_it.Next();
        }

        const std::string   value = string_it.GetTrailedString();

        m_test << "string value :" << value << std::endl;

        // skip quote
        string_it.Next();

        _Input.Set( string_it.Get() );

        _Atom.Set( Atom::Type::kString, value );
    }

    void ParseNumber( Iterator& _Input, Atom& _Atom )
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

#if 0
        if ( is_float )
        {
            const float     value_f = std::stof( value );

            m_test << "float value :" << value_f << std::endl;

            // m_DetectHandler->OnFloat( value_f );
        }
        else
        {
            const int32_t   value_i = std::stoi( value );

            m_test << "integer value :" << value_i << std::endl;

            // m_DetectHandler->OnInteger( value_i );
        }
#endif
        _Input.Set( number_it.Get() );

        _Atom.Set( is_float ? Atom::Type::kFloat : Atom::Type::kInterger, value );
    }

    void ParseSymbol( Iterator& _Input, Atom& _Atom )
    {
        Iterator    symbol_it( _Input.Get() );

        while ( !( symbol_it.IsSpace() || symbol_it.IsLeaveSequence() ) )
        {
            symbol_it.Next();
        }

        const std::string   value = symbol_it.GetTrailedString();

        m_test << "symbol value :" << value << std::endl;

        _Input.Set( symbol_it.Get() );

        _Atom.Set( Atom::Type::kSymbol, value );
    }

    void ParseSequence( Iterator& _Input, Atom& _Atom )
    {
        const bool                      is_list             = _Input.IsEnterList();
        const Atom::Type                sequence_type       = is_list ? Atom::Type::kList : Atom::Type::kVector;
        const char                      sequence_leave_code = is_list ? ')' : ']';
        Iterator                        it( _Input.Get() );
        size_t                          index               = 0;
        Atom                            atom;
        Atom                            prev_atom;
        IDetectHandler::SequenceContext context;

        context.m_Type         = sequence_type;
        context.m_ParentSymbol = &( m_ParseLayer.top() );

        // skip begin bracket
        it.Next();

        // start sequence
        m_DetectHandler->SetSequenceDepth( IncreaseDepth() );
        m_DetectHandler->OnEnterSequence( context );

        while ( !it.Is( sequence_leave_code ) )
        {
            it.SkipSpace();

            if ( it.IsEnterSequence() )
            {
                // sequence
                m_ParseLayer.push( prev_atom.IsType( Atom::Type::kSymbol ) ? prev_atom.m_Value : "" );
                ParseSequence( it, atom );
                m_ParseLayer.pop();
            }
            else
            {
                // atom element
                // symbol, string, interger, float
                if ( it.IsEnterString() )
                {
                    ParseString( it, atom );
                }
                else if ( it.IsEnterNumber() )
                {
                    ParseNumber( it, atom );
                }
                else
                {
                    ParseSymbol( it, atom );
                }

                m_DetectHandler->OnAtom( index, atom );
            }

            // property list element parse
            if ( ( context.m_Mode == IDetectHandler::SequenceContext::ParseMode::kPropertyList ) 
                 && ( index % 2 ) 
                 && prev_atom.IsType( Atom::Type::kSymbol ) )
            {
                m_DetectHandler->OnProperty( index, prev_atom.m_Value, atom );
            }

            ++index;
            prev_atom = std::move( atom );
        }

        context.m_Length = index;

        // end sequence
        m_DetectHandler->OnLeaveSequence( context );
        DecreaseDepth();

        // skip end bracket
        it.Next();

        _Input.Set( it.Get() );
        _Atom.Set( Atom::Type::kSequence, "" );
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
    IDetectHandler*             m_DetectHandler = nullptr;
    uint32_t                    m_SequenceDepth = 0;
    std::ostringstream          m_test;
    std::stack< std::string >   m_ParseLayer;
};




}





}




/*================================================================================================*/
/*  Class Inline Method                                                                           */
/*================================================================================================*/





#endif
/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
