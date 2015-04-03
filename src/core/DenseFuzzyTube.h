// Copyright 2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef DENSE_FUZZY_TUBE_H_
#define DENSE_FUZZY_TUBE_H_

#include "SparseFuzzyTube.h"

class DenseFuzzyTube: public Tube
{
 public:
  DenseFuzzyTube(const SparseFuzzyTube& sparseFuzzyTube, const unsigned int nbOfHyperplanes);

  void print(vector<unsigned int>& prefix, ostream& out) const;
  const bool setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<unordered_map<unsigned int, unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts);
  const unsigned int setSelfLoopsInSymmetricAttribute(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId);

  const unsigned int setPresent(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  const unsigned int setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const;
  const unsigned int setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  const unsigned int setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;

  const unsigned int countNoise(const vector<vector<Element>>::iterator dimensionIt) const;
  pair<unsigned int, const bool> countNoiseUpToThresholds(const vector<unsigned int>::const_iterator noiseThresholdIt, const vector<vector<Element>>::iterator dimensionIt, const vector<vector<Element>::iterator>::iterator tupleIt) const;

#ifdef ASSERT
  const unsigned int countNoiseOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const;
  const unsigned int countNoiseOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const;
#endif

 protected:
  vector<unsigned int> tube;

  DenseFuzzyTube* clone() const;

  const unsigned int noiseOnValues(const vector<Attribute*>::const_iterator attributeIt, const vector<unsigned int>& valueOriginalIds) const;
  const unsigned int presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const;
  const unsigned int presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const;
  void presentFixPotentialValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  void presentFixAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  const unsigned int absentFixPresentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  const unsigned int absentFixPotentialValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  const unsigned int absentFixPresentValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;
  const unsigned int absentFixPotentialValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;
  void absentFixAbsentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
};

#endif /*DENSE_FUZZY_TUBE_H_*/
