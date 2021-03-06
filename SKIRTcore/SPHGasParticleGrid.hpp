/*//////////////////////////////////////////////////////////////////
////       SKIRT -- an advanced radiative transfer code         ////
////       © Astronomical Observatory, Ghent University         ////
///////////////////////////////////////////////////////////////// */

#ifndef SPHGASPARTICLEGRID_HPP
#define SPHGASPARTICLEGRID_HPP

#include <vector>
#include "Array.hpp"
#include "Box.hpp"
#include "SPHGasParticle.hpp"
#include "Vec.hpp"

////////////////////////////////////////////////////////////////////

/** SPHGasParticle is a technical class for organizing SPHGasParticle instances in a smart grid, so
    that it is easy to retrieve a list of all particles that may overlap a particular point in
    space. The Box object on which this class is based specifies a cuboid guaranteed to enclose all
    particles in the grid. */
class SPHGasParticleGrid : public Box
{
public:
    /** The constructor creates a cuboidal grid of the specified number of grid cells in each
        spatial direction, and for each of the cells it builds a list of all particles (partially
        or fully) overlapping the cell. In an attempt to distribute the particles evenly over the
        cells, the sizes of the grid cells in each spatial direction are chosen so that the
        particle centers are evenly distributed over the cells. The internal particle lists store
        pointers to the particle objects contained in the provided list \em pv, so that list
        must not be modified or deallocated as long as this grid instance exists. */
    SPHGasParticleGrid(const std::vector<SPHGasParticle>& pv, int gridsize);

    /** This function returns the smallest number of particles overlapping a single cell. */
    int minParticlesPerCell() const;

    /** This function returns the largest number of particles overlapping a single cell. */
    int maxParticlesPerCell() const;

    /** This function returns the total number of particle references for all cells in the grid. */
    int totalParticles() const;

    /** This function returns a list of all particles that may overlap the specified position. It
        locates the cell containing the specified position and returns the list of particles
        overlapping that cell. Thus the list may include particles that don't actually overlap the
        specified position. The objective of this class is to make this function very fast, while
        limiting the number of unnecessary particles in the returned list. */
    const std::vector<const SPHGasParticle*>& particlesFor(Vec r) const;

    /** This function returns a list containing all particles that may overlap a given box (i.e. a
        cuboid lined up with the coordinate axes). Note that the list may include particles that
        don't actually overlap the specified box. The function locates all grid cells overlapping
        the box and calculates the union of the list of particles overlapping each of these cells
        (i.e. removing any duplicates). */
    std::vector<const SPHGasParticle*> particlesFor(const Box& box) const;

private:
    int _m;  // number of grid cells in each spatial direction
    Array _xgrid, _ygrid, _zgrid;  // the m+1 grid separation points for each spatial direction
    std::vector< std::vector<const SPHGasParticle*> > _listv; // the m*m*m lists of particles overlapping each grid cell
    int _pmin, _pmax, _ptotal;  // minimum, maximum nr of particles in list; total nr of particles in listv
};

////////////////////////////////////////////////////////////////////

#endif // SPHGASPARTICLEGRID_HPP
