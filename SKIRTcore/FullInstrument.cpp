/*//////////////////////////////////////////////////////////////////
////       SKIRT -- an advanced radiative transfer code         ////
////       © Astronomical Observatory, Ghent University         ////
///////////////////////////////////////////////////////////////// */

#include "DustEmissivity.hpp"
#include "FatalError.hpp"
#include "FullInstrument.hpp"
#include "LockFree.hpp"
#include "PanDustSystem.hpp"
#include "PhotonPackage.hpp"
#include "WavelengthGrid.hpp"

using namespace std;

////////////////////////////////////////////////////////////////////

FullInstrument::FullInstrument()
    : _Nscatt(0), _dustsystem(false), _dustemission(false), _polarization(false)
{
}

////////////////////////////////////////////////////////////////////

void FullInstrument::setupSelfBefore()
{
    SingleFrameInstrument::setupSelfBefore();

    // determine whether the simulation contains a dust system
    try
    {
        // attempt to locate the dust system without performing setup
        // to avoid catching (and hiding) fatal errors during such setup
        find<DustSystem>(false);
        _dustsystem = true;
    }
    catch (FatalError) { }

    if (_dustsystem)
    {
        // determine whether the simulation includes dust emission
        try
        {
            // attempt to locate a panchromatic dust system without performing setup
            // to avoid catching (and hiding) fatal errors during such setup
            // and, if successful, ask it whether dust emission has been turned on
            _dustemission = find<PanDustSystem>(false)->dustemission();
        }
        catch (FatalError) { }

        // determine whether the simulation includes polarization;
        // a dust mix knows whether it supports polarization only after it has been setup,
        // so here we need to fully setup the dust system before querying it
        _polarization = find<DustSystem>()->polarization();
    }

    // resize the detector arrays only when meaningful
    int Nlambda = find<WavelengthGrid>()->Nlambda();
    _fdirv.resize(Nlambda*_Nframep);
    _Fdirv.resize(Nlambda);
    if (_dustsystem)
    {
        _ftrav.resize(Nlambda*_Nframep);
        _Ftrav.resize(Nlambda);
        _fscav.resize(Nlambda*_Nframep);
        _Fscav.resize(Nlambda);
        if (_Nscatt > 0)
        {
            _fscavv.resize(_Nscatt+1, Nlambda*_Nframep);
            _Fscavv.resize(_Nscatt+1, Nlambda);
        }
        if (_dustemission)
        {
            _fdusv.resize(Nlambda*_Nframep);
            _Fdusv.resize(Nlambda);
        }
        if (_polarization)
        {
            _ftotQv.resize(Nlambda*_Nframep);
            _FtotQv.resize(Nlambda);
            _ftotUv.resize(Nlambda*_Nframep);
            _FtotUv.resize(Nlambda);
            _ftotVv.resize(Nlambda*_Nframep);
            _FtotVv.resize(Nlambda);
        }
    }
}

////////////////////////////////////////////////////////////////////

void FullInstrument::setScatteringLevels(int value)
{
    _Nscatt = value;
}

////////////////////////////////////////////////////////////////////

int FullInstrument::scatteringLevels() const
{
    return _Nscatt;
}

////////////////////////////////////////////////////////////////////

void FullInstrument::detect(PhotonPackage* pp)
{
    int l = pixelondetector(pp);
    int ell = pp->ell();
    double L = pp->luminosity();
    double taupath = opticalDepth(pp);
    double extf = exp(-taupath);
    double Lextf = L*extf;

    // SEDs
    if (pp->isStellar())
    {
        int nscatt = pp->nScatt();
        if (nscatt==0)
        {
            if (_dustsystem) LockFree::add(_Ftrav[ell], L);
            LockFree::add(_Fdirv[ell], Lextf);
        }
        else
        {
            LockFree::add(_Fscav[ell], Lextf);
            if (nscatt<=_Nscatt) LockFree::add(_Fscavv[nscatt][ell], Lextf);
        }
    }
    else
    {
        LockFree::add(_Fdusv[ell], Lextf);
    }
    if (_polarization)
    {
        LockFree::add(_FtotQv[ell], Lextf*pp->stokesQ());
        LockFree::add(_FtotUv[ell], Lextf*pp->stokesU());
        LockFree::add(_FtotVv[ell], Lextf*pp->stokesV());
    }

    // frames
    if (l>=0)
    {
        size_t m = l + ell*_Nframep;
        if (pp->isStellar())
        {
            int nscatt = pp->nScatt();
            if (nscatt==0)
            {
                if (_dustsystem) LockFree::add(_ftrav[m], L);
                LockFree::add(_fdirv[m], Lextf);
            }
            else
            {
                LockFree::add(_fscav[m], Lextf);
                if (nscatt<=_Nscatt) LockFree::add(_fscavv[nscatt][m], Lextf);
            }
        }
        else
        {
            LockFree::add(_fdusv[m], Lextf);
        }
        if (_polarization)
        {
            LockFree::add(_ftotQv[m], Lextf*pp->stokesQ());
            LockFree::add(_ftotUv[m], Lextf*pp->stokesU());
            LockFree::add(_ftotVv[m], Lextf*pp->stokesV());
        }
    }
}

////////////////////////////////////////////////////////////////////

void FullInstrument::write()
{
    // compute the total flux in temporary arrays
    Array ftotv;
    Array Ftotv;
    if (_dustemission)
    {
        ftotv = _fdirv + _fscav + _fdusv;
        Ftotv = _Fdirv + _Fscav + _Fdusv;
    }
    else if (_dustsystem)
    {
        ftotv = _fdirv + _fscav;
        Ftotv = _Fdirv + _Fscav;
    }
    else
    {
        // don't output direct or transparent frame's separately because they are identical to the total frame
        ftotv = _fdirv;
        _fdirv.resize(0);
        // do output integrated fluxes to avoid confusing zeros
        Ftotv = _Fdirv;
        _Ftrav = _Fdirv;
    }

    // lists of f-array and F-array pointers, and the corresponding file and column names
    QList< Array* > farrays, Farrays;
    QStringList fnames, Fnames;
    farrays << &ftotv << &_fdirv << &_fscav << &_fdusv << &_ftrav;
    Farrays << &Ftotv << &_Fdirv << &_Fscav << &_Fdusv << &_Ftrav;
    fnames << "total" << "direct" << "scattered" << "dust" << "transparent";
    Fnames << "total flux" << "direct stellar flux" << "scattered stellar flux" << "dust flux" << "transparent flux";
    if (_polarization)
    {
        farrays << &_ftotQv << &_ftotUv << &_ftotVv;
        Farrays << &_FtotQv << &_FtotUv << &_FtotVv;
        fnames << "stokesQ" << "stokesU" << "stokesV";
        Fnames << "total Stokes Q" << "total Stokes U" << "total Stokes V";
    }
    for (int nscatt=1; nscatt<=_Nscatt; nscatt++)
    {
        farrays << &(_fscavv[nscatt]);
        Farrays << &(_Fscavv[nscatt]);
        fnames << ("scatteringlevel" + QString::number(nscatt));
        Fnames << (QString::number(nscatt) + "-times scattered flux");
    }

    // Sum the flux arrays element-wise across the different processes
    sumResults(farrays);
    sumResults(Farrays);

    // calibrate and output the arrays
    calibrateAndWriteDataCubes(farrays, fnames);
    calibrateAndWriteSEDs(Farrays, Fnames);
}

////////////////////////////////////////////////////////////////////
