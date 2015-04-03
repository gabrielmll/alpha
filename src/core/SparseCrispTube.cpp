// Copyright 2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "SparseCrispTube.h"

float SparseCrispTube::densityThreshold;

SparseCrispTube::SparseCrispTube() : tube()
{
}

SparseCrispTube* SparseCrispTube::clone() const
{
  return new SparseCrispTube(*this);
}

void SparseCrispTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  for (const unsigned int hyperplane : tube)
    {
      for (const unsigned int id : prefix)
	{
	  out << id << ' ';
	}
      out << hyperplane << " 1" << endl;
    }
}

const bool SparseCrispTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<unordered_map<unsigned int, unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->substractPotentialNoise(element, Attribute::noisePerUnit);
  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
    {
      (*intersectionIt)[element] -= Attribute::noisePerUnit;
    }
  tube.insert(element);
  return tube.bucket_count() + 2 * tube.size() * sizeof(unsigned int*) > (*attributeIt)->sizeOfPotential() * densityThreshold; // In the worst case (all values in th same bucket), the unordered_set<unsigned int> takes more space than a vector<unsigned int> * densityThreshold
}

const unsigned int SparseCrispTube::setSelfLoopsInSymmetricAttribute(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId)
{
  // Necessarily symmetric
  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
    {
      (*intersectionIt)[hyperplaneId] -= Attribute::noisePerUnit;
    }
  (*attributeIt)->substractPotentialNoise(hyperplaneId, Attribute::noisePerUnit);
  tube.insert(hyperplaneId);
  return Attribute::noisePerUnit;
}

const unordered_set<unsigned int>& SparseCrispTube::getTube() const
{
  return tube;
}

void SparseCrispTube::setDensityThreshold(const float densityThresholdParam)
{
  densityThreshold = densityThresholdParam / 8;
}

const unsigned int SparseCrispTube::noiseOnValues(const vector<Attribute*>::const_iterator attributeIt, const vector<unsigned int>& valueOriginalIds) const
{
  unsigned int oldNoise = 0;
  for (const unsigned int valueOriginalId : valueOriginalIds)
    {
      if (tube.find(valueOriginalId) == tube.end())
	{
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

const unsigned int SparseCrispTube::setPresent(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the present attribute
  if (tube.find(presentValue.getOriginalId()) == tube.end())
    {
      return Attribute::noisePerUnit;
    }
  return 0;
}

const unsigned int SparseCrispTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // *this necessarily relates to the present attribute
  const unsigned int presentValueId = presentValue.getId();
  if (tube.find(presentValue.getOriginalId()) == tube.end())
    {
      (*potentialOrAbsentValueIntersectionIt)[presentValueId] += Attribute::noisePerUnit;
      return Attribute::noisePerUnit;
    }
  return 0;
}


const unsigned int SparseCrispTube::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValues(absentAttributeIt, absentValueOriginalIds);
}

const unsigned int SparseCrispTube::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValues(absentAttributeIt, absentValueOriginalIds);
}

const unsigned int SparseCrispTube::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*valueIt)->addPresentNoise(Attribute::noisePerUnit);
	  newNoise += Attribute::noisePerUnit;
	}
    }
  return newNoise;
}

const unsigned int SparseCrispTube::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*potentialOrAbsentValueIntersectionIt)[(*valueIt)->getId()] += Attribute::noisePerUnit;
	  newNoise += Attribute::noisePerUnit;
	}
    }
  return newNoise;
}

void SparseCrispTube::presentFixPotentialValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*valueIt)->addPresentNoise(Attribute::noisePerUnit);
	  const unsigned int valueId = (*valueIt)->getId();
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] += Attribute::noisePerUnit;
	    }
	}
    }
}

void SparseCrispTube::presentFixAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*valueIt)->addPresentNoise(Attribute::noisePerUnit);
	  const unsigned int valueId = (*valueIt)->getId();
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] += Attribute::noisePerUnit;
	    }
	}
    }
}

const unsigned int SparseCrispTube::absentFixPresentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*valueIt)->substractPotentialNoise(Attribute::noisePerUnit);
	  const unsigned int valueId = (*valueIt)->getId();
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] -= Attribute::noisePerUnit;
	    }
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

const unsigned int SparseCrispTube::absentFixPotentialValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*valueIt)->substractPotentialNoise(Attribute::noisePerUnit);
	  const unsigned int valueId = (*valueIt)->getId();
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] -= Attribute::noisePerUnit;
	    }
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

const unsigned int SparseCrispTube::absentFixPresentValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*absentValueIntersectionIt)[(*valueIt)->getId()] -= Attribute::noisePerUnit;
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

const unsigned int SparseCrispTube::absentFixPotentialValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*absentValueIntersectionIt)[(*valueIt)->getId()] -= Attribute::noisePerUnit;
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

void SparseCrispTube::absentFixAbsentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  (*valueIt)->substractPotentialNoise(Attribute::noisePerUnit);
	  const unsigned int valueId = (*valueIt)->getId();
	  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	    {
	      (*intersectionIt)[valueId] -= Attribute::noisePerUnit;
	    }
	}
    }
}

const unsigned int SparseCrispTube::countNoise(const vector<vector<Element>>::iterator dimensionIt) const
{
  unsigned int noise = 0;
  for (Element& element : *dimensionIt)
    {
      if (tube.find(element.getId()) == tube.end())
	{
	  noise += Attribute::noisePerUnit;
	  element.addNoise(Attribute::noisePerUnit);
	}
    }
  return noise;
}

pair<unsigned int, const bool> SparseCrispTube::countNoiseUpToThresholds(const vector<unsigned int>::const_iterator noiseThresholdIt, const vector<vector<Element>>::iterator dimensionIt, const vector<vector<Element>::iterator>::iterator tupleIt) const
{
  unsigned int noise = 0;
  for (; *tupleIt != dimensionIt->end(); ++*tupleIt)
    {
      if (tube.find((*tupleIt)->getId()) == tube.end())
	{
	  noise += Attribute::noisePerUnit;
	  (*tupleIt)->addNoise(Attribute::noisePerUnit);
	  if ((*tupleIt)->getNoise() > *noiseThresholdIt)
	    {
	      ++*tupleIt;
	      return pair<unsigned int, const bool>(noise, true);
	    }
	}
    }
  *tupleIt = dimensionIt->begin();
  return pair<unsigned int, const bool>(noise, false);
}

#ifdef ASSERT
const unsigned int SparseCrispTube::countNoiseOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      if (tube.find(value.getOriginalId()) == tube.end())
	{
	  return Attribute::noisePerUnit;
	}
      return 0;
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  noise += Attribute::noisePerUnit;
	}
    }
  return noise;
}

const unsigned int SparseCrispTube::countNoiseOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      if (tube.find(value.getOriginalId()) == tube.end())
	{
	  return Attribute::noisePerUnit;
	}
      return 0;
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  noise += Attribute::noisePerUnit;
	}
    }
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->potentialBegin(); valueIt != (*attributeIt)->potentialEnd(); ++valueIt)
    {
      if (tube.find((*valueIt)->getOriginalId()) == tube.end())
	{
	  noise += Attribute::noisePerUnit;
	}
    }
  return noise;
}
#endif
