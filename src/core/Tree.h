// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef TREE_H_
#define TREE_H_

#ifdef MIN_SIZE_ELEMENT_PRUNING
#include <utility>
#endif

#include "../measures/MinSize.h"
#include "../measures/MaxSize.h"
#include "../measures/MinArea.h"
#include "../measures/MaxArea.h"
#include "../measures/MinGroupCover.h"
#include "../measures/MaxGroupCover.h"
#include "../measures/MinGroupCoverRatio.h"
#include "../measures/MinGroupCoverPiatetskyShapiro.h"
#include "../measures/MinGroupCoverLeverage.h"
#include "../measures/MinGroupCoverForce.h"
#include "../measures/MinGroupCoverYulesQ.h"
#include "../measures/MinGroupCoverYulesY.h"
#include "SymmetricAttribute.h"
#include "MetricAttribute.h"
#include "Trie.h"
#include "NoisyTupleFileReader.h"
#include "NoisyTuples.h"
#include "IndistinctSkyPatterns.h"
#include "Node.h"

class Tree
{
 public:
  Tree() = delete;
  Tree(const Tree&) = delete;
  Tree(Tree&&) = delete;
  Tree(const char* dataFileName, const float densityThreshold, const vector<double>& epsilonVector, const vector<unsigned int>& cliqueDimensions, const vector<double>& tauVector, const vector<unsigned int>& minSizes, const unsigned int minArea, const bool isReductionOnly, const unsigned int maximalNbOfClosedNSetsForAgglomeration, const char* inputElementSeparator, const char* inputDimensionSeparator, const char* outputFileName, const char* outputDimensionSeparator, const char* patternSizeSeparator, const char* sizeSeparator, const char* sizeAreaSeparator, const bool isSizePrinted, const bool isAreaPrinted);

  virtual ~Tree();

  Tree& operator=(const Tree&) = delete;
  Tree& operator=(Tree&&) = delete;

  void initMeasures(const vector<unsigned int>& maxSizes, const int maxArea, const vector<string>& groupFileNames, vector<unsigned int>& groupMinSizes, const vector<unsigned int>& groupMaxSizes, const vector<vector<float>>& groupMinRatios, const vector<vector<float>>& groupMinPiatetskyShapiros, const vector<vector<float>>& groupMinLeverages, const vector<vector<float>>& groupMinForces, const vector<vector<float>>& groupMinYulesQs, const vector<vector<float>>& groupMinYulesYs, const char* groupElementSeparator, const char* groupDimensionElementsSeparator);
  void peel();
  virtual void terminate();

 protected:
  vector<Attribute*> attributes;
  vector<Measure*> mereConstraints;

#ifdef VERBOSE_DIM_CHOICE
  static vector<unsigned int> internal2ExternalAttributeOrder;
#endif
  static vector<unsigned int> external2InternalAttributeOrder;
  static vector<unordered_map<string, unsigned int>> labels2Ids;
  static unsigned int firstSymmetricAttributeId;
  static unsigned int lastSymmetricAttributeId;
  static Trie* data;
  static vector<unsigned int> minSizes;
  static double minArea;

  static bool isAgglomeration;
  static ofstream outputFile;
  static string outputDimensionSeparator;
  static string patternSizeSeparator;
  static string sizeSeparator;
  static string sizeAreaSeparator;
  static bool isSizePrinted;
  static bool isAreaPrinted;

#ifdef TIME
  static steady_clock::time_point overallBeginning;
#endif
#ifdef DETAILED_TIME
  static steady_clock::time_point startingPoint;
  static double parsingDuration;
  static double preProcessingDuration;
#endif
  // CLEAN: All these static attributes should not be static for a detailed analysis of the enumeration
#ifdef NB_OF_LEFT_NODES
  static unsigned int nbOfLeftNodes;
#endif
#ifdef NB_OF_CLOSED_N_SETS
  static unsigned int nbOfClosedNSets;
#endif

  Tree(const Tree& parent, const vector<Measure*>& mereConstraints);

  friend ostream& operator<<(ostream& out, const Tree& tree);
#ifdef DEBUG
  void printNode(ostream& out) const;
#endif

  virtual void leftSubtree(const unsigned int presentAttributeId, const unsigned int originalValueId) const;
  void rightSubtree(const vector<Attribute*>::iterator absentAttributeIt, const vector<Value*>::iterator valueIt);

  void setPresent(const unsigned int presentAttributeId, const unsigned int valueOriginalId);
  const bool setAbsent(vector<IrrelevantValueIds>& irrelevantValueIdsVector);
#ifdef MIN_SIZE_ELEMENT_PRUNING
  const bool findMinSizeIrrelevantValuesAndCheckConstraints(vector<IrrelevantValueIds>& irrelevantValueIdsVector, const vector<Attribute*>::iterator previousAbsentAttributeIt);
  virtual vector<unsigned int> minSizeIrrelevancyThresholds() const;
#endif

  virtual const bool violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent);
  virtual const bool violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent);
  virtual const bool dominated() const;
  virtual void validPattern() const;

  static void setMinParametersInClique(vector<unsigned int>& parameterVector);
  static void setMaxParametersInClique(vector<unsigned int>& parameterVector);
  static vector<Measure*> childMeasures(const vector<Measure*>& parentMeasures, const unsigned int presentAttributeId, const unsigned int originalValueId);
  static void deleteMeasures(vector<Measure*>& measures);

#ifdef ASSERT
  void assertValues(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const vector<Attribute*>::const_iterator attributeIt, const vector<Attribute*>::const_iterator attributeBegin, ostream& out) const;
#endif
};

#endif /*TREE_H_*/
