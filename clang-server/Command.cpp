/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/13.22:14:00 */

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

#include "Command.hpp"


using   namespace   std;



/*================================================================================================*/
/*  Global Class Method Definitions Section                                                       */
/*================================================================================================*/


CommandContext::CommandContext( void )
{
}


void CommandContext::AllocateDataObject( IDataObject::EType _InputType, IDataObject::EType _OutputType )
{
    auto    allocator = []( IDataObject::EType _Type ) -> std::shared_ptr< IDataObject >
    {
        switch ( _Type ) 
        {
            case IDataObject::EType::kSExpression:
            {
                std::shared_ptr< IDataObject >   data_object = std::make_shared< DataObject< SExpression::TextObject > >();

                return data_object;
            }
            break;
            case IDataObject::EType::kJson:
            {
                std::shared_ptr< IDataObject >   data_object = std::make_shared< DataObject< Json > >();

                return data_object;
            }
            break;
            default:
            assert( 0 );
            break;
        }

        return nullptr;
    };

    m_Input  = allocator( _InputType );
    m_Output = allocator( _OutputType );
}


void CommandContext::SetInputData( const uint8_t* _Data )
{
    m_Input->Clear();
    m_Input->SetData( _Data );
    m_Input->Decode( *this );
}

std::string CommandContext::GetOutputData( void ) const
{
    return m_Output->ToString();
}


void CommandContext::Clear( void )
{
    m_RequestId = 0;
    m_CommandType.clear();
    m_CommandName.clear();
    m_SessionName.clear();
    m_IsProfile = false;
}


void CommandContext::Read( const SExpression::TextObject& _InData )
{
    Clear();

    // RequestId, command-name, session-name?
    SExpression::SAS::CommandParseHandler           handler( m_RequestId, m_CommandType, m_CommandName, m_SessionName, m_IsProfile );
    SExpression::SAS::Parser                        parser;

    parser.Parse( _InData.GetString().c_str(), &handler );

}

void CommandContext::Read( const Json& _InData )
{
    Clear();

    // RequestId, command-name, session-name?
    m_RequestId   = _InData[ "RequestId" ];
    m_CommandType = _InData[ "CommandType" ];
    m_CommandName = _InData[ "CommandName" ];
    m_SessionName = ( _InData.find( "SessionName" ) != _InData.end() ) ? _InData[ "SessionName" ] : std::string();
    m_IsProfile   = ( _InData.find( "IsProfile" ) != _InData.end() ) ? _InData[ "IsProfile" ] : false;
}




/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
