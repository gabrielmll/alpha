// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include <boost/program_options.hpp>
#include "sysexits.h"

#include "SkyPatternTree.h"

using namespace boost::program_options;

template<typename T> vector<T> getVectorFromString(const string& str)
{
  vector<T> tokens;
  T token;
  istringstream ss(str);
  while (ss >> token)
    {
      tokens.push_back(token);
    }
  return tokens;
}

template<typename T> vector<vector<T>> getMatrixFromFile(const string& fileName) throw(NoFileException)
{
  ifstream file(fileName);
  if (!file)
    {
      throw NoFileException(fileName.c_str());
    }
  vector<vector<T>> matrix;
  while (!file.eof())
    {
      string rowString;
      getline(file, rowString);
      const vector<T> row = getVectorFromString<T>(rowString);
      if (!row.empty())
	{
	  matrix.push_back(row);
	}
    }
  file.close();
  return matrix;
}

int main(int argc, char* argv[])
{
  Tree* root;
  // Parsing the command line and the option file
  try
    {
      string optionFileName;
      int maximalNbOfClosedNSetsForAgglomeration = 0;
      vector<double> epsilonVector;
      vector<unsigned int> cliqueDimensions;
      vector<double> tauVector;
      vector<unsigned int> minSizes;
      vector<unsigned int> maxSizes;
      int minArea = 0;
      int maxArea = -1;
      vector<unsigned int> maximizedSizeDimensions;
      vector<unsigned int> minimizedSizeDimensions;
      string outputFileName;
      vector<string> groupFileNames;
      string groupElementSeparator;
      string groupDimensionElementsSeparator;
      vector<unsigned int> groupMinSizes;
      vector<unsigned int> groupMaxSizes;
      vector<vector<float>> groupMinRatios;
      vector<vector<float>> groupMinPiatetskyShapiros;
      vector<vector<float>> groupMinLeverages;
      vector<vector<float>> groupMinForces;
      vector<vector<float>> groupMinYulesQs;
      vector<vector<float>> groupMinYulesYs;
      vector<unsigned int> groupMaximizedSizes;
      vector<unsigned int> groupMinimizedSizes;
      vector<vector<float>> groupMaximizedRatios;
      vector<vector<float>> groupMaximizedPiatetskyShapiros;
      vector<vector<float>> groupMaximizedLeverages;
      vector<vector<float>> groupMaximizedForces;
      vector<vector<float>> groupMaximizedYulesQs;
      vector<vector<float>> groupMaximizedYulesYs;
      options_description generic("Generic options");
      generic.add_options()
	("help,h", value<string>(), "display help section whose name (\"size-constraints\", \"group-constraints\" or \"io\") starts with the string in argument")
	("version,V", "display version information and exit")
	("opt", value<string>(&optionFileName), "set the option file name (by default [data-file].opt if present)");
      options_description basicConfig("Basic configuration (on the command line or in the option file)");
      basicConfig.add_options()
	("epsilon,e", value<string>(), "set noise tolerance bounds for elements in each attribute (by default 0 for every attribute)")
	("clique,c", value<string>(), "set attributes on which closed cliques are searched (0 being the first attribute)")
	("tau,t", value<string>(), "set maximal differences between two contiguous elements in numerical attributes or 0 for infinity/non-numerical attribute (by default 0 for every attribute)")
	("reduction,r", "do not compute closed ET-n-sets, only output the input data without the elements that cannot be in any closed ET-n-sets given the constraints")
	("ha", value<int>(&maximalNbOfClosedNSetsForAgglomeration), "hierarchically agglomerate the less noisy closed ET-n-sets (maximal quantity in argument) and output the relevant agglomerates, more relevant first")
	("density,d", value<float>()->default_value(1), "set threshold to trigger a dense storage of the data (0 for a completely dense storage, 1 for a sparse storage)")
	("large,l", "quick computation of closed ET-n-sets that are large in all the dimensions (longer extractions when other closed ET-n-sets are valid)")
	("out,o", value<string>(&outputFileName), "set output file name (by default [data-file].out if closed ET-net sets are computed, [data-file].red if the input data is only reduced with option --reduction)")
	("psky", "print pattern skyline whenever refined");
      options_description sizeConstraints("Size constraints (on the command line or in the option file)");
      sizeConstraints.add_options()
	("sizes,s", value<string>(), "set minimal sizes in each attribute of any computed closed ET-n-set (by default 0 for every attribute)")
	("Sizes,S", value<string>(), "set maximal sizes in each attribute of any computed closed ET-n-set (unconstrained by default)")
	("area,a", value<int>(&minArea), "set minimal area of any computed closed ET-n-set (by default 0)")
	("Area,A", value<int>(&maxArea), "set maximal area of any computed closed ET-n-set (unconstrained by default)")
	("sky-s", value<string>(), "set attributes whose sizes are to be maximized (0 being the first attribute)")
	("sky-S", value<string>(), "set attributes whose sizes are to be minimized (0 being the first attribute)")
	("sky-a", "maximize area")
	("sky-A", "minimize area");
      options_description groupConstraints("Group constraints (on the command line or in the option file)");
      groupConstraints.add_options()
	("groups,g", value<string>(), "set the names of the files describing every group (none by default)")
	("gs", value<string>(), "set minimal number of elements in each group (by default, all elements if no group measure is used, otherwise 0 for every group)")
	("gS", value<string>(), "set maximal number of elements in each group (unconstrained by default)")
	("gr", value<string>(), "set file name specifying minimal ratios between the number of elements in each group (by default 0 for every pair of groups)")
	("gps", value<string>(), "set file name specifying minimal Piatetsky-Shapiro's measures between the number of elements in each group (by default unconstrained for every pair of groups)")
	("gl", value<string>(), "set file name specifying minimal leverages between the number of elements in each group (by default unconstrained for every pair of groups)")
	("gf", value<string>(), "set file name specifying minimal forces between the number of elements in each group (by default 0 for every pair of groups)")
	("gyq", value<string>(), "set file name specifying minimal Yule's Q between the number of elements in each group (by default -1 for every pair of groups)")
	("gyy", value<string>(), "set file name specifying minimal Yule's Y between the number of elements in each group (by default -1 for every pair of groups)")
	("sky-gs", value<string>(), "set groups whose covers are to be maximized (0 being the first group)")
	("sky-gS", value<string>(), "set groups whose covers are to be minimized (0 being the first group)")
	("sky-gr", value<string>(), "set file name specifying the pairs of groups whose ratios of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gps", value<string>(), "set file name specifying the pairs of groups whose Piatetsky-Shapiro's measures of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gl", value<string>(), "set file name specifying the pairs of groups whose leverages of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gf", value<string>(), "set file name specifying the pairs of groups whose forces of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gyq", value<string>(), "set file name specifying the pairs of groups whose Yule's Q of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gyy", value<string>(), "set file name specifying the pairs of groups whose Yule's Y of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)");
      options_description io("Input/Output format (on the command line or in the option file)");
      io.add_options()
	("ies", value<string>()->default_value(","), "set any character separating two elements in input data")
	("ids", value<string>()->default_value(" "), "set any character separating two attributes in input data")
	("ges", value<string>(&groupElementSeparator), "set any character separating two elements in a group (by default same as --ies)")
	("gds", value<string>(&groupDimensionElementsSeparator), "set any character separating the dimension from its elements in a group (by default same as --ids)")
	("oes", value<string>()->default_value(","), "set string separating two elements in output data")
	("ods", value<string>()->default_value(" "), "set string separating two attributes in output data")
	("empty", value<string>()->default_value("ø"), " set string specifying an empty set in output data")
	("pn", "print noise on every element in output data; relative noise with --ha, absolute noise otherwise unless sky-patterns are searched (--pn has no effect)")
	("ens", value<string>()->default_value("#"), "set string separating every element from the noise on it")
	("ps", "print sizes in output data")
	("css", value<string>()->default_value(" : "), "set string separating closed ET-n-sets from sizes in output data")
	("ss", value<string>()->default_value(" "), "set string separating sizes of the different attributes in output data")
	("pa", "print areas in output data")
	("sas", value<string>()->default_value(" : "), "set string separating sizes from areas in output data");
      options_description hidden("Hidden options");
      hidden.add_options()
	("data-file", value<string>(), "set input data file");
      positional_options_description p;
      p.add("data-file", -1);
      options_description commandLineOptions;
      commandLineOptions.add(generic).add(basicConfig).add(sizeConstraints).add(groupConstraints).add(io).add(hidden);
      variables_map vm;
      store(command_line_parser(argc, argv).options(commandLineOptions).positional(p).run(), vm);
      notify(vm);
      if (vm.count("help"))
	{
	  const string helpSection = vm["help"].as<string>();
	  if (helpSection == string("size-constraints").substr(0, helpSection.size()))
	    {
	      cout << sizeConstraints;
	      return EX_OK;
	    }
	  if (helpSection == string("group-constraints").substr(0, helpSection.size()))
	    {
	      cout << groupConstraints;
	      return EX_OK;
	    }
	  if (helpSection == string("io").substr(0, helpSection.size()))
	    {
	      cout << io;
	      return EX_OK;
	    }
	  cout << "Usage: multidupehack [options] data-file" << endl << generic << basicConfig;
	  return EX_OK;
	}
      if (vm.count("version"))
	{
	  cout << "multidupehack version 0.11.8" << endl;
	  return EX_OK;
	}
      if (!vm.count("data-file"))
	{
	  cerr << "Usage: multidupehack [options] data-file" << endl << generic << basicConfig;
	  return EX_OK;
	}
      ifstream optionFile;
      if (vm.count("opt"))
	{
	  optionFile.open(optionFileName.c_str());
	  if (!optionFile)
	    {
	      throw NoFileException(optionFileName.c_str());
	    }
	  optionFile.close();
	}
      else
	{
	  optionFileName = vm["data-file"].as<string>() + ".opt";
	}
      options_description config;
      config.add(basicConfig).add(sizeConstraints).add(groupConstraints).add(io).add(hidden);
      optionFile.open(optionFileName.c_str());
      store(parse_config_file(optionFile, config), vm);
      notify(vm);
      optionFile.close();
      if (vm.count("clique"))
	{
	  cliqueDimensions = getVectorFromString<unsigned int>(vm["clique"].as<string>());
	  if (cliqueDimensions.size() != 2)
	    {
	      throw UsageException("clique option should provide two attributes ids!");
	    }
	  if (cliqueDimensions.back() < cliqueDimensions.front())
	    {
	      const unsigned int tmp = cliqueDimensions.front();
	      cliqueDimensions.front() = cliqueDimensions.back();
	      cliqueDimensions.back() = tmp;
	    }
	}
      if (vm.count("ha") && maximalNbOfClosedNSetsForAgglomeration < 1)
	{
	  throw UsageException("ha option should provide a strictly positive integer!");
	}
      if (vm.count("area") && minArea < 0)
	{
	  throw UsageException("area option should provide a positive integer!");
	}
      if (vm.count("sizes"))
	{
	  minSizes = getVectorFromString<unsigned int>(vm["sizes"].as<string>());
	}
      else
	{
	  if (!vm.count("area"))
	    {
	      cerr << "Warning: Either you really know what you are doing or you forgot the sizes" << endl << "option or the area option (or both)" << endl;
	    }
	}
      if (vm.count("epsilon"))
	{
	  epsilonVector = getVectorFromString<double>(vm["epsilon"].as<string>());
	}
      if (vm.count("tau"))
	{
	  tauVector = getVectorFromString<double>(vm["tau"].as<string>());
	}
      if (vm.count("Sizes"))
	{
	  maxSizes = getVectorFromString<unsigned int>(vm["Sizes"].as<string>());
	}
      if (vm.count("groups"))
	{
	  char_separator<char> fileSeparator(" ");
	  tokenizer<char_separator<char>> tokens(vm["groups"].as<string>(), fileSeparator);
	  for (const string groupFileName : tokens)
	    {
	      groupFileNames.push_back(groupFileName);
	    }
	}
      if (vm.count("gs"))
	{
	  groupMinSizes = getVectorFromString<unsigned int>(vm["gs"].as<string>());
	  if (groupMinSizes.size() > groupFileNames.size())
	    {
	      throw UsageException(("gs option provides " + lexical_cast<string>(groupMinSizes.size()) + " sizes but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (vm.count("gS"))
	{
	  groupMaxSizes = getVectorFromString<unsigned int>(vm["gS"].as<string>());
	  if (groupMaxSizes.size() > groupFileNames.size())
	    {
	      throw UsageException(("gS option provides " + lexical_cast<string>(groupMaxSizes.size()) + " sizes but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (vm.count("gr"))
	{
	  groupMinRatios = getMatrixFromFile<float>(vm["gr"].as<string>());
	  if (groupMinRatios.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gr option has " + lexical_cast<string>(groupMinRatios.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (vm.count("gps"))
	{
	  groupMinPiatetskyShapiros = getMatrixFromFile<float>(vm["gps"].as<string>());
	  if (groupMinPiatetskyShapiros.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gps option has " + lexical_cast<string>(groupMinPiatetskyShapiros.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (vm.count("gl"))
	{
	  groupMinLeverages = getMatrixFromFile<float>(vm["gl"].as<string>());
	  if (groupMinLeverages.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gl option has " + lexical_cast<string>(groupMinLeverages.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (vm.count("gf"))
	{
	  groupMinForces = getMatrixFromFile<float>(vm["gf"].as<string>());
	  if (groupMinForces.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gf option has " + lexical_cast<string>(groupMinForces.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (vm.count("gyq"))
	{
	  groupMinYulesQs = getMatrixFromFile<float>(vm["gyq"].as<string>());
	  if (groupMinYulesQs.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gyq option has " + lexical_cast<string>(groupMinYulesQs.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (vm.count("gyy"))
	{
	  groupMinYulesYs = getMatrixFromFile<float>(vm["gyy"].as<string>());
	  if (groupMinYulesYs.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gyy option has " + lexical_cast<string>(groupMinYulesYs.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (!vm.count("ges"))
	{
	  groupElementSeparator = vm["ies"].as<string>();
	}
      if (!vm.count("gds"))
	{
	  groupDimensionElementsSeparator = vm["ids"].as<string>();
	}
      if (!vm.count("out"))
	{
	  if (vm.count("reduction"))
	    {
	      outputFileName = vm["data-file"].as<string>() + ".red";
	    }
	  else
	    {
	      outputFileName = vm["data-file"].as<string>() + ".out";
	    }
	}
      bool isSkyMining = false;
      if (vm.count("sky-s"))
	{
	  maximizedSizeDimensions = getVectorFromString<unsigned int>(vm["sky-s"].as<string>());
	  isSkyMining = true;
	}
      if (vm.count("sky-S"))
	{
	  minimizedSizeDimensions = getVectorFromString<unsigned int>(vm["sky-S"].as<string>());
	  isSkyMining = true;
	}
      if (vm.count("sky-gs"))
	{
	  groupMaximizedSizes = getVectorFromString<unsigned int>(vm["sky-gs"].as<string>());
	  sort(groupMaximizedSizes.begin(), groupMaximizedSizes.end());
	  if (groupMaximizedSizes.back() >= groupFileNames.size())
	    {
	      throw UsageException(("sky-gs option should provide group ids between 0 and " + lexical_cast<string>(groupFileNames.size() - 1)).c_str());
	    }
	  isSkyMining = true;
	}
      if (vm.count("sky-gS"))
	{
	  groupMinimizedSizes = getVectorFromString<unsigned int>(vm["sky-gS"].as<string>());
	  sort(groupMinimizedSizes.begin(), groupMinimizedSizes.end());
	  if (groupMinimizedSizes.back() >= groupFileNames.size())
	    {
	      throw UsageException(("sky-gS option should provide group ids between 0 and " + lexical_cast<string>(groupFileNames.size() - 1)).c_str());
	    }
	  isSkyMining = true;
	}
      if (vm.count("sky-gr"))
	{
	  groupMaximizedRatios = getMatrixFromFile<float>(vm["sky-gr"].as<string>());
	  if (groupMaximizedRatios.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gr option has " + lexical_cast<string>(groupMaximizedRatios.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	  isSkyMining = true;
	}
      if (vm.count("sky-gps"))
	{
	  groupMaximizedPiatetskyShapiros = getMatrixFromFile<float>(vm["sky-gps"].as<string>());
	  if (groupMaximizedPiatetskyShapiros.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gps option has " + lexical_cast<string>(groupMaximizedPiatetskyShapiros.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	  isSkyMining = true;
	}
      if (vm.count("sky-gl"))
	{
	  groupMaximizedLeverages = getMatrixFromFile<float>(vm["sky-gl"].as<string>());
	  if (groupMaximizedLeverages.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gl option has " + lexical_cast<string>(groupMaximizedLeverages.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	  isSkyMining = true;
	}
      if (vm.count("sky-gf"))
	{
	  groupMaximizedForces = getMatrixFromFile<float>(vm["sky-gf"].as<string>());
	  if (groupMaximizedForces.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gf option has " + lexical_cast<string>(groupMaximizedForces.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	  isSkyMining = true;
	}
      if (vm.count("sky-gyq"))
	{
	  groupMaximizedYulesQs = getMatrixFromFile<float>(vm["sky-gyq"].as<string>());
	  if (groupMaximizedYulesQs.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gyq option has " + lexical_cast<string>(groupMaximizedYulesQs.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	  isSkyMining = true;
	}
      if (vm.count("sky-gyy"))
	{
	  groupMaximizedYulesYs = getMatrixFromFile<float>(vm["sky-gyy"].as<string>());
	  if (groupMaximizedYulesYs.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gyy option has " + lexical_cast<string>(groupMaximizedYulesYs.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	  isSkyMining = true;
	}
      if (isSkyMining || vm.count("sky-a") || vm.count("sky-A"))
	{
	  root = new SkyPatternTree(vm["data-file"].as<string>().c_str(), vm["density"].as<float>(), epsilonVector, cliqueDimensions, tauVector, minSizes, minArea, vm.count("reduction"), maximalNbOfClosedNSetsForAgglomeration, vm["ies"].as<string>().c_str(), vm["ids"].as<string>().c_str(), outputFileName.c_str(), vm["ods"].as<string>().c_str(), vm["css"].as<string>().c_str(), vm["ss"].as<string>().c_str(), vm["sas"].as<string>().c_str(), vm.count("ps"), vm.count("pa"), vm.count("psky"));
	  try
	    {
	      static_cast<SkyPatternTree*>(root)->initMeasures(maxSizes, maxArea, maximizedSizeDimensions, minimizedSizeDimensions, vm.count("sky-a"), vm.count("sky-A"), groupFileNames, groupMinSizes, groupMaxSizes, groupMinRatios, groupMinPiatetskyShapiros, groupMinLeverages, groupMinForces, groupMinYulesQs, groupMinYulesYs, groupElementSeparator.c_str(), groupDimensionElementsSeparator.c_str(), groupMaximizedSizes, groupMinimizedSizes, groupMaximizedRatios, groupMaximizedPiatetskyShapiros, groupMaximizedLeverages, groupMaximizedForces, groupMaximizedYulesQs, groupMaximizedYulesYs);
	    }
	  catch (std::exception& e)
	    {
	      delete root;
	      rethrow_exception(current_exception());
	    }
	}
      else
	{
	  root = new Tree(vm["data-file"].as<string>().c_str(), vm["density"].as<float>(), epsilonVector, cliqueDimensions, tauVector, minSizes, minArea, vm.count("reduction"), maximalNbOfClosedNSetsForAgglomeration, vm["ies"].as<string>().c_str(), vm["ids"].as<string>().c_str(), outputFileName.c_str(), vm["ods"].as<string>().c_str(), vm["css"].as<string>().c_str(), vm["ss"].as<string>().c_str(), vm["sas"].as<string>().c_str(), vm.count("ps"), vm.count("pa"));
	  try
	    {
	      root->initMeasures(maxSizes, maxArea, groupFileNames, groupMinSizes, groupMaxSizes, groupMinRatios, groupMinPiatetskyShapiros, groupMinLeverages, groupMinForces, groupMinYulesQs, groupMinYulesYs, groupElementSeparator.c_str(), groupDimensionElementsSeparator.c_str());
	    }
	  catch (std::exception& e)
	    {
	      delete root;
	      rethrow_exception(current_exception());
	    }
	}
      Attribute::setDensityPrecedenceAndOutputFormat(vm.count("large"), vm["oes"].as<string>().c_str(), vm["empty"].as<string>().c_str(), vm["ens"].as<string>().c_str(), vm.count("pn"));
    }
  catch (unknown_option& e)
    {
      cerr << "Unknown option!" << endl;
      return EX_USAGE;
    }
  catch (NoFileException& e)
    {
      cerr << e.what() << endl;
      return EX_IOERR;
    }
  catch (UsageException& e)
    {
      cerr << e.what() << endl;
      return EX_USAGE;
    }
  catch (DataFormatException& e)
    {
      cerr << e.what() << endl;
      return EX_DATAERR;
    }
  root->peel();
  root->terminate();
  delete root;
  return EX_OK;
}
