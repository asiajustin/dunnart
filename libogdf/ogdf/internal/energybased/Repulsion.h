/*
 * $Revision: 1.5 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-08 16:10:37 +0100 (Thu, 08 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class Repulsion which implements an enrgy
 *        function, where the energy of the layout grows with the
 *        proximity of the vertices.
 *
 * Thus, a layout has lower energy if all vertices are far appart.
 * 
 * \author Rene Weiskircher
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_REPULSION_H
#define OGDF_REPULSION_H


#include <ogdf/internal/energybased/NodePairEnergy.h>


namespace ogdf {

class Repulsion: public NodePairEnergy {
public:
	//Initializes data structures to speed up later computations
	Repulsion(GraphAttributes &AG);
private:
	//computes for two vertices an the given positions the repulsive energy
	double computeCoordEnergy(node, node, const DPoint&, const DPoint&) const;
};


}// namespace ogdf

#endif
