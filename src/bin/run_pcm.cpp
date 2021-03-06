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

#include <cstdlib>
#include <cmath>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Config.hpp"

#include <Eigen/Core>

// Include Boost headers here
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "Cavity.hpp"
#include "RegisterCavityToFactory.hpp"
#include "IGreensFunction.hpp"
#include "RegisterGreensFunctionToFactory.hpp"
#include "Input.hpp"
#include "Molecule.hpp"
#include "PCMSolver.hpp"
#include "RegisterSolverToFactory.hpp"
#include "Atom.hpp"
#include "Citation.hpp"
#include "cnpy.hpp"
#include "PhysicalConstants.hpp"
#include "Solvent.hpp"
#include "Sphere.hpp"

//#include "BuildInfo.hpp"
extern "C"
void host_writer(const char * message, size_t message_length);

int main(int argc, char * argv[])
{
    // Open output file pcmsolver.out
    std::ofstream out_stream;
    out_stream.open("pcmsolver.out");

    if (argc > 2) PCMSOLVER_ERROR("Too many arguments supplied to run_pcm");
    Input parsed(argv[1]);
    parsed.initMolecule();

    // Create cavity
    Cavity * cavity = Factory<Cavity, cavityData>::TheFactory().create(parsed.cavityType(), parsed.cavityParams());
    // Create Green's functions
    // INSIDE
    IGreensFunction * gf_i = Factory<IGreensFunction, greenData>::TheFactory().create(parsed.greenInsideType(),
		                                              parsed.insideGreenParams());
    // OUTSIDE
    IGreensFunction * gf_o = Factory<IGreensFunction, greenData>::TheFactory().create(parsed.greenOutsideType(),
		                                              parsed.outsideStaticGreenParams());
    PCMSolver * solver = Factory<PCMSolver, solverData>::TheFactory().create(parsed.solverType(), parsed.solverParams());
    solver->buildSystemMatrix(*cavity, *gf_i, *gf_o);
    // Always save the cavity in a cavity.npz binary file
    // Cavity should be saved to file in initCavity(), due to the dependencies of
    // the WaveletCavity on the wavelet solvers it has to be done here...
    cavity->saveCavity();

    // Printout relevant info
    out_stream << "~~~~~~~~~~ PCMSolver ~~~~~~~~~~" << std::endl;
    // How the executable was built
    //out_stream << buildInfo() << std::endl;
    // How the calculation was set up
    out_stream << "Using CODATA " << parsed.CODATAyear() << " set of constants." << std::endl;
    out_stream << "Input parsing done " << parsed.providedBy() << std::endl;
    out_stream << "========== Cavity " << std::endl;
    out_stream << *cavity << std::endl;
    out_stream << "========== Solver " << std::endl;
    out_stream << *solver << std::endl;
    out_stream << "============ Medium " << std::endl;
    if (parsed.fromSolvent()) {
        out_stream << "Medium initialized from solvent built-in data." << std::endl;
        Solvent solvent = parsed.solvent();
        out_stream << solvent << std::endl;
    }
    out_stream << ".... Inside " << std::endl;
    out_stream << *gf_i << std::endl;
    out_stream << ".... Outside " << std::endl;
    out_stream << *gf_o;
    out_stream << parsed.molecule() << std::endl;

    // Form vector with electrostatic potential
    Eigen::VectorXd mep = computeMEP(parsed.molecule(), cavity->elements());
    // Compute apparent surface charge
    Eigen::VectorXd asc = solver->computeCharge(mep);
    // Compute energy and print it out
    out_stream << "Solvation energy = " << 0.5 * (asc * mep) << std::endl;
    out_stream << "DONE!" << std::endl;

    out_stream.close();
    // Rename output file

    // Clean-up
    delete cavity;
    delete gf_i;
    delete gf_o;
    delete solver;

    // Write timings out
    TIMER_DONE("pcmsolver.timer.dat");

    return EXIT_SUCCESS;
}

void host_writer(const char * /* message */, size_t /* message_length */) {}

