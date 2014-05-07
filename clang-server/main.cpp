/* -*- mode: c++ ; coding: utf-8-unix -*- */
/*	last updated : 2014/05/07.23:05:06 */

/*
 * Copyright (c) 2013-2014 yaruopooner [https://github.com/yaruopooner]
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






int	main( int argc, char *argv[] )
{
	ClangFlagConverters	flag_converter;
	ClangServer			server;

	server.ParseCommand();

	return ( 0 );
}




/*================================================================================================*/
/*  EOF                                                                                           */
/*================================================================================================*/
