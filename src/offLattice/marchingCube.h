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

#ifndef MARCHING_CUBE_H
#define MARCHING_CUBE_H

#include "core/globalDefs.h"
#include "offLattice/boundaryShapes3D.h"
#include "offLattice/triangleSet.h"
#include "offLattice/triangularSurfaceMesh.h"
#include "offLattice/offLatticeBoundaryProfiles3D.h"
#include "offLattice/triangleBoundary3D.h"
#include "latticeBoltzmann/geometricOperationTemplates.h"
#include "dataProcessors/dataAnalysisFunctional3D.hh"
#include <vector>
#include <numeric>

namespace plb {

//template<typename T>
//class IsoSurfaceDefinition3D;




template<typename T>
class IsoSurfaceDefinition3D{
public:
    virtual ~IsoSurfaceDefinition3D() { }
    virtual bool isInside (
            plint surfaceId, Array<plint,3> const& position ) const =0;
    virtual bool isValid(Array<plint,3> const& position) const { return true; }
    virtual Array<T,3> getSurfacePosition (
             plint surfaceId, Array<plint,3> const& p1, Array<plint,3> const& p2 ) const =0;
    virtual void setArguments(std::vector<AtomicBlock3D*> const& arguments) =0;
    virtual IsoSurfaceDefinition3D<T>* clone() const =0;
    virtual plint getNumArgs() const =0;
    virtual std::vector<plint> getSurfaceIds() const=0;
public:
    bool edgeIsValid(plint iX, plint iY, plint iZ, int edge) const{
		switch(edge) {
			case 0: {
				Array<plint,3> p0(iX  ,iY+1,iZ  );
				Array<plint,3> p1(iX+1,iY+1,iZ  );
				return this->isValid(p0) && this->isValid(p1); // x-edge of y-neighbor.
			}
			case 1: {
				Array<plint,3> p1(iX+1,iY+1,iZ  );
				Array<plint,3> p2(iX+1,iY  ,iZ  );
				return this->isValid(p1) && this->isValid(p2); // y-edge of x-neighbor.
			}
			case 2: {
				Array<plint,3> p2(iX+1,iY  ,iZ  );
				Array<plint,3> p3(iX  ,iY  ,iZ  );
				return this->isValid(p2) && this->isValid(p3); // x-edge of current cell.
			}
			case 3: {
				Array<plint,3> p3(iX  ,iY  ,iZ  );
				Array<plint,3> p0(iX  ,iY+1,iZ  );
				return this->isValid(p3) && this->isValid(p0); // y-edge of current cell.
			}
			case 4: {
				Array<plint,3> p4(iX  ,iY+1,iZ+1);
				Array<plint,3> p5(iX+1,iY+1,iZ+1);
				return this->isValid(p4) && this->isValid(p5); // x-edge of y-z-neighbor.
			}
			case 5: {
				Array<plint,3> p5(iX+1,iY+1,iZ+1);
				Array<plint,3> p6(iX+1,iY  ,iZ+1);
				return this->isValid(p5) && this->isValid(p6); // y-edge of x-z-neighbor.
			}
			case 6: {
				Array<plint,3> p6(iX+1,iY  ,iZ+1);
				Array<plint,3> p7(iX  ,iY  ,iZ+1);
				return this->isValid(p6) && this->isValid(p7); // x-edge of z-neighbor.
			}
			case 7: {
				Array<plint,3> p7(iX  ,iY  ,iZ+1);
				Array<plint,3> p4(iX  ,iY+1,iZ+1);
				return this->isValid(p7) && this->isValid(p4); // y-edge of z-neighbor.
			}
			case 8: {
				Array<plint,3> p0(iX  ,iY+1,iZ  );
				Array<plint,3> p4(iX  ,iY+1,iZ+1);
				return this->isValid(p0) && this->isValid(p4); // z-edge of y-neighbor.
			}
			case 9: {
				Array<plint,3> p1(iX+1,iY+1,iZ  );
				Array<plint,3> p5(iX+1,iY+1,iZ+1);
				return this->isValid(p1) && this->isValid(p5); // z-edge of x-y-neighbor.
			}
			case 10: {
				Array<plint,3> p2(iX+1,iY  ,iZ  );
				Array<plint,3> p6(iX+1,iY  ,iZ+1);
				return this->isValid(p2) && this->isValid(p6); // z-edge of x-neighbor.
			}
			case 11: {
				Array<plint,3> p3(iX  ,iY  ,iZ  );
				Array<plint,3> p7(iX  ,iY  ,iZ+1);
				return this->isValid(p3) && this->isValid(p7); // z-edge of current cell.
			}
			default:
				PLB_ASSERT(false);
				return false;
		}
	}
};

template<typename T>
class ScalarFieldIsoSurface3D : public IsoSurfaceDefinition3D<T> {
public:
    ScalarFieldIsoSurface3D(std::vector<T> const& isoValues_);
    virtual bool isInside (
            plint surfaceId, Array<plint,3> const& position ) const;
    virtual Array<T,3> getSurfacePosition (
            plint surfaceId, Array<plint,3> const& p1, Array<plint,3> const& p2 ) const;
    virtual void setArguments(std::vector<AtomicBlock3D*> const& arguments);
    virtual ScalarFieldIsoSurface3D<T>* clone() const;
    virtual plint getNumArgs() const { return 1; }
    virtual std::vector<plint> getSurfaceIds() const;
private:
    std::vector<T> isoValues;
    ScalarField3D<T>* scalar;
    Dot3D location;
};


template<typename T, class Function>
class AnalyticalIsoSurface3D : public IsoSurfaceDefinition3D<T> {
public:
    AnalyticalIsoSurface3D(Function const& function_)
        : function(function_)
    { }
    virtual bool isInside (
            plint surfaceId, Array<plint,3> const& position ) const;
    virtual Array<T,3> getSurfacePosition (
            plint surfaceId, Array<plint,3> const& p1, Array<plint,3> const& p2 ) const;
    virtual void setArguments(std::vector<AtomicBlock3D*> const& arguments) { }
    virtual AnalyticalIsoSurface3D<T,Function>* clone() const;
    virtual plint getNumArgs() const { return 0; }
    virtual std::vector<plint> getSurfaceIds() const;
private:
    class WrappedIsInside {
    public:
        WrappedIsInside(Array<T,3> const& p1_, Array<T,3> const& p2_, Function const& function_)
            : p1(p1_), p2(p2_), function(function_)
        { }
        T operator()(T position) const {
            if (function.floatIsInside(p1+position*(p2-p1))) {
                return (T)1;
            }
            else {
                return (T)-1;
            }
        }
    private:
        Array<T,3> p1, p2;
        Function function;
    };
private:
    Function function;
};



template<typename T, class SurfaceData>
class BoundaryShapeIsoSurface3D : public IsoSurfaceDefinition3D<T> {
public:
    BoundaryShapeIsoSurface3D(BoundaryShape3D<T,SurfaceData>* shape_);
    virtual ~BoundaryShapeIsoSurface3D();
    BoundaryShapeIsoSurface3D(BoundaryShapeIsoSurface3D<T,SurfaceData> const& rhs);
    BoundaryShapeIsoSurface3D<T,SurfaceData>& operator=(BoundaryShapeIsoSurface3D<T,SurfaceData> const& rhs);
    void swap(BoundaryShapeIsoSurface3D<T,SurfaceData>& rhs);
    virtual bool isInside (
            plint surfaceId, Array<plint,3> const& position ) const;
    virtual Array<T,3> getSurfacePosition (
            plint surfaceId, Array<plint,3> const& p1, Array<plint,3> const& p2 ) const;
    /// Arguments are:
    /// 1. voxelizedDomain.getVoxelMatrix()
    /// 2. voxelizedDomain.getTriangleHash()
    /// 3. Argument needed by the boundary profiles.
    virtual void setArguments(std::vector<AtomicBlock3D*> const& arguments);
    virtual BoundaryShapeIsoSurface3D<T,SurfaceData>* clone() const;
    virtual plint getNumArgs() const { return 3; }
    virtual std::vector<plint> getSurfaceIds() const;
private:
    BoundaryShape3D<T,SurfaceData>* shape;
};


template<typename T>
class MarchingCubeSurfaces3D : public BoxProcessingFunctional3D {
public:
    typedef typename TriangleSet<T>::Triangle Triangle;
public:
    MarchingCubeSurfaces3D( std::vector<plint> surfaceIds_,
                            IsoSurfaceDefinition3D<T>* isoSurface_,
                            bool edgeOrientedData_=false );
    ~MarchingCubeSurfaces3D();
    MarchingCubeSurfaces3D(MarchingCubeSurfaces3D<T> const& rhs);
    MarchingCubeSurfaces3D<T>& operator=(MarchingCubeSurfaces3D<T> const& rhs);
    void swap(MarchingCubeSurfaces3D<T>& rhs);
    virtual void processGenericBlocks (
                Box3D domain, std::vector<AtomicBlock3D*> fields );
    virtual void defaultImplementation (
                Box3D domain, AtomicContainerBlock3D* triangleContainer );
    virtual void edgeOriented (
                Box3D domain, AtomicContainerBlock3D* triangleContainer );
    virtual MarchingCubeSurfaces3D<T>* clone() const;
    virtual void getTypeOfModification(std::vector<modif::ModifT>& modified) const;
    void setEdgeOrientedEnvelope(plint edgeOrientedEnvelope_) {
        edgeOrientedEnvelope = edgeOrientedEnvelope_;
    }
public:
    class TriangleSetData : public ContainerBlockData {
    public:
        std::vector<Triangle> triangles;
        virtual TriangleSetData* clone() const {
            return new TriangleSetData(*this);
        }
    };
    class EdgeOrientedTriangleSetData : public ContainerBlockData {
    public:
        /// Holds the topology of a triangle, by stating on which edges its
        /// vertices are situated.
        struct OnEdgeTriangle {
            /** The edge-attribution is stored for two vertices only, because 
              * the first vertex is, by definition, stored on the current edge.
              * An edge is defined by four coordinates: 3 coordinates for the cell,
              * and an identifier (0,1, or 2) for the edge.
              */
            Array<plint,4> vertex1, vertex2;
        };
    public:
        EdgeOrientedTriangleSetData(plint nx, plint ny, plint nz)
            : data(nx,ny,nz)
        { }
        virtual EdgeOrientedTriangleSetData* clone() const {
            return new EdgeOrientedTriangleSetData(*this);
        }
        void addTriangle( plint iX, plint iY, plint iZ, int iEdge,
                          OnEdgeTriangle const& triangle )
        {
            switch(iEdge) {
                case 0: data.get(iX,iY,iZ).edge1triangles.push_back(triangle); break;
                case 1: data.get(iX,iY,iZ).edge2triangles.push_back(triangle); break;
                case 2: data.get(iX,iY,iZ).edge3triangles.push_back(triangle); break;
                default: PLB_ASSERT( false );
            }
        }
        std::vector<OnEdgeTriangle> const& getTriangles(plint iX, plint iY, plint iZ, int iEdge) const {
            switch(iEdge) {
                case 0: return data.get(iX,iY,iZ).edge1triangles;
                case 1: return data.get(iX,iY,iZ).edge2triangles;
                case 2: return data.get(iX,iY,iZ).edge3triangles;
                default: PLB_ASSERT( false );
            }
        }
        void setVertex(plint iX, plint iY, plint iZ, int iEdge, Array<T,3> const& vertex) {
            switch(iEdge) {
                case 0:
                    data.get(iX,iY,iZ).edge1Vertex = vertex;
                    data.get(iX,iY,iZ).edge1VertexDefined = true;
                    break;
                case 1:
                    data.get(iX,iY,iZ).edge2Vertex = vertex;
                    data.get(iX,iY,iZ).edge2VertexDefined = true;
                    break;
                case 2:
                    data.get(iX,iY,iZ).edge3Vertex = vertex;
                    data.get(iX,iY,iZ).edge3VertexDefined = true;
                    break;
                default: PLB_ASSERT( false );
            }
        }
        bool getVertex(plint iX, plint iY, plint iZ, int iEdge, Array<T,3>& vertex) const {
            switch(iEdge) {
                case 0:
                    vertex = data.get(iX,iY,iZ).edge1Vertex;
                    return data.get(iX,iY,iZ).edge1VertexDefined;
                case 1:
                    vertex = data.get(iX,iY,iZ).edge2Vertex;
                    return data.get(iX,iY,iZ).edge2VertexDefined;
                case 2:
                    vertex = data.get(iX,iY,iZ).edge3Vertex;
                    return data.get(iX,iY,iZ).edge3VertexDefined;
                default: PLB_ASSERT( false );
            }
        }
        std::vector<Array<Array<T,3>,3> > reconstructTriangles(plint iX, plint iY, plint iZ, plint iEdge) const
        {
            std::vector<Array<Array<T,3>,3> > triangles;
            EdgeData& localData = data.get(iX,iY,iZ);
            switch(iEdge) {
                case 0:
                    if (!localData.edge1VertexDefined) break;
                    for (pluint i=0; i<localData.edge1triangles; ++i) {
                        Array<T,3> vertex1 = localData.edge1Vertex;
                        Array<plint,4> v2info = localData.edge1triangles[i].vertex1;
                        Array<T,3> vertex2 = data.get(v2info[0], v2info[1], v2info[2]).getVertex(v2info[3]);
                        Array<plint,4> v3info = localData.edge1triangles[i].vertex2;
                        Array<T,3> vertex3 = data.get(v3info[0], v3info[1], v3info[2]).getVertex(v3info[3]);
                        triangles.push_back(Array<Array<T,3>,3>(vertex1,vertex2,vertex3));
                    }
                    break;
                case 1:
                    if (!localData.edge2VertexDefined) break;
                    for (pluint i=0; i<localData.edge2triangles; ++i) {
                        Array<T,3> vertex1 = localData.edge1Vertex;
                        Array<plint,4> v2info = localData.edge2triangles[i].vertex1;
                        Array<T,3> vertex2 = data.get(v2info[0], v2info[1], v2info[2]).getVertex(v2info[3]);
                        Array<plint,4> v3info = localData.edge2triangles[i].vertex2;
                        Array<T,3> vertex3 = data.get(v3info[0], v3info[1], v3info[2]).getVertex(v3info[3]);
                        triangles.push_back(Array<Array<T,3>,3>(vertex1,vertex2,vertex3));
                    }
                    break;
                case 2:
                    if (!localData.edge3VertexDefined) break;
                    for (pluint i=0; i<localData.edge3triangles; ++i) {
                        Array<T,3> vertex1 = localData.edge1Vertex;
                        Array<plint,4> v2info = localData.edge3triangles[i].vertex1;
                        Array<T,3> vertex2 = data.get(v2info[0], v2info[1], v2info[2]).getVertex(v2info[3]);
                        Array<plint,4> v3info = localData.edge3triangles[i].vertex2;
                        Array<T,3> vertex3 = data.get(v3info[0], v3info[1], v3info[2]).getVertex(v3info[3]);
                        triangles.push_back(Array<Array<T,3>,3>(vertex1,vertex2,vertex3));
                    }
                    break;
            }
            return triangles;
        }
        T getVertexArea(plint iX, plint iY, plint iZ, int iEdge) const
        {
            EdgeData& localData = data.get(iX,iY,iZ);
            switch(iEdge) {
                case 0:
                    if (!localData.edge1VertexDefined) {
                        return -1.0;
                    }
                    else {
                        std::vector<Array<Array<T,3>,3> > triangles = reconstructTriangles(iX,iY,iZ, iEdge);
                        T area=T();
                        for (pluint i=0; i<triangles.size(); ++i) {
                            Array<Array<T,3>,3> const& triangle = triangles[i];
                            T nextArea = computeTriangleArea(triangle[0], triangle[1], triangle[2]);
                            area += nextArea;
                        }
                        return area / (T)3.0;
                    }
                case 1:
                    if (!localData.edge2VertexDefined) {
                        return -1.0;
                    }
                    else {
                        std::vector<Array<Array<T,3>,3> > triangles = reconstructTriangles(iX,iY,iZ, iEdge);
                        T area=T();
                        for (pluint i=0; i<triangles.size(); ++i) {
                            Array<Array<T,3>,3> const& triangle = triangles[i];
                            T nextArea = computeTriangleArea(triangle[0], triangle[1], triangle[2]);
                            area += nextArea;
                        }
                        return area / (T)3.0;
                    }
                case 2:
                    if (!localData.edge3VertexDefined) {
                        return -1.0;
                    }
                    else {
                        std::vector<Array<Array<T,3>,3> > triangles = reconstructTriangles(iX,iY,iZ, iEdge);
                        T area=T();
                        for (pluint i=0; i<triangles.size(); ++i) {
                            Array<Array<T,3>,3> const& triangle = triangles[i];
                            T nextArea = computeTriangleArea(triangle[0], triangle[1], triangle[2]);
                            area += nextArea;
                        }
                        return area / (T)3.0;
                    }
                default: PLB_ASSERT( false );
            }
        }
        std::vector<T> const& getScalars(plint iX, plint iY, plint iZ, int iEdge) const {
            switch(iEdge) {
                case 0: return data.get(iX,iY,iZ).scalars1;
                case 1: return data.get(iX,iY,iZ).scalars2;
                case 2: return data.get(iX,iY,iZ).scalars3;
                default: PLB_ASSERT( false );
            }
        }
        std::vector<T>& getScalars(plint iX, plint iY, plint iZ, int iEdge) {
            switch(iEdge) {
                case 0: return data.get(iX,iY,iZ).scalars1;
                case 1: return data.get(iX,iY,iZ).scalars2;
                case 2: return data.get(iX,iY,iZ).scalars3;
                default: PLB_ASSERT( false );
            }
        }
        bool isEdgeVertexDefined(plint iX, plint iY, plint iZ, int iEdge) {
            switch(iEdge) {
                case 0: return data.get(iX,iY,iZ).edge1VertexDefined;
                case 1: return data.get(iX,iY,iZ).edge2VertexDefined;
                case 2: return data.get(iX,iY,iZ).edge3VertexDefined;
                default: PLB_ASSERT( false );
            }
        }
        plint getNx() const { return data.getNx(); }
        plint getNy() const { return data.getNy(); }
        plint getNz() const { return data.getNz(); }
        Box3D getBoundingBox() const { return data.getBoundingBox(); }
    private:
        struct EdgeData {
            EdgeData()
                : edge1VertexDefined(false),
                  edge2VertexDefined(false),
                  edge3VertexDefined(false)
            { }
            Array<T,3> const& getVertex(plint iEdge) const {
                switch(iEdge) {
                    case 0: return edge1Vertex;
                    case 1: return edge2Vertex;
                    case 2: return edge3Vertex;
                }
            }
            // Each element in the three following vectors defines the topology of
            // a triangle.
            std::vector<OnEdgeTriangle> edge1triangles;
            std::vector<OnEdgeTriangle> edge2triangles;
            std::vector<OnEdgeTriangle> edge3triangles;
            // Every edge has at most one vertex which is shared by all
            // triangles that have a vertex on this edge.
            Array<T,3> edge1Vertex, edge2Vertex, edge3Vertex;
            std::vector<T> scalars1, scalars2, scalars3;
            bool edge1VertexDefined, edge2VertexDefined, edge3VertexDefined;
        };
    private:
        ScalarField3D<EdgeData> data;
    };
private:
    void marchingCubeImpl (
             plint iX, plint iY, plint iZ, plint surfaceId,
             std::vector<Triangle>& triangles,
             int& cubeIndex, std::vector<Array<T,3> >& vertlist );
    void polygonize (
             plint iX, plint iY, plint iZ, plint surfaceId,
             std::vector<Triangle>& triangles );
    /// Edge attribution contains three integers to label the cell ID,
    /// and one integer to label one of the three edges assigned to this cell.
    void polygonize (
             plint iX, plint iY, plint iZ, plint surfaceId,
             std::vector<Triangle>& triangles,
             std::vector<Array<plint,4> >& edgeAttributions );
    static void removeFromVertex (
            Array<T,3> const& p0, Array<T,3> const& p1, Array<T,3>& intersection );
private:
    std::vector<plint> surfaceIds;
    IsoSurfaceDefinition3D<T>* isoSurface;
    bool edgeOrientedData;
    plint edgeOrientedEnvelope;
};

struct MarchingCubeConstants {
    static const int edgeTable[256];
    static const int triTable[256][16];
    static const int edgeNeighb[12][3];
    static const int edgeOrient[12];
};

/// Get an iso-surface by means of the marching cube algorithms.
/** The iso-surface is defined in very generic terms by the isoSurfaceDefinition,
  * and the surfDefinitionArgs are whatever arguments the isoSurfaceDefinition
  * needs. The isoSurfaceDefinition can compute a finite amount of iso-surfaces,
  * the IDs of which are provided by the last argument. If the last argument is
  * omitted, all available iso-surfaces are computed.
  * The iso-surface is returned as a set of triangles, in the first argument.
  **/
template<typename T>
void isoSurfaceMarchingCube (
        std::vector<typename TriangleSet<T>::Triangle>& triangles,
        std::vector<MultiBlock3D*> surfDefinitionArgs,
        IsoSurfaceDefinition3D<T>* isoSurfaceDefinition,
        Box3D const& domain, std::vector<plint> surfaceIds = std::vector<plint>() );

/// This wrapper call to the marching-cube algorithm remeshes the surface of a voxelized domain.
template<typename T>
void isoSurfaceMarchingCube (
        std::vector<typename TriangleSet<T>::Triangle>& triangles,
        VoxelizedDomain3D<T>& voxelizedDomain, Box3D const& domain );

/// This wrapper call to the marching-cube algorithm computes iso-surfaces from a scalar-field.
template<typename T>
void isoSurfaceMarchingCube (
        std::vector<typename TriangleSet<T>::Triangle>& triangles,
        MultiScalarField3D<T>& scalarField, std::vector<T> const& isoLevels, Box3D const& domain );

/// This wrapper call to the marching-cube algorithm computes iso-surfaces from an analytical description.
template<typename T, class Function>
void isoSurfaceMarchingCube (
        std::vector<typename TriangleSet<T>::Triangle>& triangles, MultiBlock3D& block, Function const& function, Box3D const& domain );

template<typename T, template<typename U> class Descriptor>
TriangleSet<T> vofToTriangles(MultiScalarField3D<T>& scalarField, T threshold, Box3D domain);

template<typename T, template<typename U> class Descriptor>
TriangleSet<T> vofToTriangles(MultiScalarField3D<T>& scalarField, T threshold);

/* ****** class ScalarFieldIsoSurface3D ***************** */

template<typename T>
ScalarFieldIsoSurface3D<T>::ScalarFieldIsoSurface3D(std::vector<T> const& isoValues_)
    : isoValues(isoValues_),
      scalar(0),
      location(0,0,0)
{ }

template<typename T>
void ScalarFieldIsoSurface3D<T>::setArguments (
            std::vector<AtomicBlock3D*> const& arguments )
{
    PLB_ASSERT( arguments.size() >= 1 );
    scalar = dynamic_cast<ScalarField3D<T>*>(arguments[0]);
    PLB_ASSERT(scalar);
    location = scalar->getLocation();
}

template<typename T>
bool ScalarFieldIsoSurface3D<T>::isInside (
            plint surfaceId, Array<plint,3> const& position ) const
{
    PLB_ASSERT(scalar);
    PLB_ASSERT(surfaceId < (plint)isoValues.size());
    return scalar->get(position[0]-location.x, position[1]-location.y, position[2]-location.z) < isoValues[surfaceId];
}

template<typename T>
Array<T,3> ScalarFieldIsoSurface3D<T>::getSurfacePosition (
            plint surfaceId, Array<plint,3> const& p1, Array<plint,3> const& p2 ) const
{
    static const T epsilon = 1.e-5;
    PLB_ASSERT(scalar);
    PLB_ASSERT(surfaceId < (plint)isoValues.size());
    T valp1 = scalar->get(p1[0]-location.x, p1[1]-location.y, p1[2]-location.z);
    T valp2 = scalar->get(p2[0]-location.x, p2[1]-location.y, p2[2]-location.z);

    T isolevel = isoValues[surfaceId];
    if (std::fabs(isolevel-valp1) < epsilon) return(p1);
    if (std::fabs(isolevel-valp2) < epsilon) return(p2);
    if (std::fabs(valp1-valp2) < epsilon) return(p1);
    T mu = (isolevel - valp1) / (valp2 - valp1);
    return Array<T,3> (
               (T)p1[0] + mu * (p2[0] - p1[0]),
               (T)p1[1] + mu * (p2[1] - p1[1]),
               (T)p1[2] + mu * (p2[2] - p1[2]) );
}

template<typename T>
ScalarFieldIsoSurface3D<T>* ScalarFieldIsoSurface3D<T>::clone() const {
    return new ScalarFieldIsoSurface3D<T>(*this);
}

template<typename T>
std::vector<plint> ScalarFieldIsoSurface3D<T>::getSurfaceIds() const {
    std::vector<plint> surfaceIds;
    for (plint i=0; i<(plint)isoValues.size(); ++i) {
        surfaceIds.push_back(i);
    }
    return surfaceIds;
}


/* ****** class BoundaryShapeIsoSurface3D ***************** */

template<typename T, class SurfaceData>
BoundaryShapeIsoSurface3D<T,SurfaceData>::BoundaryShapeIsoSurface3D(BoundaryShape3D<T,SurfaceData>* shape_)
    : shape(shape_)
{ }

template<typename T, class SurfaceData>
BoundaryShapeIsoSurface3D<T,SurfaceData>::~BoundaryShapeIsoSurface3D() {
    delete shape;
}

template<typename T, class SurfaceData>
BoundaryShapeIsoSurface3D<T,SurfaceData>::BoundaryShapeIsoSurface3D(BoundaryShapeIsoSurface3D<T,SurfaceData> const& rhs)
    : shape(rhs.shape->clone())
{ }

template<typename T, class SurfaceData>
BoundaryShapeIsoSurface3D<T,SurfaceData>& BoundaryShapeIsoSurface3D<T,SurfaceData>::operator= (
        BoundaryShapeIsoSurface3D<T,SurfaceData> const& rhs )
{
    BoundaryShapeIsoSurface3D<T,SurfaceData>(rhs).swap(*this);
    return *this;
}

template<typename T, class SurfaceData>
void BoundaryShapeIsoSurface3D<T,SurfaceData>::swap(BoundaryShapeIsoSurface3D<T,SurfaceData>& rhs) {
    std::swap(shape, rhs.shape);
}

template<typename T, class SurfaceData>
void BoundaryShapeIsoSurface3D<T,SurfaceData>::setArguments(std::vector<AtomicBlock3D*> const& arguments)
{
    BoundaryShape3D<T,SurfaceData>* newShape = shape->clone(arguments);
    std::swap(shape,newShape);
    delete newShape;
}

template<typename T, class SurfaceData>
BoundaryShapeIsoSurface3D<T,SurfaceData>* BoundaryShapeIsoSurface3D<T,SurfaceData>::clone() const {
    return new BoundaryShapeIsoSurface3D<T,SurfaceData>(*this);
}

template<typename T, class SurfaceData>
std::vector<plint> BoundaryShapeIsoSurface3D<T,SurfaceData>::getSurfaceIds() const {
    std::vector<plint> surfaceIds;
    surfaceIds.push_back(0); // Only one surface can be produced, ID does not matter.
    return surfaceIds;
}

template<typename T, class SurfaceData>
bool BoundaryShapeIsoSurface3D<T,SurfaceData>::isInside (
            plint surfaceId, Array<plint,3> const& position ) const
{
    return shape->isInside(Dot3D(position[0],position[1],position[2]));
}

template<typename T, class SurfaceData>
Array<T,3> BoundaryShapeIsoSurface3D<T,SurfaceData>::getSurfacePosition (
        plint surfaceId, Array<plint,3> const& p1, Array<plint,3> const& p2 ) const
{
    Array<T,3> realP1(p1), realP2(p2);
    Array<T,3> surfacePosition, wallNormal;
    T distance;
    SurfaceData surfaceData;
    OffBoundary::Type bdType;
    plint id=-1;
    bool ok =
        shape->pointOnSurface( realP1, realP2-realP1, surfacePosition,
                               distance, wallNormal, surfaceData, bdType, id );
    //PLB_ASSERT( ok );
    if (!ok) {
        ok =
            shape->pointOnSurface( realP1-(T)0.5*(realP2-realP1), (T)2.0*(realP2-realP1), surfacePosition,
                                   distance, wallNormal, surfaceData, bdType, id );
        if (!ok) {
            surfacePosition = (T)0.5*(realP2+realP1);
        }
    }
    return surfacePosition;
}


/* ****** class MarchingCubeSurfaces3D ***************** */

template<typename T>
MarchingCubeSurfaces3D<T>::MarchingCubeSurfaces3D (
        std::vector<plint> surfaceIds_, IsoSurfaceDefinition3D<T>* isoSurface_,
        bool edgeOrientedData_ )
    : surfaceIds(surfaceIds_),
      isoSurface(isoSurface_),
      edgeOrientedData(edgeOrientedData_),
      edgeOrientedEnvelope(1)
{ }

template<typename T>
MarchingCubeSurfaces3D<T>::~MarchingCubeSurfaces3D() {
    delete isoSurface;
}

template<typename T>
void MarchingCubeSurfaces3D<T>::getTypeOfModification(std::vector<modif::ModifT>& modified) const {
    modified[0] = modif::staticVariables;
    for (pluint i=1; i<modified.size(); ++i) {
        modified[i] = modif::nothing;
    }
}

template<typename T>
MarchingCubeSurfaces3D<T>::MarchingCubeSurfaces3D(MarchingCubeSurfaces3D<T> const& rhs)
    : surfaceIds(rhs.surfaceIds),
      isoSurface(rhs.isoSurface->clone()),
      edgeOrientedData(rhs.edgeOrientedData),
      edgeOrientedEnvelope(rhs.edgeOrientedEnvelope)
{ }

template<typename T>
MarchingCubeSurfaces3D<T>& MarchingCubeSurfaces3D<T>::operator=(MarchingCubeSurfaces3D<T> const& rhs)
{
    MarchingCubeSurfaces3D<T>(rhs).swap(*this);
    return *this;
}

template<typename T>
MarchingCubeSurfaces3D<T>* MarchingCubeSurfaces3D<T>::clone() const
{
    return new MarchingCubeSurfaces3D<T>(*this);
}

template<typename T>
void MarchingCubeSurfaces3D<T>::swap(MarchingCubeSurfaces3D<T>& rhs)
{
    surfaceIds.swap(rhs.surfaceIds);
    std::swap(isoSurface, rhs.isoSurface);
    std::swap(edgeOrientedData, rhs.edgeOrientedData);
    std::swap(edgeOrientedEnvelope, rhs.edgeOrientedEnvelope);
}

template<typename T>
void MarchingCubeSurfaces3D<T>::processGenericBlocks (
                Box3D domain, std::vector<AtomicBlock3D*> fields )
{
    PLB_PRECONDITION( (plint)fields.size() >= 1 + isoSurface->getNumArgs() );

    AtomicContainerBlock3D* triangleContainer =
        dynamic_cast<AtomicContainerBlock3D*>(fields[0]);
    PLB_ASSERT( triangleContainer );

    if (isoSurface->getNumArgs()>0) {
        std::vector<AtomicBlock3D*> isoSurfaceParameters(isoSurface->getNumArgs());
        for (plint i=0; i<isoSurface->getNumArgs(); ++i) {
            isoSurfaceParameters[i] = fields[i+1];
        }
        isoSurface->setArguments(isoSurfaceParameters);
    }
    if (edgeOrientedData) {
        edgeOriented(domain, triangleContainer);
    }
    else {
        defaultImplementation(domain, triangleContainer);
    }
}

template<typename T>
void MarchingCubeSurfaces3D<T>::defaultImplementation (
        Box3D domain, AtomicContainerBlock3D* triangleContainer )
{
    std::vector<Triangle> triangles;
    Dot3D location = triangleContainer->getLocation();

    for (plint iX=domain.x0; iX<=domain.x1; ++iX) {
        for (plint iY=domain.y0; iY<=domain.y1; ++iY) {
            for (plint iZ=domain.z0; iZ<=domain.z1; ++iZ) {
                for (pluint i=0; i<surfaceIds.size(); ++i) {
                    polygonize(iX+location.x,iY+location.y,iZ+location.z, surfaceIds[i], triangles);
                }
            }
        }
    }

    TriangleSetData* data = new TriangleSetData;
    data->triangles = triangles;
    triangleContainer -> setData(data);
}

template<typename T>
void MarchingCubeSurfaces3D<T>::edgeOriented (
        Box3D domain, AtomicContainerBlock3D* triangleContainer )
{
    EdgeOrientedTriangleSetData* data = new EdgeOrientedTriangleSetData (
            triangleContainer -> getNx(), triangleContainer -> getNy(),
            triangleContainer -> getNz() );
    Dot3D location = triangleContainer->getLocation();

    // Include at least one envelope layer, so the outer edges get the full information
    // from all surrounding triangles.
    plint env=edgeOrientedEnvelope;
    for (plint iX=domain.x0-env; iX<=domain.x1+env; ++iX) {
        for (plint iY=domain.y0-env; iY<=domain.y1+env; ++iY) {
            for (plint iZ=domain.z0-env; iZ<=domain.z1+env; ++iZ) {
                for (pluint iSurf=0; iSurf<surfaceIds.size(); ++iSurf) {
                    std::vector<Triangle> triangles;
                    std::vector<Array<plint,4> > edgeAttributions;
                    // Get all triangles computed by the marching-cube algorithm for
                    // the current cell.
                    polygonize(iX+location.x,iY+location.y,iZ+location.z, surfaceIds[iSurf],
                               triangles, edgeAttributions);
                    // Additionally to the triangle coordinates, the algorithm returns
                    // the coordinates of the edges on which the triangle vertices are placed.
                    // There's an edge attribution for every triangle vertex, as it is checked by the
                    // following assertion:
                    PLB_ASSERT( edgeAttributions.size() == 3*triangles.size() );
                    plint i=0; // i runs over the edge-attribution index.
                    for (pluint iTriangle=0; iTriangle<triangles.size(); ++iTriangle) {
                        Triangle triangle = triangles[iTriangle];
                        // Turn the cell coordinates of the edge attribution into coordinates
                        // that are local to the current atomic-block.
                        Array<Array<plint,4>,3> localEdgeAttribution;
                        // Do the conversion for each of the three vertices of the current triangle.
                        for (int j=0; j<3; ++j, ++i) {
                            localEdgeAttribution[j] = Array<plint,4> (
                                edgeAttributions[i][0] - location.x,
                                edgeAttributions[i][1] - location.y,
                                edgeAttributions[i][2] - location.z,
                                edgeAttributions[i][3] );
                        }
                        // Add the triangle to the data structure. Each triangle is added three
                        // times, once on each edge on which it has a vertex.
                        for (int j=0; j<3; ++j) {
                            typename EdgeOrientedTriangleSetData::OnEdgeTriangle onEdgeTriangle;
                            // The first vertex is always the one which is on the current edge.
                            // The two subsequent vertices are defined so as to preserve the
                            // triangle orientation.
                            onEdgeTriangle.vertex1 = localEdgeAttribution[(j+1)%3];
                            onEdgeTriangle.vertex2 = localEdgeAttribution[(j+2)%3];
                            plint iX=localEdgeAttribution[j][0];
                            plint iY=localEdgeAttribution[j][1];
                            plint iZ=localEdgeAttribution[j][2];
                            plint edgeId = localEdgeAttribution[j][3];
                            data->addTriangle(iX,iY,iZ, edgeId, onEdgeTriangle);
                            // In the end, the edge vertex position will be set multiple times (once for each
                            // triangle that has a vertex on the edge). This doesn't matter, because the
                            // marching-cube algorithm in every case produces the same vertex position.
                            data->setVertex(iX,iY,iZ, edgeId, triangle[j]);
                        }
                    }
                }
            }
        }
    }

    triangleContainer -> setData(data);
}

template<typename T>
void MarchingCubeSurfaces3D<T>::removeFromVertex (
        Array<T,3> const& p0, Array<T,3> const& p1, Array<T,3>& intersection )
{
    static const T triangleEpsilon= 1.e-3;
    static const T triangleEpsilonSqr = util::sqr(triangleEpsilon);
   if (normSqr(p0-intersection) < triangleEpsilonSqr) {
       intersection = p0 + triangleEpsilon*(p1-p0);
   }
   else if (normSqr(p1-intersection) < triangleEpsilonSqr) {
       intersection = p1 - triangleEpsilon*(p1-p0);
   }
}

template<typename T>
void MarchingCubeSurfaces3D<T>::marchingCubeImpl (
             plint iX, plint iY, plint iZ, plint surfaceId,
             std::vector<Triangle>& triangles,
             int& cubeindex, std::vector<Array<T,3> >& vertlist )
{
    typedef MarchingCubeConstants mcc;

    Array<plint,3> p0(iX  ,iY+1,iZ  );
    Array<plint,3> p1(iX+1,iY+1,iZ  );
    Array<plint,3> p2(iX+1,iY  ,iZ  );
    Array<plint,3> p3(iX  ,iY  ,iZ  );
    Array<plint,3> p4(iX  ,iY+1,iZ+1);
    Array<plint,3> p5(iX+1,iY+1,iZ+1);
    Array<plint,3> p6(iX+1,iY  ,iZ+1);
    Array<plint,3> p7(iX  ,iY  ,iZ+1);

    cubeindex = 0;
    if (isoSurface->isInside(surfaceId,p0)) cubeindex |= 1;   // Point 0
    if (isoSurface->isInside(surfaceId,p1)) cubeindex |= 2;   // Point 1
    if (isoSurface->isInside(surfaceId,p2)) cubeindex |= 4;   // Point 2
    if (isoSurface->isInside(surfaceId,p3)) cubeindex |= 8;   // Point 3
    if (isoSurface->isInside(surfaceId,p4)) cubeindex |= 16;  // Point 4
    if (isoSurface->isInside(surfaceId,p5)) cubeindex |= 32;  // Point 5
    if (isoSurface->isInside(surfaceId,p6)) cubeindex |= 64;  // Point 6
    if (isoSurface->isInside(surfaceId,p7)) cubeindex |= 128; // Point 7

    vertlist.resize(12);
    /* Cube is entirely in/out of the surface */
    if (mcc::edgeTable[cubeindex] == 0) return;

    /* Find the vertices where the surface intersects the cube */
    if (mcc::edgeTable[cubeindex] & 1) {
        vertlist[0] = isoSurface->getSurfacePosition(surfaceId, p0, p1); // x-edge of y-neighbor.
        removeFromVertex(p0, p1, vertlist[0]);
    }
    if (mcc::edgeTable[cubeindex] & 2) {
        vertlist[1] = isoSurface->getSurfacePosition(surfaceId, p1, p2); // y-edge of x-neighbor.
        removeFromVertex(p1, p2, vertlist[1]);
    }
    if (mcc::edgeTable[cubeindex] & 4) {
        vertlist[2] = isoSurface->getSurfacePosition(surfaceId, p2, p3); // x-edge of current cell.
        removeFromVertex(p2, p3, vertlist[2]);
    }
    if (mcc::edgeTable[cubeindex] & 8) {
        vertlist[3] = isoSurface->getSurfacePosition(surfaceId, p3, p0); // y-edge of current cell.
        removeFromVertex(p3, p0, vertlist[3]);
    }
    if (mcc::edgeTable[cubeindex] & 16) {
        vertlist[4] = isoSurface->getSurfacePosition(surfaceId, p4, p5); // x-edge of y-z-neighbor.
        removeFromVertex(p4, p5, vertlist[4]);
    }
    if (mcc::edgeTable[cubeindex] & 32) {
        vertlist[5] = isoSurface->getSurfacePosition(surfaceId, p5, p6); // y-edge of x-z-neighbor.
        removeFromVertex(p5, p6, vertlist[5]);
    }
    if (mcc::edgeTable[cubeindex] & 64) {
        vertlist[6] = isoSurface->getSurfacePosition(surfaceId, p6, p7); // x-edge of z-neighbor.
        removeFromVertex(p6, p7, vertlist[6]);
    }
    if (mcc::edgeTable[cubeindex] & 128) {
        vertlist[7] = isoSurface->getSurfacePosition(surfaceId, p7, p4); // y-edge of z-neighbor.
        removeFromVertex(p7, p4, vertlist[7]);
    }
    if (mcc::edgeTable[cubeindex] & 256) {
        vertlist[8] = isoSurface->getSurfacePosition(surfaceId, p0, p4); // z-edge of y-neighbor.
        removeFromVertex(p0, p4, vertlist[8]);
    }
    if (mcc::edgeTable[cubeindex] & 512) {
        vertlist[9] = isoSurface->getSurfacePosition(surfaceId, p1, p5); // z-edge of x-y-neighbor.
        removeFromVertex(p1, p5, vertlist[9]);
    }
    if (mcc::edgeTable[cubeindex] & 1024) {
        vertlist[10] = isoSurface->getSurfacePosition(surfaceId, p2, p6); // z-edge of x-neighbor.
        removeFromVertex(p2, p6, vertlist[10]);
    }
    if (mcc::edgeTable[cubeindex] & 2048) {
        vertlist[11] = isoSurface->getSurfacePosition(surfaceId, p3, p7); // z-edge of current cell.
        removeFromVertex(p3, p7, vertlist[11]);
    }
}

template<typename T>
void MarchingCubeSurfaces3D<T>::polygonize (
             plint iX, plint iY, plint iZ, plint surfaceId,
             std::vector<Triangle>& triangles )
{
    static const T epsilon = std::numeric_limits<T>::epsilon()*1.e4;
    typedef MarchingCubeConstants mcc;
    int cubeindex;
    std::vector<Array<T,3> > vertlist(12);
    marchingCubeImpl(iX,iY,iZ, surfaceId, triangles, cubeindex, vertlist);

    /* Create the triangle */
    for (plint i=0;mcc::triTable[cubeindex][i]!=-1;i+=3) {
        int edge1 = mcc::triTable[cubeindex][i  ];
        int edge2 = mcc::triTable[cubeindex][i+1];
        int edge3 = mcc::triTable[cubeindex][i+2];
        if (isoSurface->edgeIsValid(iX,iY,iZ, edge1) &&
            isoSurface->edgeIsValid(iX,iY,iZ, edge2) &&
            isoSurface->edgeIsValid(iX,iY,iZ, edge3) )
        {
            Triangle triangle;
            triangle[0] = vertlist[edge1];
            triangle[1] = vertlist[edge2];
            triangle[2] = vertlist[edge3];
            if (computeTriangleArea(triangle[0],triangle[1],triangle[2])>epsilon) {
                triangles.push_back(triangle);
            }
        }
    }
}

template<typename T>
void MarchingCubeSurfaces3D<T>::polygonize (
             plint iX, plint iY, plint iZ, plint surfaceId,
             std::vector<Triangle>& triangles,
             std::vector<Array<plint,4> >& edgeAttributions )
{
    typedef MarchingCubeConstants mcc;
    int cubeindex;
    std::vector<Array<T,3> > vertlist(12);
    marchingCubeImpl(iX,iY,iZ, surfaceId, triangles, cubeindex, vertlist);

    /* Create the triangle */
    for (plint i=0;mcc::triTable[cubeindex][i]!=-1;i+=3) {
        int edge1 = mcc::triTable[cubeindex][i  ];
        int edge2 = mcc::triTable[cubeindex][i+1];
        int edge3 = mcc::triTable[cubeindex][i+2];
        if (isoSurface->edgeIsValid(iX,iY,iZ, edge1) &&
            isoSurface->edgeIsValid(iX,iY,iZ, edge2) &&
            isoSurface->edgeIsValid(iX,iY,iZ, edge3) )
        {
            Triangle triangle;
            triangle[0] = vertlist[edge1];
            triangle[1] = vertlist[edge2];
            triangle[2] = vertlist[edge3];
            triangles.push_back(triangle);
            edgeAttributions.push_back (
                    Array<plint,4>(iX+mcc::edgeNeighb[edge1][0],
                                   iY+mcc::edgeNeighb[edge1][1],
                                   iZ+mcc::edgeNeighb[edge1][2],
                                   mcc::edgeOrient[edge1]) );
            edgeAttributions.push_back (
                    Array<plint,4>(iX+mcc::edgeNeighb[edge2][0],
                                   iY+mcc::edgeNeighb[edge2][1],
                                   iZ+mcc::edgeNeighb[edge2][2],
                                   mcc::edgeOrient[edge2] ) );
            edgeAttributions.push_back (
                    Array<plint,4>(iX+mcc::edgeNeighb[edge3][0],
                                   iY+mcc::edgeNeighb[edge3][1],
                                   iZ+mcc::edgeNeighb[edge3][2],
                                   mcc::edgeOrient[edge3] ) );
        }
    }
}

/* ****** Free Functions ***************** */


/// Get an iso-surface by means of the marching cube algorithms. The iso-surface
/// is defined in very generic terms by the isoSurfaceDefinition, and the
/// surfDefinitionArgs are whatever arguments the isoSurfaceDefinition needs.
/// The isoSurfaceDefinition can compute a finite amount of iso-surfaces, the
/// IDs of which are provided by the last argument. If the last argument is omitted,
/// all available iso-surfaces are computed.
///
/// The iso-surface is returned as a set of triangles, in the first argument.
template<typename T>
void isoSurfaceMarchingCube (
        std::vector<typename TriangleSet<T>::Triangle>& triangles,
        std::vector<MultiBlock3D*> surfDefinitionArgs,
        IsoSurfaceDefinition3D<T>* isoSurfaceDefinition, Box3D const& domain,
        std::vector<plint> surfaceIds )
{
    typedef typename TriangleSet<T>::Triangle Triangle;
    PLB_ASSERT( surfDefinitionArgs.size()>0 );
    if (surfaceIds.empty()) {
        surfaceIds = isoSurfaceDefinition->getSurfaceIds();
    }
    MultiContainerBlock3D triangleContainer(*surfDefinitionArgs[0]);
    std::vector<MultiBlock3D*> args;
    args.push_back(&triangleContainer);
    for (pluint i=0; i<surfDefinitionArgs.size(); ++i) {
        args.push_back(surfDefinitionArgs[i]);
    }
    applyProcessingFunctional (
        new MarchingCubeSurfaces3D<T>(surfaceIds, isoSurfaceDefinition), domain, args );

    MultiBlockManagement3D const& management = triangleContainer.getMultiBlockManagement();
    ThreadAttribution const& threadAttribution = management.getThreadAttribution();
    SparseBlockStructure3D const& sparseBlock = management.getSparseBlockStructure();

    std::map<plint,Box3D> const& domains = sparseBlock.getBulks();
    std::vector<plint> numTriangles(domains.size());

    std::vector<plint> myPositions;
    std::vector<std::vector<Triangle> > myTriangles;

    std::map<plint,Box3D>::const_iterator it = domains.begin();
    plint pos = 0;
    for (; it != domains.end(); ++it) {
        plint id = it->first;
        if (threadAttribution.isLocal(id)) {
            myPositions.push_back(pos);
            AtomicContainerBlock3D const& atomicContainer = triangleContainer.getComponent(id);
            typename MarchingCubeSurfaces3D<T>::TriangleSetData const* data =
                dynamic_cast<typename MarchingCubeSurfaces3D<T>::TriangleSetData const*> (atomicContainer.getData());
            if (data) {
                PLB_ASSERT((plint)numTriangles.size()>pos);
                numTriangles[pos] = data->triangles.size();
                myTriangles.push_back(data->triangles);
            }
            else {
                PLB_ASSERT((plint)numTriangles.size()>pos);
                numTriangles[pos] = 0;
                myTriangles.push_back(std::vector<Triangle>());
            }
        }
        else {
            PLB_ASSERT((plint)numTriangles.size()>pos);
            numTriangles[pos] = 0;
        }
        ++pos;
    }
#ifdef PLB_MPI_PARALLEL
    if (numTriangles.size()>0) {
        std::vector<plint> tmp(numTriangles.size());
        global::mpi().reduceVect(numTriangles, tmp, MPI_SUM);
        PLB_ASSERT(tmp.size()>0);
        global::mpi().bCast(&tmp[0], tmp.size());
        tmp.swap(numTriangles);
    }
#endif
    std::vector<plint> cumNumTriangles(numTriangles.size()+1);
    PLB_ASSERT(cumNumTriangles.size()>0);
    cumNumTriangles[0] = 0;
    std::partial_sum(numTriangles.begin(), numTriangles.end(), cumNumTriangles.begin()+1);
    plint totNumTriangles = cumNumTriangles.back();

    triangles.clear();
    if (global::mpi().isMainProcessor()) {
        triangles.resize(totNumTriangles);
        std::map<plint,Box3D>::const_iterator it = domains.begin();
        plint iDomain=0;
        plint iMyPositions=0;
        for (; it != domains.end(); ++it, ++iDomain) {
            PLB_ASSERT((plint)cumNumTriangles.size()>iDomain);
            plint startPos = cumNumTriangles[iDomain];
            PLB_ASSERT((plint)cumNumTriangles.size()>iDomain+1);
            plint endPos = cumNumTriangles[iDomain+1];
            plint id = it->first;
            int mpiThread = threadAttribution.getMpiProcess(id);
            if (mpiThread==0) {
                PLB_ASSERT((plint)myTriangles.size()>iMyPositions);
                PLB_ASSERT((plint)triangles.size()>= (plint)myTriangles[iMyPositions].size()+startPos);
                std::copy(myTriangles[iMyPositions].begin(), myTriangles[iMyPositions].end(),
                          triangles.begin()+startPos);
                ++iMyPositions;
            }
            else {
                PLB_ASSERT(endPos>=startPos);
                std::vector<T> receiveVect(9*(endPos-startPos));
#ifdef PLB_MPI_PARALLEL
                if (receiveVect.empty()) receiveVect.resize(1);
                global::mpi().receive(&receiveVect[0], receiveVect.size(), mpiThread);
#endif
                for (plint i=startPos; i<endPos; ++i) {
                    plint k=0;
                    for (pluint iVertex=0; iVertex<3; ++iVertex) {
                        for (pluint iCoord=0; iCoord<3; ++iCoord) {
                            PLB_ASSERT((plint)triangles.size()>i);
                            PLB_ASSERT((plint)receiveVect.size()>9*(i-startPos)+k);
                            triangles[i][iVertex][iCoord] = receiveVect[9*(i-startPos)+k];
                            ++k;
                        }
                    }
                }
            }
        }
    }
    else {  // isMainProcessor
        for (pluint iPos=0; iPos<myPositions.size(); ++iPos) {
            plint myPos = myPositions[iPos];
            PLB_ASSERT((plint)cumNumTriangles.size()>myPos);
            plint startPos = cumNumTriangles[myPos];
            PLB_ASSERT((plint)cumNumTriangles.size()>myPos+1);
            plint endPos = cumNumTriangles[myPos+1];
            std::vector<T> sendVect(9*(endPos-startPos));
            for (plint i=0; i<endPos-startPos; ++i) {
                plint k=0;
                for (pluint iVertex=0; iVertex<3; ++iVertex) {
                    for (pluint iCoord=0; iCoord<3; ++iCoord) {
                        PLB_ASSERT((plint)sendVect.size()>9*i+k);
                        PLB_ASSERT(myTriangles.size()>iPos);
                        PLB_ASSERT((plint)myTriangles[iPos].size()>i);
                        sendVect[9*i+k] = myTriangles[iPos][i][iVertex][iCoord];
                        ++k;
                    }
                }
            }
#ifdef PLB_MPI_PARALLEL
            if (sendVect.empty()) sendVect.resize(1);
            global::mpi().send(&sendVect[0], sendVect.size(), 0);
#endif
        }
    }
}

template<typename T>
void isoSurfaceMarchingCube (
        std::vector<typename TriangleSet<T>::Triangle>& triangles,
        VoxelizedDomain3D<T>& voxelizedDomain, Box3D const& domain )
{
    BoundaryProfiles3D<T,Array<T,3> > profiles;
    TriangleFlowShape3D<T,Array<T,3> >* flowShape =
        new TriangleFlowShape3D<T,Array<T,3> >(voxelizedDomain.getBoundary(), profiles);
    std::vector<MultiBlock3D*> triangleShapeArg;
    triangleShapeArg.push_back(&voxelizedDomain.getVoxelMatrix());
    triangleShapeArg.push_back(&voxelizedDomain.getTriangleHash());
    triangleShapeArg.push_back(&voxelizedDomain.getVoxelMatrix()); // dummy argument.
    isoSurfaceMarchingCube(triangles, triangleShapeArg, new BoundaryShapeIsoSurface3D<T,Array<T,3> >(flowShape), domain);
}

template<typename T>
void isoSurfaceMarchingCube (
        std::vector<typename TriangleSet<T>::Triangle>& triangles,
        MultiScalarField3D<T>& scalarField, std::vector<T> const& isoLevels, Box3D const& domain )
{
    std::vector<MultiBlock3D*> scalarFieldArg;
    scalarFieldArg.push_back(&scalarField);
    isoSurfaceMarchingCube(triangles, scalarFieldArg, new ScalarFieldIsoSurface3D<T>(isoLevels), domain);
}

template<typename T, class Function>
void isoSurfaceMarchingCube (
        std::vector<typename TriangleSet<T>::Triangle>& triangles, MultiBlock3D& block,
        Function const& function, Box3D const& domain )
{
    std::vector<MultiBlock3D*> surfDefinitionArgs;
    surfDefinitionArgs.push_back(&block);
    AnalyticalIsoSurface3D<T,Function>* isoSurface = new AnalyticalIsoSurface3D<T,Function>(function);
    std::vector<plint> surfaceIds;
    surfaceIds.push_back(0);
    isoSurfaceMarchingCube(triangles, surfDefinitionArgs, isoSurface, domain, surfaceIds);
}

template<typename T, template<typename U> class Descriptor>
TriangleSet<T> vofToTriangles(MultiScalarField3D<T>& scalarField, T threshold, Box3D domain)
{
    std::vector<T> isoLevels;
    isoLevels.push_back(threshold);
    typedef typename TriangleSet<T>::Triangle Triangle;
    std::vector<Triangle> triangles;
    isoSurfaceMarchingCube (
            triangles,
            *LBMsmoothen3D<T,Descriptor>(*LBMsmoothen3D<T,Descriptor>(scalarField, domain),domain),
            isoLevels, scalarField.getBoundingBox().enlarge(-2) );
    TriangleSet<T> triangleSet(triangles);
    return triangleSet;
}

template<typename T, template<typename U> class Descriptor>
TriangleSet<T> vofToTriangles(MultiScalarField3D<T>& scalarField, T threshold)
{
    Box3D domain = scalarField.getBoundingBox();
    return vofToTriangles(scalarField, threshold, domain);
}

template<typename T, class Function>
bool AnalyticalIsoSurface3D<T,Function>::isInside (
            plint surfaceId, Array<plint,3> const& position ) const
{
    return function.intIsInside(position);
}

template<typename T, class Function>
Array<T,3> AnalyticalIsoSurface3D<T,Function>::getSurfacePosition (
            plint surfaceId, Array<plint,3> const& p1, Array<plint,3> const& p2 ) const
{
    static const T epsilon = 1.e-4;

    plint maxIter = 40;
    plint countMax = 10;
    plint count = 0;
    T pos = T();
    bool ok = bisect(WrappedIsInside(p1, p2, function), (T)0-epsilon, (T)1+epsilon, epsilon, maxIter, pos);
    while (!ok && count < countMax) {
        count++;
        maxIter *= 2;
        pos = T();
        ok = bisect(WrappedIsInside(p1, p2, function), (T)0-epsilon, (T)1+epsilon, epsilon, maxIter, pos);
    }
    PLB_ASSERT( ok );

    return Array<T,3> (
               (T)p1[0] + pos * (p2[0] - p1[0]),
               (T)p1[1] + pos * (p2[1] - p1[1]),
               (T)p1[2] + pos * (p2[2] - p1[2]) );
}

template<typename T, class Function>
AnalyticalIsoSurface3D<T,Function>* AnalyticalIsoSurface3D<T,Function>::clone() const {
    return new AnalyticalIsoSurface3D<T,Function>(*this);
}

template<typename T, class Function>
std::vector<plint> AnalyticalIsoSurface3D<T,Function>::getSurfaceIds() const {
    std::vector<plint> surfaceIds;
    surfaceIds.push_back(0);
    return surfaceIds;
}

}  // namespace plb

#endif  // MARCHING_CUBE_H

