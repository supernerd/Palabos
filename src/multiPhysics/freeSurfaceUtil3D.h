/* This file is part of the Palabos library.
 *
 * Copyright (C) 2011-2015 FlowKit Sarl
 * Route d'Oron 2
 * 1010 Lausanne, Switzerland
 * E-mail contact: contact@flowkit.com
 *
 * The most recent release of Palabos can be downloaded at 
 * <http://www.palabos.org/>
 *
 * The library Palabos is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * The library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FREE_SURFACE_UTIL_3D_H
#define FREE_SURFACE_UTIL_3D_H

#include "core/globalDefs.h"
#include "core/plbDebug.h"
#include "multiBlock/multiContainerBlock3D.h"
#include "multiBlock/multiDataField3D.h"
#include "multiBlock/multiBlockLattice3D.h"

#include <vector>
#include <set>
#include <string>

namespace plb {

/// Constants used in a free surface flag matrix for cell tagging.
namespace twoPhaseFlag {
    enum Flag {empty=0, interface=1, fluid=2, wall=4, protect=5, protectEmpty=6};
    inline std::string flagToString(int flag) {
        switch(flag) {
            case empty:     return "empty";
            case interface: return "interface";
            case fluid:     return "fluid";
            case wall:      return "wall";
            case protect:   return "protect";
            case protectEmpty:   return "protectEmpty";
            default: PLB_ASSERT( false );
        }
        return std::string();
    }
    inline Flag invert(int flag) {
        switch(flag) {
            case empty:     return fluid;
            case interface: return interface;
            case fluid:     return empty;
            case wall:      return wall;
            case protect:   return protect;
            case protectEmpty: return protectEmpty;
            default: PLB_ASSERT( false );
        }
        return (Flag) (-1);
    }
    inline bool isWet(int flag) {
        return flag==interface || flag==fluid || flag==protect; 
    }
    inline bool isFullWet(int flag) {
        return flag==fluid || flag==protect; 
    }
    inline bool isEmpty(int flag) {
        return flag==empty || flag==protectEmpty; 
    }
}

/// Create a parameter-list for most free-surface data processors.
template< typename T,template<typename U> class Descriptor>
std::vector<MultiBlock3D*> aggregateFreeSurfaceParams (
        MultiBlockLattice3D<T,Descriptor>& fluid, MultiScalarField3D<T>& rhoBar,
        MultiTensorField3D<T,3>& j, MultiScalarField3D<T>& mass,
        MultiScalarField3D<T>& volumeFraction, MultiScalarField3D<int>& flag,
        MultiTensorField3D<T,3>& normal,
        MultiContainerBlock3D& interfaceLists, MultiScalarField3D<T>& curvature,
        MultiScalarField3D<T>& outsideDensity )
{
    std::vector<MultiBlock3D*> aggregation;

    aggregation.push_back(&fluid);
    aggregation.push_back(&rhoBar);
    aggregation.push_back(&j);
    aggregation.push_back(&mass);
    aggregation.push_back(&volumeFraction);
    aggregation.push_back(&flag);
    aggregation.push_back(&normal);
    aggregation.push_back(&interfaceLists);
    aggregation.push_back(&curvature);
    aggregation.push_back(&outsideDensity);

    return aggregation;
}

/// Data structure for holding lists of cells along the free surface in an AtomicContainerBlock.
template< typename T,template<typename U> class Descriptor>
struct InterfaceLists : public ContainerBlockData {
    typedef Array<plint,Descriptor<T>::d> Node;
    /// Holds all nodes which have excess mass from interface->fluid conversion.
    std::map<Node,T> filledMassExcess;
    /// Holds all nodes which have excess mass from interface->empty conversion.
    std::map<Node,T> emptiedMassExcess;
    /// Holds all nodes that need to change status from interface to fluid.
    std::set<Node>   interfaceToFluid;
    /// Holds all nodes that need to change status from interface to empty.
    std::set<Node>   interfaceToEmpty;
    /// Holds all nodes that need to change status from empty to interface.
    std::set<Node>   emptyToInterface;

    virtual InterfaceLists<T,Descriptor>* clone() const {
        return new InterfaceLists<T,Descriptor>(*this);
    }
};

/// A wrapper offering convenient access to the free-surface data provided to
/// data processors. Avoids verbous casting, asserting, etc.
template<typename T,template<typename U> class Descriptor>
class FreeSurfaceProcessorParam3D {
public:
    typedef typename InterfaceLists<T,Descriptor>::Node Node;
    FreeSurfaceProcessorParam3D(std::vector<AtomicBlock3D*>& atomicBlocks)
    {
        PLB_ASSERT(atomicBlocks.size() >= 10); 

        fluid_ = dynamic_cast<BlockLattice3D<T,Descriptor>*>(atomicBlocks[0]);
        PLB_ASSERT(fluid_);

        rhoBar_ = dynamic_cast<ScalarField3D<T>*>(atomicBlocks[1]);
        PLB_ASSERT(rhoBar_);

        j_ = dynamic_cast<TensorField3D<T,3>*>(atomicBlocks[2]);
        PLB_ASSERT(j_);

        mass_ = dynamic_cast<ScalarField3D<T>*>(atomicBlocks[3]);               
        PLB_ASSERT(mass_);

        volumeFraction_ = dynamic_cast<ScalarField3D<T>*>(atomicBlocks[4]);
        PLB_ASSERT(volumeFraction_);

        flag_ = dynamic_cast<ScalarField3D<int>*>(atomicBlocks[5]);
        PLB_ASSERT(flag_);

        normal_ = dynamic_cast<TensorField3D<T,3>*>(atomicBlocks[6]);
        PLB_ASSERT(normal_);

        containerInterfaceLists_ = dynamic_cast<AtomicContainerBlock3D*>(atomicBlocks[7]);
        PLB_ASSERT(containerInterfaceLists_);

        interfaceLists_ = dynamic_cast<InterfaceLists<T,Descriptor>*>(containerInterfaceLists_->getData());
        //PLB_ASSERT(interfaceLists_);
        //Put the assertion at the usage of interfaceLists, so we can still work with both freeSurfaceProcessorParam and twoPhaseProcessorParam.
                

        curvature_ = dynamic_cast<ScalarField3D<T>*>(atomicBlocks[8]);
        PLB_ASSERT(curvature_);

        outsideDensity_ = dynamic_cast<ScalarField3D<T>*>(atomicBlocks[9]);
        PLB_ASSERT(outsideDensity_);

        absoluteOffset       = fluid_->getLocation();
        relativeOffsetRhoBar = computeRelativeDisplacement(*fluid_,*rhoBar_);      
        relativeOffsetJ      = computeRelativeDisplacement(*fluid_,*j_);      
        relativeOffsetMass   = computeRelativeDisplacement(*fluid_,*mass_);      
        relativeOffsetVF     = computeRelativeDisplacement(*fluid_,*volumeFraction_);      
        relativeOffsetFS     = computeRelativeDisplacement(*fluid_,*flag_); 
        relativeOffsetNormal = computeRelativeDisplacement(*fluid_,*normal_); 
        relativeOffsetC      = computeRelativeDisplacement(*fluid_,*curvature_); 
        relativeOffsetOD     = computeRelativeDisplacement(*fluid_,*outsideDensity_); 
    }
    Cell<T,Descriptor>& cell(plint iX, plint iY, plint iZ) { return fluid_->get(iX,iY,iZ); }
    T& mass(plint iX, plint iY, plint iZ) {
        return mass_->get(iX+relativeOffsetMass.x,iY+relativeOffsetMass.y,iZ+relativeOffsetMass.z);
    }
    T& volumeFraction(plint iX, plint iY, plint iZ) {
        return volumeFraction_->get(iX+relativeOffsetVF.x,iY+relativeOffsetVF.y,iZ+relativeOffsetVF.z);
    }
    T& curvature(plint iX, plint iY, plint iZ) {
        return curvature_->get(iX+relativeOffsetC.x,iY+relativeOffsetC.y,iZ+relativeOffsetC.z);
    }
    T& outsideDensity(plint iX, plint iY, plint iZ) {
        return outsideDensity_->get(iX+relativeOffsetOD.x,iY+relativeOffsetOD.y,iZ+relativeOffsetOD.z);
    }
    int& flag(plint iX, plint iY, plint iZ) {
        return flag_->get(iX+relativeOffsetFS.x,iY+relativeOffsetFS.y,iZ+relativeOffsetFS.z);
    }
    void setForce(plint iX, plint iY, plint iZ, Array<T,3> const& force) {
            force.to_cArray(cell(iX,iY,iZ).getExternal(forceOffset));
    }
    Array<T,3> getForce(plint iX, plint iY, plint iZ) {
        Array<T,3> force; force.from_cArray(cell(iX,iY,iZ).getExternal(forceOffset));
        return force;
    }
    void setMomentum(plint iX, plint iY, plint iZ, Array<T,3> const& momentum) {
        j_->get(iX+relativeOffsetJ.x,iY+relativeOffsetJ.y,iZ+relativeOffsetJ.z) = momentum;
    }
    Array<T,3> getMomentum(plint iX, plint iY, plint iZ) {
        return j_->get(iX+relativeOffsetJ.x,iY+relativeOffsetJ.y,iZ+relativeOffsetJ.z);
    }
    T getDensity(plint iX, plint iY, plint iZ) {
        return Descriptor<T>::fullRho (
                    rhoBar_->get(iX+relativeOffsetRhoBar.x, iY+relativeOffsetRhoBar.y, iZ+relativeOffsetRhoBar.z) );
    }
    void setDensity(plint iX, plint iY, plint iZ, T rho) {
        rhoBar_->get(iX+relativeOffsetRhoBar.x, iY+relativeOffsetRhoBar.y, iZ+relativeOffsetRhoBar.z)
            = Descriptor<T>::rhoBar(rho);
    }
    void setNormal(plint iX, plint iY, plint iZ, Array<T,3> const& normal) {
        normal_->get(iX+relativeOffsetNormal.x,iY+relativeOffsetNormal.y,iZ+relativeOffsetNormal.z) = normal;
    }
    Array<T,3> getNormal(plint iX, plint iY, plint iZ) {
        return normal_->get(iX+relativeOffsetNormal.x,iY+relativeOffsetNormal.y,iZ+relativeOffsetNormal.z);
    }

    void attributeDynamics(plint iX, plint iY, plint iZ, Dynamics<T,Descriptor>* dynamics) {
        fluid_->attributeDynamics(iX,iY,iZ, dynamics);
    }

    bool isBoundary(plint iX, plint iY, plint iZ) {
        return cell(iX, iY, iZ).getDynamics().isBoundary();
    }

    void addToTotalMass(T addedTotalMass) {
        fluid_->getInternalStatistics().gatherSum(0, addedTotalMass);
    }
    void addToLostMass(T addedLostMass) {
        fluid_->getInternalStatistics().gatherSum(1, addedLostMass);
    }
    void addToInterfaceCells(plint addedInterfaceCells) {
        fluid_->getInternalStatistics().gatherIntSum(0, addedInterfaceCells);
    }
    T getSumMassMatrix() const {
        return fluid_->getInternalStatistics().getSum(0);
    }
    T getSumLostMass() const {
        return fluid_->getInternalStatistics().getSum(1);
    }
    T getTotalMass() const {
        return getSumMassMatrix() + getSumLostMass();
    }
    plint getNumInterfaceCells() const {
        return fluid_->getInternalStatistics().getIntSum(0);
    }

    T smoothVolumeFraction(plint iX, plint iY, plint iZ)
    {
        using namespace twoPhaseFlag;

        if (flag_->get(iX+relativeOffsetFS.x,iY+relativeOffsetFS.y,iZ+relativeOffsetFS.z) == wall) {
            return volumeFraction_->get(iX+relativeOffsetVF.x,iY+relativeOffsetVF.y,iZ+relativeOffsetVF.z);
        }

        T val = 0.0;
        int n = 0;
        for (int i = -1; i < 2; i++) {
            plint nextX = iX + i;
            for (int j = -1; j < 2; j++) {
                plint nextY = iY + j;
                for (int k = -1; k < 2; k++) {
                    plint nextZ = iZ + k;
                    if (!(i == 0 && j == 0 && k == 0) &&
                            flag_->get(nextX+relativeOffsetFS.x,nextY+relativeOffsetFS.y,nextZ+relativeOffsetFS.z) != wall) {
                        n++;
                        val += volumeFraction_->get(nextX+relativeOffsetVF.x,nextY+relativeOffsetVF.y,nextZ+relativeOffsetVF.z);
                    }
                }
            }
        }
        if (n != 0) {
            val /= (T) n;
        } else {
            val = volumeFraction_->get(iX+relativeOffsetVF.x,iY+relativeOffsetVF.y,iZ+relativeOffsetVF.z);
        }

        return val;
    }

    std::map<Node,T>& filledMassExcess() { PLB_ASSERT(interfaceLists_); return interfaceLists_ -> filledMassExcess; }
    std::map<Node,T>& emptiedMassExcess() { PLB_ASSERT(interfaceLists_); return interfaceLists_ -> emptiedMassExcess; }
    std::set<Node>& interfaceToFluid() { PLB_ASSERT(interfaceLists_); return interfaceLists_ -> interfaceToFluid; }
    std::set<Node>& interfaceToEmpty() { PLB_ASSERT(interfaceLists_); return interfaceLists_ -> interfaceToEmpty; }
    std::set<Node>& emptyToInterface() { PLB_ASSERT(interfaceLists_); return interfaceLists_ -> emptyToInterface; }

    Dot3D const& absOffset() const { return absoluteOffset; }
    Box3D getBoundingBox() const { return volumeFraction_->getBoundingBox(); }
private:
    BlockLattice3D<T,Descriptor>* fluid_;
    ScalarField3D<T>* rhoBar_;
    TensorField3D<T,3>* j_;
    ScalarField3D<T>* mass_;
    ScalarField3D<T>* volumeFraction_;
    ScalarField3D<int>* flag_;
    TensorField3D<T,3>* normal_;
    AtomicContainerBlock3D* containerInterfaceLists_;
    InterfaceLists<T,Descriptor>* interfaceLists_;
    ScalarField3D<T>* curvature_;
    ScalarField3D<T>* outsideDensity_;

    Dot3D absoluteOffset, relativeOffsetRhoBar, relativeOffsetJ, relativeOffsetMass,
          relativeOffsetVF, relativeOffsetFS, relativeOffsetNormal, relativeOffsetC,
          relativeOffsetOD;

    static const int forceOffset = Descriptor<T>::ExternalField::forceBeginsAt;
};

}  // namespace plb

#endif  // FREE_SURFACE_UTIL_3D_H

