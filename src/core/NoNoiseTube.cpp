// Copyright 2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "NoNoiseTube.h"

NoNoiseTube* NoNoiseTube::clone() const
{
  return new NoNoiseTube(*this);
}

void NoNoiseTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  for (const unsigned int id : prefix)
    {
      out << id << ' ';
    }
  out << "- 1" << endl;
}

const unsigned int NoNoiseTube::depth() const
{
  return 0;
}

const bool NoNoiseTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<unordered_map<unsigned int, unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts)
{
  // Never called
  return false;
}

const unsigned int NoNoiseTube::setSelfLoopsBeforeSymmetricAttributes(const unsigned int firstSymmetricAttributeId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId)
{
  // Never called
  return 0;
}

const unsigned int NoNoiseTube::setSelfLoopsInSymmetricAttribute(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId)
{
  // Never called
  return 0;
}

const unsigned int NoNoiseTube::setSelfLoopsAfterSymmetricAttributes(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts)
{
  // Never called
  return 0;
}

const unsigned int NoNoiseTube::setPresent(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

const unsigned int NoNoiseTube::setPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

const unsigned int NoNoiseTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  return 0;
}

const unsigned int NoNoiseTube::setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  return 0;
}

const unsigned int NoNoiseTube::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

const unsigned int NoNoiseTube::setAbsentAfterAbsentValuesMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  return 0;
}

const unsigned int NoNoiseTube::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  return 0;
}

const unsigned int NoNoiseTube::setAbsentAfterAbsentValuesMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  return 0;
}

const unsigned int NoNoiseTube::countNoise(const vector<vector<Element>>::iterator dimensionIt) const
{
  return 0;
}

pair<unsigned int, const bool> NoNoiseTube::countNoiseUpToThresholds(const vector<unsigned int>::const_iterator noiseThresholdIt, const vector<vector<Element>>::iterator dimensionIt, const vector<vector<Element>::iterator>::iterator tupleIt) const
{
  return pair<unsigned int, const bool>(0, false);
}

#ifdef ASSERT
const unsigned int NoNoiseTube::countNoiseOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  return 0;
}

const unsigned int NoNoiseTube::countNoiseOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  return 0;
}
#endif
