#ifndef COLLOCATIONINTEGRATOR_HPP
#define COLLOCATIONINTEGRATOR_HPP

#include <iosfwd>

#include "Config.hpp"

#include <Eigen/Dense>

class Element;

#include "DerivativeTypes.hpp"
#include "DiagonalIntegrator.hpp"
#include "DiagonalIntegratorFactory.hpp"
#include "AnisotropicLiquid.hpp"
#include "IonicLiquid.hpp"
#include "UniformDielectric.hpp"
#include "Vacuum.hpp"
#include "TanhSphericalDiffuse.hpp"

/*! \file CollocationIntegrator.hpp
 *  \class CollocationIntegrator
 *  \brief Implementation of diagonal elements of S and D using approximate collocation
 *  \author Roberto Di Remigio
 *  \date 2014
 *
 *  Calculates the diagonal elements of S as:
 *  \f[
 *  	S_{ii} = factor_ * \sqrt{\frac{4\pi}{a_i}}
 *  \f]
 *  while the diagonal elements of D are:
 *  \f[
 *  	D_{ii} = -factor_ * \sqrt{\frac{\pi}{a_i}} \frac{1}{R_I}
 *  \f]
 */

class CollocationIntegrator : public DiagonalIntegrator
{
public:
    CollocationIntegrator() : factor_(1.07) {}
    virtual ~CollocationIntegrator() {}

    virtual double computeS(const Vacuum<double> * gf, const Element & e) const;
    virtual double computeS(const Vacuum<AD_directional> * gf, const Element & e) const;
    virtual double computeS(const Vacuum<AD_gradient> * gf, const Element & e) const;
    virtual double computeS(const Vacuum<AD_hessian> * gf, const Element & e) const;

    virtual double computeD(const Vacuum<double> * gf, const Element & e) const;
    virtual double computeD(const Vacuum<AD_directional> * gf, const Element & e) const;
    virtual double computeD(const Vacuum<AD_gradient> * gf, const Element & e) const;
    virtual double computeD(const Vacuum<AD_hessian> * gf, const Element & e) const;

    virtual double computeS(const UniformDielectric<double> * gf, const Element & e) const;
    virtual double computeS(const UniformDielectric<AD_directional> * gf, const Element & e) const;
    virtual double computeS(const UniformDielectric<AD_gradient> * gf, const Element & e) const;
    virtual double computeS(const UniformDielectric<AD_hessian> * gf, const Element & e) const;

    virtual double computeD(const UniformDielectric<double> * gf, const Element & e) const;
    virtual double computeD(const UniformDielectric<AD_directional> * gf, const Element & e) const;
    virtual double computeD(const UniformDielectric<AD_gradient> * gf, const Element & e) const;
    virtual double computeD(const UniformDielectric<AD_hessian> * gf, const Element & e) const;

    virtual double computeS(const IonicLiquid<double> * gf, const Element & e) const;
    virtual double computeS(const IonicLiquid<AD_directional> * gf, const Element & e) const;
    virtual double computeS(const IonicLiquid<AD_gradient> * gf, const Element & e) const;
    virtual double computeS(const IonicLiquid<AD_hessian> * gf, const Element & e) const;

    virtual double computeD(const IonicLiquid<double> * gf, const Element & e) const;
    virtual double computeD(const IonicLiquid<AD_directional> * gf, const Element & e) const;
    virtual double computeD(const IonicLiquid<AD_gradient> * gf, const Element & e) const;
    virtual double computeD(const IonicLiquid<AD_hessian> * gf, const Element & e) const;

    virtual double computeS(const AnisotropicLiquid<double> * gf, const Element & e) const;
    virtual double computeS(const AnisotropicLiquid<AD_directional> * gf, const Element & e) const;
    virtual double computeS(const AnisotropicLiquid<AD_gradient> * gf, const Element & e) const;
    virtual double computeS(const AnisotropicLiquid<AD_hessian> * gf, const Element & e) const;

    virtual double computeD(const AnisotropicLiquid<double> * gf, const Element & e) const;
    virtual double computeD(const AnisotropicLiquid<AD_directional> * gf, const Element & e) const;
    virtual double computeD(const AnisotropicLiquid<AD_gradient> * gf, const Element & e) const;
    virtual double computeD(const AnisotropicLiquid<AD_hessian> * gf, const Element & e) const;

    virtual double computeS(const TanhSphericalDiffuse * gf, const Element & e) const;

    virtual double computeD(const TanhSphericalDiffuse * gf, const Element & e) const;
private:
    /// Scaling factor for the collocation formulas
    double factor_;
};

namespace
{
    DiagonalIntegrator * createCollocationIntegrator()
    {
        return new CollocationIntegrator();
    }
    const std::string COLLOCATION("COLLOCATION");
    const bool registeredCollocationIntegrator = DiagonalIntegratorFactory::TheDiagonalIntegratorFactory().registerDiagonalIntegrator(
                                         COLLOCATION, createCollocationIntegrator);
}

#endif // COLLOCATIONINTEGRATOR_HPP