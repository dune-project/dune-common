// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$
#ifndef DUNE_AMG_LINK_HH
#define DUNE_AMG_LINK_HH

namespace Dune
{
  namespace amg
  {
    /**
     * @addtogroup ISTL_PAAMG
     *
     * @{
     * @file
     * @author Markus Blatt
     * @brief Provides class representing the attributes of
     * a matrix graph link.
     */

    /**
     * @brief Class representing a link in the matrix graph.
     *
     * Contains methods for getting and setting link attributes.
     */
    class Link
    {
    private:
      /** @brief Flags of the link.*/
      char flags_;
    public:
      /** @brief Constructor. */
      Link();
      /**
       * @brief Checks wether the node the link points to depends on
       * the node the link starts.
       * @return True if it depends on the starting point.
       */
      bool depends();

      /**
       * @brief Marks the link as one of which the end point depends on
       * the starting point.
       */
      void setDepends();

      /**
       * @brief Resets the depends flag.
       */
      void resetDepends();

      /**
       * @brief Checks wether the start node is influenced by the end node.
       * @return True if it is influenced.
       */
      bool influences();

      /**
       * @brief Marks the link as one of which the start node by the end node.
       */
      void setInfluences();

      /**
       * @brief Resets the influence flag.
       */
      void resetInfluences();

      /**
       * @brief Checks wether the link is one way.
       * I.e. either the influence or the depends flag but is set.
       */
      bool isOneWay();

      /**
       * @brief Checks wether the link is two way.
       * I.e. both the influence flag and the depends flag are that.
       */
      bool isTwoWay();

      /**
       * @brief Checks wether the link is strong.
       * I.e. the influence or depends flag is set.
       */
      bool isStrong();

      /**
       * @brief Prints the attributes of the link for debugging.
       */
      void printFlags();
    };


    /**
     * @brief All the links of a matrix graph.
     */
    class Links
    {

      struct NodeLinks
      {
        Link* first_;
      };

    public:
      typedef const Link* const_iterator;
      typedef Link* iterator;

      class BuildIterator
      {
      public:
        /** @brief Constructor. */
        BuildIterator(Links& links)
          : links_(links), currentNode_(0), noLinks_(0)
        {

          if(links_.nodes_>=0)
            links_.start_[currentNode_]=noLinks_;
        }
        /**
         * @brief Save Link information for the current node and inkrement iterator.
         */
        BuildIterator& operator++()
        {
          ++currentNode_;

          if(currentNode_<links_.nodes_)
            links_.start_[currentNode_]=noLinks_;
          else{
            links_.start_[currentNode_]=noLinks_;
            links_.links_ = new Link[noLinks_];
            links_.linksBuilt_ = true;
          }
          return *this;
        }

        /**
         * @brief Set the number of links for the current node.
         * @param links The number of links.
         */
        void setNoLinks(int links)
        {
          noLinks_+=links;
        }

      private:
        /** @brief The links we build. */
        Links& links_;
        /** @brief The current node. */
        int currentNode_;
        /** @brief The total number of links. */
        int noLinks_;
      };

      /**
       * @brief Free allocated memory.
       */
      void free();
      /**
       * @brief Reserve space for links.
       * @param nodes The number of nodes the graph contains.
       */
      void reserveNodes(int nodes);

      /**
       * @brief Get an iterator for building the link structure.
       * @return An Iterator for building then link structure.
       */
      BuildIterator begin();

      iterator operator[](int i);

      const_iterator operator[](int i) const;
      /**
       * @brief Constructor.
       */
      Links();

      /**
       * @brief Destructor.
       */
      ~Links();

    private:
      /** @brief The indices of the first link for each node.*/
      int* start_;
      /** @brief All links ordered by source nodes. */
      Link* links_;
      /** @brief True if the links array is allocated. */
      bool linksBuilt_;
      /** @brief True if the start array is allocated. */
      bool startBuilt_;
      /** @brief The number of nodes of the graph. */
      int nodes_;

    };

    inline Link::Link()
      : flags_(0)
    {}

    inline void Link::setInfluences()
    {
      (flags_) = (flags_) | (0x02);
    }

    inline bool Link::influences()
    {
      return (flags_) & (0x02);
    }

    inline void Link::setDepends()
    {
      (flags_) = (flags_) | (0x01);
    }

    inline void Link::resetDepends()
    {
      (flags_) = (flags_) & (0xFE);
    }

    inline bool Link::depends()
    {
      return (flags_) & (0x01);
    }

    inline void Link::resetInfluences()
    {
      (flags_) = (flags_) & (0xFD);
    }

    inline bool Link::isOneWay()
    {
      return ((flags_)&0x03)==1;
    }

    inline bool Link::isTwoWay()
    {
      return ((flags_)&0x03)==3;
    }

    inline bool Link::isStrong()
    {
      return ((flags_) | 0x03);
    }

    inline Links::Links()
      : start_(), links_(), linksBuilt_(false), startBuilt_(false), nodes_(-1)
    {}

    inline void Links::free()
    {
      if(linksBuilt_)
        delete[] links_;
      if(startBuilt_)
        delete[] start_;
      linksBuilt_ = false;
      startBuilt_ = false;
      nodes_=-1;
    }

    inline void Links::reserveNodes(int nodes)
    {
      nodes_=nodes;
      start_ = new int[nodes_+1];
      startBuilt_ = true;
    }

    inline Links::BuildIterator Links::begin()
    {
      return BuildIterator(*this);
    }

    inline Link* Links::operator[](int i)
    {
      return links_+start_[i];
    }


    inline const Link* Links::operator[](int i) const
    {
      return links_+start_[i];
    }

    inline Links::~Links()
    {
      if(startBuilt_)
        delete[] start_;
      if(linksBuilt_)
        delete[] links_;
    }
    /** @} */
  }
}

#endif
