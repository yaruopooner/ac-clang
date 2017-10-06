/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/06.20:16:18 */


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
        *this << _Symbol << " ";
    }
#if 0
    void AddElement( const char* _Element )
    {
        *this << _Element << " ";
    }
    void AddStringElement( const char* _Element )
    {
        // *this << "\"" << _Element << "\" ";
        *this << std::quoted( _Element ) << " ";
    }
#endif
    void AddSeparator( void )
    {
        *this << " ";
    }

    template< typename ElementType >
    void AddElement( const ElementType& _Element )
    {
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
        // *this << "\"" << _Element << "\" ";
        *this << std::quoted( _Element ) << " ";
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
    // operator TextObject&() const
    // {
    //     return m_Object;
    // }

    // void AddSymbol( const std::string& _Symbol )
    // {
    //     AddSymbol( _Symbol.c_str() );
    // }
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
        
    virtual ~AddVector( void ) override
    {
        m_Object.Add( "]" );
    }

};



    


}




/*================================================================================================*/
/*  Class Inline Method                                                                           */
/*================================================================================================*/





#endif
/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
