#ifndef VARIABLES_HH
#define VARIABLES_HH

#include "variables.h"
#include <palabos3D.hh>
#include "myheaders3D.hh"

namespace plb{

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	Variables<T,BoundaryType,SurfaceData,Descriptor>::Variables()
	{
		if(objCount == 0)
		{
			master = global::mpi().isMainProcessor();
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Constructing Variables";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
			#endif
			this->v.reset(this);
			objCount++;
		}
		else
		{
			std::string ex = "Static Class Variables already defined";
			std::string line = std::to_string(__LINE__);
			std::string mesg = "[ERROR]: "+ex+" [FILE:"+__FILE__+",LINE:"+line+"]";
			global::log(mesg);
			throw std::runtime_error(mesg);
		}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	Variables<T,BoundaryType,SurfaceData,Descriptor>::~Variables()
	{
		#ifdef PLB_DEBUG
			std::string mesg = "[DEBUG] Destroying Variables";
			if(master){std::cout << mesg << std::endl;}
			global::log(mesg);
		#endif
		objCount--;
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	void Variables<T,BoundaryType,SurfaceData,Descriptor>::initialize()
	{
		try{
			resolution = 0; gridLevel=0; reynolds=0;
			location = Array<T,3>();
			dx = 1;
			dt = 1;
			iter = 0;
			nprocs = 0;
			nprocs_side = 0;
			master = global::mpi().isMainProcessor();
			nprocs = global::mpi().getSize();
			nprocs_side = (int)cbrt(nprocs);
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	void Variables<T,BoundaryType,SurfaceData,Descriptor>::update(const plint& _gridLevel, const plint& _reynolds)
	{
		try{
			Constants<T>* c = Constants<T>::c.get();
			#ifdef PLB_DEBUG
				std::string mesg ="[DEBUG] Updating Resolution";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
			#endif
			gridLevel = _gridLevel;
			resolution = Constants<T>::referenceResolution * util::twoToThePowerPlint(_gridLevel);
			scaled_u0lb = Constants<T>::u0lb * util::twoToThePowerPlint(_gridLevel);
			if(Constants<T>::test){reynolds = Constants<T>::testRe;}
			else{reynolds = _reynolds;}
			p = IncomprFlowParam<T>(scaled_u0lb,reynolds,resolution,1,1,1);
			dx = p.getDeltaX();
			dt = p.getDeltaT();
			scalingFactor = (T)(resolution)/dx;
			#ifdef PLB_DEBUG
				mesg = "[DEBUG] Resolution="+std::to_string(resolution);
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg = "[DEBUG] Done Updating Resolution";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
			#endif
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	bool Variables<T,BoundaryType,SurfaceData,Descriptor>::checkConvergence()
	{
		try{
			p = IncomprFlowParam<T>(scaled_u0lb,reynolds,resolution,1,1,1);
			util::ValueTracer<T> tracer(p.getLatticeU(), p.getDeltaX(), Constants<T>::epsilon);
			return tracer.hasConverged();
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::unique_ptr<DEFscaledMesh<T> > Variables<T,BoundaryType,SurfaceData,Descriptor>::createMesh(const TriangleSet<T>& triangleSet,
		const plint& referenceDirection, const int& flowType){
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating Mesh";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").start();
			#endif
			// Create Mesh
			std::unique_ptr<DEFscaledMesh<T> > mesh(nullptr);
			mesh.reset(new DEFscaledMesh<T>(triangleSet, resolution, referenceDirection, Constants<T>::margin, Constants<T>::extraLayer));
			#ifdef PLB_DEBUG
				mesg ="[DEBUG] Mesh address= "+adr_string(mesh.get());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg ="[DEBUG] Elapsed Time="+std::to_string(global::timer("boundary").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			return mesh;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::unique_ptr<TriangleBoundary3D<T> > Variables<T,BoundaryType,SurfaceData,Descriptor>::createTB(const DEFscaledMesh<T>& mesh){
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating Triangle Boundary";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			// Create Mesh
			std::unique_ptr<TriangleBoundary3D<T> > triangleBoundary(nullptr);
			triangleBoundary.reset(new TriangleBoundary3D<T>(mesh,true));
			triangleBoundary->getMesh().inflate();
			#ifdef PLB_DEBUG
				mesg ="[DEBUG] TriangleBoundary address= "+adr_string(triangleBoundary.get());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg ="[DEBUG] Elapsed Time="+std::to_string(global::timer("boundary").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			return triangleBoundary;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::shared_ptr<VoxelizedDomain3D<T> > Variables<T,BoundaryType,SurfaceData,Descriptor>::createVoxels(const TriangleBoundary3D<T>& tb,
		const int& flowType)
	{
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating Voxelized Domain";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				std::cout << "[DEBUG] TB Address=" << &tb << " FlowType="<<flowType<<" ExtraLayer="<<Constants<T>::extraLayer <<
					" Borderwidth= "<<Constants<T>::borderWidth<<" EnvelopeWidth= "<<Constants<T>::envelopeWidth<<" Blocksize= "<<
					Constants<T>::blockSize<<" GridLevel= "<<gridLevel<<" Dynamic Mesh= "<<Constants<T>::dynamicMesh<<std::endl;
				global::timer("boundary").restart();
			#endif
			std::shared_ptr<VoxelizedDomain3D<T> > voxelizedDomain(nullptr);
			voxelizedDomain.reset(
				new VoxelizedDomain3D<T>(
					tb,
					flowType,
					Constants<T>::extraLayer,
					Constants<T>::borderWidth,
					Constants<T>::envelopeWidth,
					Constants<T>::blockSize,
					gridLevel,
					Constants<T>::dynamicMesh));
			#ifdef PLB_DEBUG
				mesg ="[DEBUG] VoxelizedDomain  address= "+adr_string(voxelizedDomain.get());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg ="[DEBUG] Elapsed Time="+std::to_string(global::timer("boundary").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			return voxelizedDomain;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::shared_ptr<MultiBlockLattice3D<T,Descriptor> > Variables<T,BoundaryType,SurfaceData,Descriptor>::createLattice(
		const VoxelizedDomain3D<T>& voxelizedDomain)
	{
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating Partial Lattice";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif

			std::shared_ptr<MultiBlockLattice3D<T,Descriptor> > partial_lattice(nullptr);
			/*
			partial_lattice.reset(
				generateMultiBlockLattice<T,Descriptor>(
					voxelizedDomain.getVoxelMatrix(),
					Constants<T>::envelopeWidth,
					new IncBGKdynamics<T,Descriptor>(p.getOmega())
					)
				);
			*/

			#ifdef PLB_DEBUG
				mesg ="[DEBUG] Partial Lattice address= "+adr_string(partial_lattice.get());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg ="[DEBUG] Elapsed Time="+std::to_string(global::timer("boundary").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			return partial_lattice;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::unique_ptr<BoundaryProfiles3D<T,SurfaceData> > Variables<T,BoundaryType,SurfaceData,Descriptor>::createBP()
	{
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating Boundary Profile";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif

			std::unique_ptr<BoundaryProfiles3D<T,SurfaceData> > profile(nullptr);
			profile.reset(new BoundaryProfiles3D<T,SurfaceData>());

			#ifdef PLB_DEBUG
				mesg ="[DEBUG] Boundary Profile address= "+adr_string(profile.get());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg ="[DEBUG] Elapsed Time="+std::to_string(global::timer("boundary").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			return profile;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::unique_ptr<TriangleFlowShape3D<T,SurfaceData> > Variables<T,BoundaryType,SurfaceData,Descriptor>::createFS(
		const VoxelizedDomain3D<T>& voxelizedDomain, const BoundaryProfiles3D<T,SurfaceData>& profile)
	{
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating Triangle Flow Shape";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif

			std::unique_ptr<TriangleFlowShape3D<T,SurfaceData> > flowShape(nullptr);
			flowShape.reset(
				new TriangleFlowShape3D<T,SurfaceData>(
					voxelizedDomain.getBoundary(),
					profile)
			);

			#ifdef PLB_DEBUG
				mesg ="[DEBUG] Triangle Flow Shape address= "+adr_string(flowShape.get());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg ="[DEBUG] Elapsed Time="+std::to_string(global::timer("boundary").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			return flowShape;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::unique_ptr<GuoOffLatticeModel3D<T,Descriptor> > Variables<T,BoundaryType,SurfaceData,Descriptor>::createModel(
		TriangleFlowShape3D<T,SurfaceData>* flowShape, const int& flowType)
	{
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating Model";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif

			std::unique_ptr<GuoOffLatticeModel3D<T,Descriptor> > model(nullptr);
			model.reset(
				new GuoOffLatticeModel3D<T,Descriptor>(
					flowShape,
					flowType)
			);

			#ifdef PLB_DEBUG
				mesg ="[DEBUG] Model address= "+adr_string(model.get());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg ="[DEBUG] Elapsed Time="+std::to_string(global::timer("boundary").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			return model;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::unique_ptr<OffLatticeBoundaryCondition3D<T,Descriptor,BoundaryType> > Variables<T,BoundaryType,SurfaceData,Descriptor>::createBC(
		GuoOffLatticeModel3D<T,Descriptor>* model, VoxelizedDomain3D<T>& voxelizedDomain, MultiBlockLattice3D<T,Descriptor>& lt)
	{
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating BoundaryCondition";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif

			std::unique_ptr<OffLatticeBoundaryCondition3D<T,Descriptor,BoundaryType> > boundaryCondition(nullptr);
			boundaryCondition.reset(
				new OffLatticeBoundaryCondition3D<T,Descriptor,BoundaryType>(
					model,
					voxelizedDomain,
					lt)
			);

			#ifdef PLB_DEBUG
				mesg ="[DEBUG] BoundaryCondition address= "+adr_string(boundaryCondition.get());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				mesg ="[DEBUG] Elapsed Time="+std::to_string(global::timer("boundary").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("boundary").restart();
			#endif
			return boundaryCondition;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::shared_ptr<MultiBlockLattice3D<T,Descriptor> > Variables<T,BoundaryType,SurfaceData,Descriptor>::makeParallel(
		std::shared_ptr<plb::MultiBlockLattice3D<T,Descriptor> > lt)
	{
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Parallelizing Lattice ";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("parallel").start();
			#endif
			Box3D box = lt->getBoundingBox();
			std::map<plint, BlockLattice3D< T, Descriptor > * > blockMap = lt->getBlockLattices();
			plint size = blockMap.size();
			std::vector< std::vector<Box3D> > domains;
			domains.resize(size);

			for(int i=0; i<size; i++){
				std::vector<Box3D> block;
				plint nx = blockMap[i]->getNx()-1;
				plint ny = blockMap[i]->getNy()-1;
				plint nz = blockMap[i]->getNz()-1;
				for(int x = 0; x<nx; x++){
					for(int y = 0; y<nx; y++){
						for(int z=0; z<nz; z++){
							Box3D cell(x,x+1,y,y+1,z,z+1);
							block.push_back(cell);
						}
					}
				}
				domains.push_back(block);
			}
			ParallellizeByCubes3D parallel(domains, box, nprocs_side, nprocs_side, nprocs_side);
			parallel.parallelize();
			#ifdef PLB_DEBUG
				mesg = "[DEBUG] Done Parallelizing Lattice time="+std::to_string(global::timer("parallel").getTime());
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
				global::timer("parallel").stop();
			#endif
			return lt;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::shared_ptr<MultiBlockLattice3D<T,Descriptor> > Variables<T,BoundaryType,SurfaceData,Descriptor>::getLattice()
	{
		try{
			#ifdef PLB_DEBUG
				std::string mesg = "[DEBUG] Creating Lattices";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
			#endif

			Wall<T,BoundaryType,SurfaceData,Descriptor>::mesh = createMesh(Wall<T,BoundaryType,SurfaceData,Descriptor>::triangleSet,
				Wall<T,BoundaryType,SurfaceData,Descriptor>::referenceDirection, Wall<T,BoundaryType,SurfaceData,Descriptor>::flowType);

			Wall<T,BoundaryType,SurfaceData,Descriptor>::tb = createTB(*Wall<T,BoundaryType,SurfaceData,Descriptor>::mesh);
			Wall<T,BoundaryType,SurfaceData,Descriptor>::location = Wall<T,BoundaryType,SurfaceData,Descriptor>::tb->getPhysicalLocation();

			Wall<T,BoundaryType,SurfaceData,Descriptor>::vd = createVoxels(*Wall<T,BoundaryType,SurfaceData,Descriptor>::tb,
				Wall<T,BoundaryType,SurfaceData,Descriptor>::flowType);

			Wall<T,BoundaryType,SurfaceData,Descriptor>::lattice = createLattice(*Wall<T,BoundaryType,SurfaceData,Descriptor>::vd);

			Wall<T,BoundaryType,SurfaceData,Descriptor>::bp = createBP();

			Wall<T,BoundaryType,SurfaceData,Descriptor>::fs = createFS(*Wall<T,BoundaryType,SurfaceData,Descriptor>::vd,
				*Wall<T,BoundaryType,SurfaceData,Descriptor>::bp);

			Wall<T,BoundaryType,SurfaceData,Descriptor>::model = createModel(Wall<T,BoundaryType,SurfaceData,Descriptor>::fs.get(),
				Wall<T,BoundaryType,SurfaceData,Descriptor>::flowType);

			Wall<T,BoundaryType,SurfaceData,Descriptor>::bc = createBC(Wall<T,BoundaryType,SurfaceData,Descriptor>::model.get(),
				*Wall<T,BoundaryType,SurfaceData,Descriptor>::vd, *Wall<T,BoundaryType,SurfaceData,Descriptor>::lattice);

			Wall<T,BoundaryType,SurfaceData,Descriptor>::lattice->toggleInternalStatistics(false);

			//Wall<T,BoundaryType,SurfaceData,Descriptor>::lattice = makeParallel(*lattice);

			#ifdef PLB_DEBUG
				mesg = "[DEBUG] Done Creating Lattices";
				if(master){std::cout << mesg << std::endl;}
				global::log(mesg);
			#endif
			return Wall<T,BoundaryType,SurfaceData,Descriptor>::lattice;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

	template<typename T, class BoundaryType, class SurfaceData, template<class U> class Descriptor>
	std::shared_ptr<MultiBlockLattice3D<T,Descriptor> > Variables<T,BoundaryType,SurfaceData,Descriptor>::saveFields(
		std::shared_ptr<plb::MultiBlockLattice3D<T,Descriptor> > lt)
	{
		try{
			if(iter % p.nStep(Constants<T>::imageSave) == 0){
				lt->toggleInternalStatistics(true);
				boundingBox = lt->getMultiBlockManagement().getBoundingBox();
				MultiTensorField3D<T,3> v(boundingBox.getNx(), boundingBox.getNy(), boundingBox.getNz());
				computeVelocity(*lt, v, boundingBox);
				velocity.push_back(v);
				lt->toggleInternalStatistics(false);
			}
			Obstacle<T,BoundaryType,Descriptor>::o->move();
			return lt;
		}
		catch(const std::exception& e){exHandler(e,__FILE__,__FUNCTION__,__LINE__);}
	}

} // namespace plb

#endif // VARIABLES_HH

