// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "Value.h"

Value::Value(const unsigned int idParam): id(idParam), originalId(idParam), presentNoise(0), presentAndPotentialNoise(0), noiseInIntersectionWithPresentValues(), noiseInIntersectionWithPresentAndPotentialValues()
{
}

Value::Value(const unsigned int idParam, const unsigned int presentAndPotentialNoiseParam, const vector<unsigned int>::const_iterator nbOfValuesPerAttributeBegin, const vector<unsigned int>::const_iterator nbOfValuesPerAttributeEnd, const vector<unsigned int>& noisesInIntersections): id(idParam), originalId(idParam), presentNoise(0), presentAndPotentialNoise(presentAndPotentialNoiseParam), noiseInIntersectionWithPresentValues(), noiseInIntersectionWithPresentAndPotentialValues()
{
  noiseInIntersectionWithPresentValues.reserve(noisesInIntersections.size());
  noiseInIntersectionWithPresentAndPotentialValues.reserve(noisesInIntersections.size());
  vector<unsigned int>::const_iterator sizeOfIntersectionIt = noisesInIntersections.begin();
  for (vector<unsigned int>::const_iterator nbOfValuesPerAttributeIt = nbOfValuesPerAttributeBegin; nbOfValuesPerAttributeIt != nbOfValuesPerAttributeEnd; ++nbOfValuesPerAttributeIt)
    {
      vector<unsigned int> noiseInIntersectionWithPresent(*nbOfValuesPerAttributeIt);
      noiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresent);
      vector<unsigned int> noiseInIntersectionWithPresentAndPotential(*nbOfValuesPerAttributeIt, *sizeOfIntersectionIt++);
      noiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotential);
    }
}

Value::Value(const Value& parent, const unsigned int idParam, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd): id(idParam), originalId(parent.originalId), presentNoise(parent.presentNoise), presentAndPotentialNoise(parent.presentAndPotentialNoise), noiseInIntersectionWithPresentValues(), noiseInIntersectionWithPresentAndPotentialValues()
{
  const unsigned int sizeOfIntersections = parent.noiseInIntersectionWithPresentValues.size();
  noiseInIntersectionWithPresentValues.resize(sizeOfIntersections);
  vector<vector<unsigned int>>::iterator noiseInIntersectionWithPresentValuesIt = noiseInIntersectionWithPresentValues.begin();
  noiseInIntersectionWithPresentAndPotentialValues.resize(sizeOfIntersections);
  vector<vector<unsigned int>>::iterator noiseInIntersectionWithPresentAndPotentialValuesIt = noiseInIntersectionWithPresentAndPotentialValues.begin();
  for (vector<unsigned int>::const_iterator sizeIt = sizeOfAttributeIt; sizeIt != sizeOfAttributeEnd; ++sizeIt)
    {
      noiseInIntersectionWithPresentValuesIt->reserve(*sizeIt);
      ++noiseInIntersectionWithPresentValuesIt;
      noiseInIntersectionWithPresentAndPotentialValuesIt->reserve(*sizeIt);
      ++noiseInIntersectionWithPresentAndPotentialValuesIt;
    }
}

const bool Value::operator<(const Value& otherValue) const
{
#if ENUMERATION_PROCESS == 0
  const unsigned int potentialNoise = presentAndPotentialNoise - presentNoise;
  const unsigned int otherPotentialNoise = otherValue.presentAndPotentialNoise - otherValue.presentNoise;
  return potentialNoise < otherPotentialNoise || (potentialNoise == otherPotentialNoise && presentNoise < otherValue.presentNoise);
#endif
#if ENUMERATION_PROCESS == 1
  return presentNoise < otherValue.presentNoise || (presentNoise == otherValue.presentNoise && presentAndPotentialNoise < otherValue.presentAndPotentialNoise);
#endif
}

const unsigned int Value::getId() const
{
  return id;
}

const unsigned int Value::getOriginalId() const
{
  return originalId;
}

const unsigned int Value::getPresentNoise() const
{
  return presentNoise;
}

const unsigned int Value::getPresentAndPotentialNoise() const
{
  return presentAndPotentialNoise;
}

void Value::addPresentNoise(const unsigned int noise)
{
  presentNoise += noise;
}

void Value::substractPotentialNoise(const unsigned int noise)
{
  presentAndPotentialNoise -= noise;
}

vector<vector<unsigned int>>::iterator Value::getIntersectionsBeginWithPresentValues()
{
  return noiseInIntersectionWithPresentValues.begin();
}

vector<vector<unsigned int>>::const_iterator Value::getIntersectionsBeginWithPresentValues() const
{
  return noiseInIntersectionWithPresentValues.begin();
}

vector<vector<unsigned int>>::iterator Value::getIntersectionsBeginWithPresentAndPotentialValues()
{
  return noiseInIntersectionWithPresentAndPotentialValues.begin();
}

vector<vector<unsigned int>>::const_iterator Value::getIntersectionsBeginWithPresentAndPotentialValues() const
{
  return noiseInIntersectionWithPresentAndPotentialValues.begin();
}

const bool Value::extendsPastPresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const
{
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentNoise + (*valueIt)->noiseInIntersectionWithPresentValues[intersectionIndex][id] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

const bool Value::extendsFuturePresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const
{
  const vector<unsigned int>& presentIntersectionsWithFutureValues = noiseInIntersectionWithPresentValues[noiseInIntersectionWithPresentValues.size() - reverseAttributeIndex];
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentNoise + presentIntersectionsWithFutureValues[(*valueIt)->id] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

const bool Value::extendsPastPresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const
{
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentAndPotentialNoise + (*valueIt)->noiseInIntersectionWithPresentAndPotentialValues[intersectionIndex][id] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

const bool Value::extendsFuturePresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const
{
  const vector<unsigned int>& presentAndPotentialIntersectionsWithFutureValues = noiseInIntersectionWithPresentAndPotentialValues[noiseInIntersectionWithPresentAndPotentialValues.size() - reverseAttributeIndex];
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentAndPotentialNoise + presentAndPotentialIntersectionsWithFutureValues[(*valueIt)->id] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

const bool Value::symmetricValuesExtendPastPresent(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const
{
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentNoise + (*valueIt)->noiseInIntersectionWithPresentValues[intersectionIndex - 1][id] + (*valueIt)->noiseInIntersectionWithPresentValues[intersectionIndex][id] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

const bool Value::symmetricValuesExtendFuturePresent(const Value& symmetricValue, const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const
{
  const unsigned int intersectionIndex = noiseInIntersectionWithPresentValues.size() - reverseAttributeIndex;
  const vector<unsigned int>& presentIntersectionsWithFutureValues1 = noiseInIntersectionWithPresentValues[intersectionIndex];
  const vector<unsigned int>& presentIntersectionsWithFutureValues2 = symmetricValue.noiseInIntersectionWithPresentValues[intersectionIndex + 1];
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentNoise + presentIntersectionsWithFutureValues1[(*valueIt)->id] + presentIntersectionsWithFutureValues2[(*valueIt)->id] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

const bool Value::symmetricValuesExtendPastPresentAndPotential(const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int intersectionIndex) const
{
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentAndPotentialNoise + (*valueIt)->noiseInIntersectionWithPresentAndPotentialValues[intersectionIndex - 1][id] + (*valueIt)->noiseInIntersectionWithPresentAndPotentialValues[intersectionIndex][id] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

const bool Value::symmetricValuesExtendFuturePresentAndPotential(const Value& symmetricValue, const vector<Value*>::const_iterator valueBegin, const vector<Value*>::const_iterator valueEnd, const unsigned int threshold, const unsigned int reverseAttributeIndex) const
{
  const unsigned int intersectionIndex = noiseInIntersectionWithPresentAndPotentialValues.size() - reverseAttributeIndex;
  const vector<unsigned int>& presentAndPotentialIntersectionsWithFutureValues1 = noiseInIntersectionWithPresentAndPotentialValues[intersectionIndex];
  const vector<unsigned int>& presentAndPotentialIntersectionsWithFutureValues2 = symmetricValue.noiseInIntersectionWithPresentAndPotentialValues[intersectionIndex + 1];
  vector<Value*>::const_iterator valueIt = valueBegin;
  for (; valueIt != valueEnd && (*valueIt)->presentAndPotentialNoise + presentAndPotentialIntersectionsWithFutureValues1[(*valueIt)->id] + presentAndPotentialIntersectionsWithFutureValues2[(*valueIt)->id] <= threshold; ++valueIt)
    {
    }
  return valueIt == valueEnd;
}

/*debug*/
// vector<vector<unsigned int>> Value::getNoiseInIntersectionsWithPresentValues() const
// {
//   return noiseInIntersectionWithPresentValues;
// }

// vector<vector<unsigned int>> Value::getNoiseInIntersectionsWithPresentAndPotentialValues() const
// {
//   return noiseInIntersectionWithPresentAndPotentialValues;
// }
/*/debug*/

const bool Value::smallerId(const Value* value, const Value* otherValue)
{
  return value->getId() < otherValue->getId();
}
