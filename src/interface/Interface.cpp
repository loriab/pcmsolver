/* pcmsolver_copyright_start */
/*
 *     PCMSolver, an API for the Polarizable Continuum Model
 *     Copyright (C) 2013 Roberto Di Remigio, Luca Frediani and contributors
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
 *     PCMSolver API, see: <http://pcmsolver.github.io/pcmsolver-doc>
 */
/* pcmsolver_copyright_end */

/*

  Interface functions implementation

*/
#include "Interface.hpp"

#include <cmath>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Config.hpp"

#include <Eigen/Dense>

// Include Boost headers here
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

// Core classes
// This list all header files that need to be included here.
// It is automatically generated by CMake during configuration
#include "Includer.hpp"

typedef boost::shared_ptr<SurfaceFunction> SharedSurfaceFunction;
typedef std::map<std::string, SharedSurfaceFunction > SharedSurfaceFunctionMap;
typedef std::pair<std::string, SharedSurfaceFunction > SharedSurfaceFunctionPair;

// We need globals as they must be accessible across all the functions defined in this interface...
// The final objective is to have only a pointer to Cavity and a pointer to PCMSolver (our abstractions)
// then maybe manage them through "objectification" of this interface.
Cavity        * _cavity = NULL;
#if defined (DEVELOPMENT_CODE)
WaveletCavity * _waveletCavity = NULL;

PWCSolver * _PWCSolver = NULL;
PWCSolver * _noneqPWCSolver = NULL;
PWLSolver * _PWLSolver = NULL;
PWLSolver * _noneqPWLSolver = NULL;
#endif
PCMSolver * _solver = NULL;
PCMSolver * _noneqSolver = NULL;
bool noneqExists = false;

SharedSurfaceFunctionMap functions;
boost::shared_ptr<Input> parsedInput;
std::vector<std::string> input_strings; // Used only by Fortran hosts

/*

	Functions visible to host program

*/

extern "C" void hello_pcm(int * a, double * b)
{
    std::ostringstream out_stream;
    out_stream << "Hello, PCM!" << std::endl;
    out_stream << "The integer is: " << *a << std::endl;
    out_stream << "The double is: " << *b << std::endl;
    printer(out_stream);
}

extern "C" void set_up_pcm(int * host_provides_input)
{
    bool from_host = false;
    if (*host_provides_input != 0) {
	    from_host = true;
    }
    setupInput(from_host);
    initCavity();
    initSolver();
}

extern "C" void tear_down_pcm()
{
    // Delete all the global pointers, maybe in a more refined way...

    functions.clear();

    safe_delete(_cavity);
    safe_delete(_solver);
}

extern "C" void compute_asc(char * potString, char * chgString, int * irrep)
{
    std::string potFuncName(potString);
    std::string chgFuncName(chgString);

    // Get the proper iterators
    SharedSurfaceFunctionMap::const_iterator iter_pot = functions.find(potFuncName);
    // Here we check whether the function exists already or not
    // 1. find the lower bound of the map
    SharedSurfaceFunctionMap::iterator iter_chg = functions.lower_bound(chgFuncName);
    // 2. if iter_chg == end, or if iter_chg is not a match,
    //    then this element was not in the map, so we need to insert it
    if ( iter_chg == functions.end()  ||  iter_chg->first != chgFuncName ) {
        // move iter_chg to the element preceeding the insertion point
        if ( iter_chg != functions.begin() ) --iter_chg;
        // insert it
	SharedSurfaceFunction func( new SurfaceFunction(chgFuncName,
                                          _cavity->size()) );
        SharedSurfaceFunctionPair insertion = SharedSurfaceFunctionMap::value_type(chgFuncName, func);
        iter_chg = functions.insert(iter_chg, insertion);
    }

    // If it already exists, we will pass a reference to its values to
    // _solver->computeCharge(const Eigen::VectorXd &, Eigen::VectorXd &) so they will be automagically updated!
    // We clear the ASC surface function. Needed when using symmetry for response calculations
    iter_chg->second->clear();
    _solver->computeCharge(iter_pot->second->vector(), iter_chg->second->vector(), *irrep);
    // Renormalization of charges: divide by the number of symmetry operations in the group
    (*iter_chg->second) /= double(_cavity->pointGroup().nrIrrep());
}

extern "C" void compute_nonequilibrium_asc(char * potString, char * chgString, int * irrep)
{
    // Check that the nonequilibrium solver has been created
    if (!noneqExists) {
	initNonEqSolver();
    }

    std::string potFuncName(potString);
    std::string chgFuncName(chgString);

    // Get the proper iterators
    SharedSurfaceFunctionMap::const_iterator iter_pot = functions.find(potFuncName);
    // Here we check whether the function exists already or not
    // 1. find the lower bound of the map
    SharedSurfaceFunctionMap::iterator iter_chg = functions.lower_bound(chgFuncName);
    // 2. if iter_chg == end, or if iter_chg is not a match,
    //    then this element was not in the map, so we need to insert it
    if ( iter_chg == functions.end()  ||  iter_chg->first != chgFuncName ) {
        // move iter_chg to the element preceeding the insertion point
        if ( iter_chg != functions.begin() ) --iter_chg;
        // insert it
	SharedSurfaceFunction func( new SurfaceFunction(chgFuncName,
                                          _cavity->size()) );
        SharedSurfaceFunctionPair insertion = SharedSurfaceFunctionMap::value_type(chgFuncName, func);
        iter_chg = functions.insert(iter_chg, insertion);
    }

    // If it already exists there's no problem, we will pass a reference to its values to
    _solver->computeCharge(iter_pot->second->vector(), iter_chg->second->vector(), *irrep);
    // Renormalization of charges: divide by the number of symmetry operations in the group
    (*iter_chg->second) /= double(_cavity->pointGroup().nrIrrep());
}

extern "C" void compute_polarization_energy(double * energy)
{
    // Check if NucMEP && EleASC surface functions exist.
    bool is_separate = (surfaceFunctionExists("NucMEP")
                        && surfaceFunctionExists("EleASC"));

    if (is_separate) {
        // Using separate potentials and charges
        SharedSurfaceFunctionMap::const_iterator iter_nuc_pot = functions.find("NucMEP");
        SharedSurfaceFunctionMap::const_iterator iter_nuc_chg = functions.find("NucASC");
        SharedSurfaceFunctionMap::const_iterator iter_ele_pot = functions.find("EleMEP");
        SharedSurfaceFunctionMap::const_iterator iter_ele_chg = functions.find("EleASC");

        double UNN = (*iter_nuc_pot->second) *  (*iter_nuc_chg->second);
        double UEN = (*iter_ele_pot->second) *  (*iter_nuc_chg->second);
        double UNE = (*iter_nuc_pot->second) *  (*iter_ele_chg->second);
        double UEE = (*iter_ele_pot->second) *  (*iter_ele_chg->second);

        std::ostringstream out_stream;
        out_stream << "Polarization energy components" << std::endl;
        out_stream << "   U_ee = " << boost::format("%20.14f") % UEE;
        out_stream << " , U_en = " << boost::format("%20.14f") % UEN;
        out_stream << " , U_ne = " << boost::format("%20.14f") % UNE;
        out_stream << " , U_nn = " << boost::format("%20.14f\n") % UNN;
        printer(out_stream);

        *energy = 0.5 * ( UNN + UEN + UNE + UEE );
    } else {
        SharedSurfaceFunctionMap::const_iterator iter_pot = functions.find("TotMEP");
        SharedSurfaceFunctionMap::const_iterator iter_chg = functions.find("TotASC");

        *energy = 0.5 * (*iter_pot->second) * (*iter_chg->second);
    }
}

extern "C" void save_surface_functions()
{
    printer("\nDumping surface functions to .npy files");
    SharedSurfaceFunctionPair pair;
    BOOST_FOREACH(pair, functions) {
        unsigned int dim = static_cast<unsigned int>(pair.second->nPoints());
        const unsigned int shape[] = {dim};
        std::string fname = pair.second->name() + ".npy";
        cnpy::npy_save(fname, pair.second->vector().data(), shape, 1, "w", true);
    }
}

extern "C" void save_surface_function(const char * name)
{
    typedef SharedSurfaceFunctionMap::const_iterator surfMap_iter;
    std::string functionName(name);
    std::string fname = functionName + ".npy";

    surfMap_iter it = functions.find(functionName);
    unsigned int dim = static_cast<unsigned int>(it->second->nPoints());
    const unsigned int shape[] = {dim};
    cnpy::npy_save(fname, it->second->vector().data(), shape, 1, "w", true);
}

extern "C" void load_surface_function(const char * name)
{
    std::string functionName(name);
    printer("\nLoading surface function " + functionName + " from .npy file");
    std::string fname = functionName + ".npy";
    cnpy::NpyArray raw_surfFunc = cnpy::npy_load(fname);
    int dim = raw_surfFunc.shape[0];
    if (dim != _cavity->size()) { 
        throw std::runtime_error("Inconsistent dimension of loaded surface function!");
    } else {
	Eigen::VectorXd values = getFromRawBuffer<double>(dim, 1, raw_surfFunc.data); 
        SharedSurfaceFunction func( new SurfaceFunction(functionName, dim, values) );
	// Append to global map
        SharedSurfaceFunctionMap::iterator iter = functions.lower_bound(functionName);
        if ( iter == functions.end()  ||  iter->first != functionName ) {                                   
            if ( iter != functions.begin() ) --iter;
            SharedSurfaceFunctionPair insertion = SharedSurfaceFunctionMap::value_type(functionName, func);
            iter = functions.insert(iter, insertion);
        }
   }
}

extern "C" void dot_surface_functions(double * result, const char * potString,
                                      const char * chgString)
{
    // Convert C-style strings to std::string
    std::string potFuncName(potString);
    std::string chgFuncName(chgString);

    // Setup iterators
    SharedSurfaceFunctionMap::const_iterator iter_pot = functions.find(potFuncName);
    SharedSurfaceFunctionMap::const_iterator iter_chg = functions.find(chgFuncName);

    if ( iter_pot == functions.end()  ||  iter_chg == functions.end() ) {
        throw std::runtime_error("One or both of the SurfaceFunction specified is non-existent.");
    } else {
        // Calculate the dot product
        *result = (*iter_pot->second) * (*iter_chg->second);
    }
}

extern "C" void get_cavity_size(int * nts, int * ntsirr)
{
    *nts    = _cavity->size();
    *ntsirr = _cavity->irreducible_size();
}

extern "C" void get_tesserae(double * centers)
{
    // Use some Eigen magic
    for ( int i = 0; i < (3 * _cavity->size()); ++i) {
        centers[i] = *(_cavity->elementCenter().data() + i);
    }
}

extern "C" void get_tesserae_centers(int * its, double * center)
{
    Eigen::Vector3d tess = _cavity->elementCenter(*its-1);
    center[0] = tess(0);
    center[1] = tess(1);
    center[2] = tess(2);
}

extern "C" void print_citation()
{
    printer(citation_message());
}

extern "C" void print_pcm()
{
    // I don't think this will work with wavelets as of now (8/7/13)
    // we should work towards this though: "Program to an interface, not an implementation."
    // Initialize a stream
    std::ostringstream out_stream;
    out_stream << "\n" << std::endl;
    out_stream << "~~~~~~~~~~ PCMSolver ~~~~~~~~~~" << std::endl;
    out_stream << "Using CODATA " << parsedInput->CODATAyear() << " set of constants." << std::endl;
    out_stream << "Input parsing done " << parsedInput->providedBy() << std::endl;
    out_stream << "========== Cavity " << std::endl;
    out_stream << *_cavity << std::endl;
    out_stream << "========== Solver " << std::endl;
    out_stream << *_solver << std::endl;
    out_stream << "============ Medium " << std::endl;
    bool fromSolvent = parsedInput->fromSolvent();
    if (fromSolvent) {
        out_stream << "Medium initialized from solvent built-in data." << std::endl;
        Solvent solvent = parsedInput->solvent();
        out_stream << solvent << std::endl;
    }
    out_stream << ".... Inside " << std::endl;
    out_stream << *(_solver->greenInside()) << std::endl;
    out_stream << ".... Outside " << std::endl;
    out_stream << *(_solver->greenOutside()) << std::endl;
    printer(out_stream);
}

extern "C" void set_surface_function(int * nts, double * values, char * name)
{
    int nTess = _cavity->size();
    if ( nTess != *nts )
        throw std::runtime_error("You are trying to allocate a SurfaceFunction bigger than the cavity!");

    std::string functionName(name);
    // Here we check whether the function exists already or not
    // 1. find the lower bound of the map
    SharedSurfaceFunctionMap::iterator iter = functions.lower_bound(functionName);
    // 2. if iter == end, or if iter is not a match,
    //    then this element was not in the map, so we need to insert it
    if ( iter == functions.end()  ||  iter->first != functionName ) {
        // move iter to the element preceeding the insertion point
        if ( iter != functions.begin() ) --iter;
        // insert it
        SharedSurfaceFunction func( new SurfaceFunction(functionName, *nts, values) );
        SharedSurfaceFunctionPair insertion = SharedSurfaceFunctionMap::value_type(functionName, func);
        iter = functions.insert(iter, insertion);
    } else {
        iter->second->setValues(values);
    }
}

extern "C" void get_surface_function(int * nts, double * values, char * name)
{
    int nTess = _cavity->size();
    if ( nTess != *nts )
        throw std::runtime_error("You are trying to access a SurfaceFunction bigger than the cavity!");

    std::string functionName(name);

    SharedSurfaceFunctionMap::const_iterator iter = functions.find(functionName);
    if ( iter == functions.end() )
        throw std::runtime_error("You are trying to access a non-existing SurfaceFunction.");

    for ( int i = 0; i < nTess; ++i ) {
        values[i] = iter->second->value(i);
    }
}

extern "C" void add_surface_function(char * result, double * coeff, char * part)
{
    std::string resultName(result);
    std::string partName(part);

    append_surface_function(result);

    SharedSurfaceFunctionMap::const_iterator iter_part = functions.find(partName);
    SharedSurfaceFunctionMap::const_iterator iter_result = functions.find(resultName);

    // Using iterators and operator overloading: so neat!
    (*iter_result->second) += (*coeff) * (*iter_part->second);
}

extern "C" void print_surface_function(char * name)
{
    std::string functionName(name);

    SharedSurfaceFunctionMap::const_iterator iter = functions.find(name);
    std::ostringstream out_stream;
    out_stream << "\n" << std::endl;
    out_stream << *(iter->second) << std::endl;
    printer(out_stream);
}

extern "C" void clear_surface_function(char * name)
{
    std::string functionName(name);

    SharedSurfaceFunctionMap::const_iterator iter = functions.find(name);

    iter->second->clear();
}

extern "C" void append_surface_function(char * name)
{
    int nTess = _cavity->size();
    std::string functionName(name);

    // Here we check whether the function exists already or not
    // 1. find the lower bound of the map
    SharedSurfaceFunctionMap::iterator iter = functions.lower_bound(functionName);
    // 2. if iter == end, or if iter is not a match,
    //    then this element was not in the map, so we need to insert it
    if ( iter == functions.end()  ||  iter->first != functionName ) {
        // move iter to the element preceeding the insertion point
        if ( iter != functions.begin() ) --iter;
        // insert it
        SharedSurfaceFunction func( new SurfaceFunction(functionName, nTess) );
        SharedSurfaceFunctionPair insertion = SharedSurfaceFunctionMap::value_type(functionName, func);
        iter = functions.insert(iter, insertion);
    } else {
        // What happens if it is already in the map? The values need to be updated.
        // Nothing, I assume that if one calls append_surface_function will then also call
        // set_surface_function somewhere else, hence the update will be done there.
    }
}

extern "C" void scale_surface_function(char * func, double * coeff)
{
    std::string resultName(func);

    SharedSurfaceFunctionMap::const_iterator iter_func = functions.find(func);

    // Using iterators and operator overloading: so neat!
    (*iter_func->second) *= (*coeff);
}

extern "C" void push_input_string(char * s)
{
	std::string str(s);
	// Save the string inside a std::vector<std::string>
	input_strings.push_back(str);
}

/*

	Functions not visible to host program

*/

void setupInput(bool from_host)
{
    if (from_host) { // Set up input from host data structures
	    cavityInput cav;
	    cav.cleaner();
	    solverInput solv;
	    solv.cleaner();
	    greenInput green;
	    green.cleaner();
	    host_input(&cav, &solv, &green);
	    // Put string passed with the alternative method in the input structures
	    if (!input_strings.empty()) {
	    	for (size_t i = 0; i < input_strings.size(); ++i) {
		    // Trim strings aka remove blanks
		    boost::algorithm::trim(input_strings[i]);
	    	}
	    	strncpy(cav.cavity_type,    input_strings[0].c_str(), input_strings[0].length());
	        strncpy(cav.radii_set,      input_strings[1].c_str(), input_strings[1].length());
	    	strncpy(cav.restart_name,   input_strings[2].c_str(), input_strings[2].length());
	    	strncpy(solv.solver_type,   input_strings[3].c_str(), input_strings[3].length());
	    	strncpy(solv.solvent,       input_strings[4].c_str(), input_strings[4].length());
	    	strncpy(solv.equation_type, input_strings[5].c_str(), input_strings[5].length());
	    	strncpy(green.inside_type,  input_strings[6].c_str(), input_strings[6].length());
	    	strncpy(green.outside_type, input_strings[7].c_str(), input_strings[7].length());
	    }
    	    parsedInput = boost::make_shared<Input>(Input(cav, solv, green));
    } else {
	    parsedInput = boost::make_shared<Input>(Input("@pcmsolver.inp"));
    }
    std::string _mode = parsedInput->mode();
    // The only thing we can't create immediately is the molecule
    // from which the cavity is to be built.
    if (_mode != "EXPLICIT") {
       Molecule molec;
       initMolecule(molec);
       parsedInput->molecule(molec);
    }
}

void initCavity()
{
    // Get the right cavity from the Factory
    // TODO: since WaveletCavity extends cavity in a significant way, use of the Factory Method design pattern does not work for wavelet cavities. (8/7/13)
    std::string modelType = parsedInput->solverType();
#if defined (DEVELOPMENT_CODE)    
    if (modelType == "WAVELET" || modelType == "LINEAR") {
        // Both PWC and PWL require a WaveletCavity
        initWaveletCavity();
    } else {
        // This means in practice that the CavityFactory is now working only for GePol.
        _cavity = CavityFactory::TheCavityFactory().createCavity(parsedInput->cavityType(), parsedInput->cavityParams());
    }
#else    
    _cavity = CavityFactory::TheCavityFactory().createCavity(parsedInput->cavityType(), parsedInput->cavityParams());
#endif    
}

void initSolver()
{
    GreensFunctionFactory & factory = GreensFunctionFactory::TheGreensFunctionFactory();
    // Get the input data for generating the inside & outside Green's functions
    // INSIDE
    IGreensFunction * gfInside = factory.createGreensFunction(parsedInput->greenInsideType(), 
		                                              parsedInput->insideGreenParams());
    // OUTSIDE, reuse the variables holding the parameters for the Green's function inside.
    IGreensFunction * gfOutside = factory.createGreensFunction(parsedInput->greenOutsideType(), 
		                                               parsedInput->outsideStaticGreenParams());
    // And all this to finally create the solver!
    std::string modelType = parsedInput->solverType();
    solverData solverInput(gfInside, gfOutside, 
		           parsedInput->correction(),
			   parsedInput->equationType(), 
			   parsedInput->hermitivitize());

    // This thing is rather ugly I admit, but will be changed (as soon as wavelet PCM is working with DALTON)
    // it is needed because: 1. comment above on cavities; 2. wavelet cavity and solver depends on each other
    // (...not our fault, but should remedy somehow)
#if defined (DEVELOPMENT_CODE)    
    if (modelType == "WAVELET") {
        _PWCSolver = new PWCSolver(gfInside, gfOutside);
        _PWCSolver->buildSystemMatrix(*_waveletCavity);
        _waveletCavity->uploadPoints(_PWCSolver->getQuadratureLevel(), _PWCSolver->getT_(),
                                     false); // WTF is happening here???
        _cavity = _waveletCavity;
        _solver = _PWCSolver;
    } else if (modelType == "LINEAR") {
        _PWLSolver = new PWLSolver(gfInside, gfOutside);
        _PWLSolver->buildSystemMatrix(*_waveletCavity);
        _waveletCavity->uploadPoints(_PWLSolver->getQuadratureLevel(),_PWLSolver->getT_(),
                                     true); // WTF is happening here???
        _cavity = _waveletCavity;
        _solver = _PWLSolver;
    } else {
        // This means that the factory is properly working only for IEFSolver and CPCMSolver
        _solver = SolverFactory::TheSolverFactory().createSolver(modelType, solverInput);
        _solver->buildSystemMatrix(*_cavity);
    }
#else
    _solver = SolverFactory::TheSolverFactory().createSolver(modelType, solverInput);
    _solver->buildSystemMatrix(*_cavity);
#endif    
    // Always save the cavity in a cavity.npz binary file
    // Cavity should be saved to file in initCavity(), due to the dependencies of
    // the WaveletCavity on the wavelet solvers it has to be done here...
    _cavity->saveCavity();
}

void initNonEqSolver()
{
    GreensFunctionFactory & factory = GreensFunctionFactory::TheGreensFunctionFactory();
    // Get the input data for generating the inside & outside Green's functions
    // INSIDE
    IGreensFunction * gfInside = factory.createGreensFunction(parsedInput->greenInsideType(), 
		                                              parsedInput->insideGreenParams());
    // OUTSIDE, reuse the variables holding the parameters for the Green's function inside.
    IGreensFunction * gfOutside = factory.createGreensFunction(parsedInput->greenOutsideType(), 
		                                               parsedInput->outsideDynamicGreenParams());
    // And all this to finally create the solver!
    std::string modelType = parsedInput->solverType();
    solverData solverInput(gfInside, gfOutside, 
		           parsedInput->correction(),
			   parsedInput->equationType(), 
			   parsedInput->hermitivitize());

    // This thing is rather ugly I admit, but will be changed (as soon as wavelet PCM is working with DALTON)
    // it is needed because: 1. comment above on cavities; 2. wavelet cavity and solver depends on each other
    // (...not our fault, but should remedy somehow)
#if defined (DEVELOPMENT_CODE)    
    if (modelType == "WAVELET") {
        _noneqPWCSolver = new PWCSolver(gfInside, gfOutside);
        _noneqPWCSolver->buildSystemMatrix(*_waveletCavity);
        _waveletCavity->uploadPoints(_noneqPWCSolver->getQuadratureLevel(), _noneqPWCSolver->getT_(),
                                     false); // WTF is happening here???
        _cavity = _waveletCavity;
        _noneqSolver = _noneqPWCSolver;
    } else if (modelType == "LINEAR") {
        _noneqPWLSolver = new PWLSolver(gfInside, gfOutside);
        _noneqPWLSolver->buildSystemMatrix(*_waveletCavity);
        _waveletCavity->uploadPoints(_noneqPWLSolver->getQuadratureLevel(), _noneqPWLSolver->getT_(),
                                     true); // WTF is happening here???
        _cavity = _waveletCavity;
        _noneqSolver = _noneqPWLSolver;
    } else {
        // This means that the factory is properly working only for IEFSolver and CPCMSolver
        _noneqSolver = SolverFactory::TheSolverFactory().createSolver(modelType, solverInput);
        _noneqSolver->buildSystemMatrix(*_cavity);
    }
#else
    _noneqSolver = SolverFactory::TheSolverFactory().createSolver(modelType, solverInput);
    _noneqSolver->buildSystemMatrix(*_cavity);
#endif    
    // Always save the cavity in a cavity.npz binary file
    // Cavity should be saved to file in initCavity(), due to the dependencies of
    // the WaveletCavity on the wavelet solvers it has to be done here...
    _cavity->saveCavity();
}

void initAtoms(Eigen::VectorXd & charges_, Eigen::Matrix3Xd & sphereCenter_)
{
    int nuclei;
    collect_nctot(&nuclei);
    sphereCenter_.resize(Eigen::NoChange, nuclei);
    charges_.resize(nuclei);
    double * chg = charges_.data();
    double * centers = sphereCenter_.data();
    collect_atoms(chg, centers);
}

void initMolecule(Molecule & molecule_)
{
    // Gather information necessary to build molecule_
    // 1. number of atomic centers
    int nuclei;
    collect_nctot(&nuclei);
    // 2. position and charges of atomic centers
    Eigen::Matrix3Xd centers; 
    centers.resize(Eigen::NoChange, nuclei);
    Eigen::VectorXd charges  = Eigen::VectorXd::Zero(nuclei);
    double * chg = charges.data();
    double * pos = centers.data();
    collect_atoms(chg, pos);
    // 3. list of atoms and list of spheres
    bool scaling = parsedInput->scaling();
    std::string set = parsedInput->radiiSet();
    double factor = angstromToBohr(parsedInput->CODATAyear());
    std::vector<Atom> radiiSet, atoms;
    if ( set == "UFF" ) {
        radiiSet = Atom::initUFF();
    } else {
        radiiSet = Atom::initBondi();
    }
    std::vector<Sphere> spheres;
    for (int i = 0; i < charges.size(); ++i) {
        int index = int(charges(i)) - 1;
	atoms.push_back(radiiSet[index]);
        double radius = radiiSet[index].atomRadius() * factor;
        if (scaling) {
            radius *= radiiSet[index].atomRadiusScaling();
        }
        spheres.push_back(Sphere(centers.col(i), radius));
    }
    // 4. masses
    Eigen::VectorXd masses = Eigen::VectorXd::Zero(nuclei);
    for (int i = 0; i < masses.size(); ++i) {
	 masses(i) = atoms[i].atomMass();
    }
    // Based on the creation mode (Implicit or Atoms)
    // the spheres list might need postprocessing
    std::string _mode = parsedInput->mode();
    if ( _mode == "ATOMS" ) {
       initSpheresAtoms(centers, spheres);	 
    }
    // 5. molecular point group
    int nr_gen;
    int gen1, gen2, gen3;
    set_point_group(&nr_gen, &gen1, &gen2, &gen3);
    Symmetry pg = buildGroup(nr_gen, gen1, gen2, gen3);

    // OK, now get molecule_
    molecule_ = Molecule(nuclei, charges, masses, centers, atoms, spheres, pg);
}

void initSpheresAtoms(const Eigen::Matrix3Xd & sphereCenter_,
                      std::vector<Sphere> & spheres_)
{
    vector<int> atomsInput = parsedInput->atoms();
    vector<double> radiiInput = parsedInput->radii();

    // Loop over the atomsInput array to get which atoms will have a user-given radius
    for (size_t i = 0; i < atomsInput.size(); ++i) {
        int index = atomsInput[i] - 1; // -1 to go from human readable to machine readable
        // Put the new Sphere in place of the implicit-generated one
        spheres_[index] = Sphere(sphereCenter_.col(index), radiiInput[i]);
    }
}

void initSpheresImplicit(const Eigen::VectorXd & charges_,
                         const Eigen::Matrix3Xd & sphereCenter_, std::vector<Sphere> & spheres_)
{
    bool scaling = parsedInput->scaling();
    std::string set = parsedInput->radiiSet();
    double factor = angstromToBohr(parsedInput->CODATAyear());

    std::vector<Atom> radiiSet;
    if ( set == "UFF" ) {
        radiiSet = Atom::initUFF();
    } else {
        radiiSet = Atom::initBondi();
    }

    for (int i = 0; i < charges_.size(); ++i) {
        int index = charges_(i) - 1;
        double radius = radiiSet[index].atomRadius() * factor;
        if (scaling) {
            radius *= radiiSet[index].atomRadiusScaling();
        }
        spheres_.push_back(Sphere(sphereCenter_.col(i), radius));
    }
}

#if defined (DEVELOPMENT_CODE)
void initWaveletCavity()
{
    // Just throw at this point if the user asked for a cavity for a single sphere...
    // the wavelet code will die without any further notice anyway
    if (spheres.size() == 1) {
        throw std::runtime_error("Wavelet cavity generator cannot manage a single sphere...");
    }

    _waveletCavity = new WaveletCavity(parsedInput->spheres(), 
		                       parsedInput->cavityParams().probeRadius_, 
				       parsedInput->cavityParams().patchLevel_, 
				       parsedInput->cavityParams().coarsity_);
    _waveletCavity->readCavity("molec_dyadic.dat");
}
#endif

bool surfaceFunctionExists(const std::string & name)
{
    SharedSurfaceFunctionMap::const_iterator iter = functions.find(name);

    return iter != functions.end();
}

template <typename T>
void safe_delete(T *& ptr)
{
    delete ptr;
    ptr = NULL;
}

inline void printer(const std::string & message)
{
    // Extract C-style string from C++-style string and get its length
    const char * message_C = message.c_str();
    size_t message_length = strlen(message_C);
    // Call the host_writer
    host_writer(message_C, &message_length);
}

inline void printer(std::ostringstream & stream)
{
    // Extract C++-style string from stream
    std::string message = stream.str();
    // Extract C-style string from C++-style string and get its length
    const char * message_C = message.c_str();
    size_t message_length = strlen(message_C);
    // Call the host_writer
    host_writer(message_C, &message_length);
}
