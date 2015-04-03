// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef METRIC_ATTRIBUTE_H_
#define METRIC_ATTRIBUTE_H_

#include "Attribute.h"

class MetricAttribute: public Attribute
{
 public:
  MetricAttribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon, const double tau);
  MetricAttribute(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd);

  MetricAttribute* clone(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd) const;

  unordered_map<unsigned int, unsigned int> setLabels(unordered_map<string, unsigned int>& labels2Ids) const;

  /* FIXME: adapt to new semantics (see Attribute) */
  Value* moveValueFromPotentialToPresent(const unsigned int valueOriginalId);
  Value* moveValueFromPotentialToAbsent(const vector<Value*>::iterator valueIt);

  pair<bool, vector<unsigned int>> findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd, IrrelevantValueIds& irrelevantValueIds);
#ifdef MIN_SIZE_ELEMENT_PRUNING
  pair<bool, vector<unsigned int>> findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity(const unsigned int presentAndPotentialIrrelevancyThreshold, IrrelevantValueIds& irrelevantValueIds);
#endif
  const bool closed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  pair<const bool, vector<unsigned int>> tauFarValueOriginalValueIdsAndCheckConstraints(const Value* absentValue);
  const bool finalizable() const;
  vector<unsigned int> finalize(); /* returns the original ids of the elements moved to present */

  void eraseAbsentValuesBeneathTimestamp(const double timestamp);
  void eraseAbsentValuesBeyondTimestamp(const double timestamp);

 protected:
  static vector<double> tauVector; // 0 if the attribute is not totally ordered
  static vector<vector<double>> timestampsVector;

  void setChildValueIntersections(const vector<unsigned int>& noiseInIntersectionWithPresentValues, const vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& childNoiseInIntersectionWithPresentValues, vector<unsigned int>& childNoiseInIntersectionWithPresentAndPotentialValues) const;

  void absentValueToCleanFound(vector<Value*>::iterator& absentValueIt);
};

#endif /*METRIC_ATTRIBUTE_H_*/
