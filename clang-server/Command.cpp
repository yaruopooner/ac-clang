/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2017/10/17.16:09:59 */

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
using   namespace   Lisp::SAS;



/*================================================================================================*/
/*  Global Class Method Definitions Section                                                       */
/*================================================================================================*/


void CommandContext::AllocateDataObject( IDataObject::EType _InputType, IDataObject::EType _OutputType )
{
    auto    allocator = []( IDataObject::EType _Type ) -> std::shared_ptr< IDataObject >
        {
            switch ( _Type ) 
            {
                case IDataObject::EType::kLisp:
                {
                    std::shared_ptr< IDataObject >   data_object = std::make_shared< DataObject< Lisp::TextObject > >();

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


void CommandContext::Read( const Lisp::TextObject& _InData )
{
    Clear();

    // RequestId, command-type, command-name, session-name, is-profile
    Lisp::SAS::DetectHandler    handler;
    Lisp::SAS::Parser           parser;
    uint32_t                    read_count = 0;

    // handler.m_OnEnterSequence = []( DetectHandler::SequenceContext& _Context ) -> bool
    //     {
    //         if ( *_Context.m_ParentSymbol == ":CFLAGS" )
    //         {
    //             _Context.m_Mode = DetectHandler::SequenceContext::ParseMode::kNormal;
    //         }
    //         else
    //         {
    //             _Context.m_Mode = DetectHandler::SequenceContext::ParseMode::kPropertyList;
    //         }

    //         return true;
    //     };
    handler.m_OnProperty = [this, &read_count]( const size_t _Index, const std::string& _Symbol, const SExpression& _SExpression ) -> bool
        {
            if ( _Symbol == ":RequestId" )
            {
                m_RequestId = _SExpression.GetValue< uint32_t >();
                ++read_count;
            }
            else if ( _Symbol == ":CommandType" )
            {
                m_CommandType = _SExpression.GetValue< std::string >();
                ++read_count;
            }
            else if ( _Symbol == ":CommandName" )
            {
                m_CommandName = _SExpression.GetValue< std::string >();
                ++read_count;
            }
            else if ( _Symbol == ":SessionName" )
            {
                m_SessionName = _SExpression.GetValue< std::string >();
                ++read_count;
            }
            else if ( _Symbol == ":IsProfile" )
            {
                m_IsProfile = _SExpression.GetValue< bool >();
                ++read_count;
            }

            if ( read_count == 5 )
            {
                return false;
            }

            return true;
        };

    parser.Parse( _InData, handler );
}

void CommandContext::Read( const Json& _InData )
{
    Clear();

    // RequestId, command-type, command-name, session-name, is-profile
    m_RequestId   = _InData[ "RequestId" ];
    m_CommandType = _InData[ "CommandType" ];
    m_CommandName = _InData[ "CommandName" ];
    m_SessionName = ( _InData.find( "SessionName" ) != _InData.end() ) ? _InData[ "SessionName" ] : std::string();
    m_IsProfile   = ( _InData.find( "IsProfile" ) != _InData.end() ) ? _InData[ "IsProfile" ] : false;
}




/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
