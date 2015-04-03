// Copyright 2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "DenseCrispTube.h"

DenseCrispTube::DenseCrispTube(const SparseCrispTube& sparseCrispTube, const unsigned int nbOfHyperplanes) : tube()
{
  tube.resize(nbOfHyperplanes, true);
  const unordered_set<unsigned int>& sparseTube = sparseCrispTube.getTube();
  for (const unsigned int presentHyperplaneId : sparseTube)
    {
      tube.set(presentHyperplaneId, false);
    }
}

DenseCrispTube* DenseCrispTube::clone() const
{
  return new DenseCrispTube(*this);
}

void DenseCrispTube::print(vector<unsigned int>& prefix, ostream& out) const
{
  unsigned int hyperplaneId = 0;
  for (dynamic_bitset<>::size_type absentHyperplaneId = tube.find_first(); absentHyperplaneId != dynamic_bitset<>::npos; absentHyperplaneId = tube.find_next(absentHyperplaneId))
    {
      for (; hyperplaneId != absentHyperplaneId; ++hyperplaneId)
	{
	  for (const unsigned int id : prefix)
	    {
	      out << id << ' ';
	    }
	  out << hyperplaneId << " 1" << endl;	  
	}
      ++hyperplaneId;
    }
}

const bool DenseCrispTube::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<unordered_map<unsigned int, unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->substractPotentialNoise(element, Attribute::noisePerUnit);
  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
    {
      (*intersectionIt)[element] -= Attribute::noisePerUnit;
    }
  tube.set(element, false);
  return false;
}

const unsigned int DenseCrispTube::setSelfLoopsInSymmetricAttribute(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId)
{
  // Never called
  return 0;
}

const unsigned int DenseCrispTube::noiseOnValues(const vector<Attribute*>::const_iterator attributeIt, const vector<unsigned int>& valueOriginalIds) const
{
  unsigned int oldNoise = 0;
  for (const unsigned int valueOriginalId : valueOriginalIds)
    {
      if (tube[valueOriginalId])
	{
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

const unsigned int DenseCrispTube::setPresent(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the present attribute
  if (tube[presentValue.getOriginalId()])
    {
      return Attribute::noisePerUnit;
    }
  return 0;
}

const unsigned int DenseCrispTube::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  // *this necessarily relates to the present attribute
  if (tube[presentValue.getOriginalId()])
    {
      (*potentialOrAbsentValueIntersectionIt)[presentValue.getId()] += Attribute::noisePerUnit;
      return Attribute::noisePerUnit;
    }
  return 0;
}

const unsigned int DenseCrispTube::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValues(absentAttributeIt, absentValueOriginalIds);
}

const unsigned int DenseCrispTube::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  // *this necessarily relates to the absent attribute
  return noiseOnValues(absentAttributeIt, absentValueOriginalIds);
}

const unsigned int DenseCrispTube::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
	{
	  (*valueIt)->addPresentNoise(Attribute::noisePerUnit);
	  newNoise += Attribute::noisePerUnit;
	}
    }
  return newNoise;
}

const unsigned int DenseCrispTube::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
	{
	  (*potentialOrAbsentValueIntersectionIt)[(*valueIt)->getId()] += Attribute::noisePerUnit;
	  newNoise += Attribute::noisePerUnit;
	}
    }
  return newNoise;
}

void DenseCrispTube::presentFixPotentialValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
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

void DenseCrispTube::presentFixAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
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

const unsigned int DenseCrispTube::absentFixPresentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
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

const unsigned int DenseCrispTube::absentFixPotentialValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
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

const unsigned int DenseCrispTube::absentFixPresentValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
	{
	  (*absentValueIntersectionIt)[(*valueIt)->getId()] -= Attribute::noisePerUnit;
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

const unsigned int DenseCrispTube::absentFixPotentialValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
	{
	  (*absentValueIntersectionIt)[(*valueIt)->getId()] -= Attribute::noisePerUnit;
	  oldNoise += Attribute::noisePerUnit;
	}
    }
  return oldNoise;
}

void DenseCrispTube::absentFixAbsentValuesAfterAbsentValuesMet(Attribute& currentAttribute, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
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

const unsigned int DenseCrispTube::countNoise(const vector<vector<Element>>::iterator dimensionIt) const
{
  unsigned int noise = 0;
  for (Element& element : *dimensionIt)
    {
      if (tube[element.getId()])
	{
	  noise += Attribute::noisePerUnit;
	  element.addNoise(Attribute::noisePerUnit);
	}
    }
  return noise;
}

pair<unsigned int, const bool> DenseCrispTube::countNoiseUpToThresholds(const vector<unsigned int>::const_iterator noiseThresholdIt, const vector<vector<Element>>::iterator dimensionIt, const vector<vector<Element>::iterator>::iterator tupleIt) const
{
  unsigned int noise = 0;
  for (; *tupleIt != dimensionIt->end(); ++*tupleIt)
    {
      if (tube[(*tupleIt)->getId()])
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
const unsigned int DenseCrispTube::countNoiseOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      if (tube[value.getOriginalId()])
	{
	  return Attribute::noisePerUnit;
	}
      return 0;
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
	{
	  noise += Attribute::noisePerUnit;
	}
    }
  return noise;
}

const unsigned int DenseCrispTube::countNoiseOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  if (attributeIt == valueAttributeIt)
    {
      if (tube[value.getOriginalId()])
	{
	  return Attribute::noisePerUnit;
	}
      return 0;
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
	{
	  noise += Attribute::noisePerUnit;
	}
    }
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->potentialBegin(); valueIt != (*attributeIt)->potentialEnd(); ++valueIt)
    {
      if (tube[(*valueIt)->getOriginalId()])
	{
	  noise += Attribute::noisePerUnit;
	}
    }
  return noise;
}
#endif
