/* pcmsolver_copyright_start */
/*
 *     PCMSolver, an API for the Polarizable Continuum Model
 *     Copyright (C) 2013-2015 Roberto Di Remigio, Luca Frediani and contributors
 *
 *     This file is part of PCMSolver.
 *
 *     PCMSolver is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Lesser General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     PCMSolver is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Lesser General Public License for more details.
 *
 *     You should have received a copy of the GNU Lesser General Public License
 *     along with PCMSolver.  If not, see <http://www.gnu.org/licenses/>.
 *
 *     For information on the complete list of contributors to the
 *     PCMSolver API, see: <http://pcmsolver.readthedocs.org/>
 */
/* pcmsolver_copyright_end */

#include "catch.hpp"

#include <iostream>

#include "Config.hpp"

#include <Eigen/Core>

#include "CollocationIntegrator.hpp"
#include "DerivativeTypes.hpp"
#include "GePolCavity.hpp"
#include "Molecule.hpp"
#include "Vacuum.hpp"
#include "UniformDielectric.hpp"
#include "IEFSolver.hpp"
#include "TestingMolecules.hpp"

SCENARIO("Test solver for the IEFPCM for a point charge in different Abelian point groups", "[solver][iefpcm][iefpcm_symmetry]")
{
    GIVEN("An isotropic environment and a point charge")
    {
        double permittivity = 78.39;
        Vacuum<AD_directional, CollocationIntegrator> gfInside = Vacuum<AD_directional, CollocationIntegrator>();
        UniformDielectric<AD_directional, CollocationIntegrator> gfOutside =
            UniformDielectric<AD_directional, CollocationIntegrator>(permittivity);
        bool symm = true;

        double charge = 8.0;
        double totalASC = - charge * (permittivity - 1) / permittivity;

        /*! \class IEFSolver
         *  \test \b pointChargeGePolC1 tests IEFSolver using a point charge with a GePol cavity in C1 symmetry
         */
        WHEN("the point group is C1")
        {
            Molecule point = dummy<0>(2.929075493);
            double area = 0.4;
            double probeRadius = 0.0;
            double minRadius = 100.0;
            GePolCavity cavity(point, area, probeRadius, minRadius, "C1");

            size_t size = cavity.size();
            Eigen::VectorXd fake_mep = computeMEP(cavity.elements(), charge);

            IEFSolver solver(symm);
            solver.buildSystemMatrix(cavity, gfInside, gfOutside);
            THEN("the total apparent surface charge is")
            {
                size_t irr_size = cavity.irreducible_size();
                Eigen::VectorXd fake_asc = Eigen::VectorXd::Zero(size);
                fake_asc = solver.computeCharge(fake_mep);
                int nr_irrep = cavity.pointGroup().nrIrrep();
                double totalFakeASC = fake_asc.sum() * nr_irrep;
                CAPTURE(totalASC - totalFakeASC);
                REQUIRE(totalASC == Approx(totalFakeASC).epsilon(1.0e-03));
            }
        }

        /*! \class IEFSolver
         *  \test \b pointChargeGePolC2 tests IEFSolver using a point charge with a GePol cavity in C2 symmetry
         */
        WHEN("the point group is C2")
        {
            Molecule point = dummy<1>(2.929075493);
            double area = 0.4;
            double probeRadius = 0.0;
            double minRadius = 100.0;
            GePolCavity cavity(point, area, probeRadius, minRadius, "C2");

            size_t size = cavity.size();
            Eigen::VectorXd fake_mep = computeMEP(cavity.elements(), charge);

            IEFSolver solver(symm);
            solver.buildSystemMatrix(cavity, gfInside, gfOutside);
            THEN("the total apparent surface charge is")
            {
                size_t irr_size = cavity.irreducible_size();
                Eigen::VectorXd fake_asc = Eigen::VectorXd::Zero(size);
                fake_asc = solver.computeCharge(fake_mep);
                int nr_irrep = cavity.pointGroup().nrIrrep();
                double totalFakeASC = fake_asc.sum() * nr_irrep;
                CAPTURE(totalASC - totalFakeASC);
                REQUIRE(totalASC == Approx(totalFakeASC).epsilon(1.0e-03));
            }
        }

        /*! \class IEFSolver
         *  \test \b pointChargeGePolCs tests IEFSolver using a point charge with a GePol cavity in Cs symmetry
         */
        WHEN("the point group is Cs")
        {
            Molecule point = dummy<2>(2.929075493);
            double area = 0.4;
            double probeRadius = 0.0;
            double minRadius = 100.0;
            GePolCavity cavity(point, area, probeRadius, minRadius, "Cs");

            size_t size = cavity.size();
            Eigen::VectorXd fake_mep = computeMEP(cavity.elements(), charge);
            for (size_t i = 0; i < size; ++i) {
                Eigen::Vector3d center = cavity.elementCenter(i);
                double distance = center.norm();
                fake_mep(i) = charge / distance;
            }

            IEFSolver solver(symm);
            solver.buildSystemMatrix(cavity, gfInside, gfOutside);
            THEN("the total apparent surface charge is")
            {
                size_t irr_size = cavity.irreducible_size();
                Eigen::VectorXd fake_asc = Eigen::VectorXd::Zero(size);
                fake_asc = solver.computeCharge(fake_mep);
                int nr_irrep = cavity.pointGroup().nrIrrep();
                double totalFakeASC = fake_asc.sum() * nr_irrep;
                CAPTURE(totalASC - totalFakeASC);
                REQUIRE(totalASC == Approx(totalFakeASC).epsilon(1.0e-03));
            }
        }

        /*! \class IEFSolver
         *  \test \b pointChargeGePolCi tests IEFSolver using a point charge with a GePol cavity in Ci symmetry
         */
        WHEN("the point group is Ci")
        {
            Molecule point = dummy<3>(2.929075493);
            double area = 0.4;
            double probeRadius = 0.0;
            double minRadius = 100.0;
            GePolCavity cavity(point, area, probeRadius, minRadius, "Ci");

            size_t size = cavity.size();
            Eigen::VectorXd fake_mep = computeMEP(cavity.elements(), charge);
            for (size_t i = 0; i < size; ++i) {
                Eigen::Vector3d center = cavity.elementCenter(i);
                double distance = center.norm();
                fake_mep(i) = charge / distance;
            }

            IEFSolver solver(symm);
            solver.buildSystemMatrix(cavity, gfInside, gfOutside);
            THEN("the total apparent surface charge is")
            {
                size_t irr_size = cavity.irreducible_size();
                Eigen::VectorXd fake_asc = Eigen::VectorXd::Zero(size);
                fake_asc = solver.computeCharge(fake_mep);
                int nr_irrep = cavity.pointGroup().nrIrrep();
                double totalFakeASC = fake_asc.sum() * nr_irrep;
                CAPTURE(totalASC - totalFakeASC);
                REQUIRE(totalASC == Approx(totalFakeASC).epsilon(1.0e-03));
            }
        }

        /*! \class IEFSolver
         *  \test \b pointChargeGePolD2 tests IEFSolver using a point charge with a GePol cavity in D2 symmetry
         */
        WHEN("the point group is D2")
        {
            Molecule point = dummy<4>(2.929075493);
            double area = 0.4;
            double probeRadius = 0.0;
            double minRadius = 100.0;
            GePolCavity cavity(point, area, probeRadius, minRadius, "D2");

            size_t size = cavity.size();
            Eigen::VectorXd fake_mep = computeMEP(cavity.elements(), charge);
            for (size_t i = 0; i < size; ++i) {
                Eigen::Vector3d center = cavity.elementCenter(i);
                double distance = center.norm();
                fake_mep(i) = charge / distance;
            }

            IEFSolver solver(symm);
            solver.buildSystemMatrix(cavity, gfInside, gfOutside);
            THEN("the total apparent surface charge is")
            {
                size_t irr_size = cavity.irreducible_size();
                Eigen::VectorXd fake_asc = Eigen::VectorXd::Zero(size);
                fake_asc = solver.computeCharge(fake_mep);
                int nr_irrep = cavity.pointGroup().nrIrrep();
                double totalFakeASC = fake_asc.sum() * nr_irrep;
                CAPTURE(totalASC - totalFakeASC);
                REQUIRE(totalASC == Approx(totalFakeASC).epsilon(1.0e-03));
            }
        }

        /*! \class IEFSolver
         *  \test \b pointChargeGePolC2v tests IEFSolver using a point charge with a GePol cavity in C2v symmetry
         */
        WHEN("the point group is C2v")
        {
            Molecule point = dummy<5>(2.929075493);
            double area = 0.4;
            double probeRadius = 0.0;
            double minRadius = 100.0;
            GePolCavity cavity(point, area, probeRadius, minRadius, "C2v");

            size_t size = cavity.size();
            Eigen::VectorXd fake_mep = computeMEP(cavity.elements(), charge);
            for (size_t i = 0; i < size; ++i) {
                Eigen::Vector3d center = cavity.elementCenter(i);
                double distance = center.norm();
                fake_mep(i) = charge / distance;
            }

            IEFSolver solver(symm);
            solver.buildSystemMatrix(cavity, gfInside, gfOutside);
            THEN("the total apparent surface charge is")
            {
                size_t irr_size = cavity.irreducible_size();
                Eigen::VectorXd fake_asc = Eigen::VectorXd::Zero(size);
                fake_asc = solver.computeCharge(fake_mep);
                int nr_irrep = cavity.pointGroup().nrIrrep();
                double totalFakeASC = fake_asc.sum() * nr_irrep;
                CAPTURE(totalASC - totalFakeASC);
                REQUIRE(totalASC == Approx(totalFakeASC).epsilon(1.0e-03));
            }
        }

        /*! \class IEFSolver
         *  \test \b pointChargeGePolC2h tests IEFSolver using a point charge with a GePol cavity in C2h symmetry
         */
        WHEN("the point group is C2h")
        {
            Molecule point = dummy<6>(2.929075493);
            double area = 0.4;
            double probeRadius = 0.0;
            double minRadius = 100.0;
            GePolCavity cavity(point, area, probeRadius, minRadius, "C2h");

            size_t size = cavity.size();
            Eigen::VectorXd fake_mep = computeMEP(cavity.elements(), charge);
            for (size_t i = 0; i < size; ++i) {
                Eigen::Vector3d center = cavity.elementCenter(i);
                double distance = center.norm();
                fake_mep(i) = charge / distance;
            }

            IEFSolver solver(symm);
            solver.buildSystemMatrix(cavity, gfInside, gfOutside);
            THEN("the total apparent surface charge is")
            {
                size_t irr_size = cavity.irreducible_size();
                Eigen::VectorXd fake_asc = Eigen::VectorXd::Zero(size);
                fake_asc = solver.computeCharge(fake_mep);
                int nr_irrep = cavity.pointGroup().nrIrrep();
                double totalFakeASC = fake_asc.sum() * nr_irrep;
                CAPTURE(totalASC - totalFakeASC);
                REQUIRE(totalASC == Approx(totalFakeASC).epsilon(1.0e-03));
            }
        }

        /*! \class IEFSolver
         *  \test \b pointChargeGePolD2h tests IEFSolver using a point charge with a GePol cavity in D2h symmetry
         */
        WHEN("the point group is D2h")
        {
            Molecule point = dummy<7>(2.929075493);
            double area = 0.4;
            double probeRadius = 0.0;
            double minRadius = 100.0;
            GePolCavity cavity(point, area, probeRadius, minRadius, "D2h");

            size_t size = cavity.size();
            Eigen::VectorXd fake_mep = computeMEP(cavity.elements(), charge);
            for (size_t i = 0; i < size; ++i) {
                Eigen::Vector3d center = cavity.elementCenter(i);
                double distance = center.norm();
                fake_mep(i) = charge / distance;
            }

            IEFSolver solver(symm);
            solver.buildSystemMatrix(cavity, gfInside, gfOutside);
            THEN("the total apparent surface charge is")
            {
                size_t irr_size = cavity.irreducible_size();
                Eigen::VectorXd fake_asc = Eigen::VectorXd::Zero(size);
                fake_asc = solver.computeCharge(fake_mep);
                int nr_irrep = cavity.pointGroup().nrIrrep();
                double totalFakeASC = fake_asc.sum() * nr_irrep;
                CAPTURE(totalASC - totalFakeASC);
                REQUIRE(totalASC == Approx(totalFakeASC).epsilon(1.0e-03));
            }
        }
    }
}
