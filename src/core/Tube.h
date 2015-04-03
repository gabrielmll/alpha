// Copyright 2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef TUBE_H_
#define TUBE_H_

#include "AbstractData.h"

class Tube: public AbstractData
{
 public:
  virtual ~Tube();

  const unsigned int depth() const;
  const unsigned int setSelfLoopsBeforeSymmetricAttributes(const unsigned int firstSymmetricAttributeId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId);
  const unsigned int setSelfLoopsAfterSymmetricAttributes(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts);

  const unsigned int setPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  const unsigned int setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const;
  const unsigned int setAbsentAfterAbsentValuesMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const;
  const unsigned int setAbsentAfterAbsentValuesMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const;

 protected:
  virtual const unsigned int presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const = 0;
  virtual const unsigned int presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const = 0;
  virtual void presentFixPotentialValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual void presentFixAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual const unsigned int absentFixPresentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual const unsigned int absentFixPotentialValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
  virtual const unsigned int absentFixPresentValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const = 0;
  virtual const unsigned int absentFixPotentialValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const = 0;
  virtual void absentFixAbsentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const = 0;
};

#endif /*TUBE_H_*/
