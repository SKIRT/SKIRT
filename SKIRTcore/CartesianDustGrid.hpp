/*//////////////////////////////////////////////////////////////////
////       SKIRT -- an advanced radiative transfer code         ////
////       © Astronomical Observatory, Ghent University         ////
///////////////////////////////////////////////////////////////// */

#ifndef CARTESIANDUSTGRID_HPP
#define CARTESIANDUSTGRID_HPP

#include "Array.hpp"
#include "BoxDustGrid.hpp"
#include "MoveableMesh.hpp"
#include "Random.hpp"

////////////////////////////////////////////////////////////////////

/** The CartesianDustGrid class is subclass of the BoxDustGrid class, and represents
    three-dimensional dust grids based on a regular cartesian grid. Each cell in such a
    grid is a little cuboid (not necessarily all with the same size or axis ratios). */
class CartesianDustGrid : public BoxDustGrid
{
    Q_OBJECT
    Q_CLASSINFO("Title", "a cartesian dust grid")

    Q_CLASSINFO("Property", "meshX")
    Q_CLASSINFO("Title", "the bin distribution in the X direction")
    Q_CLASSINFO("Default", "LinMesh")

    Q_CLASSINFO("Property", "meshY")
    Q_CLASSINFO("Title", "the bin distribution in the Y direction")
    Q_CLASSINFO("Default", "LinMesh")

    Q_CLASSINFO("Property", "meshZ")
    Q_CLASSINFO("Title", "the bin distribution in the Z direction")
    Q_CLASSINFO("Default", "LinMesh")

    //============= Construction - Setup - Destruction =============

public:
    /** The default constructor. */
    Q_INVOKABLE CartesianDustGrid();

    /** This function sets up a number of data members that depend on the Mesh objects configured
        for this grid. */
    void setupSelfAfter();

    //======== Setters & Getters for Discoverable Attributes =======

public:
    /** Sets the bin distribution in the X direction. */
    Q_INVOKABLE void setMeshX(MoveableMesh* value);

    /** Returns the bin distribution in the X direction. */
    Q_INVOKABLE MoveableMesh* meshX() const;

    /** Sets the bin distribution in the Y direction. */
    Q_INVOKABLE void setMeshY(MoveableMesh* value);

    /** Returns the bin distribution in the Y direction. */
    Q_INVOKABLE MoveableMesh* meshY() const;

    /** Sets the bin distribution in the Z direction. */
    Q_INVOKABLE void setMeshZ(MoveableMesh* value);

    /** Returns the bin distribution in the Z direction. */
    Q_INVOKABLE MoveableMesh* meshZ() const;

    //======================== Other Functions =======================

public:
    /** This function returns the number of cells in the dust grid. */
    int numCells() const;

    /** This function returns the volume of the dust cell with cell number \f$m\f$. For a cartesian
        dust grid, the function determines the bin indices \f$i\f$, \f$j\f$ and \f$k\f$
        corresponding to the X, Y and Z directions. The volume is then easily calculated as \f$V =
        (x_{i+1}-x_i)\, (y_{j+1}-y_j)\, (z_{k+1}-z_k) \f$. */
    double volume(int m) const;

    /** This function returns the number of the dust cell that contains the position
        \f${\bf{r}}\f$. For a cartesian dust grid, the function determines the bin
        indices in the X, Y and Z directions and calculates the correct cell number based on these
        indices. */
    int whichcell(Position bfr) const;

    /** This function returns the central location from the dust cell with cell number \f$m\f$. For
        a cartesian dust grid, the function first determines the bin indices \f$i\f$ and
        \f$k\f$ in the X, Y and Z directions that correspond to the cell number \f$m\f$. Then the
        coordinates \f$x\f$, \f$y\f$ and \f$z\f$ are determined using \f[ \begin{split} x &=
        \frac{x_i + x_{i+1}}{2} \\ y &= \frac{y_j + y_{j+1}}{2} \\ z &= \frac{z_k + z_{k+1}}{2}
        \end{split} \f] A position with these cartesian coordinates is returned. */
    Position centralPositionInCell(int m) const;

    /** This function returns a random location from the dust cell with cell number \f$m\f$. For a
        cartesian dust grid, the function first determines the bin indices \f$i\f$ and
        \f$k\f$ in the X, Y and Z directions that correspond to the cell number \f$m\f$. Then
        random coordinates \f$x\f$, \f$y\f$ and \f$z\f$ are determined using \f[ \begin{split} x &=
        x_i + {\cal{X}}_1\,(x_{i+1}-x_i) \\ y &= y_j + {\cal{X}}_2\,(y_{j+1}-y_j) \\ z &= z_k +
        {\cal{X}}_3\, (z_{k+1}-z_k), \end{split} \f] with \f${\cal{X}}_1\f$, \f${\cal{X}}_2\f$ and
        \f${\cal{X}}_3\f$ three uniform deviates. A position with these cartesian coordinates is
        returned. */
    Position randomPositionInCell(int m) const;

    /** This function calculates a path through the grid. The DustGridPath object passed as an
        argument specifies the starting position \f${\bf{r}}\f$ and the direction \f${\bf{k}}\f$
        for the path. The data on the calculated path are added back into the same object. */
    void path(DustGridPath* path) const;

private:
    /** This function writes the intersection of the dust grid structure with the xy plane to the
        specified DustGridPlotFile object. */
    void write_xy(DustGridPlotFile* outfile) const;

    /** This function writes the intersection of the dust grid structure with the xz plane to the
        specified DustGridPlotFile object. */
    void write_xz(DustGridPlotFile* outfile) const;

    /** This function writes the intersection of the dust grid structure with the yz plane to the
        specified DustGridPlotFile object. */
    void write_yz(DustGridPlotFile* outfile) const;

    /** This function writes 3D information for all cells in the dust grid structure to the
        specified DustGridPlotFile object. */
    void write_xyz(DustGridPlotFile* outfile) const;

   /** This function returns the cell number \f$m\f$ corresponding to the three bin indices
       \f$i\f$, \f$j\f$ and \f$k\f$. The correspondence is \f$m=k+j\,N_z+i\,N_y\,N_z\f$. */
    int index(int i, int j, int k) const;

    /** This function calculates the three bin indices \f$i\f$, \f$j\f$ and \f$k\f$ of the cell
        number \f$m\f$, and then returns the coordinates of the corresponding cell as a Box object.
        Since the relation between the cell number and the three bin indices is
        \f$m=k+j\,N_z+i\,N_y\,N_z\f$, we can use the formulae \f[ \begin{split} i &= \lfloor
        m/(N_y\,N_z) \rfloor \\ j &= \lfloor (m-i\,N_y\,N_z)/N_z \rfloor \\ k &=
        m\,{\text{mod}}\,N_z. \end{split} \f] */
    Box box(int m) const;

    //======================== Data Members ========================

private:
    // discoverable properties
    MoveableMesh* _meshx;
    MoveableMesh* _meshy;
    MoveableMesh* _meshz;

    // other data members
    Random* _random;
    int _Nx;
    int _Ny;
    int _Nz;
    Array _xv;
    Array _yv;
    Array _zv;
};

////////////////////////////////////////////////////////////////////

#endif // CARTESIANDUSTGRID_HPP
