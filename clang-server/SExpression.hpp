/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/12.17:28:56 */


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



namespace   SAS
{

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

    size_t SkipDelimiter( void )
    {
        const char* skip_begin = m_Current;

        while ( IsSpace() || IsEscape() )
        {
            ++m_Current;
        }

        return ( m_Current - skip_begin );
    }
    
private:
    const char* m_Begin   = nullptr;
    const char* m_Current = nullptr;
};



class Parser
{
public:
    Parser( void ) = default;
    virtual ~Parser( void ) = default;


    void Parse( const char* _Input )
    {
        m_test.str("");
        m_test.clear();

        m_Depth = 0;
        Iterator        it( _Input );

        it.SkipSpace();

        if ( it.IsSequence() )
        {
            ParseSequence( it );
        }
    }

    void ParseString( Iterator& _Input )
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

        m_test << "string value :" << value << std::endl;
    }

    void ParseNumber( Iterator& _Input )
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

        const std::string   number = number_it.GetTrailedString();

        if ( is_float )
        {
            const float     value = std::stof( number );
            // ParseFloat( it.Get() );
            m_test << "float value :" << value << std::endl;
        }
        else
        {
            const int32_t   value = std::stoi( number );
            // ParseInteger( it.Get() );
            m_test << "integer value :" << value << std::endl;
        }

        _Input.Set( number_it.Get() );
    }

    void ParseSymbol( Iterator& _Input )
    {
        Iterator    symbol_it( _Input.Get() );

        while ( !( symbol_it.IsSpace() || symbol_it.IsLeaveSequence() ) )
        {
            symbol_it.Next();
        }

        const std::string   value = symbol_it.GetTrailedString();

        _Input.Set( symbol_it.Get() );

        m_test << "symbol value :" << value << std::endl;
    }

    void ParseSequence( Iterator& _Input )
    {
        ++m_Depth;
        const bool      is_list = _Input.IsEnterList();
        const char      leave_sequence_code = is_list ? ')' : ']';
        Iterator        it( _Input.Get() );

        // skip begin bracket
        it.Next();

        while ( !it.Is( leave_sequence_code ) )
        {
            it.SkipSpace();

            if ( it.IsEnterSequence() )
            {
                // sequence
                ParseSequence( it );
            }
            else
            {
                // atom element
                // symbol, string, interger, float
                if ( it.IsEnterString() )
                {
                    ParseString( it );
                }
                else if ( it.IsEnterNumber() )
                {
                    ParseNumber( it );
                }
                else
                {
                    ParseSymbol( it );
                }
            }
        }

        // skip end bracket
        it.Next();

        _Input.Set( it.Get() );
    }

    void ParseSequence0( Iterator& _Input )
    {
        ++m_Depth;
        const bool      is_list = _Input.IsEnterList();
        const char      leave_sequence_code = is_list ? ')' : ']';
        Iterator        it( _Input.Get() );

        // skip begin bracket
        it.Next();

        // while ( !it.IsLeaveList() )
        while ( !it.Is( leave_sequence_code ) )
        {
            it.SkipSpace();

            if ( it.IsEnterSequence() )
            {
                // sequence
                ParseSequence( it );
            }
            else
            {
                // element
                // symbol, string, interger, float
                if ( it.IsEnterString() )
                {
                    // skip quote
                    it.Next();

                    Iterator    string_it( it.Get() );

                    while ( !( string_it.IsEOS() || string_it.IsLeaveString() ) )
                    {
                        string_it.Next();
                    }

                    const std::string   value = string_it.GetTrailedString();

                    // skip quote
                    string_it.Next();

                    it.Set( string_it.Get() );

                    m_test << "string value :" << value << std::endl;
                }
                else if ( it.IsEnterNumber() )
                {
                    Iterator    number_it( it.Get() );

                    bool    is_float = false;

                    while ( number_it.IsNumber() )
                    {
                        if ( number_it.Is( '.' ) || number_it.Is( 'e' ) )
                        {
                            is_float = true;
                        }

                        number_it.Next();
                    }

                    const std::string     number = number_it.GetTrailedString();

                    if ( is_float )
                    {
                        const float value = std::stof( number );
                        // ParseFloat( it.Get() );
                        m_test << "float value :" << value << std::endl;
                    }
                    else
                    {
                        const int32_t value = std::stoi( number );
                        // ParseInteger( it.Get() );
                        m_test << "integer value :" << value << std::endl;
                    }

                    it.Set( number_it.Get() );
                }
                else
                {
                    Iterator    symbol_it( it.Get() );

                    while ( !( symbol_it.IsSpace() || symbol_it.IsLeaveSequence() ) )
                    {
                        symbol_it.Next();
                    }

                    const std::string     value = symbol_it.GetTrailedString();

                    it.Set( symbol_it.Get() );

                    m_test << "symbol value :" << value << std::endl;
                }
            }
        }

        // skip end bracket
        it.Next();

        _Input.Set( it.Get() );
    }

    

public:
    uint32_t                m_Depth = 0;
    std::ostringstream      m_test;
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
