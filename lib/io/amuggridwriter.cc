// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// ///////////////////////////////////////////////
// Specialization of the AmiraMesh writer for SimpleGrid<3,3>
// ///////////////////////////////////////////////

#include "../../grid/uggrid.hh"

#include <amiramesh/AmiraMesh.h>

namespace Dune {

  template<>
  class AmiraMeshWriter<UGGrid<3,3>, double> {

  public:

    static void write(const UGGrid<3,3>& grid,
                      const Array<double>& sol,
                      const std::string& filename);


    AmiraMeshWriter() {}

  };

}

//template<>
void Dune::AmiraMeshWriter<Dune::UGGrid<3,3>, double>::write(const Dune::UGGrid<3,3>& grid,
                                                             const Array<double>& sol,
                                                             const std::string& filename)
{
  printf("This is the AmiraMesh writer for UGGrid<3,3>!\n");

  const int DIM = 3;

  int noOfNodes = 0;  //grid.size(0, 3);

  printf("Ähem\n");
  int fl, tl, k, i, noOfElem, noOfBndTri, MarkKey, ncomp, maxSubDom;
  //UG3d::VECDATA_DESC *sol = NULL;
  UG3d::ELEMENT *t, **elemList;
  UG3d::VECTOR *vec;
  UG3d::NODE* theNode;
  std::string solFilename;


  fl = 0;
  //     tl = TOPLEVEL(theMG);

  // Construct the name for the geometry file
  std::string geoFilename(filename);
  geoFilename += ".am";

#if 0
  /* reset vcflag */
  for (k=fl; k<=tl; k++)
    for (vec=PFIRSTVECTOR(GRID_ON_LEVEL(theMG,k)); vec!= NULL; vec=SUCCVC(vec))
      SETVCFLAG(vec, 0);


  /*count nodes and enumerate nodes */
  noOfNodes = 0;
  for (k=fl; k<=tl; k++)
    for (t=FIRSTELEMENT(GRID_ON_LEVEL(theMG,k)); t!=NULL; t=SUCCE(t))
      if(AmiraElement(t, tl, k))
        if(EnumerateAmiraNodes(t, sol, &noOfNodes))
          return(PARAMERRORCODE);
#endif

#if 0
  /* build up list of all surface elements and count nodes */
  noOfElem = 0;
  maxSubDom = 0;
  for (k=fl; k<=tl; k++)
    for (t=FIRSTELEMENT(GRID_ON_LEVEL(theMG,k)); t!=NULL; t=SUCCE(t))
      if(AmiraElement(t, tl, k))
      {
        noOfElem++;
        maxSubDom = MAX(maxSubDom, SUBDOMAIN(t));
      }

  MarkTmpMem(MGHEAP(theMG), &MarkKey);
  elemList = (ELEMENT **) GetTmpMem(MGHEAP(theMG),  (noOfElem+1)*sizeof(ELEMENT *), MarkKey);
  if(elemList == NULL)
    return(1);


  i = noOfNodes;
  noOfNodes = 0;
  for (k=fl; k<=tl; k++)
    for (vec=PFIRSTVECTOR(GRID_ON_LEVEL(theMG,k)); vec!= NULL; vec=SUCCVC(vec))
    {

      if(!VCFLAG(vec))
        continue;
      VINDEX(vec) = ++noOfNodes;
      if(VOTYPE(vec) != NODEVEC)
      {
        PrintErrorMessage('E',"WriteAmiraGeometry","vector not of type NODEVEC");
        return (PARAMERRORCODE);
      }
    }

  if(FULLREFINELEVEL(theMG) < TOPLEVEL(theMG))
    for (k = tl; k >= MAX(fl, BOTTOMLEVEL(theMG)+1); k--)
      for (theNode=FIRSTNODE(GRID_ON_LEVEL(theMG,k)); theNode!=NULL; theNode=SUCCN(theNode))
      {
        NODE* fatherNode;
        VECTOR *fatherVec;

        vec = NVECTOR(theNode);

        if(vec == NULL)
          continue;

        if(!CORNERTYPE(theNode))
          continue;

        fatherNode =  (NODE*) NFATHER(theNode);

        if(fatherNode == NULL)
          continue;

        fatherVec = (VECTOR*) NVECTOR(fatherNode);

        if(fatherVec == NULL)
        {
          PrintErrorMessage('E',"WriteAmiraGeometry","father node without vector");
          return (PARAMERRORCODE);
        }
        if(VCFLAG(fatherVec))
        {
          PrintErrorMessage('E',"WriteAmiraGeometry","father vec with VCFLAG");
          return (PARAMERRORCODE);
        }
        VINDEX(fatherVec) = VINDEX(vec);
      }

  if(i != noOfNodes)
    PrintErrorMessage('E',"WriteAmira","wrong number of nodes");

  assert(i == noOfNodes);
  /* write elemts to list */

  noOfElem   = 0;
  noOfBndTri = 0;
  for (k=fl; k<=tl; k++)
    for (t=FIRSTELEMENT(GRID_ON_LEVEL(theMG,k)); t!=NULL; t=SUCCE(t))
      if (AmiraElement(t, tl, k))
      {
        elemList[noOfElem++] = t;
#ifndef ModelP
        if (OBJT(t) != BEOBJ)
          continue;
#endif
        for (i=0; i<SIDES_OF_ELEM(t); i++)
          if (ElemSideOnPlotBnd(t, i, NULL) > 0)
            noOfBndTri++;
      }
#endif


  // create amiramesh object
  AmiraMesh am_geometry;

  // write grid vertex coordinates
  AmiraMesh::Location* geo_nodes = new AmiraMesh::Location("Nodes", noOfNodes);
  am_geometry.insert(geo_nodes);

  AmiraMesh::Data* geo_node_data = new AmiraMesh::Data("Coordinates", geo_nodes,
                                                       McPrimType::mc_float, DIM);
  am_geometry.insert(geo_node_data);


  //    i=0;
  //    for (k=fl; k<=tl; k++)
  //        for (vec=FIRSTVECTOR(GRID_ON_LEVEL(theMG,k)); vec!= NULL; vec=SUCCVC(vec))
  //            if ((VOTYPE(vec) == NODEVEC) && ((k==tl) || (VNCLASS(vec)<1) ))
  //                {
  //                    /* write coordinates to geo file */
  //                    DOUBLE pos[DIM];

  //                    if(VectorPosition(vec, pos))
  //                        return(PARAMERRORCODE);

  //                    ((float*)geo_node_data->dataPtr())[i++] = pos[0];
  //                    ((float*)geo_node_data->dataPtr())[i++] = pos[1];
  //                    ((float*)geo_node_data->dataPtr())[i++] = pos[2];

  //                }

  UGGridLevelIterator<3, 3, 3> vertex = grid.lbegin<3>(0);
  printf("level: %d\n", vertex.level());



  UGGridLevelIterator<0, 3, 3> element = grid.lbegin<0>(0);

  for (i=0; element!=grid.lend<0>(0); element++, i++)
    printf("Element! %d\n", i);

#if 0
  // handle materials
  HxParamBundle* materials = new HxParamBundle("Materials");

  for (k=0; k<=maxSubDom; k++) {

    char buffer[100];
    sprintf(buffer, "Material%d", k);
    HxParamBundle* newMaterial = new HxParamBundle(buffer);

    HxParameter* newId = new HxParameter("Id", k);
    newMaterial->insert(newId);

    materials->insert(newMaterial);

  }

  am_geometry.parameters.insert(materials);

  ncomp = 0;
  for(i=0; i<NVECTYPES; i++)
    ncomp = MAX(ncomp,VD_NCMPS_IN_TYPE(sol, i));


  /* write element section to geo - file */
  AmiraMesh::Location* element_loc = new AmiraMesh::Location("Tetrahedra", noOfElem);
  am_geometry.insert(element_loc);

  AmiraMesh::Data* element_data = new AmiraMesh::Data("Nodes", element_loc,
                                                      McPrimType::mc_int32, DIM+1);
  am_geometry.insert(element_data);

  int (*dPtr)[DIM+1] = (int(*)[DIM+1])element_data->dataPtr();

  for(i=0; i<noOfElem; i++)
    if(WriteAmiraGeometry(dPtr[i], elemList[i], sol))
      return(PARAMERRORCODE);

  // write material section to geo-file
  AmiraMesh::Data* element_materials = new AmiraMesh::Data("Materials", element_loc, McPrimType::mc_uint8, 1);
  am_geometry.insert(element_materials);

  for(i=0; i<noOfElem; i++)
    ((unsigned char*)element_materials->dataPtr())[i] = SUBDOMAIN(elemList[i]);

  //    fprintf(geoFile, "\n@%d\n", 4 /* BND_TRIDATA_SECTION */);
  //    for(i=0;i<noOfElem;i++)
  //      if(WriteAmiraBndTriData(geoFile, elemList[i], ncomp))
  //        return(PARAMERRORCODE);

  //    fprintf(geoFile, "\n@%d\n", 5 /* BND_TRI_SECTION */);
  //    for(i=0;i<noOfElem;i++)
  //      if(WriteAmiraBndTri(geoFile, elemList[i]))
  //        return(PARAMERRORCODE);




  //////////////////////////////////////////////////////

  // Now save the solution
  // Construct the name for the solution file

  //strcpy(solFilename, (const char*) BaseName);
#ifdef ModelP
  //strcat(solFilename, procNumber);
#endif
  //strcat(solFilename, ".sol");

  //AmiraMesh am_solution;

  //AmiraMesh::Location* nodes = new AmiraMesh::Location("Nodes", noOfNodes);
  //am_solution.insert(nodes);

  AmiraMesh::Data* nodeData = new AmiraMesh::Data("Data", geo_nodes, McPrimType::mc_float, ncomp);
  am_geometry.insert(nodeData);

  AmiraMesh::Field* nodeField = new AmiraMesh::Field("f", ncomp, McPrimType::mc_float,
                                                     AmiraMesh::t_linear, nodeData);
  am_geometry.insert(nodeField);

  i=0;
  for (k=fl; k<=tl; k++)
    for (vec=FIRSTVECTOR(GRID_ON_LEVEL(theMG,k)); vec!= NULL; vec=SUCCVC(vec))
      if ((VOTYPE(vec) == NODEVEC) && ((k==tl) || (VNCLASS(vec)<1) ))
      {
        SHORT vtype   = VTYPE(vec);
        /** \bug This definition of ncomp shadows another one! */
        SHORT ncomp   = VD_NCMPS_IN_TYPE(sol, vtype);
        SHORT *cmpptr = VD_CMPPTR_OF_TYPE(sol, vtype);
        SHORT j;

        for (j=0; j<ncomp; j++)
          ((float*)nodeData->dataPtr())[ncomp*i+j] = VVALUE(vec, cmpptr[j]);

        i++;
      }

  // actually save the solution file
  //    if (!am_solution.write(solFilename, 1) ) {
  //        printf("An error has occured writing file %s.\n", solFilename);
  //        return PARAMERRORCODE;
  //    }

#endif
  if(!am_geometry.write(geoFilename.c_str(), 1)) {
    printf("writing geometry file failed in amira : \n");
    /** \todo Do a decent error handling */
    return;
  }

  printf("Grid written successfully to:\n %s \n", geoFilename.c_str());
}
