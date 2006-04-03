// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AGCOMMUNICATOR_HH
#define DUNE_AGCOMMUNICATOR_HH

// use this define to control if Albert should use the found MPI

#if defined(HAVE_MPI) && defined(ALBERT_USES_MPI)
#include <mpi.h>
#endif

#include <dune/common/dlist.hh>
#include <dune/io/file/grapedataio.hh>

#define _ANSI_HEADER

#if defined(HAVE_MPI) && defined(ALBERT_USES_MPI)
#include <dune/grid/bsgrid/systemincludes.hh>
namespace BernhardSchuppGrid {
#include <dune/grid/bsgrid/serialize.h>
  //#include <dune/grid/bsgrid/mpAccess_MPI.h>
#include <dune/grid/bsgrid/mpAccess_MPI.cc>
#include "loadbalancer.cc"
}
typedef BernhardSchuppGrid :: ObjectStream ObjectStreamType;
typedef BernhardSchuppGrid :: ObjectStream AlbertaObjectStream;
#endif

#include <map>

namespace Dune {


  enum ObjectId { BEGINELEMENT = -665 , ENDOFSTREAM = -666 , REFINEEL = 1 , STOPHERE = 0 };

  static const int COMMUNICATOR_COMM_TAG = 457;

  /*!
     ProcListElement describes the link between two processors.
     It contains all needed information for cummunication between
     these two procs.
   */

  template <class DofManagerType>
  class CommunicatorInterface
  {
  public:
    virtual bool firstMark() = 0;
    virtual bool secondMark() = 0;
    virtual bool thirdMark() = 0;

    virtual bool markFirstLevel() = 0;
    virtual bool markNextLevel () = 0;

    virtual bool xtractData (DofManagerType & dm) = 0;

    virtual bool repartition (DofManagerType & dm) = 0;
    virtual bool communicate (DofManagerType & dm) = 0;
    virtual bool consistencyGhosts () = 0;
  };

#if defined(HAVE_MPI) && defined(ALBERT_USES_MPI)
  typedef BernhardSchuppGrid :: ObjectStream ObjectStreamType;
  typedef BernhardSchuppGrid :: LoadBalancer LoadBalancer;
  typedef BernhardSchuppGrid :: MpAccessMPI MpAccessMPI;
  template <class T>
  struct MPIType
  {
    enum { MpiType = MPI_BYTE };
  };

  template <> struct MPIType<double> { enum { MpiType = MPI_DOUBLE }; };
  template <> struct MPIType<int>    { enum { MpiType = MPI_INT    }; };

  static int cycle_ = 0;

  template <class GridType, class DofManagerType>
  class AlbertGridCommunicator : public CommunicatorInterface<DofManagerType>
  {
  public:

    //! Constructor
    AlbertGridCommunicator(MPI_Comm mpiComm, GridType &grid, int mySize)
      : grid_ (grid) , myRank_ (grid.myRank() ) , _ldbOver(1.2) , _ldbUnder(0.0)
        , mpAccess_ ( mpiComm )
        , elmap_ (mySize)
        , elmap2_(mySize)
        , osv_ (0), interiorEls_ (0) , ghostEls_(0)
        , pSize_(-1)
    {
      int test = MPI_Comm_dup (mpiComm, &_mpiComm) ;
      assert (test == MPI_SUCCESS) ;

      int i ;
      test = MPI_Comm_size (_mpiComm, & i) ;
      assert (test == MPI_SUCCESS) ;
      const_cast< int & > (pSize_) = i ;
      createLinkage();
      //sprintf(name,"data_%d/grid",myRank_);
      sprintf(name,"data/p_%d/grid",myRank_);
      cyc2 = -1;
    }

    char name [1024];

    void createLinkage ()
    {
      mpAccess_.removeLinkage();
      std::set < int, std::less < int > > s ;
      secondScan (s);
      mpAccess_.insertRequestSymetric ( s );
      //std::cout << "Proc " <<myRank_ << " =";
      mpAccess_.printLinkage(std::cout);
    }

    template <class EntityType>
    int unmarkAllChildren ( EntityType & en )
    {
      typedef typename EntityType :: Traits :: HierarchicIterator HierIt;
      int mxl = grid_.maxlevel();
      if(en.isLeaf()) return;

      en.mark( -1 );
      HierIt endit = en.hend( mxl );
      for(HierIt it = en.hbegin( mxl ); it != endit ; ++it )
      {
        it->mark( -1 );
      }
    }

    typedef std :: map < int , int > OlderElsMap;
    std::vector < OlderElsMap > * interiorEls_;
    std::vector < OlderElsMap > * ghostEls_;


    std::vector < std:: map < int , ObjectStreamType > > elmap_;
    std::vector < std:: map < int , std::map < int , int > > > elmap2_;

    void xtractRefinementTree ()
    {
      const int nl = mpAccess_.nlinks();

      assert(osv_);
      assert(osv_->size() == nl);

      for(int link=0; link<nl; link++)
      {

        elmap_[link].clear();
        elmap2_[link].clear();

        std:: map < int , ObjectStreamType > & elmap = elmap_[link];
        typedef std::map < int , ObjectStreamType > LevelMap;

        ObjectStreamType & os = (*osv_)[link];

        int buff;
        int newmxl = 0;
        os.readObject( buff );
        //std::cout << buff << " Read buff \n";
        if(buff == ENDOFSTREAM ) return ;
        else
        {
          assert(buff == BEGINELEMENT );
          while( buff == BEGINELEMENT )
          {
            os.readObject( buff ); // read elnum
            int elnum = buff;
            //std::cout << "Unpack el = " << elnum << "\n";
            os.readObject(buff); // read refine info
            if(buff == BEGINELEMENT ) continue;
            if(buff == ENDOFSTREAM  ) break;
            if(buff == 1) // means that macro element has children
            {
              //std::cout << "Read level info = " << buff << "\n";
              if(elmap.find(elnum) == elmap.end())
              {
                ObjectStreamType elstr;
                elmap[elnum] = elstr;
              }
              ObjectStreamType & elstr = elmap[elnum];

              os.readObject(buff); // read level
              while((buff != ENDOFSTREAM) && (buff != BEGINELEMENT ))
              {
                if(buff < 0) newmxl = std::max( newmxl, std::abs( buff ));
                elstr.writeObject( buff );
                os.readObject( buff );
              }
            }
          }
        }
        oldmxl_ = std::max( newmxl ,oldmxl_ );
      }
    }

    bool markFirstLevel ()
    {
      bool marked = false;
      const int nl = mpAccess_.nlinks();
      for(int link = 0; link<nl; link++)
      {
        typedef std :: map < int , int > HierMap ;
        std:: map < int , std::map < int , int > > & elmap2 = elmap2_[link];
        std:: map < int , ObjectStreamType > & elmap = elmap_[link];
        typedef std::map < int , ObjectStreamType > LevelMap;
        {
          // now refine grid
          typedef typename GridType :: template Codim<0>::LevelIterator LevelIteratorType;
          LevelIteratorType endit  = grid_.template lend<0>  (0);
          for(LevelIteratorType it = grid_.template lbegin<0>(0);
              it != endit ; ++it )
          {
            int id = it->globalIndex();
            if(elmap.find(id) != elmap.end())
            {
              std::map < int , int > hiertree;
              elmap2[id] = hiertree;
              marked = true;
              //std::cout << "On p=" << myRank_ << "mark El= " << id << "\n";
              if(it->isLeaf())
              {
                grid_.mark(1,(*it));
              }
            }
          }
        }
      }
      nextlevel_ = 1;
      return marked;
    }

    bool markNextLevel ()
    {
      if(nextlevel_ > oldmxl_) return false;
      bool marked = false;
      const int nl = mpAccess_.nlinks();
      typedef std :: map < int , int > HierMap ;
      for(int link=0; link < nl; link++)
      {
        typedef std :: map < int , int > HierMap ;
        std:: map < int , std::map < int , int > > & elmap2 = elmap2_[link];
        std:: map < int , ObjectStreamType > & elmap = elmap_[link];

        // std::cout << "Begin on Level l = " << mxl << "\n";
        // now refine grid
        typedef typename GridType :: template Codim<0>::LevelIterator LevelIteratorType;
        LevelIteratorType endit  = grid_.template lend<0>  (0);
        for(LevelIteratorType it = grid_.template lbegin<0>(0);
            it != endit ; ++it )
        {
          int id = it->globalIndex();
          //std::cout << "Begin LevelIter it = " << id << "\n";
          if(elmap.find(id) != elmap.end())
          {
            int mxl = nextlevel_;
            int buff;
            // build a new hier tree
            ObjectStreamType & levstr = elmap[id];
            try {
              levstr.readObject( buff );
            }
            catch (ObjectStreamType :: EOFException)
            {
              continue;
            }
            assert( buff < 0);
            assert( std::abs( buff ) == mxl );

            HierMap  & hiertree = elmap2[id];
            typedef typename GridType ::template Codim<0> :: Entity :: HierarchicIterator HierIt;

            // Hier muss eine ineinandergeschateltes HierarchiIt kommen.

            typedef typename GridType :: template Codim<0> :: Entity EntityType;
            typedef typename GridType :: template Codim<0> :: EntityPointer EntityPointer;
            hiertree[id] = 1;

            HierIt hendit = it->hend(mxl);
            for(HierIt hit = it->hbegin(mxl); hit != hendit; ++hit)
            {
              if(hit->level() != mxl) continue;
              //std::cout << "Next " << hit->globalIndex() << " on p = " << myRank_ << "\n";
              //std::cout << mxl <<  " " << hit->level() << "\n";
              // if father isnt in tree then we dont do anything here
              EntityPointer vati = hit->father();
              //std::cout << "fathe is " << vati.globalIndex() << "\n";
              if( hiertree.find( vati->globalIndex() ) ==  hiertree.end()) continue;

              int mark = 0;
              try {
                levstr.readObject ( mark );
              }
              catch (ObjectStreamType :: EOFException)
              {
                std::cout << "Read " << mark << " from ObjectStream \n";
                assert(false);
              }

              if(mark == 1)
              {
                //std::cout << "Mark el = "<<hit->globalIndex()<<" on p="<<myRank_  << "\n";
                hiertree[hit->globalIndex()] = mark;
                marked = true;
                if(hit->isLeaf())
                {
                  grid_.mark(1,(*hit));
                }
              }
            }
          }
        }
      }

      nextlevel_ ++ ;
      return marked;
    }

    bool xtractData (DofManagerType & dm)
    {
      dm.resize();

      const int nl = mpAccess_.nlinks();
      assert(osv_);
      assert(osv_->size() == nl);
      for(int link=0; link < nl; link++)
      {
        //std::cout << "Read link " << link << "\n";
        ObjectStreamType & os = (*osv_)[link];
        int id = 0;
        os.readObject( id );
        //std::cout << "first On p=" <<myRank_ << " ident = " << id << "\n";
        while ( id != ENDOFSTREAM )
        {
          typedef std :: map < int , int > HierMap ;
          std:: map < int , std::map < int , int > > & elmap2 = elmap2_[link];
          //std:: map < int , ObjectStreamType > & elmap = elmap_[link];

          // now refine grid
          typedef typename GridType::template Codim<0>::template Partition<Interior_Partition>::LevelIterator LevelIteratorType;
          LevelIteratorType endit  = grid_.template lend<0,Interior_Partition>  (0);
          for(LevelIteratorType it = grid_.template lbegin<0,Interior_Partition>(0);
              it != endit ; ++it )
          {
            if( id == it->globalIndex())
            {
              //std::cout << "Read macro el = " << it->globalIndex() << " on p = " << myRank_ << "\n";
              // read macro data
              dm.gather( os, *it );
              int count =1;

              int mxl = grid_.maxlevel();
              HierMap & hiertree = elmap2[id];
              typedef typename GridType :: template Codim<0> :: Entity EntityType;
              typedef typename GridType :: template Codim<0> :: EntityPointer EntityPointer;

              typedef typename GridType :: template Codim<0> :: Entity :: HierarchicIterator HierIt;
              HierIt hendit  = it->hend(mxl);
              for(HierIt hit = it->hbegin(mxl); hit != hendit; ++hit)
              {
                //if(hit->level() != l) continue;
                EntityPointer vati = hit->father();
                if(hiertree.find(vati->globalIndex()) == hiertree.end()) continue;
                //std::cout << "Read el = " << hit->globalIndex() << " on p = " << myRank_ << "\n";
                dm.gather(os, *hit );
                count ++;
              }
              //std::cout << "On p="<<myRank_ << " read els = " << count << "\n";
            }
          }
          os.readObject( id );
          //std::cout << "On p=" <<myRank_ << " ident = " << id << "\n";
        }
      }

      std::cout << "Done Start xtract DAta \n";

      coarsenNotNeeded();

      for(int l=0; l < elmap_.size(); l++)
      {
        elmap_[l].clear();
        elmap2_[l].clear();
      }
      delete osv_; osv_ = 0;
    }

    void coarsenNotNeeded ()
    {
      std::cout << "Check coarsen on p= " << myRank_ << "\n";
      bool notDone = true;
      int mxl = grid_.maxlevel();
      for(int i=mxl; i > 0 ; i--)
      {
        typedef typename GridType :: LeafIterator LeafIteratorType;
        LeafIteratorType endit  = grid_.leafend  (mxl);
        for(LeafIteratorType it = grid_.leafbegin(mxl);
            it != endit ; ++it )
        {
          if((grid_.owner(*it) != grid_.myRank()) && (it->partitionType() != GhostEntity))
          {
            grid_.mark(-1, (*it));
          }
        }

        notDone = grid_.adapt();
      }
    }

    template <class EntityType>
    int checkRefineStatus ( EntityType & en , PartitionType pitype )
    {
      typedef typename EntityType :: Traits :: HierarchicIterator HierIt;

      // if we don have children, do nothing
      if(en.isLeaf()) return 0;

      int mxl = grid_.maxlevel();
      int count = en.level();
      HierIt endit = en.hend(mxl);
      for(HierIt it = en.hbegin(mxl); it != endit; ++it)
      {
        if(it->partitionType() == pitype)
          count = std::max(count, (*it).level());
      }
      // return deep below entity
      return count - en.level();
    }

    template <class EntityType>
    void writeChildren ( ObjectStreamType & os, EntityType & en)
    {
      typedef typename EntityType :: HierarchicIterator HierIt;
      assert( !en.isLeaf() );

      int mxl = en.level() + 1;
      HierIt endit = en.hend(mxl);
      for(HierIt it = en.hbegin(mxl); it != endit; ++it)
      {
        //if(it->partitionType() != BorderEntity ) continue;
        //std::cout << "write State of " << it->globalIndex() << " on p = "<< myRank_ << "\n";
        os.writeObject( (it->isLeaf()) ? 0 : 1 );
      }
      // return deep below entity
    }

    template <class EntityType>
    void readChildren ( ObjectStreamType & os, EntityType & en )
    {
      typedef typename EntityType ::  HierarchicIterator HierIt;
      assert( !en.isLeaf() );

      int mxl = en.level() + 1;
      HierIt endit = en.hend(mxl);
      for(HierIt it = en.hbegin(mxl); it != endit; ++it)
      {
        //if(it->partitionType() != GhostEntity ) continue;
        int m = 0;
        os.readObject( m );
        assert( m != ENDOFSTREAM );
        if( m > 0 ) grid_.mark(1, (*it));
      }
      // return deep below entity
    }

    template <class EntityType>
    void markChildren ( EntityType & en, int m )
    {
      typedef typename EntityType ::  HierarchicIterator HierIt;
      if(en.isLeaf())
      {
        grid_.mark(1,en);
        return ;
      }

      int mxl = grid_.maxlevel();
      HierIt endit = en.hend(mxl);
      for(HierIt it = en.hbegin(mxl); it != endit; ++it)
      {
        if((it->isLeaf()) && (it->partitionType() == GhostEntity ))
        {
          //std::cout << "on p=" << myRank_ << " mark child " << it->globalIndex() << "\n";
          grid_.mark(1,(*it));
        }
      }
      // return deep below entity
    }

    void unmarkNotOwned()
    {
      int mxl = grid_.maxlevel();
      typedef typename GridType:: LeafIterator LeafIteratorType;
      LeafIteratorType endit  = grid_.leafend  (mxl);
      for(LeafIteratorType it = grid_.leafbegin(mxl);
          it != endit; ++it)
      {
        if(grid_.owner(*it) != grid_.myRank() && (it->partitionType() != GhostEntity))
        {
          grid_.mark(-1,(*it));
        }
      }
    }

    bool firstMark ()
    {
      cycle_++;
      int ts = (cycle_ * 100) + 1;
      GrapeDataIO < GridType > dataIO;
      //dataIO.writeGrid( grid_, xdr , name, (double) ts, ts);

      const int nl = mpAccess_.nlinks();
      std::vector < ObjectStreamType > osv (nl) ;
      std::vector < int > d =  mpAccess_.dest();

      if(interiorEls_) delete interiorEls_;
      if(ghostEls_ ) delete ghostEls_;

      interiorEls_ = new std::vector < OlderElsMap > ( nl );
      ghostEls_    = new std::vector < OlderElsMap > ( nl );

      //unmarkNotOwned();

      int checkmxl =0;
      {
        int mxl = grid_.maxlevel();
        for(int link=0; link<nl; link++)
        {
          OlderElsMap & interiorEls = (*interiorEls_)[link];
          interiorEls.clear();
          int count = 0;
          osv[link].writeObject( mxl );
          {
            //typedef typename GridType::template
            //  LeafIteratorDef<InteriorBorder_Partition>::LeafIteratorType IBLevelIteratorType;
            typedef typename GridType:: LeafIterator IBLevelIteratorType;
            IBLevelIteratorType endit  = grid_.template leafend<InteriorBorder_Partition>  ( mxl, d[link] );
            for(IBLevelIteratorType it = grid_.template leafbegin<InteriorBorder_Partition>( mxl, d[link] );
                it != endit; ++it)
            {
              int id = it->globalIndex();
              checkmxl = std::max(checkmxl , it->level());

              int m = std::max ( grid_.getMark(*it) , 0 );
              grid_.mark(m,*it);

              // if element is not marked, check afterwards
              if(interiorEls.find(id) == interiorEls.end()) interiorEls[id] = count;
              count ++ ;

              osv[link].writeObject( m );
            }
          }
        }
      }

      for(int link=0; link<nl; link++)
      {
        osv[link].writeObject( checkmxl );

        OlderElsMap & interiorEls = (*interiorEls_)[link];
        int s = interiorEls.size();
        osv[link].writeObject( s );

        osv[link].writeObject( ENDOFSTREAM );
      }

      osv = mpAccess_.exchange(osv);

      // unpack
      int oldmxl = 0;
      {
        int mxl = grid_.maxlevel();
        //assert(mxl == checkmxl);
        for(int link=0; link<nl; link++)
        {
          osv[link].readObject( oldmxl );
          oldmxl = std::max ( mxl , oldmxl );

          OlderElsMap & ghostEls = (*ghostEls_)[link];
          ghostEls.clear();
          {
            int count = 0;
            typedef typename GridType::LeafIterator GLeafIteratorType;
            //typedef typename GridType::template LeafIteratorDef<Ghost_Partition>::LeafIteratorType GLeafIteratorType;
            GLeafIteratorType endit  = grid_.template leafend<Ghost_Partition> ( mxl, d[link] );
            for(GLeafIteratorType it = grid_.template leafbegin<Ghost_Partition> (mxl, d[link] );
                it != endit; ++it)
            {
              int id = it->globalIndex();
              if(ghostEls.find(id) == ghostEls.end()) ghostEls[id] = count;

              int m = 0;
              osv[link].readObject(m);
              assert(m != ENDOFSTREAM );

              if( m == -1 )
              {
                if(ghostEls.find(-id) == ghostEls.end()) ghostEls[-id] = count;
              }
              m = std::max( m , 0 ); // do not mark for coarsen, first check if coarsen ok
              grid_.mark(m,(*it));
              count ++ ;
            }
          }
        }
      }

      for(int link=0; link<nl; link++)
      {
        OlderElsMap & interiorEls = (*interiorEls_)[link];
        int buff = 0;
        osv[link].readObject( buff );
        osv[link].readObject( buff );
        //assert(buff == interiorEls.size() );
      }

      oldmxl_ = checkmxl;

      return true;
    }

    bool secondMark ()
    {
      int ts = (cycle_*100) + 2;
      GrapeDataIO < GridType > dataIO;
      //dataIO.writeGrid( grid_, xdr , name, (double) ts, ts );

      int oldmxl = oldmxl_;

      const int nl = mpAccess_.nlinks();
      std::vector < ObjectStreamType > osv (nl) ;
      std::vector < int > d =  mpAccess_.dest();

      //unmarkNotOwned();

      //std::cout << "Starte secondAdapt on proc = " << grid_.myRank() << " with intEls = " << intEls.size() << "\n";

      {
        int mxl = oldmxl;
        //std::cout << "Iterate over level = " << mxl << "\n";
        for(int link=0; link<nl; link++)
        {
          OlderElsMap & interiorEls = (*interiorEls_)[link];
          OlderElsMap & ghostEls    = (*ghostEls_)   [link];
          for(int l=0; l<=mxl; l++)
          {
            // over all levels
            //typedef typename GridType::template Codim<0>::InteriorBorderLevelIterator IBLevelIteratorType;
            typedef typename GridType::template Codim<0>::template Partition<InteriorBorder_Partition>::LevelIterator IBLevelIteratorType;
            IBLevelIteratorType endit  = grid_.template lend  <0,InteriorBorder_Partition> ( l, d[link] );
            for(IBLevelIteratorType it = grid_.template lbegin<0,InteriorBorder_Partition> ( l, d[link] );
                it != endit; ++it)
            {
              int id = it->globalIndex();
              // if element is not in former leaf list, then go on
              if( interiorEls.find(id) == interiorEls.end() ) continue;

              int mak = 0;
              if( ! (it->isLeaf() ) )
              {
                // this means this element was refined, so send 1 to other
                // side
                mak = 1;
              }

              osv[link].writeObject( interiorEls[id] );
              osv[link].writeObject( mak );
            }
          }

          for(int l=0; l<=mxl; l++)
          {
            // over all levels
            typedef typename GridType::template Codim<0>::template Partition<Ghost_Partition>::LevelIterator GLevelIteratorType;
            GLevelIteratorType endit  = grid_.template lend  <0,Ghost_Partition> ( l, d[link] );
            for(GLevelIteratorType it = grid_.template lbegin<0,Ghost_Partition> ( l, d[link] );
                it != endit; ++it)
            {
              int id = it->globalIndex();

              // if element is not in former leaf list, then go on
              if( ghostEls.find(id) == ghostEls.end() ) continue;

              int mak = 0;
              if( !(it->isLeaf()) )
              {
                mak = 1;
                //std::cout << it->globalIndex() << " write of entity = " << mak << " on p =" <<myRank_ << "\n";
              }

              osv[link].writeObject( ghostEls[id] );
              osv[link].writeObject( mak );
            }
          }
        }
      }

      //std::cout << "SecondAD " << intEls.size () << " " << count << " InteriorBorder Els on p=" << myRank_ << "\n";

      for(int link=0; link<nl; link++)
      {
        osv[link].writeObject( ENDOFSTREAM );
      }

      osv = mpAccess_.exchange(osv);

      std::vector < std::vector < int > > markerIB (nl);
      std::vector < std::vector < int > > markerGH (nl);
      for(int l=0; l<nl; l++)
      {
        OlderElsMap & interiorEls = (*interiorEls_)[l];
        OlderElsMap & ghostEls    = (*ghostEls_)[l];
        markerIB[l].resize(interiorEls.size());
        markerGH[l].resize(ghostEls.size());
        for(int i=0; i<markerIB[l].size(); i++) markerIB[l][i] = -2;
        for(int i=0; i<markerGH[l].size(); i++) markerGH[l][i] = -2;
      }

      for(int link=0; link<nl; link++)
      {
        int buff;
        osv[link].readObject( buff );
        while (buff != ENDOFSTREAM )
        {
          int id = buff;
          osv[link].readObject( buff );
          assert( buff != ENDOFSTREAM );
          if(markerGH[link][id] == -2) markerGH[link][id] = buff;
          else markerIB[link][id] = buff;
          osv[link].readObject( buff );
        }
      }

      {
        int mxl = oldmxl;
        for(int link=0; link<nl; link++)
        {
          OlderElsMap & interiorEls = (*interiorEls_)[link];
          OlderElsMap & ghostEls    = (*ghostEls_)[link];
          for(int l=0; l<=mxl; l++)
          {
            typedef typename GridType::template Codim<0>::template Partition<Ghost_Partition>::LevelIterator GLevelIteratorType;
            //typedef typename GridType::template LeafIteratorDef<Ghost_Partition>::LeafIteratorType GLevelIteratorType;
            GLevelIteratorType endit  = grid_.template lend  <0,Ghost_Partition>(l, d[link] );
            for(GLevelIteratorType it = grid_.template lbegin<0,Ghost_Partition>(l, d[link] );
                it != endit; ++it)
            {
              int id = it->globalIndex();
              if(ghostEls.find(id) == ghostEls.end()) continue;

              int m = markerGH[link][ghostEls[id]];
              if((m<=0) && (it->isLeaf()))
              {
                ghostEls.erase( id );
                continue;
              }
              else
              {
                //if(it->isLeaf())
                {
                  //std::cout << "Mark el =" << it->globalIndex() << " on p = "<< myRank_ << "\n";
                  grid_.mark(m,(*it));
                }
              }

              //std::cout << count2 << " on el \n";
            }
          }

          for(int l=0; l<=mxl; l++)
          {

            typedef typename GridType::template Codim<0>::template Partition<InteriorBorder_Partition>::LevelIterator IBLevelIteratorType;
            //typedef typename GridType::template LeafIteratorDef<Ghost_Partition>::LeafIteratorType GLevelIteratorType;
            IBLevelIteratorType endit  = grid_.template lend  <0,InteriorBorder_Partition>(l, d[link] );
            for(IBLevelIteratorType it = grid_.template lbegin<0,InteriorBorder_Partition>(l, d[link] );
                it != endit; ++it)
            {
              int id = it->globalIndex();
              if(interiorEls.find(id) == interiorEls.end()) continue;

              int m = markerIB[link][interiorEls[id]];
              if((m<=0) && (it->isLeaf()))
              {
                interiorEls.erase( id );
                continue;
              }
              else
              {
                grid_.mark(m,(*it));
              }
            }
          }
        }
      }

      return true;
    }

    bool thirdMark ()
    {
      int ts = (cycle_*100) + 3;
      GrapeDataIO < GridType > dataIO;
      //dataIO.writeGrid( grid_, xdr , name, (double) ts, ts );

      int oldmxl = oldmxl_;
      const int nl = mpAccess_.nlinks();
      std::vector < ObjectStreamType > osv (nl) ;
      std::vector < int > d =  mpAccess_.dest();

      //std::cout << "Starte thirdAdapt on proc = " << grid_.myRank() << " with intEls = " << intEls.size() << "\n";

      int count = 0;
      {
        int mxl = oldmxl;
        //std::cout << "Iterate over level = " << mxl << "\n";
        for(int link=0; link<nl; link++)
        {
          OlderElsMap & interiorEls = (*interiorEls_)[link];
          OlderElsMap & ghostEls    = (*ghostEls_)[link];
          for(int l=0; l<=mxl; l++)
          {
            // over all levels
            typedef typename GridType::template Codim<0>::template Partition<InteriorBorder_Partition>::LevelIterator IBLevelIteratorType;
            IBLevelIteratorType endit  = grid_.template lend  <0,InteriorBorder_Partition> ( l, d[link] );
            for(IBLevelIteratorType it = grid_.template lbegin<0,InteriorBorder_Partition> ( l, d[link] );
                it != endit; ++it)
            {
              int id = it->globalIndex();

              // if element is not in former leaf list, then go on
              if( interiorEls.find(id) == interiorEls.end()) continue;

              writeChildren ( osv[link] , *it );
              count++;
            }
          }
        }
      }

      //std::cout << "SecondAD " << intEls.size () << " " << count << " InteriorBorder Els on p=" << myRank_ << "\n";

      for(int link=0; link<nl; link++)
        osv[link].writeObject( ENDOFSTREAM );

      osv = mpAccess_.exchange(osv);

      // unpack
      int count2 = 0;
      {
        int mxl = oldmxl;
        for(int link=0; link<nl; link++)
        {
          OlderElsMap & interiorEls = (*interiorEls_)[link];
          OlderElsMap & ghostEls    = (*ghostEls_)[link];
          for(int l=0; l<=mxl; l++)
          {
            typedef typename GridType::template Codim<0>::template Partition<Ghost_Partition>::LevelIterator GLevelIteratorType;
            GLevelIteratorType endit  = grid_.template lend  <0,Ghost_Partition>(l, d[link] );
            for(GLevelIteratorType it = grid_.template lbegin<0,Ghost_Partition>(l, d[link] );
                it != endit; ++it)
            {
              int id = it->globalIndex();
              if(ghostEls.find(id) == ghostEls.end()) continue;

              //ghostEls.erase( id );
              readChildren( osv[link], *it );
            }
          }
        }
      }

      return true;
    }

    int cyc2;
    //! send and recive DiscreteFunction
    bool communicate( DofManagerType & dm)
    {
      if(cyc2 == cycle_)
      {
        cycle_++;
      }
      cyc2 = cycle_;
      int ts = (cycle_*100) + 4;
      GrapeDataIO < GridType > dataIO;
      //dataIO.writeGrid( grid_, xdr , name, (double) ts, ts );

      const int nl = mpAccess_.nlinks();
      std::vector < ObjectStreamType > osv (nl) ;
      std::vector < int > d =  mpAccess_.dest();

      // pack
#ifndef NDEBUG
      {
        for(int link=0; link<nl; link++)
        {
          int count = 0;
          typedef typename GridType::LeafIterator IBLevelIteratorType;
          //typedef typename GridType::template LeafIteratorDef<InteriorBorder_Partition>::LeafIteratorType IBLevelIteratorType;
          IBLevelIteratorType endit  = grid_.template leafend<InteriorBorder_Partition>   ( grid_.maxlevel(), d[link] );
          for(IBLevelIteratorType it = grid_.template leafbegin<InteriorBorder_Partition> ( grid_.maxlevel(), d[link] );
              it != endit; ++it)
          {
            count ++ ;
          }
          osv[link].writeObject( count );
        }
      }
#endif

      {
        for(int link=0; link<nl; link++)
        {
          typedef typename GridType::LeafIterator IBLevelIteratorType;
          //typedef typename GridType::template LeafIteratorDef<InteriorBorder_Partition>::LeafIteratorType IBLevelIteratorType;
          IBLevelIteratorType endit  = grid_.template leafend<InteriorBorder_Partition>   ( grid_.maxlevel(), d[link] );
          for(IBLevelIteratorType it = grid_.template leafbegin<InteriorBorder_Partition> ( grid_.maxlevel(), d[link] );
              it != endit; ++it)
          {
            dm.scatter(osv[link],*it);
          }
        }
      }

      osv = mpAccess_.exchange(osv);

      dm.resize();

#ifndef NDEBUG
      // unpack
      {
        for(int link=0; link<nl; link++)
        {
          int s = 0;
          int count = 0;
          osv[link].readObject( s );
          typedef typename GridType::LeafIterator GLevelIteratorType;
          //typedef typename GridType::template LeafIteratorDef<Ghost_Partition>::LeafIteratorType GLevelIteratorType;
          GLevelIteratorType endit  = grid_.template leafend<Ghost_Partition>   ( grid_.maxlevel(), d[link] );
          for(GLevelIteratorType it = grid_.template leafbegin<Ghost_Partition> ( grid_.maxlevel(), d[link] );
              it != endit; ++it)
          {
            count ++ ;
          }
          assert( s == count );
        }
      }
#endif

      {
        for(int link=0; link<nl; link++)
        {
          typedef typename GridType::LeafIterator GLevelIteratorType;
          //typedef typename GridType::template LeafIteratorDef<Ghost_Partition>::LeafIteratorType GLevelIteratorType;
          GLevelIteratorType endit  = grid_.template leafend<Ghost_Partition>   ( grid_.maxlevel(), d[link] );
          for(GLevelIteratorType it = grid_.template leafbegin<Ghost_Partition> ( grid_.maxlevel(), d[link] );
              it != endit; ++it)
          {
            dm.gather( osv[link], *it);
          }
        }
      }
      return true;
    }

    template <class T>
    T globalMax (T val) const
    {
      return mpAccess_.gmax(val);
    }

    template <class T>
    T globalMin (T val) const
    {
      return mpAccess_.gmin(val);
    }

    template <class T>
    T globalSum (T val) const
    {
      return mpAccess_.gsum(val);
    }

    template <class T>
    void globalSum (T *send, int s , T *recv) const
    {
      MPI_Allreduce(send,recv, s, MPIType<T>:: MpiType , MPI_SUM, _mpiComm);
      return ;
    }

  private:
    int psize () const
    {
      return pSize_;
    }

  public:
    template <class EntityType>
    void ldbUpdateVertex ( EntityType & en , LoadBalancer :: DataBase & db )
    {
      int weight = 1; // a least weight 1
      {
        int mxl = grid_.maxlevel();
        typedef typename EntityType :: HierarchicIterator HierIt;
        HierIt endit = en.hend(mxl);
        for(HierIt it = en.hbegin(mxl); it != endit; ++it)
        {
          weight++;
        }
        double center[3] = {0.0,0.0,0.0};
        db.vertexUpdate ( LoadBalancer ::
                          GraphVertex (en.globalIndex(),weight, center) ) ;
      }

      {
        typedef typename EntityType :: IntersectionIterator InterIt;
        InterIt endit = en.iend();
        for(InterIt nit = en.ibegin(); nit != endit; ++nit)
        {
          if(nit.neighbor())
            if(en.globalIndex() < (*nit).globalIndex())
            {
              db.edgeUpdate ( LoadBalancer :: GraphEdge ( en.globalIndex(),
                                                          (*nit).globalIndex(), weight) );
            }
        }
      }
    }

    bool calcRepartition (DofManagerType & dm)
    {
      std::vector <int> procPart (grid_.size(0,0));
      for(int i=0; i<procPart.size() ; i++) procPart[i] = -1;
      LoadBalancer :: DataBase db ;
      // build up loadbalance data base with macro vertices and edges
      {
        typedef typename GridType::template Codim<0>::template Partition<Interior_Partition>::LevelIterator InteriorLevelIterator;
        InteriorLevelIterator endit  = grid_.template lend   <0,Interior_Partition> (0);
        for(InteriorLevelIterator it = grid_.template lbegin <0,Interior_Partition> (0);
            it != endit; ++it )
        {
          ldbUpdateVertex ( *it , db );
        }
      }

      //db.printLoad ();
      bool neu = false ;
      {
        const int np = psize ();
        // Kriterium, wann eine Lastneuverteilung vorzunehmen ist:
        //
        // load  - eigene ElementLast
        // mean  - mittlere ElementLast
        // nload - Lastverh"altnis

        double load = db.accVertexLoad () ;
        std :: vector < double > v (mpAccess_.gcollect (load)) ;
        double mean = std::accumulate (v.begin (), v.end (), 0.0) / double (np) ;

        for (std :: vector < double > :: iterator i = v.begin () ; i != v.end () ; i ++)
          neu |= (*i > mean ? (*i > (_ldbOver * mean) ? true : false) :
                  (*i < (_ldbUnder * mean) ? true : false)) ;

        //std::cout << load << " Load \n";
      }
      int val = (neu) ? 1 : 0;
      int v2  = mpAccess_.gmax(val);
      neu = (v2 == 1) ? true : false;

      if (neu)
      {
        db.repartition( mpAccess_ ,  LoadBalancer :: DataBase :: METIS_PartGraphRecursive );
        {
          int countMyEls = 0;
          int firstEl = 0;
          typedef typename GridType::template Codim<0>::template Partition<Interior_Partition>::LevelIterator InteriorLevelIterator;
          InteriorLevelIterator endit= grid_.template lend  <0,Interior_Partition> (0);
          InteriorLevelIterator it   = grid_.template lbegin<0,Interior_Partition> (0);
          if(it != endit) firstEl = it->globalIndex();
          for(; it != endit; ++it )
          {
            int id = (*it).globalIndex();
            procPart[id] = db.getDestination ( id ) ;
            if(procPart[id] == grid_.myRank()) countMyEls++;
          }
          //assert(countMyEls > 0);
          if(countMyEls == 0) procPart[firstEl] = grid_.myRank();

        }

        //std::cout << "Do repartition of MacroGrid on p= "<<myRank_ <<"\n";
        repartitionMacroGrid( procPart , dm );
        return true;
      }

      return false;
    }

    void  secondScan ( std::set < int, std::less < int > >  & s )
    {
      {
        // fake , all to all linkage at the moment
        {
          std :: vector < int > l (pSize_-1);
          int count =0;
          for(int i=0; i<pSize_; i++)
          {
            if(i!=myRank_)
            {
              l[count] = i;
              count ++;
            }
          }

          for (std::vector < int > :: const_iterator i = l.begin () ; i != l.end () ; s.insert (* i ++)) ;
          int pos = *(s.find(1-myRank_));
          //std::cout << " link = " << pos << "\n";
        }
      }
    }

    std::vector < ObjectStreamType > * osv_ ;

    void repartitionMacroGrid ( std::vector<int> & procPart , DofManagerType & dm )
    {
      for( int l = grid_.maxlevel(); l>0; l-- )
      {
        unmarkNotOwned();
        grid_.adapt();
      }


      std::vector < int > d = mpAccess_.dest();
      const int nlinks = mpAccess_.nlinks ();

      if(osv_) delete osv_;
      osv_ = new std::vector < ObjectStreamType > (nlinks);
      std::vector < ObjectStreamType > & osv = *osv_;

      {
        // write new element owners, write the number only if you are the owner
        {
          typedef typename GridType::template Codim<0> :: LevelIterator LevelIteratorType;
          LevelIteratorType it    = grid_.template lbegin<0> ( 0 );
          LevelIteratorType endit = grid_.template lend<0>   ( 0 );
          for( ; it != endit; ++it)
          {
            for(int p=0; p<nlinks; p++)
            {
              if(grid_.owner(*it) == grid_.myRank())
              {
                osv[p].writeObject( procPart[ (*it).globalIndex()] );
              }
              else
              {
                osv[p].writeObject( -1 );
              }
            }
          }
        }

        {
          // walk over my interior macro elements
          typedef typename GridType::template Codim<0>::template Partition<Interior_Partition>::LevelIterator InteriorLevelIteratorType;
          InteriorLevelIteratorType it    = grid_.template lbegin<0,Interior_Partition> ( 0 );
          InteriorLevelIteratorType endit = grid_.template lend<0,Interior_Partition> ( 0 );
          for( ; it != endit; ++it)
          {
            if(grid_.owner(*it) == grid_.myRank())
            {
              int id = it->globalIndex();
              if(procPart[id] != grid_.myRank() && (procPart[id] != -1))
              {
                //std::cout << grid_.myRank() << " mr|pp " << procPart[id] <<"\n";
                //std::cout << "Pack Element ! = " << id << "\n";
                int newProc = mpAccess_.link ( procPart[id] );
                grid_.packAll ( osv[newProc] , (*it) );
              }
            }
          }
        }

        // set new owners
        for(int p=0; p<nlinks; p++)
        {
          osv[p].writeObject(ENDOFSTREAM); // end stream
        }

        // pack data
        {
          // walk over my interior macro elements
          typedef typename GridType::template Codim<0>::template Partition<Interior_Partition>::LevelIterator InteriorLevelIteratorType;
          InteriorLevelIteratorType it    = grid_.template lbegin<0,Interior_Partition> ( 0 );
          InteriorLevelIteratorType endit = grid_.template lend<0,Interior_Partition> ( 0 );
          for( ; it != endit; ++it)
          {
            if(grid_.owner(*it) == grid_.myRank())
            {
              int id = it->globalIndex();
              if(procPart[id] != grid_.myRank() && (procPart[id] != -1))
              {
                int link = mpAccess_.link ( procPart[id] );
                grid_.partition( procPart[id] , (*it) );

                osv[link].writeObject( id );
                packAllData ( osv[link], dm , *it );
                //dm.inlineData( osv[link], *it );
              }
            }
          }
        }

        for(int p=0; p<nlinks; p++)
        {
          osv[p].writeObject(ENDOFSTREAM); // end stream
        }
      }

      osv = mpAccess_.exchange( osv );

      {
        typedef typename GridType::template Codim<0> :: LevelIterator LevelIteratorType;
        LevelIteratorType it    = grid_.template lbegin<0> ( 0 );
        LevelIteratorType endit = grid_.template lend<0>   ( 0 );
        for( ; it != endit; ++it)
        {
          for(int p=0; p<nlinks; p++)
          {
            int proc = d[p];
            int np = 0 ;
            osv[p].readObject( np );
            //std::cout << myRank_ << "mr | " <<np << "\n";
            //std::cout << it->owner() << "own | " << proc << "\n";

            // the new number comes only from the old owner
            if((grid_.owner(*it) == proc) && (np >= 0))
            {
              grid_.partition ( np , (*it));
            }
          }
        }
      }

      {
        typedef typename GridType::template Codim<0>::template Partition<Interior_Partition>::LevelIterator InteriorLevelIteratorType;
        InteriorLevelIteratorType it    = grid_.template lbegin<0,Interior_Partition> ( 0 );
        InteriorLevelIteratorType endit = grid_.template lend<0,Interior_Partition>   ( 0 );
        for( ; it != endit; ++it)
        {
          // the new number comes only from the old owner
          if((grid_.owner(*it) == grid_.myRank()))
          {
            int id = (*it).globalIndex();
            //std::cout << "Setze part von el = " << id << " auf Proc = " << myRank_ << "\n";
            //std::cout << procPart[id] << " Procpart = \n";
            if(procPart[id] >= 0)
              grid_.partition ( procPart[ id ] , (*it) );
          }
        }
      }

      xtractRefinementTree();
      //ts++;
      //dataIO.writeGrid( grid_, xdr , name, (double) ts, ts );
    }

    template <class EntityType>
    void packAllData ( ObjectStreamType & os, DofManagerType & dm , EntityType & en )
    {
      typedef typename EntityType :: HierarchicIterator HierIt;
      dm.scatter( os , en );
      //std::cout << "Write Macro el = " << en.globalIndex()<<" on p=" <<myRank_ <<"\n";

      int count = 1;
      if(! (en.isLeaf() ))
      {
        //int mxl = en.level() + 1;
        int mxl = grid_.maxlevel();
        HierIt endit  = en.hend(mxl);
        for(HierIt it = en.hbegin(mxl); it != endit; ++it)
        {
          //std::cout << "Pack el = " << it->globalIndex() << " on p=" << myRank_ <<"\n";
          dm.scatter( os, *it );
          count ++;
        }
      }
      //std::cout << "Packed "<<count<< " El on p=" << myRank_ <<"\n";
    }

    bool consistencyGhosts ()
    {
      std::vector < int > d = mpAccess_.dest();
      const int nlinks = mpAccess_.nlinks ();
      std::vector < ObjectStreamType > osv (nlinks) ;

      {
        for(int link=0; link<nlinks; link++)
        {
          // walk over my interior macro elements
          typedef typename GridType::template Codim<0>::template Partition<InteriorBorder_Partition>::LevelIterator IBLevelIteratorType;
          IBLevelIteratorType it    = grid_.template lbegin<0,InteriorBorder_Partition>( 0, d[link] );
          IBLevelIteratorType endit = grid_.template lend  <0,InteriorBorder_Partition>( 0, d[link] );
          for( ; it != endit; ++it)
          {
            grid_.packBorder ( osv[link] , (*it) );
          }
        }
      }

      // set new owners
      for(int p=0; p<nlinks; p++)
      {
        osv[p].writeObject(ENDOFSTREAM); // end stream
      }

      osv = mpAccess_.exchange( osv );

      {
        for(int link=0; link<nlinks; link++)
        {
          grid_.unpackAll ( osv[link] );
        }
      }

      grid_.createGhosts ();
      return true;
    }


    bool consistencyAdapt ()
    {
      assert(false);
      std::vector < int > d = mpAccess_.dest();
      const int nlinks = mpAccess_.nlinks ();
      std::vector < ObjectStreamType > osv (nlinks) ;

      {
        for(int link=0; link<nlinks; link++)
        {
          int mxl = grid_.maxlevel();
          // walk over my interior macro elements
          typedef typename GridType::LeafIterator IBLeafIteratorType;
          IBLeafIteratorType it    = grid_.template leafbegin<InteriorBorder_Partition>( mxl, d[link] );
          IBLeafIteratorType endit = grid_.template leafend  <InteriorBorder_Partition>( mxl, d[link] );
          for( ; it != endit; ++it)
          {
            osv[link].writeObject( it->level() );
          }
        }
      }

      // set new owners
      for(int p=0; p<nlinks; p++)
      {
        osv[p].writeObject(ENDOFSTREAM); // end stream
      }

      {
        for(int link=0; link<nlinks; link++)
        {
          int mxl = grid_.maxlevel();
          // walk over my interior macro elements
          typedef typename GridType::LeafIterator GLeafIteratorType;
          GLeafIteratorType it    = grid_.template leafbegin<Ghost_Partition>( mxl, d[link] );
          GLeafIteratorType endit = grid_.template leafend  <Ghost_Partition>( mxl, d[link] );
          for( ; it != endit; ++it)
          {
            int lev = 0;
            osv[link].readObject( lev );
            assert( lev != ENDOFSTREAM );
            if(lev > it->level()) it->mark(1);
          }
        }
      }

      grid_.adapt();
      return true;
    }

    bool repartition (DofManagerType & dm)
    {
      return calcRepartition (dm);
    }

    // reference to corresponding grid
    GridType & grid_;

    MPI_Comm _mpiComm;

    // rank of my thread
    const int myRank_;
    const int pSize_;

    int oldmxl_;
    int nextlevel_;

    MpAccessMPI mpAccess_;

    double _ldbOver;
    double _ldbUnder;

  };
#else

  class ObjectStream
  {
  public:
    class EOFException {} ;
    template <class T>
    void readObject (T &) {}
    void readObject (int) {}
    void readObject (double) {}
    template <class T>
    void writeObject (T &) {}
    void writeObject (int) {}
    void writeObject (double) {}
  };

  typedef ObjectStream ObjectStreamType;
  typedef ObjectStream AlbertaObjectStream;

  template <class GridType, class DofManagerType>
  class AlbertGridCommunicator : public CommunicatorInterface<DofManagerType>
  {
  public:

#if defined(HAVE_MPI) && defined(ALBERT_USES_MPI)
    AlbertGridCommunicator(MPI_Comm mpiComm, GridType &grid, int anzp) {}
#else
    AlbertGridCommunicator(GridType &grid) {}
#endif

    template <class T>
    T globalMax (T val) const
    {
      return val;
    }

    template <class T>
    T globalMin (T val) const
    {
      return val;
    }

    template <class T>
    T globalSum (T val) const
    {
      return val;
    }

    template <class T>
    void globalSum (T *send, int s , T *recv) const
    {
      std::memcpy(recv,send,s*sizeof(T));
      return ;
    }

    bool loadBalance ( DofManagerType & dm) { return false; }
    void loadBalance ()  { return false; }

    bool firstMark() { return false; }
    bool secondMark() { return false; }
    bool thirdMark() { return false;}

    bool markFirstLevel() { return false; }
    bool markNextLevel () { return false; }

    bool xtractData (DofManagerType & dm) { return false; }

    bool repartition (DofManagerType & dm) { return false; }
    bool communicate (DofManagerType & dm) { return false; }
    bool consistencyGhosts () { return false; }

  };
#endif

  template <class GridType, class CritType>
  void makeParallelGrid (GridType &grid, CritType &crit)
  {
    for(int l=0; l <= grid.maxlevel(); l++)
    {
      typedef typename GridType::template Codim<0>::LevelIterator LevelIteratorType;

      LevelIteratorType it    = grid.template lbegin<0> (l);
      LevelIteratorType endit = grid.template lend  <0> (l);

      for( ; it != endit; ++it )
      {
        crit.classify( *it );
      }
    }
  }

} // end namespace Dune


#endif
