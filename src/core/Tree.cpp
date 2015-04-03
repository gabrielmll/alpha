// Copyright 2007,2008,2009,2010,2011,2012,2013,2014,2015 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "Tree.h"

const bool smallerDimension(const Dimension* dimension, const Dimension* otherDimension)
{
  return *dimension < *otherDimension;
}

#ifdef TIME
steady_clock::time_point Tree::overallBeginning;
#endif
#ifdef DETAILED_TIME
steady_clock::time_point Tree::startingPoint;
double Tree::parsingDuration;
double Tree::preProcessingDuration;
#endif
#ifdef NB_OF_LEFT_NODES
unsigned int Tree::nbOfLeftNodes = 0;
#endif
#ifdef NB_OF_CLOSED_N_SETS
unsigned int Tree::nbOfClosedNSets = 0;
#endif

vector<unsigned int> Tree::external2InternalAttributeOrder;
vector<unordered_map<string, unsigned int>> Tree::labels2Ids;
unsigned int Tree::firstSymmetricAttributeId;
unsigned int Tree::lastSymmetricAttributeId;
Trie* Tree::data;
vector<unsigned int> Tree::minSizes;
double Tree::minArea;

bool Tree::isAgglomeration;
ofstream Tree::outputFile;
string Tree::outputDimensionSeparator;
string Tree::patternSizeSeparator;
string Tree::sizeSeparator;
string Tree::sizeAreaSeparator;
bool Tree::isSizePrinted;
bool Tree::isAreaPrinted;

Tree::Tree(const char* dataFileName, const float densityThreshold, const vector<double>& epsilonVectorParam, const vector<unsigned int>& cliqueDimensionsParam, const vector<double>& tauVectorParam, const vector<unsigned int>& minSizesParam, const unsigned int minAreaParam, const bool isReductionOnly, const unsigned int maximalNbOfClosedNSetsForAgglomeration, const char* inputElementSeparator, const char* inputDimensionSeparator, const char* outputFileName, const char* outputDimensionSeparatorParam, const char* patternSizeSeparatorParam, const char* sizeSeparatorParam, const char* sizeAreaSeparatorParam, const bool isSizePrintedParam, const bool isAreaPrintedParam) : attributes(), mereConstraints()
{
#ifdef TIME
  overallBeginning = steady_clock::now();
#endif
  vector<unsigned int> numDimensionIds;
  vector<unsigned int>::const_iterator cliqueDimensionIt = cliqueDimensionsParam.begin();
  unsigned int dimensionId = 0;
  for (const double tau : tauVectorParam)
    {
      if (tau != 0)
	{
	  for (; cliqueDimensionIt != cliqueDimensionsParam.end() && *cliqueDimensionIt < dimensionId; ++cliqueDimensionIt)
	    {
	    }
	  if (cliqueDimensionIt != cliqueDimensionsParam.end() && *cliqueDimensionIt == dimensionId)
	    {
	      throw UsageException(("clique and tau options indicate that attribute " + lexical_cast<string>(*cliqueDimensionIt) + " is both symmetric and almost-contiguous. This is not supported yet. Would you implement that feature?").c_str());
	    }
	  numDimensionIds.push_back(dimensionId);
	}
      ++dimensionId;
    }
#ifdef DETAILED_TIME
  startingPoint = steady_clock::now();
#endif
  NoisyTupleFileReader noisyTupleFileReader(dataFileName, cliqueDimensionsParam, numDimensionIds, inputDimensionSeparator, inputElementSeparator);
  pair<vector<unsigned int>, double> noisyTuple = noisyTupleFileReader.next();
  const unsigned int n = noisyTuple.first.size();
  if (tauVectorParam.size() > n)
    {
      throw UsageException(("tau option should provide at most " + lexical_cast<string>(n) + " coefficients!").c_str());
    }
  if (minSizesParam.size() > n)
    {
      throw UsageException(("sizes option should provide at most " + lexical_cast<string>(n) + " sizes!").c_str());
    }
  if (epsilonVectorParam.size() > n)
    {
      throw UsageException(("epsilon option should provide at most " + lexical_cast<string>(n) + " coefficients!").c_str());
    }
  double minMembership = 1;
  if (epsilonVectorParam.size() == n)
    {
      minMembership -= *min_element(epsilonVectorParam.begin(), epsilonVectorParam.end());
    }
  if (minMembership > 1. / numeric_limits<unsigned int>::max())
    {
      noisyTupleFileReader.setMinMembership(minMembership);
      if (minMembership > noisyTuple.second)
      	{
	  noisyTupleFileReader.startOverFromNextLine();
      	  noisyTuple = noisyTupleFileReader.next();
      	}
    }
  // Parse
  // TODO: Less memory-consuming parsing for dense datasets (one copy of the data and no pre-process)
  vector<vector<NoisyTuples*>> hyperplanes(n);
  bool isCrisp = true;
  for (; noisyTuple.second != 0; noisyTuple = noisyTupleFileReader.next())
    {
      isCrisp = isCrisp && noisyTuple.second == 1;
      dimensionId = 0;
      vector<unsigned int>::const_iterator cliqueDimensionIt = cliqueDimensionsParam.begin();
      vector<unsigned int> projectedTuple(noisyTuple.first.begin() + 1, noisyTuple.first.end());
      vector<unsigned int>::iterator projectedTupleIt = projectedTuple.begin();
      vector<unsigned int>::const_iterator elementIt = noisyTuple.first.begin();
      for (vector<vector<NoisyTuples*>>::iterator hyperplanesInDimensionIt = hyperplanes.begin(); ; ++hyperplanesInDimensionIt)
	{
	  if (cliqueDimensionIt != cliqueDimensionsParam.end() && *cliqueDimensionIt == dimensionId++)
	    {
	      ++cliqueDimensionIt;
	      while (*elementIt >= hyperplanesInDimensionIt->size())
		{
		  for (const unsigned int cliqueDimension2 : cliqueDimensionsParam)
		    {
		      hyperplanes[cliqueDimension2].push_back(new NoisyTuples());
		    }
		}
	    }
	  else
	    {
	      while (*elementIt >= hyperplanesInDimensionIt->size())
		{
		  hyperplanesInDimensionIt->push_back(new NoisyTuples());
		}
	    }
	  (*hyperplanesInDimensionIt)[*elementIt]->insert(projectedTuple, noisyTuple.second);
	  if (projectedTupleIt == projectedTuple.end())
	    {
	      break;
	    }
	  *projectedTupleIt++ = *elementIt++;
	}
    }
  vector<unsigned int> cardinalities = noisyTupleFileReader.getCardinalities();
  vector<vector<NoisyTuples*>>::iterator hyperplanesInDimensionIt = hyperplanes.begin();
  for (const unsigned int cardinality : cardinalities)
    {
      while (cardinality != hyperplanesInDimensionIt->size())
	{
	  hyperplanesInDimensionIt->push_back(new NoisyTuples());
	}
      ++hyperplanesInDimensionIt;
    }
#ifdef DETAILED_TIME
  parsingDuration = duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
  startingPoint = steady_clock::now();
#endif
  // Initialize epsilonVector, minSizeVector, minimalNbOfNonSelfLoopTuples and maximalNbOfNonSelfLoopTuples and cardinalities considering the input data order of the attributes
  vector<double> epsilonVector = epsilonVectorParam;
  epsilonVector.resize(n);
  minArea = static_cast<double>(minAreaParam);
  vector<unsigned int> minSizeVector = minSizesParam;
  if (minArea == 0)
    {
      minSizeVector.resize(n);
    }
  else
    {
      minSizeVector.resize(n, 1);
    }
  vector<unsigned int> minimalNbOfNonSelfLoopTuples;
  minimalNbOfNonSelfLoopTuples.reserve(n);
  vector<unsigned int> nbOfNonSelfLoopTuples;
  nbOfNonSelfLoopTuples.reserve(n);
  unsigned int minNbOfSymmetricElements = 0;
  if (cliqueDimensionsParam.empty())
    {
      IndistinctSkyPatterns::setParametersToComputePresentAndPotentialIrrelevancyThresholds(numeric_limits<unsigned int>::max(), 0);
      for (dimensionId = 0; dimensionId != n; ++dimensionId)
	{
	  minimalNbOfNonSelfLoopTuples.push_back(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(minSizeVector, dimensionId, 0));
	  nbOfNonSelfLoopTuples.push_back(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(cardinalities, dimensionId, 0));
	}
    }
  else
    {
      // Move the number and the minimal number of symmetric elements at the end (they need to be contiguous when calling IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern)
      vector<unsigned int> reorderedCardinalities;
      reorderedCardinalities.reserve(cardinalities.size());
      dimensionId = 0;
      vector<unsigned int>::const_iterator cliqueDimensionIt = cliqueDimensionsParam.begin();
      for (const unsigned int cardinality : cardinalities)
	{
	  if (cliqueDimensionIt != cliqueDimensionsParam.end() && *cliqueDimensionIt == dimensionId++)
	    {
	      ++cliqueDimensionIt;
	    }
	  else
	    {
	      reorderedCardinalities.push_back(cardinality);
	    }
	}
      reorderedCardinalities.insert(reorderedCardinalities.end(), cliqueDimensionsParam.size(), cardinalities[cliqueDimensionsParam.front()]);
      vector<unsigned int> reorderedMinSizes;
      reorderedMinSizes.reserve(minSizeVector.size());
      dimensionId = 0;
      cliqueDimensionIt = cliqueDimensionsParam.begin();
      for (const unsigned int minSize : minSizeVector)
	{
	  if (cliqueDimensionIt != cliqueDimensionsParam.end() && *cliqueDimensionIt == dimensionId++)
	    {
	      ++cliqueDimensionIt;
	      if (minSize > minNbOfSymmetricElements)
		{
		  minNbOfSymmetricElements = minSize;
		}
	    }
	  else
	    {
	      reorderedMinSizes.push_back(minSize);
	    }
	}
      reorderedMinSizes.insert(reorderedMinSizes.end(), cliqueDimensionsParam.size(), minNbOfSymmetricElements);
      for (const unsigned int cliqueDimensionId : cliqueDimensionsParam)
	{
	  minSizeVector[cliqueDimensionId] = minNbOfSymmetricElements;
	}
      IndistinctSkyPatterns::setParametersToComputePresentAndPotentialIrrelevancyThresholds(n - cliqueDimensionsParam.size(), n - 1);
      const unsigned int nbOfNonSelfLoopTuplesInSymmetricHyperplane = IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(reorderedCardinalities, n - 1, reorderedCardinalities.back());
      const unsigned int minimalNbOfNonSelfLoopTuplesInSymmetricHyperplane = IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(reorderedMinSizes, n - 1, minNbOfSymmetricElements);
      unsigned int internalDimensionId = 0;
      cliqueDimensionIt = cliqueDimensionsParam.begin();
      for (dimensionId = 0; dimensionId != n; ++dimensionId)
	{
	  if (cliqueDimensionIt != cliqueDimensionsParam.end() && *cliqueDimensionIt == dimensionId)
	    {
	      ++cliqueDimensionIt;
	      nbOfNonSelfLoopTuples.push_back(nbOfNonSelfLoopTuplesInSymmetricHyperplane);
	      minimalNbOfNonSelfLoopTuples.push_back(minimalNbOfNonSelfLoopTuplesInSymmetricHyperplane);
	    }
	  else
	    {
	      nbOfNonSelfLoopTuples.push_back(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(reorderedCardinalities, internalDimensionId, reorderedCardinalities.back()));
	      minimalNbOfNonSelfLoopTuples.push_back(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(reorderedMinSizes, internalDimensionId, minNbOfSymmetricElements));
	      ++internalDimensionId;
	    }
	}
    }
  // Pre-process
  vector<Dimension*> dimensions = NoisyTuples::preProcess(nbOfNonSelfLoopTuples, minimalNbOfNonSelfLoopTuples, epsilonVector, cliqueDimensionsParam, hyperplanes);
#ifdef DETAILED_TIME
  preProcessingDuration = duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef OUTPUT
  outputFile.open(outputFileName);
  if (!outputFile)
    {
      throw NoFileException(outputFileName);
    }
#endif
  if (dimensions.front()->getCardinality() == 0)
    {
      // The pre-process erased all tuples
      for (Dimension* dimension : dimensions)
	{
	  delete dimension;
	}
      for (vector<NoisyTuples*>& hyperplanesInDimension : hyperplanes)
	{
	  for (NoisyTuples* hyperplane : hyperplanesInDimension)
	    {
	      delete hyperplane;
	    }
	}
      return;
    }
  if (isReductionOnly)
    {
      // Delete the n - 1 last copies of the reduced data
      for (hyperplanesInDimensionIt = hyperplanes.begin(); ++hyperplanesInDimensionIt != hyperplanes.end(); )
	{
	  for (NoisyTuples* hyperplane : *hyperplanesInDimensionIt)
	    {
	      delete hyperplane;
	    }
	}
      // Output the reduced data
      unsigned int firstDimensionHyperplaneId = 0;
      bool isFirst = true;
      vector<NoisyTuples*>& hyperplanesInFirstDimension = hyperplanes.front();
      for (NoisyTuples* hyperplane : hyperplanesInFirstDimension)
	{
#ifdef OUTPUT
	  if (!hyperplane->empty())
	    {
	      if (isFirst)
		{
		  isFirst = false;
		}
	      else
		{
		  outputFile << endl;
		}
	      noisyTupleFileReader.printTuplesInFirstDimensionHyperplane(outputFile, firstDimensionHyperplaneId, hyperplane->begin(), hyperplane->end(), outputDimensionSeparatorParam);
	    }
	  ++firstDimensionHyperplaneId;
#endif
	  delete hyperplane;
	}
      for (Dimension* dimension : dimensions)
	{
	  delete dimension;
	}
      return;
    }
  // Init some static variables
  isAgglomeration = maximalNbOfClosedNSetsForAgglomeration != 0;
  if (isAgglomeration)
    {
      Node::setMaximalNbOfClosedNSetsForAgglomeration(maximalNbOfClosedNSetsForAgglomeration);
    }
  outputDimensionSeparator = outputDimensionSeparatorParam;
  patternSizeSeparator = patternSizeSeparatorParam;
  sizeSeparator = sizeSeparatorParam;
  sizeAreaSeparator = sizeAreaSeparatorParam;
  isSizePrinted = isSizePrintedParam;
  isAreaPrinted = isAreaPrintedParam;
  // Order the dimensions by increasing cardinality
  sort(dimensions.begin(), dimensions.end(), smallerDimension);
  external2InternalAttributeOrder.resize(n);
  unsigned int attributeId = 0;
#ifndef VERBOSE_DIM_CHOICE
  vector<unsigned int> internal2ExternalAttributeOrder;
#endif
  internal2ExternalAttributeOrder.reserve(n);
  for (const Dimension* dimension : dimensions)
    {
      dimensionId = dimension->getId();
      internal2ExternalAttributeOrder.push_back(dimensionId);
      external2InternalAttributeOrder[dimensionId] = attributeId++;
    }
#if defined DEBUG || defined ASSERT
  Attribute::setInternal2ExternalAttributeOrder(internal2ExternalAttributeOrder);
#endif
#ifdef DEBUG
  Measure::setInternal2ExternalAttributeOrder(internal2ExternalAttributeOrder);
#endif
  // Define symmetric attributes ids accordingly
  firstSymmetricAttributeId = numeric_limits<unsigned int>::max();
  lastSymmetricAttributeId = 0;
  for (const unsigned int cliqueDimension : cliqueDimensionsParam)
    {
      const unsigned int symmetricAttributeId = external2InternalAttributeOrder[cliqueDimension];
      if (symmetricAttributeId < firstSymmetricAttributeId)
	{
	  firstSymmetricAttributeId = symmetricAttributeId;
	}
      if (symmetricAttributeId > lastSymmetricAttributeId)
	{
	  lastSymmetricAttributeId = symmetricAttributeId;
	}
    }
  // If the data is crisp, reset the epsilons to values with .5 as the decimal part
  if (isCrisp)
    {
      vector<double>::iterator epsilonIt = epsilonVector.begin();
      for (; epsilonIt != epsilonVector.end() && *epsilonIt >= 1; ++epsilonIt)
	{
	  *epsilonIt = floor(*epsilonIt) + .5;
	}
      if (epsilonIt != epsilonVector.end())
	{
	  cerr << "Warning: exact closed n-sets searched (d-peeler applicable and probably faster)" << endl;
	  epsilonVector = vector<double>(n, .5);
	}
    }
#ifdef MIN_SIZE_ELEMENT_PRUNING
  // Initialize parameters to compute presentAndPotentialIrrelevancyThresholds given the sky-patterns
  IndistinctSkyPatterns::setParametersToComputePresentAndPotentialIrrelevancyThresholds(firstSymmetricAttributeId, lastSymmetricAttributeId);
#endif
  // Initialize minSizes and cardinalities according to the new attribute order, delete dimensions; compute the noise per unit (noise is stored in unsigned integers whose maximal value, the number of tuples in the hyperplane of the smallest dimension, is assigned to numeric_limits<unsigned int>::max())
  minSizes.reserve(n);
  vector<unsigned int>::iterator cardinalityIt = cardinalities.begin();
  unsigned int largestNoise = 1;
  vector<Dimension*>::const_iterator dimensionIt = dimensions.begin();
  for (vector<unsigned int>::const_iterator attributeIdIt = internal2ExternalAttributeOrder.begin(); attributeIdIt != internal2ExternalAttributeOrder.end(); ++attributeIdIt)
    {
      const unsigned int cardinality = (*dimensionIt)->getCardinality();
      delete *dimensionIt++;
      if (attributeIdIt != internal2ExternalAttributeOrder.begin())
	{
	  largestNoise *= cardinality;
	}
      *cardinalityIt++ = cardinality;
      minSizes.push_back(minSizeVector[*attributeIdIt]);
    }
  Attribute::noisePerUnit = numeric_limits<unsigned int>::max() / largestNoise;
  // Initialize labels2Ids
  if (lastSymmetricAttributeId == 0)
    {
      labels2Ids = noisyTupleFileReader.captureLabels2Ids();
    }
  else
    {
      labels2Ids = noisyTupleFileReader.captureLabels2Ids(internal2ExternalAttributeOrder[firstSymmetricAttributeId]);
    }
  // Initialize attributes, populate oldIds2NewIds and set the new ids in labels2Ids
  vector<unordered_map<unsigned int, unsigned int>> oldIds2NewIds;
  oldIds2NewIds.reserve(n);
  vector<unsigned int>::const_iterator attributeIdIt = internal2ExternalAttributeOrder.begin();
  attributes.reserve(n);
  for (dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      unsigned int hyperplaneId = 0;
      unordered_map<string, unsigned int>& labels2IdsInDimension = labels2Ids[*attributeIdIt];
      vector<NoisyTuples*>& hyperplanesInDimension = hyperplanes[*attributeIdIt];
      for (NoisyTuples* hyperplane : hyperplanesInDimension)
	{
	  const string label = noisyTupleFileReader.captureLabel(*attributeIdIt, hyperplaneId);
	  if (hyperplane->empty())
	    {
	      labels2IdsInDimension[label] = numeric_limits<unsigned int>::max();
	    }
	  if (attributeIdIt != internal2ExternalAttributeOrder.begin())
	    {
	      delete hyperplane;
	    }
	  ++hyperplaneId;
	}
      if (dimensionId == firstSymmetricAttributeId)
	{
	  for (; dimensionId <= lastSymmetricAttributeId; ++dimensionId)
	    {
	      attributes.push_back(new SymmetricAttribute(cardinalities, epsilonVector[*attributeIdIt]));
	    }
	  oldIds2NewIds.insert(oldIds2NewIds.end(), cliqueDimensionsParam.size(), attributes.back()->setLabels(labels2IdsInDimension));
	  for (dimensionId = firstSymmetricAttributeId; dimensionId != lastSymmetricAttributeId; ++dimensionId)
	    {
	      for (NoisyTuples* hyperplane : hyperplanes[*++attributeIdIt])
		{
		  delete hyperplane;
		}
	      labels2Ids[*attributeIdIt] = labels2IdsInDimension;
	    }
	}
      else
	{
	  if (*attributeIdIt < tauVectorParam.size() && tauVectorParam[*attributeIdIt] != 0)
	    {
	      attributes.push_back(new MetricAttribute(cardinalities, epsilonVector[*attributeIdIt], tauVectorParam[*attributeIdIt]));
	    }
	  else
	    {
	      attributes.push_back(new Attribute(cardinalities, epsilonVector[*attributeIdIt]));
	    }	  
	  oldIds2NewIds.push_back(attributes.back()->setLabels(labels2IdsInDimension));
	}
      ++attributeIdIt;
    }
  // Initialize data
  if (isCrisp)
    {
      Trie::setCrisp();
      SparseCrispTube::setDensityThreshold(densityThreshold);
    }
  else
    {
      SparseFuzzyTube::setDensityThreshold(densityThreshold);
    }
  data = new Trie(cardinalities.begin(), cardinalities.end());
  if (lastSymmetricAttributeId != 0)
    {
      // Insert every self loop
      data->setSelfLoops(firstSymmetricAttributeId, lastSymmetricAttributeId, attributes); // WARNING: start with the self loops (no code to insert self loops in dense structures)
    }
  // Compute order in which to access the tuples in hyperplanes of the first attribute
  vector<unsigned int> attributeOrderForTuplesInFirstAtributeHyperplanes;
  attributeOrderForTuplesInFirstAtributeHyperplanes.reserve(n - 1);
  for (vector<unsigned int>::const_iterator attributeIdIt = internal2ExternalAttributeOrder.begin() + 1; attributeIdIt != internal2ExternalAttributeOrder.end(); ++attributeIdIt)
    {
      if (*attributeIdIt < internal2ExternalAttributeOrder.front())
	{
	  attributeOrderForTuplesInFirstAtributeHyperplanes.push_back(*attributeIdIt);
	}
      else
	{
	  if (*attributeIdIt > internal2ExternalAttributeOrder.front())
	    {
	      attributeOrderForTuplesInFirstAtributeHyperplanes.push_back(*attributeIdIt - 1);
	    }
	}
    }
  // Insert tuples but self loops
  unsigned int hyperplaneOldId = 0;
  vector<NoisyTuples*>& hyperplanesInFirstAttribute = hyperplanes[internal2ExternalAttributeOrder.front()];
  for (NoisyTuples* hyperplane : hyperplanesInFirstAttribute)
    {
      if (!hyperplane->empty())
	{
	  data->setHyperplane(hyperplaneOldId, hyperplane->begin(), hyperplane->end(), attributeOrderForTuplesInFirstAtributeHyperplanes, oldIds2NewIds, attributes);
	}
      delete hyperplane;
      ++hyperplaneOldId;
    }
#ifdef DETAILED_TIME
  startingPoint = steady_clock::now();
#endif
}

// Constructor of a left subtree
Tree::Tree(const Tree& parent, const vector<Measure*>& mereConstraintsParam): attributes(), mereConstraints(mereConstraintsParam)
{
#ifdef NB_OF_LEFT_NODES
  ++nbOfLeftNodes;
#endif
  // Deep copy of the attributes
  const vector<Attribute*>& parentAttributes = parent.attributes;
  attributes.reserve(parentAttributes.size());
  vector<unsigned int> sizeOfAttributes;
  sizeOfAttributes.reserve(parentAttributes.size() - 1);
  for (vector<Attribute*>::const_iterator parentAttributeIt = parentAttributes.begin() + 1; parentAttributeIt != parentAttributes.end(); ++parentAttributeIt)
    {
      sizeOfAttributes.push_back((*parentAttributeIt)->globalSize());
    }
  const vector<unsigned int>::const_iterator sizeOfAttributesEnd = sizeOfAttributes.end();
  vector<unsigned int>::const_iterator sizeOfAttributeIt = sizeOfAttributes.begin();
  const vector<Attribute*>::const_iterator parentAttributeEnd = parentAttributes.end();
  for (vector<Attribute*>::const_iterator parentAttributeIt = parentAttributes.begin(); parentAttributeIt != parentAttributeEnd; ++parentAttributeIt)
    {
      attributes.push_back((*parentAttributeIt)->clone(parentAttributeIt, parentAttributeEnd, sizeOfAttributeIt++, sizeOfAttributesEnd));
    }
}

Tree::~Tree()
{
  for (Attribute* attribute : attributes)
    {
      delete attribute;
    }
  deleteMeasures(mereConstraints);
}

void Tree::initMeasures(const vector<unsigned int>& maxSizesParam, const int maxArea, const vector<string>& groupFileNames, vector<unsigned int>& groupMinSizes, const vector<unsigned int>& groupMaxSizes, const vector<vector<float>>& groupMinRatios, const vector<vector<float>>& groupMinPiatetskyShapiros, const vector<vector<float>>& groupMinLeverages, const vector<vector<float>>& groupMinForces, const vector<vector<float>>& groupMinYulesQs, const vector<vector<float>>& groupMinYulesYs, const char* groupElementSeparator, const char* groupDimensionElementsSeparator)
{
  // Helper variables
  const unsigned int n = attributes.size();
  vector<unsigned int> cardinalities;
  cardinalities.reserve(n);
  for (const Attribute* attribute : attributes)
    {
      cardinalities.push_back(attribute->sizeOfPotential());
    }
  try
    {
      // Get the maximal sizes in the internal order of the attributes
      vector<unsigned int> maxSizes(cardinalities);
      if (!maxSizesParam.empty())
	{
	  if (maxSizesParam.size() > n)
	    {
	      throw UsageException(("Sizes option should provide at most " + lexical_cast<string>(n) + " sizes!").c_str());
	    }
	  vector<unsigned int>::const_iterator external2InternalAttributeOrderIt = external2InternalAttributeOrder.begin();
	  for (const unsigned int maxSize : maxSizesParam)
	    {
	      maxSizes[*external2InternalAttributeOrderIt] = maxSize;
	      ++external2InternalAttributeOrderIt;
	    }
	  setMinParametersInClique(maxSizes);
	}
      // Initializing MinSize and MaxSize measures
      for (unsigned int attributeId = 0; attributeId != n; ++attributeId)
	{
	  unsigned int size = minSizes[attributeId];
	  if (size != 0)
	    {
	      mereConstraints.push_back(new MinSize(attributeId, cardinalities[attributeId], size));
	    }
	  size = maxSizes[attributeId];
	  if (size < cardinalities[attributeId])
	    {
	      mereConstraints.push_back(new MaxSize(attributeId, size));
	    }
	}
      // Initializing minArea measure
      if (minArea != 0)
	{
	  mereConstraints.push_back(new MinArea(cardinalities, static_cast<unsigned int>(minArea)));
	}
      // Initializing maxArea measure
      if (maxArea != -1)
	{
	  mereConstraints.push_back(new MaxArea(n, maxArea));
	}
      if (!groupFileNames.empty())
	{
	  // Initializing groups
	  GroupMeasure::initGroups(groupFileNames, groupElementSeparator, groupDimensionElementsSeparator, cardinalities, labels2Ids, external2InternalAttributeOrder);
	  bool isEveryGroupElementToBePresent = true;
	  // groupMinSizes is to be modified according to the diagonals of MinGroupCoverRatios, MinGroupCoverPiatetskyShapiros, MinGroupCoverLeverages, MinGroupCoverForces, MinGroupCoverYulesQs and  MinGroupCoverYulesYs
	  if (!groupMinSizes.empty())
	    {
	      isEveryGroupElementToBePresent = false;
	    }
	  groupMinSizes.resize(groupFileNames.size());
	  // Initializing MinGroupCoverRatio measures
	  if (!groupMinRatios.empty())
	    {
	      isEveryGroupElementToBePresent = false;
	      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
	      unsigned int rowId = 0;
	      for (const vector<float>& row : groupMinRatios)
		{
		  if (row.size() > groupFileNames.size())
		    {
		      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gr option provides " + lexical_cast<string>(row.size()) + " ratios but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
		    }
		  unsigned int columnId = 0;
		  for (const float ratio : row)
		    {
		      if (ratio > 0)
			{
			  if (rowId == columnId)
			    {
			      const unsigned int groupMinSizeAccordingToMatrix = static_cast<unsigned int>(ratio);
			      if (groupMinSizeAccordingToMatrix > *groupMinSizeIt)
				{
				  *groupMinSizeIt = groupMinSizeAccordingToMatrix;
				}
			    }
			  else
			    {
			      mereConstraints.push_back(new MinGroupCoverRatio(rowId, columnId, ratio));
			    }
			}
		      ++columnId;
		    }
		  ++groupMinSizeIt;
		  ++rowId;
		}
	    }
	  // Initializing MinGroupCoverPiatetskyShapiro measures
	  if (!groupMinPiatetskyShapiros.empty())
	    {
	      isEveryGroupElementToBePresent = false;
	      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
	      unsigned int rowId = 0;
	      for (const vector<float>& row : groupMinPiatetskyShapiros)
		{
		  if (row.size() > groupFileNames.size())
		    {
		      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gps option provides " + lexical_cast<string>(row.size()) + " Piatetsky-Shapiro's measures but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
		    }
		  unsigned int columnId = 0;
		  for (const float piatetskyShapiro : row)
		    {
		      if (rowId == columnId)
			{
			  const int groupMinSizeAccordingToMatrix = static_cast<int>(piatetskyShapiro);
			  if (groupMinSizeAccordingToMatrix > static_cast<int>(*groupMinSizeIt))
			    {
			      *groupMinSizeIt = groupMinSizeAccordingToMatrix;
			    }
			}
		      else
			{
			  if (-static_cast<float>(GroupMeasure::maxCoverOfGroup(rowId)) < piatetskyShapiro * static_cast<float>(GroupMeasure::maxCoverOfGroup(columnId)))
			    {
			      mereConstraints.push_back(new MinGroupCoverPiatetskyShapiro(rowId, columnId, piatetskyShapiro));
			    }
			}
		      ++columnId;
		    }
		  ++groupMinSizeIt;
		  ++rowId;
		}
	    }
	  // Initializing MinGroupCoverLeverage measures
	  if (!groupMinLeverages.empty())
	    {
	      isEveryGroupElementToBePresent = false;
	      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
	      unsigned int rowId = 0;
	      for (const vector<float>& row : groupMinLeverages)
		{
		  if (row.size() > groupFileNames.size())
		    {
		      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gl option provides " + lexical_cast<string>(row.size()) + " leverages but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
		    }
		  unsigned int columnId = 0;
		  for (const float leverage : row)
		    {
		      if (rowId == columnId)
			{
			  const int groupMinSizeAccordingToMatrix = static_cast<int>(leverage);
			  if (groupMinSizeAccordingToMatrix > static_cast<int>(*groupMinSizeIt))
			    {
			      *groupMinSizeIt = groupMinSizeAccordingToMatrix;
			    }
			}
		      else
			{
			  if (-static_cast<float>(GroupMeasure::maxCoverOfGroup(rowId)) < leverage * static_cast<float>(GroupMeasure::maxCoverOfGroup(columnId)))
			    {
			      mereConstraints.push_back(new MinGroupCoverLeverage(rowId, columnId, leverage));
			    }
			}
		      ++columnId;
		    }
		  ++groupMinSizeIt;
		  ++rowId;
		}
	    }
	  // Initializing MinGroupCoverForce measures
	  if (!groupMinForces.empty())
	    {
	      isEveryGroupElementToBePresent = false;
	      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
	      unsigned int rowId = 0;
	      for (const vector<float>& row : groupMinForces)
		{
		  if (row.size() > groupFileNames.size())
		    {
		      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gf option provides " + lexical_cast<string>(row.size()) + " forces but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
		    }
		  unsigned int columnId = 0;
		  for (const float force : row)
		    {
		      if (force > 0)
			{
			  if (rowId == columnId)
			    {
			      const unsigned int groupMinSizeAccordingToMatrix = static_cast<unsigned int>(force);
			      if (groupMinSizeAccordingToMatrix > *groupMinSizeIt)
				{
				  *groupMinSizeIt = groupMinSizeAccordingToMatrix;
				}
			    }
			  else
			    {
			      mereConstraints.push_back(new MinGroupCoverForce(rowId, columnId, force));
			    }
			}
		      ++columnId;
		    }
		  ++groupMinSizeIt;
		  ++rowId;
		}
	    }
	  // Initializing MinGroupCoverYulesQ measures
	  if (!groupMinYulesQs.empty())
	    {
	      isEveryGroupElementToBePresent = false;
	      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
	      unsigned int rowId = 0;
	      for (const vector<float>& row : groupMinYulesQs)
		{
		  if (row.size() > groupFileNames.size())
		    {
		      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gyq option provides " + lexical_cast<string>(row.size()) + " Yule's Q measures but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
		    }
		  unsigned int columnId = 0;
		  for (const float yulesQ : row)
		    {
		      if (rowId == columnId)
			{
			  if (yulesQ > 0)
			    {
			      const unsigned int groupMinSizeAccordingToMatrix = static_cast<unsigned int>(yulesQ);
			      if (groupMinSizeAccordingToMatrix > *groupMinSizeIt)
				{
				  *groupMinSizeIt = groupMinSizeAccordingToMatrix;
				}
			    }
			}
		      else
			{
			  if (yulesQ > -1)
			    {
			      mereConstraints.push_back(new MinGroupCoverYulesQ(rowId, columnId, yulesQ));
			    }
			}
		      ++columnId;
		    }
		  ++groupMinSizeIt;
		  ++rowId;
		}
	    }
	  // Initializing MinGroupCoverYulesY measures
	  if (!groupMinYulesYs.empty())
	    {
	      isEveryGroupElementToBePresent = false;
	      vector<unsigned int>::iterator groupMinSizeIt = groupMinSizes.begin();
	      unsigned int rowId = 0;
	      for (const vector<float>& row : groupMinYulesYs)
		{
		  if (row.size() > groupFileNames.size())
		    {
		      throw UsageException(("row " + lexical_cast<string>(rowId) + " of file set with gyy option provides " + lexical_cast<string>(row.size()) + " Yule's Y measures but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
		    }
		  unsigned int columnId = 0;
		  for (const float yulesY : row)
		    {
		      if (rowId == columnId)
			{
			  if (yulesY > 0)
			    {
			      const unsigned int groupMinSizeAccordingToMatrix = static_cast<unsigned int>(yulesY);
			      if (groupMinSizeAccordingToMatrix > *groupMinSizeIt)
				{
				  *groupMinSizeIt = groupMinSizeAccordingToMatrix;
				}
			    }
			}
		      else
			{
			  if (yulesY > -1)
			    {
			      mereConstraints.push_back(new MinGroupCoverYulesY(rowId, columnId, yulesY));
			    }
			}
		      ++columnId;
		    }
		  ++groupMinSizeIt;
		  ++rowId;
		}
	    }
	  // Initializing MinGroupCover measures
	  vector<unsigned int>::const_iterator groupMinSizeIt = groupMinSizes.begin();
	  for (unsigned int groupId = 0; groupMinSizeIt != groupMinSizes.end(); ++groupId)
	    {
	      if (*groupMinSizeIt != 0)
		{
		  mereConstraints.push_back(new MinGroupCover(groupId, *groupMinSizeIt));
		}
	      ++groupMinSizeIt;
	    }
	  // Initializing MaxGroupCover measures
	  if (!groupMaxSizes.empty())
	    {
	      isEveryGroupElementToBePresent = false;
	      unsigned int groupId = 0;
	      for (const unsigned int groupMaxSize : groupMaxSizes)
		{
		  if (groupMaxSize < GroupMeasure::maxCoverOfGroup(groupId))
		    {
		      mereConstraints.push_back(new MaxGroupCover(groupId, groupMaxSize));
		    }
		  ++groupId;
		}
	    }
	  if (isEveryGroupElementToBePresent)
	    {
	      for (unsigned int groupId = 0; groupId != groupFileNames.size(); ++groupId)
		{
		  mereConstraints.push_back(new MinGroupCover(groupId, GroupMeasure::maxCoverOfGroup(groupId)));
		}
	    }
	  GroupMeasure::allMeasuresSet();
	}
    }
  catch (std::exception& e)
    {
      outputFile.close();
      delete data;
      rethrow_exception(current_exception());
    }
  labels2Ids.clear();
}

void Tree::terminate()
{
#ifdef DETAILED_TIME
  const double miningDuration = duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
  double agglomerationDuration = 0;
#endif
  if (isAgglomeration)
    {
#ifdef DETAILED_TIME
      startingPoint = steady_clock::now();
#endif
      for (pair<list<Node*>::const_iterator, list<Node*>::const_iterator> nodeRange = Node::agglomerateAndSelect(data); nodeRange.first != nodeRange.second; ++nodeRange.first)
	{
#ifdef OUTPUT
	  unsigned int area = 0;
	  float areaTimesNoisePerUnit = Attribute::noisePerUnit;
	  if (isAreaPrinted || Attribute::noisePrinted())
	    {
	      area = (*nodeRange.first)->area();
	      areaTimesNoisePerUnit *= static_cast<float>(area);
	    }
	  bool isFirst = true;
	  for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
	    {
	      if (isFirst)
		{
		  isFirst = false;
		}
	      else
		{
		  outputFile << outputDimensionSeparator;
		}
	      bool isFirstElement = true;
	      const Attribute& attribute = *attributes[internalAttributeId];
	      const vector<Element>& patternDimension = (*nodeRange.first)->dimension(internalAttributeId);
	      const float hyperplaneAreaDividedByNoisePerUnit = areaTimesNoisePerUnit / patternDimension.size();
	      for (const Element& element : patternDimension)
		{
		  if (isFirstElement)
		    {
		      isFirstElement = false;
		    }
		  else
		    {
		      Attribute::printOutputElementSeparator(outputFile);
		    }
		  attribute.printValueFromOriginalId(element.getId(), outputFile);
		  if (area != 0)
		    {
		      Attribute::printNoise(static_cast<float>(element.getNoise()) / hyperplaneAreaDividedByNoisePerUnit, outputFile);
		    }
		}
	    }
	  if (isSizePrinted)
	    {
	      outputFile << patternSizeSeparator;
	      isFirst = true;
	      for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
		{
		  if (isFirst)
		    {
		      isFirst = false;
		    }
		  else
		    {
		      outputFile << sizeSeparator;
		    }
		  outputFile << (*nodeRange.first)->dimension(internalAttributeId).size();
		}
	    }
	  if (isAreaPrinted)
	    {
	      outputFile << sizeAreaSeparator << area;
	    }
	  outputFile << endl;
#endif
	  delete *nodeRange.first;
	}
#ifdef DETAILED_TIME
      agglomerationDuration = duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
    }
  outputFile.close();
  delete data;
#ifdef GNUPLOT
#ifdef NB_OF_CLOSED_N_SETS
  cout << nbOfClosedNSets << '\t';
#endif
#ifdef NB_OF_LEFT_NODES
  cout << nbOfLeftNodes << '\t';
#endif
#ifdef TIME
  cout << duration_cast<duration<double>>(steady_clock::now() - overallBeginning).count() << '\t';
#endif
#else
#ifdef NB_OF_CLOSED_N_SETS
  cout << "Nb of closed ET-" << attributes.size() << "-sets: " << nbOfClosedNSets << endl;
#endif
#ifdef NB_OF_LEFT_NODES
  cout << "Nb of considered " << attributes.size() << "-sets: " << nbOfLeftNodes << endl;
#endif
#ifdef TIME
  cout << "Total time: " << duration_cast<duration<double>>(steady_clock::now() - overallBeginning).count() << 's' << endl;
#endif
#endif
#ifdef DETAILED_TIME
#ifdef GNUPLOT
  cout << parsingDuration << '\t' << preProcessingDuration << '\t' << miningDuration << '\t' << agglomerationDuration << '\t';
#else
  cout << "Parsing time: " << parsingDuration << 's' << endl << "Pre-processing time: " << preProcessingDuration << 's' << endl << "Mining time: " << miningDuration << 's' << endl << "Agglomeration time: " << agglomerationDuration << 's' << endl;
#endif
  attributes.back()->printDurations(cout);
  cout << endl;
#endif
#if defined GNUPLOT && (defined NB_OF_CLOSED_N_SETS || defined NB_OF_LEFT_NODES || defined TIME)
  cout << endl;
#endif
}

void Tree::leftSubtree(const unsigned int presentAttributeId, const unsigned int originalValueId) const
{
  const vector<Measure*> childMereConstraints = childMeasures(mereConstraints, presentAttributeId, originalValueId);
  if (childMereConstraints.size() == mereConstraints.size())
    {
      Tree(*this, childMereConstraints).setPresent(presentAttributeId, originalValueId);
    }
}

vector<Measure*> Tree::childMeasures(const vector<Measure*>& parentMeasures, const unsigned int presentAttributeId, const unsigned int originalValueId)
{
  vector<Measure*> childMeasures;
  childMeasures.reserve(parentMeasures.size());
  const vector<unsigned int> elementSetPresent {originalValueId};
  for (const Measure* measure : parentMeasures)
    {
      Measure* childMeasure = measure->clone();
      if (childMeasure->violationAfterAdding(presentAttributeId, elementSetPresent))
	{
	  delete childMeasure;
	  deleteMeasures(childMeasures);
	  return childMeasures;
	}
      childMeasures.push_back(childMeasure);
    }
  // If attribute is symmetric, it always is the first one (given how chosen in peel)
  if (presentAttributeId == firstSymmetricAttributeId)
    {
      for (unsigned int symmetricAttributeId = presentAttributeId + 1; symmetricAttributeId <= lastSymmetricAttributeId; ++symmetricAttributeId)
	{
	  for (Measure* childMeasure : childMeasures)
	    {
	      if (childMeasure->violationAfterAdding(symmetricAttributeId, elementSetPresent))
		{
		  deleteMeasures(childMeasures);
		  childMeasures.clear();
		  return childMeasures;
		}
	    }
	}
    }
  return childMeasures;
}

void Tree::deleteMeasures(vector<Measure*>& measures)
{
  for (Measure* measure : measures)
    {
      delete measure;
    }
}

void Tree::setPresent(const unsigned int presentAttributeId, const unsigned int valueOriginalId)
{
  const vector<Attribute*>::iterator attributeBegin = attributes.begin();
  vector<Attribute*>::iterator presentAttributeIt = attributeBegin + presentAttributeId;
  Value* presentValue = (*presentAttributeIt)->moveValueFromPotentialToPresent(valueOriginalId);
  data->setPresent(presentAttributeIt, *presentValue, attributeBegin);
  // If attribute is symmetric, it always is the first one (given how chosen in peel)
  const bool isPresentAttributeSymmetric = presentAttributeId == firstSymmetricAttributeId;
  if (isPresentAttributeSymmetric)
    {
      for (unsigned int symmetricAttributeId = firstSymmetricAttributeId; symmetricAttributeId != lastSymmetricAttributeId; ++symmetricAttributeId)
	{
	  ++presentAttributeIt;
	  data->setPresent(presentAttributeIt, *static_cast<SymmetricAttribute*>(*presentAttributeIt)->moveSymmetricValueFromPotentialToPresent(*presentValue), attributeBegin);
	}
    }
  vector<IrrelevantValueIds> irrelevantValueIdsVector;
  irrelevantValueIdsVector.reserve(attributes.size());
  vector<Attribute*>::iterator attributeIt = attributeBegin;
  const vector<Attribute*>::const_iterator attributeEnd = attributes.end();
  if (firstSymmetricAttributeId != numeric_limits<unsigned int>::max())
    {
      IrrelevantValueIds irrelevantSymmetricValueIds;
      vector<unsigned int> newIrrelevantSymmetricValues;
      newIrrelevantSymmetricValues.reserve(attributes[firstSymmetricAttributeId]->sizeOfPotential());
      for (unsigned int symmetricAttributeId = firstSymmetricAttributeId; symmetricAttributeId <= lastSymmetricAttributeId; ++attributeIt)
      	{
      	  const unsigned int attributeId = (*attributeIt)->getId();
      	  if (attributeId == symmetricAttributeId)
      	    {
      	      if (symmetricAttributeId == lastSymmetricAttributeId)
      		{
      		  const vector<unsigned int> newestIrrelevantValues = (*attributeIt)->findIrrelevantValuesAndCheckTauContiguity(attributeBegin, attributeEnd, irrelevantSymmetricValueIds).second;
      		  newIrrelevantSymmetricValues.insert(newIrrelevantSymmetricValues.end(), newestIrrelevantValues.begin(), newestIrrelevantValues.end());
		  vector<Attribute*>::iterator symmetricAttributeIt = attributeBegin + firstSymmetricAttributeId;
      		  for (symmetricAttributeId = firstSymmetricAttributeId; symmetricAttributeId <= lastSymmetricAttributeId; ++symmetricAttributeId)
      		    {
      		      if (!newIrrelevantSymmetricValues.empty() && violationAfterRemoving(symmetricAttributeId, newIrrelevantSymmetricValues))
      			{
      			  return;
      			}
      		      irrelevantSymmetricValueIds.setAttributeIt(symmetricAttributeIt);
      		      irrelevantValueIdsVector.push_back(irrelevantSymmetricValueIds);
		      ++symmetricAttributeIt;
      		    }
      		}
      	      else
      		{
      		  const vector<unsigned int> newestIrrelevantValues = (*attributeIt)->findIrrelevantValuesAndCheckTauContiguity(attributeBegin, attributeEnd, irrelevantSymmetricValueIds).second;
      		  newIrrelevantSymmetricValues.insert(newIrrelevantSymmetricValues.end(), newestIrrelevantValues.begin(), newestIrrelevantValues.end());
      		}
	      ++symmetricAttributeId;
      	    }
      	  else
      	    {
      	      IrrelevantValueIds irrelevantValueIds(attributeIt);
      	      const pair<bool, vector<unsigned int>> isViolatingTauContiguityAndNewIrrelevantValues = (*attributeIt)->findIrrelevantValuesAndCheckTauContiguity(attributeBegin, attributeEnd, irrelevantValueIds);
      	      if (isViolatingTauContiguityAndNewIrrelevantValues.first || (!isViolatingTauContiguityAndNewIrrelevantValues.second.empty() && violationAfterRemoving(attributeId, isViolatingTauContiguityAndNewIrrelevantValues.second)))
      		{
      		  return;
      		}
      	      irrelevantValueIdsVector.push_back(irrelevantValueIds);
      	    }
      	}
    }
  for (; attributeIt != attributeEnd; ++attributeIt)
    {
      IrrelevantValueIds irrelevantValueIds(attributeIt);
      const pair<bool, vector<unsigned int>> isViolatingTauContiguityAndNewIrrelevantValues = (*attributeIt)->findIrrelevantValuesAndCheckTauContiguity(attributeBegin, attributeEnd, irrelevantValueIds);
      if (isViolatingTauContiguityAndNewIrrelevantValues.first || (!isViolatingTauContiguityAndNewIrrelevantValues.second.empty() && violationAfterRemoving((*attributeIt)->getId(), isViolatingTauContiguityAndNewIrrelevantValues.second)))
	{
	  return;
	}
      irrelevantValueIdsVector.push_back(irrelevantValueIds);
    }
  if (dominated())
    {
      return;
    }
  if (isPresentAttributeSymmetric)
    {
      for (Attribute* attributeToClean : attributes)
	{
	  attributeToClean->cleanAbsent(attributeBegin, attributeEnd);
	}
    }
  else
    {
      for (vector<Attribute*>::iterator attributeToCleanIt = attributeBegin; attributeToCleanIt != attributeEnd; ++attributeToCleanIt)
	{
	  if (attributeToCleanIt != presentAttributeIt)
	    {
	      (*attributeToCleanIt)->cleanAbsent(attributeBegin, attributeEnd);
	    }
	}
    }
  if (setAbsent(irrelevantValueIdsVector))
    {
      peel();
    }
}

void Tree::rightSubtree(const vector<Attribute*>::iterator absentAttributeIt, const vector<Value*>::iterator valueIt)
{
#ifdef DEBUG
  cout << "Right child: ";
  (*absentAttributeIt)->printValue(**valueIt, cout);
  cout << " set absent." << endl << "Node before:" << endl;
  printNode(cout);
  cout << endl;
#endif
  const vector<Attribute*>::iterator attributeBegin = attributes.begin();
  // If attribute is symmetric, it always is the first one (given how chosen in peel)
  if ((*absentAttributeIt)->getId() == firstSymmetricAttributeId)
    {
      const vector<unsigned int> elementSetAbsent {(*valueIt)->getOriginalId()};
      if (violationAfterRemoving(firstSymmetricAttributeId, elementSetAbsent))
	{
	  return;
	}
      const Value* absentValue = (*absentAttributeIt)->moveValueFromPotentialToAbsent(valueIt);
      vector<Attribute*>::iterator symmetricAttributeIt = absentAttributeIt + 1;
      for (unsigned int symmetricAttributeId = firstSymmetricAttributeId + 1; symmetricAttributeId <= lastSymmetricAttributeId; ++symmetricAttributeId)
	{
	  if (violationAfterRemoving(symmetricAttributeId, elementSetAbsent))
	    {
	      return;
	    }
	  static_cast<SymmetricAttribute*>(*(symmetricAttributeIt))->moveSymmetricValueFromPotentialToAbsent(absentValue);
	  ++symmetricAttributeIt;
	}
      if (dominated())
	{
	  return;
	}
      for (unsigned int symmetricAttributeId = firstSymmetricAttributeId; symmetricAttributeId <= lastSymmetricAttributeId; ++symmetricAttributeId)
	{
	  data->setAbsent(--symmetricAttributeIt, elementSetAbsent, attributeBegin);
	}
    }
  else
    {
      const Value* absentValue = (*absentAttributeIt)->moveValueFromPotentialToAbsent(valueIt);
      pair<const bool, vector<unsigned int>> tauFarValueOriginalIds = (*absentAttributeIt)->tauFarValueOriginalValueIdsAndCheckConstraints(absentValue);
      if (tauFarValueOriginalIds.first)
	{
	  return;
	}
      tauFarValueOriginalIds.second.push_back(absentValue->getOriginalId());
      if (violationAfterRemoving((*absentAttributeIt)->getId(), tauFarValueOriginalIds.second) || dominated())
	{
	  return;
	}
      data->setAbsent(absentAttributeIt, tauFarValueOriginalIds.second, attributeBegin);
    }
#ifdef MIN_SIZE_ELEMENT_PRUNING
  vector<IrrelevantValueIds> irrelevantValueIdsVector;
  irrelevantValueIdsVector.reserve(attributes.size());
  const vector<Attribute*>::iterator attributeEnd = attributes.end();
  for (vector<Attribute*>::iterator attributeIt = attributeBegin; attributeIt != attributeEnd; ++attributeIt)
    {
      irrelevantValueIdsVector.push_back(IrrelevantValueIds(attributeIt));
    }
  if (findMinSizeIrrelevantValuesAndCheckConstraints(irrelevantValueIdsVector, absentAttributeIt) && setAbsent(irrelevantValueIdsVector))
    {
      peel();
    }
#else
  peel();
#endif
}

const bool Tree::setAbsent(vector<IrrelevantValueIds>& irrelevantValueIdsVector)
{
  const vector<IrrelevantValueIds>::iterator irrelevantValueIdsIt = min_element(irrelevantValueIdsVector.begin(), irrelevantValueIdsVector.end());
  if (irrelevantValueIdsIt->someIrrelevantElement())
    {
      const vector<Attribute*>::iterator attributeToPurgeIt = irrelevantValueIdsIt->getAttributeIt();
      data->setAbsent(attributeToPurgeIt, (*attributeToPurgeIt)->erasePotentialValues(irrelevantValueIdsIt->irrelevantValueIds), attributes.begin());
      irrelevantValueIdsIt->clear();
#ifdef MIN_SIZE_ELEMENT_PRUNING
      return findMinSizeIrrelevantValuesAndCheckConstraints(irrelevantValueIdsVector, attributeToPurgeIt) && setAbsent(irrelevantValueIdsVector);
#else
      return setAbsent(irrelevantValueIdsVector);
#endif
    }
  return true;
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
const bool Tree::findMinSizeIrrelevantValuesAndCheckConstraints(vector<IrrelevantValueIds>& irrelevantValueIdsVector, const vector<Attribute*>::iterator previousAbsentAttributeIt)
{
  const vector<unsigned int> thresholds = minSizeIrrelevancyThresholds();
  vector<IrrelevantValueIds>::iterator irrelevantValueIdsIt = irrelevantValueIdsVector.begin();
  for (vector<Attribute*>::iterator attributeIt = attributes.begin(); attributeIt != attributes.end(); ++attributeIt)
    {
      const unsigned int attributeId = (*attributeIt)->getId();
      if (attributeId < firstSymmetricAttributeId || attributeId > lastSymmetricAttributeId)
  	{
  	  if (attributeIt != previousAbsentAttributeIt)
	    {
	      const unsigned int threshold = thresholds[attributeId];
	      if ((*attributeIt)->presentAndPotentialIrrelevant(threshold))
		{
		  return false;
		}
	      const pair<bool, vector<unsigned int>> isViolatingTauContiguityAndNewIrrelevantValues = (*attributeIt)->findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity(threshold, *irrelevantValueIdsIt);
	      if (isViolatingTauContiguityAndNewIrrelevantValues.first || (!isViolatingTauContiguityAndNewIrrelevantValues.second.empty() && violationAfterRemoving(attributeId, isViolatingTauContiguityAndNewIrrelevantValues.second)))
		{
		  return false;
		}
	      (*attributeIt)->presentAndPotentialCleanAbsent(threshold);
	    }
  	}
      else
  	{
	  const unsigned int threshold = thresholds[attributeId];
	  if ((*attributeIt)->presentAndPotentialIrrelevant(threshold))
	    {
	      return false;
	    }
	  IrrelevantValueIds tmp(*irrelevantValueIdsIt);
	  const vector<unsigned int> newIrrelevantValues = (*attributeIt)->findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity(threshold, *irrelevantValueIdsIt).second;
	  if (!newIrrelevantValues.empty())
	    {
	      if (violationAfterRemoving(attributeId, newIrrelevantValues))
		{
		  return false;
		}
	      // Copy new irrelevant values to other symmetric IrrelevantValues
	      vector<IrrelevantValueIds>::iterator otherIrrelevantValueIdsIt = irrelevantValueIdsVector.begin() + firstSymmetricAttributeId;
	      tmp.newIrrelevantValues(*irrelevantValueIdsIt);
	      for (unsigned int otherSymmetricAttributeId = firstSymmetricAttributeId; otherSymmetricAttributeId <= lastSymmetricAttributeId; ++otherSymmetricAttributeId)
		{
		  if (otherSymmetricAttributeId != attributeId)
		    {
		      if (violationAfterRemoving(otherSymmetricAttributeId, newIrrelevantValues))
			{
			  return false;
			}
		      otherIrrelevantValueIdsIt->mergeWith(tmp);
		    }
		  ++otherIrrelevantValueIdsIt;
		}
	    }
	  static_cast<SymmetricAttribute*>(*attributeIt)->presentAndPotentialCleanAbsent(threshold, attributeIt);
  	}
      ++irrelevantValueIdsIt;
    }
  return !dominated();
}

vector<unsigned int> Tree::minSizeIrrelevancyThresholds() const
{
  const unsigned int n = attributes.size();
  // Compute the minimal and the maximal sizes of a pattern, the area of the minimal pattern and the minimal/maximal number of symmetric elements
  vector<unsigned int> maximalPatternSizes;
  maximalPatternSizes.reserve(n);
  vector<unsigned int> minimalPatternSizes;
  minimalPatternSizes.reserve(n);
  vector<unsigned int>::const_iterator minSizeIt = minSizes.begin();
  if (firstSymmetricAttributeId == numeric_limits<unsigned int>::max())
    {
      for (const Attribute* attribute : attributes)
	{
	  minimalPatternSizes.push_back(max(*minSizeIt++, attribute->sizeOfPresent()));
	  maximalPatternSizes.push_back(attribute->sizeOfPresent() + attribute->sizeOfPotential());
	}
      // Sum the epsilons with the number of non-self-loops tuples in the maximal pattern and subtract the number of non-self-loops tuples in the minimal pattern
      vector<unsigned int> thresholds(Attribute::getEpsilonVector());
      vector<unsigned int>::iterator thresholdIt = thresholds.begin();
      vector<Attribute*>::const_iterator attributeIt = attributes.begin();
      for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
	{
	  // TODO: check whether round-off errors are problematic (round instead of ceil?)
	  *thresholdIt++ += Attribute::noisePerUnit * (IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(maximalPatternSizes, dimensionId, 0) - max(static_cast<double>(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(minimalPatternSizes, dimensionId, 0)), ceil(minArea / ((*attributeIt)->sizeOfPresent() + (*attributeIt)->sizeOfPotential()))));
	  ++attributeIt;
	}
      return thresholds;
    }
  unsigned int maxNbOfSymmetricElements = numeric_limits<unsigned int>::max();
  unsigned int minNbOfSymmetricElements = numeric_limits<unsigned int>::max();
  double minNbOfSymmetricElementsAccordingToArea = minArea;
  unsigned int maxAreaIgnoringSymmetricAttributes = 1;
  vector<Attribute*>::const_iterator attributeIt = attributes.begin();
  for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      unsigned int size = max(*minSizeIt, (*attributeIt)->sizeOfPresent());
      minimalPatternSizes.push_back(size);
      size = (*attributeIt)->sizeOfPresent() + (*attributeIt)->sizeOfPotential();
      if (dimensionId < firstSymmetricAttributeId || dimensionId > lastSymmetricAttributeId)
	{
	  maxAreaIgnoringSymmetricAttributes *= size;
	  minNbOfSymmetricElementsAccordingToArea /= size;
	}
      else
	{
	  if (minimalPatternSizes.back() < minNbOfSymmetricElements)
	    {
	      minNbOfSymmetricElements = minimalPatternSizes.back();
	    }
	  if (size < maxNbOfSymmetricElements)
	    {
	      maxNbOfSymmetricElements = size;
	    }
	}
      maximalPatternSizes.push_back(size);
      ++attributeIt;
      ++minSizeIt;
    }
  // TODO: check whether round-off errors are problematic (round instead of ceil?)
  minNbOfSymmetricElementsAccordingToArea = ceil(pow(minNbOfSymmetricElementsAccordingToArea, 1. / (lastSymmetricAttributeId - firstSymmetricAttributeId + 1)));
  if (static_cast<unsigned int>(minNbOfSymmetricElementsAccordingToArea) > minNbOfSymmetricElements)
    {
      minNbOfSymmetricElements = minNbOfSymmetricElementsAccordingToArea;
    }
  vector<unsigned int>::iterator minimalPatternSizeIt = minimalPatternSizes.begin() + firstSymmetricAttributeId;
  for (unsigned int dimensionId = firstSymmetricAttributeId; dimensionId <= lastSymmetricAttributeId; ++dimensionId)
    {
      *minimalPatternSizeIt++ = minNbOfSymmetricElements;
    }
  // Sum the epsilons with the number of non-self-loops tuples in the maximal pattern and subtract the number of non-self-loops tuples in the minimal pattern
  vector<unsigned int> thresholds(Attribute::getEpsilonVector());
  vector<unsigned int>::iterator thresholdIt = thresholds.begin();
  attributeIt = attributes.begin();
  for (unsigned int dimensionId = 0; dimensionId != n; ++dimensionId)
    {
      if (dimensionId < firstSymmetricAttributeId || dimensionId > lastSymmetricAttributeId)
	{
	  // TODO: check whether round-off errors are problematic (round instead of ceil?)
	  *thresholdIt++ += Attribute::noisePerUnit * (IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(maximalPatternSizes, dimensionId, minNbOfSymmetricElements) - max(static_cast<double>(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(minimalPatternSizes, dimensionId, minNbOfSymmetricElements)), ceil((minArea - maxAreaIgnoringSymmetricAttributes * maxNbOfSymmetricElements) / ((*attributeIt)->sizeOfPresent() + (*attributeIt)->sizeOfPotential()))));
	}
      else
	{
	  // TODO: check whether round-off errors are problematic (round instead of ceil?)
	  *thresholdIt++ += Attribute::noisePerUnit * (IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(maximalPatternSizes, dimensionId, minNbOfSymmetricElements) - max(static_cast<double>(IndistinctSkyPatterns::nbOfNonSelfLoopTuplesInHyperplaneOfPattern(minimalPatternSizes, dimensionId, minNbOfSymmetricElements)), ceil(minArea / ((*attributeIt)->sizeOfPresent() + (*attributeIt)->sizeOfPotential()) - maxAreaIgnoringSymmetricAttributes)));
	}
      ++attributeIt;
    }
  return thresholds;
}
#endif

const bool Tree::violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  for (Measure* measure : mereConstraints)
    {
      if (measure->violationAfterAdding(dimensionIdOfElementsSetPresent, elementsSetPresent))
	{
	  return true;
	}
    }
  return false;
}

const bool Tree::violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  for (Measure* measure : mereConstraints)
    {
      if (measure->violationAfterRemoving(dimensionIdOfElementsSetAbsent, elementsSetAbsent))
	{
	  return true;
	}
    }
  return false;
}

const bool Tree::dominated() const
{
  return false;
}

void Tree::validPattern() const
{
  if (isAgglomeration)
    {
      Node::insertOrDelete(new Node(attributes));
    }
#ifdef OUTPUT
  else
    {
      outputFile << *this;
      if (isSizePrinted)
	{
	  outputFile << patternSizeSeparator;
	  bool isFirstSize = true;
	  for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
	    {
	      if (isFirstSize)
		{
		  isFirstSize = false;
		}
	      else
		{
		  outputFile << sizeSeparator;
		}
	      outputFile << attributes[internalAttributeId]->sizeOfPresent();
	    }
	}
      if (isAreaPrinted)
	{
	  unsigned int area = 1;
	  for (const Attribute* attribute : attributes)
	    {
	      area *= attribute->sizeOfPresent();
	    }
	  outputFile << sizeAreaSeparator << area;
	}
      outputFile << endl;
    }
#endif
}

void Tree::setMinParametersInClique(vector<unsigned int>& parameterVector)
{
  if (firstSymmetricAttributeId != numeric_limits<unsigned int>::max())
    {
      unsigned int min = numeric_limits<unsigned int>::max();
      vector<unsigned int>::iterator parameterIt = parameterVector.begin() + firstSymmetricAttributeId;
      for (unsigned int symmetricAttributeId = firstSymmetricAttributeId; symmetricAttributeId <= lastSymmetricAttributeId; ++symmetricAttributeId)
	{
	  if (*parameterIt < min)
	    {
	      min = *parameterIt;
	    }
	  ++parameterIt;
	}
      for (unsigned int symmetricAttributeId = firstSymmetricAttributeId; symmetricAttributeId <= lastSymmetricAttributeId; ++symmetricAttributeId)
	{
	  *(--parameterIt) = min;
	}
    }
}

void Tree::setMaxParametersInClique(vector<unsigned int>& parameterVector)
{
  if (firstSymmetricAttributeId != numeric_limits<unsigned int>::max())
    {
      unsigned int max = 0;
      vector<unsigned int>::iterator parameterIt = parameterVector.begin() + firstSymmetricAttributeId;
      for (unsigned int symmetricAttributeId = firstSymmetricAttributeId; symmetricAttributeId <= lastSymmetricAttributeId; ++symmetricAttributeId)
	{
	  if (*parameterIt > max)
	    {
	      max = *parameterIt;
	    }
	  ++parameterIt;
	}
      for (unsigned int symmetricAttributeId = firstSymmetricAttributeId; symmetricAttributeId <= lastSymmetricAttributeId; ++symmetricAttributeId)
	{
	  *(--parameterIt) = max;
	}
    }
}

void Tree::peel()
{
#ifdef ASSERT
  for (vector<Attribute*>::const_iterator attributeIt = attributes.begin(); attributeIt != attributes.end(); ++attributeIt)
    {
      assertValues((*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), attributeIt, attributes.begin(), cout);
      assertValues((*attributeIt)->potentialBegin(), (*attributeIt)->potentialEnd(), attributeIt, attributes.begin(), cout);
      assertValues((*attributeIt)->absentBegin(), (*attributeIt)->absentEnd(), attributeIt, attributes.begin(), cout);
    }
#endif
  const vector<Attribute*>::iterator attributeBegin = attributes.begin();
  const vector<Attribute*>::iterator attributeEnd = attributes.end();
  vector<Attribute*>::iterator attributeIt = attributeBegin;
  for (; attributeIt != attributeEnd && (*attributeIt)->finalizable(); ++attributeIt)
    {
    }
  if (attributeIt == attributeEnd)
    {
      // Leaf
#ifdef DEBUG
      cout << "Every remaining potential value is present!" << endl;
#endif
      for (Attribute* attribute : attributes)
	{
	  const vector<unsigned int> elementsSetPresent = attribute->finalize();
	  if (!elementsSetPresent.empty() && violationAfterAdding(attribute->getId(), elementsSetPresent))
	    {
	      return;
	    }
	}
      if (dominated())
      	{
      	  return;
      	}
    }
  for (attributeIt = attributeBegin; attributeIt != attributeEnd && (*attributeIt)->closed(attributeBegin, attributeEnd); ++attributeIt)
    {
    }
  if (attributeIt != attributeEnd)
    {
      return;
    }
  // Not a leaf
#ifdef DEBUG
  cout << "Node after:" << endl;
  printNode(cout);
  cout << endl;
#endif
  // Choose the next value to peel
  vector<Attribute*>::iterator attributeToPeelIt = attributeBegin;
  for (attributeIt = attributeToPeelIt; attributeIt != attributeEnd && (*attributeIt)->potentialEmpty(); ++attributeIt)
    {
      ++attributeToPeelIt;
    }
  if (attributeToPeelIt == attributeEnd) // cannot be integrated to the same last test because of rounding errors
    {
#ifdef DEBUG
      cout << "*********************** closed ET-" << attributes.size() << "-set ************************" << endl << *this << endl << "****************************************************************" << endl;
#endif
#ifdef NB_OF_CLOSED_N_SETS
      ++nbOfClosedNSets;
#endif
      validPattern();
      return;
    }
  double maxAppeal = (*attributeIt)->getAppeal(attributeBegin, attributeEnd);
#ifdef VERBOSE_DIM_CHOICE
  cout << "Appeal of attribute " << internal2ExternalAttributeOrder[(*attributeIt)->getId()] << ": " << maxAppeal << endl;
#endif
  while (++attributeIt != attributeEnd)
    {
      if (!(*attributeIt)->potentialEmpty())
	{
	  double appeal = (*attributeIt)->getAppeal(attributeBegin, attributeEnd);
#ifdef VERBOSE_DIM_CHOICE
	  cout << "Appeal of attribute " << internal2ExternalAttributeOrder[(*attributeIt)->getId()] << ": " << appeal << endl;
#endif
	  if (appeal > maxAppeal)
	    {
	      attributeToPeelIt = attributeIt;
	      maxAppeal = appeal;
	    }
	}
    }
  vector<Value*>::iterator valueItToPeel((*attributeToPeelIt)->valueItToEnumerate());
  // Construct the subtrees
#ifdef DEBUG
  cout << "Left child: ";
  (*attributeToPeelIt)->printValue(**valueItToPeel, cout);
  cout << " set present." << endl << "Node before:" << endl;
  printNode(cout);
  cout << endl;
#endif
  leftSubtree((*attributeToPeelIt)->getId(), (*valueItToPeel)->getOriginalId());
  rightSubtree(attributeToPeelIt, valueItToPeel);
}

ostream& operator<<(ostream& out, const Tree& node)
{
  bool isFirstAttribute = true;
  for (const unsigned int internalAttributeId : node.external2InternalAttributeOrder)
    {
      if (isFirstAttribute)
	{
	  isFirstAttribute = false;
	}
      else
	{
	  out << Tree::outputDimensionSeparator;
	}
      out << *(node.attributes[internalAttributeId]);
    }
  return out;
}

#ifdef DEBUG
void Tree::printNode(ostream& out) const
{
  out << "  present: ";
  bool isFirstAttribute = true;
  for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
    {
      if (isFirstAttribute)
	{
	  isFirstAttribute = false;
	}
      else
	{
	  out << outputDimensionSeparator;
	}
      attributes[internalAttributeId]->printPresent(out);
    }
  out << endl << "  potential: ";
  isFirstAttribute = true;
  for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
    {
      if (isFirstAttribute)
	{
	  isFirstAttribute = false;
	}
      else
	{
	  out << outputDimensionSeparator;
	}
      attributes[internalAttributeId]->printPotential(out);
    }
  out << endl << "  absent by enumeration: ";
  isFirstAttribute = true;
  for (const unsigned int internalAttributeId : external2InternalAttributeOrder)
    {
      if (isFirstAttribute)
	{
	  isFirstAttribute = false;
	}
      else
	{
	  out << outputDimensionSeparator;
	}
      attributes[internalAttributeId]->printAbsent(out);
    }
}
#endif

#ifdef ASSERT
void Tree::assertValues(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const vector<Attribute*>::const_iterator attributeIt, const vector<Attribute*>::const_iterator attributeBegin, ostream& out) const
{
  for (vector<Value*>::const_iterator valueIt = valueBegin; valueIt != valueEnd; ++valueIt)
    {
      (*attributeIt)->printValue(**valueIt, out);
      out << " has:" << endl << "  " << data->countNoiseOnPresent(attributeIt, **valueIt, attributeBegin) << " present noise and pretends to have " << (*valueIt)->getPresentNoise() << endl << "  " << data->countNoiseOnPresentAndPotential(attributeIt, **valueIt, attributeBegin) << " present and potential noise and pretends to have " << (*valueIt)->getPresentAndPotentialNoise() << endl;
    }
}
#endif
