/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2018/01/05.23:26:12 */

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

#include "Command.hpp"
#include "Profiler.hpp"




/*================================================================================================*/
/*  Global Class Method Definitions Section                                                       */
/*================================================================================================*/

namespace 
{


std::shared_ptr< IDataObject >  AllocateDataObject( IDataObject::EType _Type )
{
    switch ( _Type ) 
    {
        case IDataObject::EType::kLispText:
            {
                std::shared_ptr< IDataObject >   data_object = std::make_shared< DataObject< Lisp::Text::Object > >();

                return data_object;
            }
            break;
        case IDataObject::EType::kLispNode:
            {
                std::shared_ptr< IDataObject >   data_object = std::make_shared< DataObject< Lisp::Node::Object > >();

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


}  // namespace 




void CommandContext::AllocateDataObject( IDataObject::EType _InputType, IDataObject::EType _OutputType )
{
    m_Input  = ::AllocateDataObject( _InputType );
    m_Output = ::AllocateDataObject( _OutputType );
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


void CommandContext::Read( const Lisp::Text::Object& _InData )
{
    Clear();

    // RequestId, command-type, command-name, session-name, is-profile
    Lisp::SAS::DetectHandler    handler;
    Lisp::SAS::Parser           parser;
    uint32_t                    read_count = 0;

    handler.m_OnEnterSequence = [this]( Lisp::SAS::DetectHandler::SequenceContext& _Context ) -> bool
        {
            _Context.m_Mode = Lisp::SAS::DetectHandler::SequenceContext::ParseMode::kPropertyList;

            return true;
        };
    handler.m_OnProperty = [this, &read_count]( const size_t _Index, const std::string& _Symbol, const Lisp::SAS::SExpression& _SExpression ) -> bool
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

void CommandContext::Read( const Lisp::Node::Object& _InData )
{
    Clear();

    // RequestId, command-type, command-name, session-name, is-profile
    Lisp::Node::PropertyListIterator    iterator = _InData.GetRootPropertyListIterator();

    for ( ; !iterator.IsEnd(); iterator.Next() )
    {
        if ( iterator.IsSameKey( ":RequestId" ) )
        {
            m_RequestId = iterator.GetValue< int32_t >();
        }
        else if ( iterator.IsSameKey( ":CommandType" ) )
        {
            m_CommandType = iterator.GetValue< std::string >();
        }
        else if ( iterator.IsSameKey( ":CommandName" ) )
        {
            m_CommandName = iterator.GetValue< std::string >();
        }
        else if ( iterator.IsSameKey( ":SessionName" ) )
        {
            m_SessionName = iterator.GetValue< std::string >();
        }
        else if ( iterator.IsSameKey( ":IsProfile" ) )
        {
            m_IsProfile = iterator.GetValue< bool >();
        }
    }
}

void CommandContext::Read( const Json& _InData )
{
    Clear();

    // RequestId, command-type, command-name, session-name, is-profile
    m_RequestId   = _InData[ "RequestId" ];
    m_CommandType = _InData[ "CommandType" ];
    m_CommandName = _InData[ "CommandName" ];
    if ( _InData.find( "SessionName" ) != _InData.end() )
    {
        m_SessionName = _InData[ "SessionName" ];
    }
    if ( _InData.find( "IsProfile" ) != _InData.end() )
    {
        m_IsProfile = _InData[ "IsProfile" ];
    }
}


void CommandContext::Write( Lisp::Text::Object& _OutData ) const
{
    Lisp::Text::AppendList  plist( _OutData );

    plist.AddSymbol( ":Profiles" );
    {
        Lisp::Text::NewVector   results_vector( plist );

        const auto&     sampled_profiles = Profiler::Sampler::GetInstance().GetProfiles();

        for ( const auto& profile : sampled_profiles )
        {
            if ( profile.m_IsFinish )
            {
                Lisp::Text::NewList profile_plist( results_vector );

                profile_plist.AddProperty( ":Name", profile.GetName() );
                profile_plist.AddProperty( ":ElapsedTime", profile.GetElapsedTime() );
            }
        }
    }
}

void CommandContext::Write( Json& _OutData ) const
{
    const auto&     sampled_profiles = Profiler::Sampler::GetInstance().GetProfiles();

    for ( const auto& profile : sampled_profiles )
    {
        if ( profile.m_IsFinish )
        {
            _OutData[ "Profiles" ].push_back(
                                            {
                                                { "Name", profile.GetName() }, 
                                                { "ElapsedTime", profile.GetElapsedTime() }, 
                                            }
                                            );
        }
    }
}





/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
