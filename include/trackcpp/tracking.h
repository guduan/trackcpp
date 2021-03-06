// TRACKCPP - Particle tracking code
// Copyright (C) 2015  LNLS Accelerator Physics Group
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _TRACKING_H
#define _TRACKING_H

#include "passmethods.h"
#include "accelerator.h"
#include "elements.h"
#include "auxiliary.h"
#include "linalg.h"
#include <vector>
#include <limits>
#include <cmath>

Status::type track_findm66     (const Accelerator& accelerator, std::vector<Pos<double> >& closed_orbit, std::vector<Matrix>& tm, Matrix& m66, Pos<double>& v0);
Status::type track_findorbit4  (const Accelerator& accelerator, std::vector<Pos<double> >& closed_orbit, const Pos<double>& fixed_point_guess = Pos<double>(0));
Status::type track_findorbit6  (const Accelerator& accelerator, std::vector<Pos<double> >& closed_orbit, const Pos<double>& fixed_point_guess = Pos<double>(0));
Pos<double>  linalg_solve4_posvec (const std::vector<Pos<double> >& M, const Pos<double>& b);
Pos<double>  linalg_solve6_posvec (const std::vector<Pos<double> >& M, const Pos<double>& b);

template <typename T>
Status::type track_elementpass (
		     const Element& el,                 // element through which to track particle
		     Pos<T> &orig_pos,                  // initial electron coordinates
		     const Accelerator& accelerator) {

	Status::type status = Status::success;

	switch (el.pass_method) {
	case PassMethod::pm_identity_pass:
		if ((status = pm_identity_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	case PassMethod::pm_drift_pass:
		if ((status = pm_drift_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	case PassMethod::pm_str_mpole_symplectic4_pass:
		if ((status = pm_str_mpole_symplectic4_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	case PassMethod::pm_bnd_mpole_symplectic4_pass:
		if ((status = pm_bnd_mpole_symplectic4_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	case PassMethod::pm_corrector_pass:
		if ((status = pm_corrector_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	case PassMethod::pm_cavity_pass:
		if ((status = pm_cavity_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	case PassMethod::pm_thinquad_pass:
		if ((status = pm_thinquad_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	case PassMethod::pm_thinsext_pass:
		if ((status = pm_thinsext_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	case PassMethod::pm_kicktable_pass:
		if ((status = pm_kicktable_pass<T>(orig_pos, el, accelerator)) != Status::success) return status;
		break;
	default:
		return Status::passmethod_not_defined;
	}

	return status;

}

// linepass
// --------
// tracks particles along a beam transport line
//
// inputs:
//		line:	 		Element vector representing the beam transport line
//		orig_pos:		Pos vector representing initial positions of particles
//		element_offset:	equivalent to shifting the lattice so that '*element_offset' is the index for the first element
//		trajectory:		flag indicating that trajectory is to be recorded at entrance of all elements
//						(otherwise only the coordinates at the exit of last element is recorded)
// outputs:
//		pos:			Pos vector of particles' final coordinates (or trajetory)
//		element_offset:	in case of problems with passmethods, '*element_offset' is the index of the corresponding element
//		RETURN:			status do tracking (see 'auxiliary.h')

template <typename T>
Status::type track_linepass (
		const Accelerator& accelerator,
		Pos<T>& orig_pos,              // initial electron coordinates
		std::vector<Pos<T> >& pos,     // vector with tracked electron coordinates at start of every element and at the end of last one.
		unsigned int& element_offset,  // index of starting element for tracking
		Plane::type& lost_plane,       // return plane in which particle was lost, if the case.
		bool trajectory) {             // whether function should return coordinates at all elements

	Status::type status = Status::success;

	const std::vector<Element>& line = accelerator.lattice;
	const Pos<T> nan_pos(nan(""),nan(""),nan(""),nan(""),nan(""),nan(""));
	int nr_elements  = line.size();

	//pos.clear(); other functions assume pos is not clearedin linepass!
	if (trajectory) {
		for(int i=0; i<nr_elements; ++i) {
			pos.push_back(nan_pos);
		}
	}

	for(int i=0; i<nr_elements; ++i) {

		const Element& element = line[element_offset];  // syntactic-sugar for read-only access to element object parameters

		// stores trajectory at entrance of each element
		if (trajectory) pos[i] = orig_pos;

		Status::type status = track_elementpass (element, orig_pos, accelerator);

		// checks if particle is lost
		if ((not isfinite(orig_pos.rx)) or
			((accelerator.vchamber_on) and
			 ((orig_pos.rx < element.hmin) or
			 (orig_pos.rx >  element.hmax)))) {
			pos.push_back(nan_pos);
			lost_plane   = Plane::x;
			return (status == Status::success) ? Status::particle_lost : status;
		}
		if ((not isfinite(orig_pos.ry)) or
			((accelerator.vchamber_on) and
			 ((orig_pos.ry < element.vmin) or
			 (orig_pos.ry >  element.vmax)))) {
			pos.push_back(nan_pos);
			lost_plane = Plane::y;
			return (status == Status::success) ? Status::particle_lost : status;
		}

		if (status != Status::success) return status;

		// moves to next element index
		element_offset = (element_offset + 1) % nr_elements;

	}

	lost_plane = Plane::no_plane;

	// stores final particle position at the end of the line
	pos.push_back(orig_pos);

	return status;

}

// ringpass
// --------
// tracks particles around a ring
//
// inputs:
//		ring: 			Element vector representing the ring
//		orig_pos:		Pos vector representing initial positions of particles
//		element_offset:	equivalent to shifting the lattice so that '*element_offset' is the index for the first element
//		nr_turns:		number of turns for tracking
//
// outputs:
//		pos:			Pos vector of particles' coordinates of the turn_by_turn data (at end of the ring at each turn)
//		turn_idx:		in case of problems with passmethods, '*turn_idx' is the index of the corresponding turn
//		element_offset:	in case of problems with passmethods, '*element_offset' is the index of the corresponding element
//		RETURN:			status do tracking (see 'auxiliary.h')



template <typename T>
Status::type track_ringpass (
		const Accelerator& accelerator,
		Pos<T> &orig_pos,
		std::vector<Pos<T> > &pos,
		const unsigned int nr_turns,
		unsigned int &lost_turn,
		unsigned int &element_offset,
		Plane::type& lost_plane,
		bool trajectory) {

	Status::type status  = Status::success;
	std::vector<Pos<T> > final_pos;

	for(lost_turn=0; lost_turn<nr_turns; ++lost_turn) {
		if ((status = track_linepass (accelerator, orig_pos, final_pos, element_offset, lost_plane, false)) != Status::success) {
			return status;
		}
		if (trajectory) {
			pos.push_back(orig_pos);
		}
	}

	if (not trajectory) {
		// stores only final position if trajectory is not requested
		pos.push_back(orig_pos);
	}
	return status;
}


#endif
