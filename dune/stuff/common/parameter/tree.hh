#ifndef DUNE_STUFF_COMMON_PARAMETER_TREE_HH
#define DUNE_STUFF_COMMON_PARAMETER_TREE_HH

#ifdef HAVE_CMAKE_CONFIG
  #include "cmake_config.h"
#else
  #include "config.h"
#endif // ifdef HAVE_CMAKE_CONFIG

#include <cstring>
#include <sstream>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#if HAVE_EIGEN
  #include <Eigen/Core>
#endif // HAVE_EIGEN

#include <dune/common/exceptions.hh>
#include <dune/common/parametertreeparser.hh>

#include <dune/stuff/common/string.hh>
#include <dune/stuff/common/color.hh>

namespace Dune {
namespace Stuff {
namespace Common {

//! ParameterTree extension for nicer output
//! \todo TODO The report method should go into dune-common
class ExtendedParameterTree
  : public Dune::ParameterTree {
public:
  typedef Dune::ParameterTree BaseType;

  ExtendedParameterTree()
  {}

  ExtendedParameterTree(int argc, char** argv, std::string filename)
    : BaseType(init(argc, argv, filename))
  {}

  ExtendedParameterTree(const Dune::ParameterTree& other)
    : BaseType(other)
  {}

  ExtendedParameterTree& operator=(const Dune::ParameterTree& other)
  {
    if (this != &other) {
      BaseType::operator=(other);
    }
    return *this;
  } // ExtendedParameterTree& operator=(const Dune::ParameterTree& other)

  ExtendedParameterTree sub(const std::string& _sub) const
  {
    if (!hasSub(_sub))
        DUNE_THROW(Dune::RangeError,
                   "\nERROR: sub '" << _sub << "' missing in the following Dune::ParameterTree:\n" << reportString("  "));
    return ExtendedParameterTree(BaseType::sub(_sub));
  }

  void report(std::ostream& stream = std::cout, const std::string& prefix = "") const
  {
    reportAsSub(stream, prefix, "");
  } // void report(std::ostream& stream = std::cout, const std::string& prefix = "") const

  std::string reportString(const std::string& prefix = "") const
  {
    std::stringstream stream;
    report(stream, prefix);
    return stream.str();
  } // std::stringstream reportString(const std::string& prefix = "") const

  std::string get(const std::string& _key, const std::string& defaultValue) const
  {
    if (!BaseType::hasKey(_key))
      std::cout << Dune::Stuff::Common::highlightString("WARNING:", 9) << " missing key '" << _key << "' is replaced by given default value '" << defaultValue << "'!" << std::endl;
    return BaseType::get< std::string >(_key, defaultValue);
  }

  std::string get(const std::string& _key, const char* defaultValue) const
  {
    if (!BaseType::hasKey(_key))
      std::cout << Dune::Stuff::Common::highlightString("WARNING:", 9) << " missing key '" << _key << "' is replaced by given default value '" << defaultValue << "'!" << std::endl;
    return BaseType::get< std::string >(_key, defaultValue);
  }

  int get(const std::string& _key, int defaultValue) const
  {
    if (!BaseType::hasKey(_key))
      std::cout << Dune::Stuff::Common::highlightString("WARNING:", 9) << " missing key '" << _key << "' is replaced by given default value '" << defaultValue << "'!" << std::endl;
    return BaseType::get< int >(_key, defaultValue);
  }

  double get(const std::string& _key, double defaultValue) const
  {
    if (!BaseType::hasKey(_key))
      std::cout << Dune::Stuff::Common::highlightString("WARNING:", 9) << " missing key '" << _key << "' is replaced by given default value '" << defaultValue << "'!" << std::endl;
    return BaseType::get< double >(_key, defaultValue);
  }

  template< typename T >
  T get(const std::string& _key, const T& defaultValue) const
  {
    if (!BaseType::hasKey(_key))
      std::cout << Dune::Stuff::Common::highlightString("WARNING:", 9) << " missing key '" << _key << "' is replaced by given default value!" << std::endl;
    return BaseType::get< T >(_key, defaultValue);
  }

  template< class T >
  T get(const std::string& _key) const
  {
    if (!BaseType::hasKey(_key))
      DUNE_THROW(Dune::RangeError,
                 "\nERROR: key '" << _key << "' missing  in the following Dune::ParameterTree:\n" << reportString("  "));
    return BaseType::get< T >(_key);
  }

  bool hasVector(const std::string& _key) const
  {
    if (hasKey(_key)) {
      const std::string str = BaseType::get< std::string >(_key, "meaningless_default_value");
      if (Dune::Stuff::Common::String::equal(str.substr(0, 1), "[")
          && Dune::Stuff::Common::String::equal(str.substr(str.size() - 1, 1), "]"))
        return true;
    }
    return false;
  } // bool hasVector(const std::string& vector) const

  template< class T >
  std::vector< T > getVector(const std::string& _key, const T& def, const unsigned int minSize) const
  {
    if (!hasKey(_key)) {
      std::cout << Dune::Stuff::Common::highlightString("WARNING:", 9) << " missing key '" << _key << "' is replaced by given default value!" << std::endl;
      return std::vector< T >(minSize, def);
    } else {
      const std::string str = BaseType::get(_key, "meaningless_default_value");
      if (Dune::Stuff::Common::String::equal(str, "")) {
        if (minSize > 0)
          std::cout << Dune::Stuff::Common::highlightString("WARNING:", 9) << " vector '" << _key << "' was too small (0) and has been enlarged to size " << minSize << "!" << std::endl;
        return std::vector< T >(minSize, def);
      } else if (str.size() < 3) {
        std::vector< T > ret;
        ret.push_back(Dune::Stuff::Common::fromString< T >(str));
        if (ret.size() < minSize) {
          std::cout << Dune::Stuff::Common::highlightString("WARNING:", 9) << " vector '" << _key << "' was too small (" << ret.size() << ") and has been enlarged to size " << minSize << "!" << std::endl;
          for (unsigned int i = ret.size(); i < minSize; ++i)
            ret.push_back(def);
        }
        return ret;
      } else {
        // the dune parametertree strips any leading and trailing whitespace
        // so we can be sure that the first and last have to be the brackets [] if this is a vector
        std::vector< T > ret;
        if (Dune::Stuff::Common::String::equal(str.substr(0, 1), "[")
            && Dune::Stuff::Common::String::equal(str.substr(str.size() - 1, 1), "]")) {
          std::vector< std::string > tokens;
          if (str.size() > 2)
            tokens = Dune::Stuff::Common::tokenize< std::string >(str.substr(1, str.size() - 2), ";");
          for (unsigned int i = 0; i < tokens.size(); ++i)
            ret.push_back(Dune::Stuff::Common::fromString< T >(boost::algorithm::trim_copy(tokens[i])));
          for (unsigned int i = ret.size(); i < minSize; ++i)
            ret.push_back(def);
        } else if (Dune::Stuff::Common::String::equal(str.substr(0, 1), "[")
                   || Dune::Stuff::Common::String::equal(str.substr(str.size() - 1, 1), "]")) {
            DUNE_THROW(Dune::RangeError, "Vectors have to be of the form '[entry_0; entry_1; ... ]'!");
        } else {
          ret = std::vector< T >(minSize, Dune::Stuff::Common::fromString< T >(boost::algorithm::trim_copy(str)));
        }
        return ret;
      }
    }
  } // std::vector< T > getVector(const std::string& key, const T def) const

  template< class T >
  std::vector< T > getVector(const std::string& _key, const unsigned int minSize) const
  {
    if (!hasKey(_key)) {
      DUNE_THROW(Dune::RangeError,
                 "\nERROR: key '" << _key << "' missing  in the following Dune::ParameterTree:\n" << reportString("  "));
    } else {
      std::vector< T > ret;
      const std::string str = BaseType::get< std::string >(_key, "meaningless_default_value");
      // the dune parametertree strips any leading and trailing whitespace
      // so we can be sure that the first and last have to be the brackets [] if this is a vector
      if (Dune::Stuff::Common::String::equal(str.substr(0, 1), "[")
          && Dune::Stuff::Common::String::equal(str.substr(str.size() - 1, 1), "]")) {
        const std::vector< std::string > tokens = Dune::Stuff::Common::tokenize< std::string >(str.substr(1, str.size() - 2), ";");
        for (unsigned int i = 0; i < tokens.size(); ++i)
          ret.push_back(Dune::Stuff::Common::fromString< T >(boost::algorithm::trim_copy(tokens[i])));
      } else if (minSize == 1)
        ret.push_back(Dune::Stuff::Common::fromString< T >(str));
      else
          DUNE_THROW(Dune::RangeError, "Vectors have to be of the form '[entry_0; entry_1; ... ]'!");
      if (ret.size() < minSize)
        DUNE_THROW(Dune::RangeError,
                   "\nERROR: vector '" << _key
                   << "' too short (is " << ret.size() << ", should be at least " << minSize
                   << ") in the following Dune::ParameterTree :\n" << reportString("  "));
      return ret;
    }
  } // std::vector< T > getVector(const std::string& key, const T def) const

#if HAVE_EIGEN
  template< class T >
  Eigen::Matrix< T, Eigen::Dynamic, 1 > getEigenVector(const std::string& _key,
                                                       const T& def,
                                                       const unsigned int minSize) const
  {
    // get correspongin vector
    std::vector< T > vec = getVector< T >(_key, def, minSize);
    // create eigen vector and return
    Eigen::Matrix< T, Eigen::Dynamic, 1 > ret(vec.size());
    for (unsigned int i = 0; i < vec.size(); ++i)
      ret(i) = vec[i];
    return ret;
  }

  template< class T >
  Eigen::Matrix< T, Eigen::Dynamic, 1 > getEigenVector(const std::string& _key, const unsigned int minSize) const
  {
    // get correspongin vector
    std::vector< T > vec = getVector< T >(_key, minSize);
    // create eigen vector and return
    Eigen::Matrix< T, Eigen::Dynamic, 1 > ret(vec.size());
    for (unsigned int i = 0; i < vec.size(); ++i)
      ret(i) = vec[i];
    return ret;
  }
#endif // HAVE_EIGEN

  void assertKey(const std::string& _key) const
  {
    if (!BaseType::hasKey(_key))
      DUNE_THROW(Dune::RangeError,
                 "\nERROR: key '" << _key << "' missing  in the following Dune::ParameterTree:\n" << reportString("  "));
  }

  void assertSub(const std::string& _sub) const
  {
    if (!BaseType::hasSub(_sub))
      DUNE_THROW(Dune::RangeError,
                 "\nERROR: sub '" << _sub << "' missing  in the following Dune::ParameterTree:\n" << reportString("  "));
  }

  /**
    \brief      Fills a Dune::ParameterTree given a parameter file or command line arguments.
    \param[in]  argc
                From \c main()
    \param[in]  argv
                From \c main()
    \param[out] paramTree
                The Dune::ParameterTree that is to be filled.
    **/
  static ParameterTree init(int argc, char** argv, std::string filename)
  {
    Dune::ParameterTree paramTree;
    if (argc == 1) {
      Dune::ParameterTreeParser::readINITree(filename, paramTree);
    } else if (argc == 2) {
      Dune::ParameterTreeParser::readINITree(argv[1], paramTree);
    } else {
      Dune::ParameterTreeParser::readOptions(argc, argv, paramTree);
    }
    if (paramTree.hasKey("paramfile")) {
      Dune::ParameterTreeParser::readINITree(paramTree.get< std::string >("paramfile"), paramTree, false);
    }
    return paramTree;
  } // static ExtendedParameterTree init(...)

private:
  void reportAsSub(std::ostream& stream, const std::string& prefix, const std::string& subPath) const
  {
    for (auto pair : values)
      stream << prefix << pair.first << " = " << pair.second << std::endl;
//      stream << prefix << pair.first << " = \"" << pair.second << "\"" << std::endl;
    for (auto pair : subs) {
      ExtendedParameterTree subTree(pair.second);
      if (subTree.getValueKeys().size())
        stream << prefix << "[ " << subPath << pair.first << " ]" << std::endl;
      subTree.reportAsSub(stream, prefix, subPath + pair.first + ".");
    }
  } // void report(std::ostream& stream = std::cout, const std::string& prefix = "") const
}; // class ExtendedParameterTree

//! \todo TODO Remove this!
typedef ExtendedParameterTree ParameterTreeX;

} // namespace Common
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_COMMON_PARAMETER_TREE_HH
