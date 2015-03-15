/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2015/03/16.02:28:40 */

/*
 * Copyright (c) 2013-2015 yaruopooner [https://github.com/yaruopooner]
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
/*  Include Files                                                                                 */
/*================================================================================================*/


#include "CommandLine.hpp"
#include "ClangServer.hpp"



/*================================================================================================*/
/*  Global Function Definitions Section                                                           */
/*================================================================================================*/


enum
{
    kStreamBuffer_Size  = 1 * 1024 * 1024, 
    kStreamBuffer_MinMB = 1, 
    kStreamBuffer_MaxMB = 5, 
};

enum 
{
    kOption_Help, 
    kOption_Version, 
    kOption_LogFile, 
    kOption_BufferSize_STDIN, 
    kOption_BufferSize_STDOUT, 
};



int main( int argc, char *argv[] )
{
    // parse options
    const std::string   version            = "Clang-Server 1.1.0";
    const std::string   generate           = CMAKE_GENERATOR;
    std::string         logfile;
    size_t              buffer_size_stdin  = kStreamBuffer_MinMB;
    size_t              buffer_size_stdout = kStreamBuffer_MinMB;

    {
        CommandLine::Parser        declare_options;

        declare_options.AddOption( kOption_Help, "help", "h", "Display available options.", true );
        declare_options.AddOption( kOption_Version, "version", "v", "Dispaly current version.", true );
        // declare_options.AddOption< std::string >( kOption_LogFile, "logfile", "l", "Enable IPC records output.(for debug)", true, true, false, "file path" );
        declare_options.AddOption< uint32_t >( kOption_BufferSize_STDIN, "buffer-size-stdin", "bssi", "Buffer size of STDIN. <size> is 1 - 5 MB", true, true, false, "size", CommandLine::RangeReader< uint32_t >( kStreamBuffer_MinMB, kStreamBuffer_MaxMB ) );
        declare_options.AddOption< uint32_t >( kOption_BufferSize_STDOUT, "buffer-size-stdout", "bsso", "Buffer size of STDOUT. <size> is 1 - 5 MB", true, true, false, "size", CommandLine::RangeReader< uint32_t >( kStreamBuffer_MinMB, kStreamBuffer_MaxMB ) );

        if ( declare_options.Parse( argc, argv ) )
        {
            for ( const auto& option_value : declare_options.GetOptionWithValueArray() )
            {
                switch ( option_value->GetId() )
                {
                    case    kOption_Help:
                        declare_options.PrintUsage( "clang-server [options] <values>" );
                        return 0;
                    case    kOption_Version:
                        std::cout << version << " (" << generate << ")" << std::endl;
                        return 0;
                    case    kOption_LogFile:
                        if ( option_value->IsValid() )
                        {
                            const std::string  result = declare_options.GetValue< std::string >( option_value );

                            logfile = result;
                        }
                        break;
                    case    kOption_BufferSize_STDIN:
                        if ( option_value->IsValid() )
                        {
                            const uint32_t  result = declare_options.GetValue< uint32_t >( option_value );

                            buffer_size_stdin = result;
                        }
                        break;
                    case    kOption_BufferSize_STDOUT:
                        if ( option_value->IsValid() )
                        {
                            const uint32_t  result = declare_options.GetValue< uint32_t >( option_value );

                            buffer_size_stdout = result;
                        }
                        break;
                }
            }
            declare_options.PrintWarnings();
        }
        else
        {
            declare_options.PrintErrors();
            return 1;
        }
    }


    // stream buffer expand
    // std::shared_ptr< char >   stdin_buffer( new char[ kStreamBuffer_Size ], std::default_delete< char[] >() );
    // std::shared_ptr< char >   stdout_buffer( new char[ kStreamBuffer_Size ], std::default_delete< char[] >() );

    buffer_size_stdin  *= kStreamBuffer_Size;
    buffer_size_stdout *= kStreamBuffer_Size;
    
    std::cout << "Version            : " << version << std::endl;
    std::cout << "Generate           : " << generate << std::endl;
    // std::cout << "Log File           : " << logfile << std::endl;
    std::cout << "STDIN  Buffer Size : " << buffer_size_stdin << " bytes" << std::endl;
    std::cout << "STDOUT Buffer Size : " << buffer_size_stdout << " bytes" << std::endl;
    // ::fflush( stdout );

    ::setvbuf( stdin, nullptr, _IOFBF, buffer_size_stdin );
    ::setvbuf( stdout, nullptr, _IOFBF, buffer_size_stdout );
    
    
    // server instance
    ClangFlagConverters flag_converter;
    ClangServer         server;

    server.ParseCommand();


    return 0;
}




/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
