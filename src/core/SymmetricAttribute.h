// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef SYMMETRIC_ATTRIBUTE_H_
#define SYMMETRIC_ATTRIBUTE_H_

#include "Attribute.h"

/* Although the code in Tree works for any number of symmetric attributes, this implementation is specific to two such attributes */
class SymmetricAttribute: public Attribute
{
 public:
  SymmetricAttribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon);
  SymmetricAttribute(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd);

  SymmetricAttribute* clone(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd) const;

  unordered_map<unsigned int, unsigned int> setLabels(unordered_map<string, unsigned int>& labels2Ids) const;

  /* FIXME: adapt to new semantics (see Attribute) */
  Value* moveValueFromPotentialToPresent(const unsigned int valueOriginalId);
  Value* moveValueFromPotentialToAbsent(const vector<Value*>::iterator valueIt);
  Value* moveSymmetricValueFromPotentialToPresent(const Value& symmetricValue);
  void moveSymmetricValueFromPotentialToAbsent(const Value* value);

  pair<bool, vector<unsigned int>> findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd, IrrelevantValueIds& irrelevantValueIds);
  void presentAndPotentialCleanAbsent(const unsigned int presentAndPotentialIrrelevancyThreshold, const vector<Attribute*>::iterator attributeIt);
  const bool closed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const;
  void cleanAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd);

 protected:
  static unsigned int firstSymmetricAttributeId;

  const bool symmetricValuesDoNotExtendPresent(const Value& value, const Value& symmetricValue, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const; /* must only be called on the second symmetric attribute */
  const bool symmetricValuesDoNotExtendPresentAndPotential(const Value& value, const Value& symmetricValue, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const; /* must only be called on the second symmetric attribute */
};

#endif /*SYMMETRIC_ATTRIBUTE_H_*/
