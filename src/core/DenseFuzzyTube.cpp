// Copyright 2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "DenseFuzzyTube.h"

DenseFuzzyTube::DenseFuzzyTube(const SparseFuzzyTube& sparseFuzzyTube, const unsigned int nbOfHyperplanes) : tube()
{
  tube.reserve(nbOfHyperplanes);
  for (unsigned int hyperplaneId = 0; hyperplaneId != nbOfHyperplanes; ++hyperplaneId)
    {
      tube.push_back(sparseFuzzyTube.noiseOnValue(hyperplaneId));
    }
}

DenseFuzzyTube* DenseFuzzyTube::clone() const
{
  return new DenseFuzzyTube(*this);
}

void DenseFuzzyTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  unsigned int hyperplaneId = 0;
  for (const unsigned int noise : tube)
    {
      if (noise != Attribute::noisePerUnit)
	{
	  for (const unsigned int id : prefix)
	    {
	      out << id << ' ';
	    }
	  out << hyperplaneId << ' ' << 1 - static_cast<double>(noise) / Attribute::noisePerUnit << endl;
	}
      ++hyperplaneId;
    }
}

const bool DenseFuzzyTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<unordered_map<unsigned int, unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->substractPotentialNoise(element, membership);
  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
    {
      (*intersectionIt)[element] -= membership;
    }
  tube[element] = Attribute::noisePerUnit - membership;
  return false;
}
const unsigned int DenseFuzzyTube::setSelfLoopsInSymmetricAttribute(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId)
{
  // Never called
  return 0;
}

const unsigned int DenseFuzzyTube::noiseOnValues(const vector<Attribute*>::const_iterator attributeIt, const vector<unsigned int>& valueOriginalIds) const
{
  unsigned int oldNoise = 0;
  for (const unsigned int valueOriginalId : valueOriginalIds)
    {
      oldNoise += tube[valueOriginalId];
    }
  return oldNoise;
}

const unsigned int DenseFuzzyTube::setPresent(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the present attribute
  return tube[presentValue.getOriginalId()];
}

const unsigned int DenseFuzzyTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // *this necessarily relates to the present attribute
  const unsigned int noise = tube[presentValue.getOriginalId()];
  (*potentialOrAbsentValueIntersectionIt)[presentValue.getId()] += noise;
  return noise;
}

const unsigned int DenseFuzzyTube::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

const unsigned int DenseFuzzyTube::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
      (*potentialOrAbsentValueIntersectionIt)[(*valueIt)->getId()] += newNoiseInHyperplane;
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

void DenseFuzzyTube::presentFixPotentialValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
    }
}

void DenseFuzzyTube::presentFixAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
    }
}

const unsigned int DenseFuzzyTube::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValues(absentAttributeIt, absentValueOriginalIds);
}

const unsigned int DenseFuzzyTube::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValues(absentAttributeIt, absentValueOriginalIds);
}

const unsigned int DenseFuzzyTube::absentFixPresentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
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

const unsigned int DenseFuzzyTube::absentFixPotentialValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
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

const unsigned int DenseFuzzyTube::absentFixPresentValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
      (*absentValueIntersectionIt)[(*valueIt)->getId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

const unsigned int DenseFuzzyTube::absentFixPotentialValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
      (*absentValueIntersectionIt)[(*valueIt)->getId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

void DenseFuzzyTube::absentFixAbsentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = tube[(*valueIt)->getOriginalId()];
      (*valueIt)->substractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

const unsigned int DenseFuzzyTube::countNoise(const vector<vector<Element>>::iterator dimensionIt) const
{
  unsigned int noise = 0;
  for (Element& element : *dimensionIt)
    {
      const unsigned int noiseInHyperplane = tube[element.getId()];
      noise += noiseInHyperplane;
      element.addNoise(noiseInHyperplane);
    }
  return noise;
}

pair<unsigned int, const bool> DenseFuzzyTube::countNoiseUpToThresholds(const vector<unsigned int>::const_iterator noiseThresholdIt, const vector<vector<Element>>::iterator dimensionIt, const vector<vector<Element>::iterator>::iterator tupleIt) const
{
  unsigned int noise = 0;
  for (; *tupleIt != dimensionIt->end(); ++*tupleIt)
    {
      const unsigned int noiseInHyperplane = tube[(*tupleIt)->getId()];
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
const unsigned int DenseFuzzyTube::countNoiseOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      return tube[value.getOriginalId()];
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      noise += tube[(*valueIt)->getOriginalId()];
    }
  return noise;
}

const unsigned int DenseFuzzyTube::countNoiseOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      return tube[value.getOriginalId()];
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      noise += tube[(*valueIt)->getOriginalId()];
    }
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->potentialBegin(); valueIt != (*attributeIt)->potentialEnd(); ++valueIt)
    {
      noise += tube[(*valueIt)->getOriginalId()];
    }
  return noise;
}
#endif
