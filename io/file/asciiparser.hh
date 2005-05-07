// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __DUNE_ASCIIPARSER_HH__
#define __DUNE_ASCIIPARSER_HH__

namespace Dune {

  static const int MAXTAB = 30;

  //! reads data folowing the given keyword
  //! if verbose is true then an output of what was read is given
  //! the token '%' stands for comment
  template <class T>
  bool readParameter (const std::basic_string<char> filename,
                      const char keywd[], T & data, bool verbose = true)
  {
    std::fstream file (filename.c_str(),std::ios::in);
    if( !file )
    {
      std::cerr << "ERROR: cannot open file '" << filename << "' in " <<  __FILE__<< " line: " << __LINE__ << std::endl;
      DUNE_THROW(IOError,"cannot open file " << filename << std::endl);
    }

    std::basic_string<char> keyword ( keywd );
    bool readData = false;
    char ch[1024];
    while (! file.eof() )
    {
      std::basic_string <char> keyHelp;
      file >> keyHelp;

      if(keyHelp[0] == '%')
      {
        file.getline(ch,1024,'\n');
      }
      else
      {
        // copy only keyword size and compare
        int pos = keyHelp.find_first_of(':');
        int pos1 = keyHelp.find_first_of(' ');

        if (pos > 0)
        {
          if(pos1 > 0)
            pos -=  pos1;
          else
            pos = 1;
        }
        else
          pos = 0;

        std::basic_string <char> key  (keyHelp,0,keyHelp.size() - pos);
        if(key == keyword)
        {
          file >> data;
          readData = true;
          break;
        }
      }
    }
    file.close();
    if(readData)
    {
      if(verbose)
      {
        int length = MAXTAB - keyword.size();
        std::cout << "Reading " << keyword;
        for(int i=0; i<length; i++) std::cout << ".";
        std::cout << " " << data << "\n";;
      }
    }
    else
    {
      std::cerr << "WARNING: couldn't read " << keyword << "\n";
    }

    return readData;
  }

} // end namespace

#endif
