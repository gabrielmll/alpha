// Copyright 2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "SparseFuzzyTube.h"

float SparseFuzzyTube::densityThreshold;

SparseFuzzyTube::SparseFuzzyTube() : tube()
{
}

SparseFuzzyTube* SparseFuzzyTube::clone() const
{
  return new SparseFuzzyTube(*this);
}

void SparseFuzzyTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  for (const pair<unsigned int, unsigned int>& hyperplane : tube)
    {
      for (const unsigned int id : prefix)
	{
	  out << id << ' ';
	}
      out << hyperplane.first << ' ' << 1 - static_cast<double>(hyperplane.second) / Attribute::noisePerUnit << endl;
    }
}

const bool SparseFuzzyTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<unordered_map<unsigned int, unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->substractPotentialNoise(element, membership);
  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
    {
      (*intersectionIt)[element] -= membership;
    }
  tube[element] = Attribute::noisePerUnit - membership;
  return tube.bucket_count() * sizeof(unsigned int) + 2 * tube.size() * sizeof(unsigned int*) > (*attributeIt)->sizeOfPotential() * densityThreshold; // In the worst case (all values in th same bucket), the unordered_map<unsigned int, unsigned int> takes more space than a vector<unsigned int> * densityThreshold
}

const unsigned int SparseFuzzyTube::setSelfLoopsInSymmetricAttribute(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId)
{
  // Necessarily symmetric
  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
    {
      (*intersectionIt)[hyperplaneId] -= Attribute::noisePerUnit;
    }
  (*attributeIt)->substractPotentialNoise(hyperplaneId, Attribute::noisePerUnit);
  tube[hyperplaneId] = 0;
  return Attribute::noisePerUnit;
}

void SparseFuzzyTube::setDensityThreshold(const float densityThresholdParam)
{
  densityThreshold = densityThresholdParam;
}

const unsigned int SparseFuzzyTube::noiseOnValue(const unsigned int valueOriginalId) const
{
  const unordered_map<unsigned int, unsigned int>::const_iterator noisyTupleIt = tube.find(valueOriginalId);
  if (noisyTupleIt == tube.end())
    {
      return Attribute::noisePerUnit;
    }
  return noisyTupleIt->second;
}

const unsigned int SparseFuzzyTube::noiseOnValues(const vector<Attribute*>::const_iterator attributeIt, const vector<unsigned int>& valueOriginalIds) const
{
  unsigned int oldNoise = 0;
  for (const unsigned int valueOriginalId : valueOriginalIds)
    {
      oldNoise += noiseOnValue(valueOriginalId);
    }
  return oldNoise;
}

const unsigned int SparseFuzzyTube::setPresent(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the present attribute
  return noiseOnValue(presentValue.getOriginalId());
}

const unsigned int SparseFuzzyTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // *this necessarily relates to the present attribute
  const unsigned int noise = noiseOnValue(presentValue.getOriginalId());
  (*potentialOrAbsentValueIntersectionIt)[presentValue.getId()] += noise;
  return noise;
}

const unsigned int SparseFuzzyTube::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

const unsigned int SparseFuzzyTube::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*potentialOrAbsentValueIntersectionIt)[(*valueIt)->getId()] += newNoiseInHyperplane;
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

void SparseFuzzyTube::presentFixPotentialValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
    }
}

void SparseFuzzyTube::presentFixAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
    }
}

const unsigned int SparseFuzzyTube::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValues(absentAttributeIt, absentValueOriginalIds);
}

const unsigned int SparseFuzzyTube::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValues(absentAttributeIt, absentValueOriginalIds);
}

const unsigned int SparseFuzzyTube::absentFixPresentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*valueIt)->substractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

const unsigned int SparseFuzzyTube::absentFixPotentialValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*valueIt)->substractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

const unsigned int SparseFuzzyTube::absentFixPresentValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*absentValueIntersectionIt)[(*valueIt)->getId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

const unsigned int SparseFuzzyTube::absentFixPotentialValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*absentValueIntersectionIt)[(*valueIt)->getId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

void SparseFuzzyTube::absentFixAbsentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = noiseOnValue((*valueIt)->getOriginalId());
      (*valueIt)->substractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

const unsigned int SparseFuzzyTube::countNoise(const vector<vector<Element>>::iterator dimensionIt) const
{
  unsigned int noise = 0;
  for (Element& element : *dimensionIt)
    {
      const unsigned int noiseInHyperplane = noiseOnValue(element.getId());
      noise += noiseInHyperplane;
      element.addNoise(noiseInHyperplane);
    }
  return noise;
}

pair<unsigned int, const bool> SparseFuzzyTube::countNoiseUpToThresholds(const vector<unsigned int>::const_iterator noiseThresholdIt, const vector<vector<Element>>::iterator dimensionIt, const vector<vector<Element>::iterator>::iterator tupleIt) const
{
  unsigned int noise = 0;
  for (; *tupleIt != dimensionIt->end(); ++*tupleIt)
    {
      const unsigned int noiseInHyperplane = noiseOnValue((*tupleIt)->getId());
      noise += noiseInHyperplane;
      (*tupleIt)->addNoise(noiseInHyperplane);
      if ((*tupleIt)->getNoise() > *noiseThresholdIt)
	{
	  ++*tupleIt;
	  return pair<unsigned int, const bool>(noise, true);
	}
    }
  *tupleIt = dimensionIt->begin();
  return pair<unsigned int, const bool>(noise, false);
}

#ifdef ASSERT
const unsigned int SparseFuzzyTube::countNoiseOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      return noiseOnValue(value.getOriginalId());
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      noise += noiseOnValue((*valueIt)->getOriginalId());
    }
  return noise;
}

const unsigned int SparseFuzzyTube::countNoiseOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      return noiseOnValue(value.getOriginalId());
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      noise += noiseOnValue((*valueIt)->getOriginalId());
    }
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->potentialBegin(); valueIt != (*attributeIt)->potentialEnd(); ++valueIt)
    {
      noise += noiseOnValue((*valueIt)->getOriginalId());
    }
  return noise;
}
#endif
