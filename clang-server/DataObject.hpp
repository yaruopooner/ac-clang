/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/24.11:46:57 */


#pragma once

#ifndef __DATA_OBJECT_HPP__
#define __DATA_OBJECT_HPP__



/*================================================================================================*/
/*  Comment                                                                                       */
/*================================================================================================*/


/*================================================================================================*/
/*  Include Files                                                                                 */
/*================================================================================================*/

#include <string>

#include "parser/Lisp.hpp"
#include "parser/json.hpp"

using Json = nlohmann::json;




/*================================================================================================*/
/*  Class                                                                                         */
/*================================================================================================*/


template< typename DataType >
class ISerializable
{
protected:
    ISerializable( void )
    {
    }
    virtual ~ISerializable( void ) = default;

public:
    virtual void Read( const DataType& /*_InData*/ )
    {
    }
    virtual void Write( DataType& /*_OutData*/ ) const
    {
    }
};


class IMultiSerializable: public ISerializable< Lisp::TextObject >, public ISerializable< Lisp::DOM::NodeObject >, public ISerializable< Json >
{
protected:
    IMultiSerializable( void )
    {
    }
    virtual ~IMultiSerializable( void ) = default;
};



class IDataObject
{
public:
    enum EType
    {
        kInvalid = -1, 
        kLispText = 0, 
        kLispNode, 
        kJson, 
    };

protected:
    IDataObject( EType _Type = EType::kInvalid ) : m_Type( _Type )
    {
    }
    virtual ~IDataObject( void ) = default;
    
public:

    EType GetType( void ) const
    {
        return m_Type;
    }

    template< typename DataType >
    bool IsSame( void ) const
    {
        return ( TypeTraits< DataType >::Value == m_Type );
    }
    

    template< typename SerializableVisitor >
    void Encode( const SerializableVisitor& _Visitor );

    template< typename SerializableVisitor >
    void Decode( SerializableVisitor& _Visitor ) const;

    virtual void SetData( const uint8_t* _Address ) = 0;
    virtual std::string ToString( void ) const = 0;
    virtual void Clear( void ) = 0;
    
    
    template< typename DataType > struct TypeTraits { enum { Value = EType::kInvalid, }; };
    template<> struct TypeTraits< Lisp::TextObject > { enum { Value = EType::kLispText, }; };
    template<> struct TypeTraits< Lisp::DOM::NodeObject > { enum { Value = EType::kLispNode, }; };
    template<> struct TypeTraits< Json > { enum { Value = EType::kJson, }; };

    // template< EType Value > struct Traits {  };
    // template<> struct Traits< EType::kLispText > { using Type = Lisp::TextObject; };
    // template<> struct Traits< EType::kJson > { using Type = Json; };

protected:
    EType           m_Type;
};


template< typename DataType >
class DataObject : public IDataObject
{
public:
    DataObject( void ) : IDataObject( static_cast< EType >( IDataObject::TypeTraits< DataType >::Value ) )
    {
    }
    virtual ~DataObject( void ) override = default;
    

    // Type&    GetData( void )
    // {
    //     return m_Data;
    // }
    
    // const Type&    GetData( void ) const
    // {
    //     return m_Data;
    // }

    void Encode( const ISerializable< DataType >& _Visitor )
    {
        _Visitor.Write( m_Data );
    }

    void Decode( ISerializable< DataType >& _Visitor ) const
    {
        _Visitor.Read( m_Data );
    }

    virtual void SetData( const uint8_t* ) override
    {
    }
    virtual std::string ToString( void ) const override
    {
        return std::string();
    }
    virtual void Clear( void ) override
    {
    }


protected:
    DataType       m_Data;
};


template<>
void DataObject< Lisp::TextObject >::SetData( const uint8_t* _Address )
{
    m_Data.Set( reinterpret_cast< const char* >( _Address ) );
}

template<>
void DataObject< Lisp::DOM::NodeObject >::SetData( const uint8_t* _Address )
{
    m_Data.Parse( reinterpret_cast< const char* >( _Address ) );
}

template<>
void DataObject< Json >::SetData( const uint8_t* _Address )
{
    m_Data = Json::parse( _Address );
}


template<>
std::string DataObject< Lisp::TextObject >::ToString( void ) const
{
    return m_Data.GetString();
}

template<>
std::string DataObject< Json >::ToString( void ) const
{
    if ( m_Data.empty() )
    {
        return std::string();
    }

    return m_Data.dump();
}


template<>
void DataObject< Lisp::TextObject >::Clear( void )
{
    m_Data.Clear();
}

template<>
void DataObject< Lisp::DOM::NodeObject >::Clear( void )
{
    m_Data.Clear();
}

template<>
void DataObject< Json >::Clear( void )
{
    m_Data.clear();
}





/*================================================================================================*/
/*  Class Inline Method                                                                           */
/*================================================================================================*/


template< typename SerializableVisitor >
void IDataObject::Encode( const SerializableVisitor& _Visitor )
{
    switch ( m_Type )
    {
        case EType::kLispText:
            {
                auto    data_object = reinterpret_cast< DataObject< Lisp::TextObject >* >( this );

                data_object->Encode( _Visitor );
            }
            break;
        case EType::kLispNode:
            {
                auto    data_object = reinterpret_cast< DataObject< Lisp::DOM::NodeObject >* >( this );

                data_object->Encode( _Visitor );
            }
            break;
        case EType::kJson:
            {
                auto    data_object = reinterpret_cast< DataObject< Json >* >( this );

                data_object->Encode( _Visitor );
            }
            break;
        default:
            break;
    }
}


template< typename SerializableVisitor >
void IDataObject::Decode( SerializableVisitor& _Visitor ) const
{
    switch ( m_Type )
    {
        case EType::kLispText:
            {
                auto    data_object = reinterpret_cast< const DataObject< Lisp::TextObject >* >( this );

                data_object->Decode( _Visitor );
            }
            break;
        case EType::kLispNode:
            {
                auto    data_object = reinterpret_cast< const DataObject< Lisp::DOM::NodeObject >* >( this );

                data_object->Decode( _Visitor );
            }
            break;
        case EType::kJson:
            {
                auto    data_object = reinterpret_cast< const DataObject< Json >* >( this );

                data_object->Decode( _Visitor );
            }
            break;
        default:
            break;
    }
}





#endif
/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
