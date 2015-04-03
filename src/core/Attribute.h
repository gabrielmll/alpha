// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef ATTRIBUTE_H_
#define ATTRIBUTE_H_

#include <string>
#include <unordered_map>
#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "Value.h"
#include "Element.h"
#include "IrrelevantValueIds.h"

#ifdef DEBUG
#include <iostream>
#endif

#if defined TIME || defined DETAILED_TIME
#include <chrono>

using namespace std::chrono;
#endif

using namespace boost;

class IrrelevantValueIds;

class Attribute
{
 public:
  static unsigned int noisePerUnit;

#ifdef VERBOSE_DIM_CHOICE
  static vector<unsigned int> internal2ExternalAttributeOrder;
#endif

#ifdef DEBUG
  void printPresent(ostream& out) const;
  void printPotential(ostream& out) const;
  void printAbsent(ostream& out) const;
  void printPotentialValuesFromIds(const vector<unsigned int>& potentialValueIds, ostream& out) const;
#endif

#if defined DEBUG || defined ASSERT
  void printValue(const Value& value, ostream& out) const;
  static void setInternal2ExternalAttributeOrder(const vector<unsigned int>& internal2ExternalAttributeOrder);
#endif

  Attribute();
  Attribute(const Attribute& otherAttribute) = delete;
  Attribute(Attribute&& otherAttribute) = delete;
  Attribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon);
  Attribute(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd);
  virtual ~Attribute();

  virtual Attribute* clone(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd) const;

  Attribute& operator=(const Attribute& otherAttribute) = delete;
  Attribute& operator=(Attribute&& otherAttribute) = delete;
  friend ostream& operator<<(ostream& out, const Attribute& attribute);
  void printValueFromOriginalId(const unsigned int valueOriginalId, ostream& out) const;

  const unsigned int getId() const;
  vector<unsigned int> getPresentOriginalIds() const;
  vector<Element> getElements() const;
  vector<Value*>::iterator presentBegin();
  vector<Value*>::iterator presentEnd();
  vector<Value*>::iterator potentialBegin();
  vector<Value*>::iterator potentialEnd();
  vector<Value*>::iterator absentBegin();
  vector<Value*>::iterator absentEnd();
  const bool presentEmpty() const;
  const unsigned int sizeOfPresent() const;
  const bool potentialEmpty() const;
  const unsigned int sizeOfPotential() const;
  const unsigned int globalSize() const;
  virtual unordered_map<unsigned int, unsigned int> setLabels(unordered_map<string, unsigned int>& labels2Ids) const;

  vector<Value*>::iterator valueItToEnumerate();
  virtual Value* moveValueFromPotentialToPresent(const unsigned int valueOriginalId);
  virtual Value* moveValueFromPotentialToAbsent(const vector<Value*>::iterator valueIt);
  vector<unsigned int> erasePotentialValues(const list<unsigned int>& potentialValueIds); /* returns the original ids of the erased elements */

  // WARNING: These two methods should be called after initialization only (all values ordered in potential)
  vector<vector<unsigned int>>::iterator getIntersectionsBeginWithPotentialValues(const unsigned int valueId);
  void substractPotentialNoise(const unsigned int valueId, const unsigned int noise);

  const double getAppeal(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  virtual pair<bool, vector<unsigned int>> findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd, IrrelevantValueIds& irrelevantValueIds);

#ifdef MIN_SIZE_ELEMENT_PRUNING
  const bool presentAndPotentialIrrelevant(const unsigned int presentAndPotentialIrrelevancyThreshold) const;
  virtual pair<bool, vector<unsigned int>> findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity(const unsigned int presentAndPotentialIrrelevancyThreshold, IrrelevantValueIds& irrelevantValueIds);
  void presentAndPotentialCleanAbsent(const unsigned int presentAndPotentialIrrelevancyThreshold);
#endif

  virtual const bool closed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  const bool valueDoesNotExtendPresentAndPotential(const Value& value, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  virtual void cleanAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd);
  virtual void absentValueToCleanFound(vector<Value*>::iterator& absentValueIt);
  virtual pair<const bool, vector<unsigned int>> tauFarValueOriginalValueIdsAndCheckConstraints(const Value* absentValue);
  virtual const bool finalizable() const;
  virtual vector<unsigned int> finalize(); /* returns the original ids of the elements moved to present */

  static const unsigned int lastAttributeId();
  static const vector<unsigned int>& getEpsilonVector();
  static void setDensityPrecedenceAndOutputFormat(const bool isDensestValuePreferredParam, const char* outputElementSeparatorParam, const char* emptySetStringParam, const char* elementNoiseSeparatorParam, const bool isNoisePrintedParam);
  static const bool noisePrinted();
  static void printOutputElementSeparator(ostream& out);
  static void printNoise(const float noise, ostream& out);

#ifdef DETAILED_TIME
  static void printDurations(ostream& out);
#endif

#ifdef ASSERT
  vector<Value*>::const_iterator presentBegin() const;
  vector<Value*>::const_iterator presentEnd() const;
  vector<Value*>::const_iterator potentialBegin() const;
  vector<Value*>::const_iterator potentialEnd() const;
  vector<Value*>::const_iterator absentBegin() const;
  vector<Value*>::const_iterator absentEnd() const;
#endif

 protected:
  unsigned int id;
  vector<Value*> potential;
  vector<Value*> absent;
  vector<Value*> present;

  static unsigned int maxId;
  static vector<unsigned int> epsilonVector;
  static vector<vector<string>> labelsVector;
  static bool isDensestValuePreferred;

  static string outputElementSeparator;
  static string emptySetString;
  static string elementNoiseSeparator;
  static bool isNoisePrinted;

#if !defined VERBOSE_DIM_CHOICE && (defined DEBUG || defined ASSERT)
  static vector<unsigned int> internal2ExternalAttributeOrder;
#endif

#ifdef DETAILED_TIME
  static double propagationCheckingDuration;
  static double closednessCheckingDuration;
  static double absentCleaningDuration;
#endif

  void printValues(const vector<Value*>& v, ostream& out) const;

  Value* createChildValue(const Value& parentValue, const unsigned int newId, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd, const vector<Attribute*>::const_iterator parentNextAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd) const;
  virtual void setChildValueIntersections(const vector<unsigned int>& noiseInIntersectionWithPresentValues, const vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& childNoiseInIntersectionWithPresentValues, vector<unsigned int>& childNoiseInIntersectionWithPresentAndPotentialValues) const;

  const bool valueDoesNotExtendPresent(const Value& value, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;

#ifdef MIN_SIZE_ELEMENT_PRUNING
  const bool presentAndPotentialIrrelevantValue(const Value& value, const unsigned int presentAndPotentialIrrelevancyThreshold) const;
#endif
};

#endif /*ATTRIBUTE_H_*/
