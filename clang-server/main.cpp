/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*  last updated : 2015/03/06.12:17:04 */

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
/*  Comment                                                                                       */
/*================================================================================================*/


/*================================================================================================*/
/*  Include Files                                                                                 */
/*================================================================================================*/

#include "ClangServer.hpp"




/*================================================================================================*/
/*  Global Function Definitions Section                                                           */
/*================================================================================================*/


enum
{
    kStreamBufferSize = 1024 * 1024, 
};



int main( int argc, char *argv[] )
{
    // stream buffer expand
    std::shared_ptr< char >   stdin_buffer( new char[ kStreamBufferSize ], std::default_delete< char[] >() );
    std::shared_ptr< char >   stdout_buffer( new char[ kStreamBufferSize ], std::default_delete< char[] >() );

    ::setvbuf( stdin, stdin_buffer.get(), _IOFBF, kStreamBufferSize );
    ::setvbuf( stdout, stdout_buffer.get(), _IOFBF, kStreamBufferSize );

    // server instance
    ClangFlagConverters flag_converter;
    ClangServer         server;
    
    server.ParseCommand();

    return ( 0 );
}




/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
