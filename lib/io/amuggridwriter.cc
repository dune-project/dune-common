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

  int noOfNodes = grid.size(0, 3);
  int noOfElem  = grid.size(0, 0);

  printf("noOfNodes %d,  nodeOfElem: %d\n", noOfNodes, noOfElem);
  int fl, tl, k, i, noOfBndTri, MarkKey, ncomp, maxSubDom;
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

  // create amiramesh object
  AmiraMesh am_geometry;

  // write grid vertex coordinates
  AmiraMesh::Location* geo_nodes = new AmiraMesh::Location("Nodes", noOfNodes);
  am_geometry.insert(geo_nodes);

  AmiraMesh::Data* geo_node_data = new AmiraMesh::Data("Coordinates", geo_nodes,
                                                       McPrimType::mc_float, DIM);
  am_geometry.insert(geo_node_data);


  UGGridLevelIterator<3, 3, 3> vertex = grid.lbegin<3>(0);
  i=0;
  for (; vertex!=grid.lend<3>(0); ++vertex)
  {
    //printf("Vertex! %d\n", vertex->index());
    UGGridElement<0,3> vertex_element = vertex->geometry();
    Vec<3, double> coords = vertex_element[0];

    //printf("coords:!! %g %g %g\n", coords(0), coords(1), coords(2));

    ((float*)geo_node_data->dataPtr())[i++] = coords(0);
    ((float*)geo_node_data->dataPtr())[i++] = coords(1);
    ((float*)geo_node_data->dataPtr())[i++] = coords(2);

  }





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
#endif

  /* write element section to geo - file */
  AmiraMesh::Location* element_loc = new AmiraMesh::Location("Tetrahedra", noOfElem);
  am_geometry.insert(element_loc);

  AmiraMesh::Data* element_data = new AmiraMesh::Data("Nodes", element_loc,
                                                      McPrimType::mc_int32, 4);
  am_geometry.insert(element_data);

  int (*dPtr)[4] = (int(*)[4])element_data->dataPtr();

  //    for(i=0; i<noOfElem; i++)
  //        if(WriteAmiraGeometry(dPtr[i], elemList[i], sol))
  //            return(PARAMERRORCODE);

  UGGridLevelIterator<0, 3, 3> element = grid.lbegin<0>(0);

  for (i=0; element!=grid.lend<0>(0); ++element, i++) {
    //        printf("Element! %d, %d number of corners\n", element->index(),
    //               element->geometry().corners());
    //        printf("corners:  %d %d %d %d\n", element->subIndex(0), element->subIndex(1),
    //               element->subIndex(2), element->subIndex(3));

    for (int j=0; j<4; j++)
      dPtr[i][j] = element->subIndex(j)+1;
  }

  // write material section to geo-file
  AmiraMesh::Data* element_materials = new AmiraMesh::Data("Materials", element_loc, McPrimType::mc_uint8, 1);
  am_geometry.insert(element_materials);

  //    for(i=0; i<noOfElem; i++)
  //        ((unsigned char*)element_materials->dataPtr())[i] = SUBDOMAIN(elemList[i]);

  for(i=0; i<noOfElem; i++)
    ((unsigned char*)element_materials->dataPtr())[i] = 0;


  //////////////////////////////////////////////////////

  // Now save the solution
#if 0
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
