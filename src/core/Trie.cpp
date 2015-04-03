// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "Trie.h"

bool Trie::isCrisp = false;
NoNoiseTube Trie::noNoiseTube = NoNoiseTube();

Trie::Trie(const Trie& otherTrie): hyperplanes()
{
  copy(otherTrie);
}

Trie::Trie(Trie&& otherTrie): hyperplanes(otherTrie.hyperplanes)
{
}

Trie::Trie(const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd): hyperplanes()
{
  const unsigned int cardinality = *cardinalityIt;
  hyperplanes.reserve(cardinality);
  if (cardinalityIt + 2 == cardinalityEnd)
    {
      if (isCrisp)
	{
	  for (unsigned int hyperplaneId = 0; hyperplaneId != cardinality; ++hyperplaneId)
	    {
	      hyperplanes.push_back(new SparseCrispTube());
	    }
	}
      else
	{
	  for (unsigned int hyperplaneId = 0; hyperplaneId != cardinality; ++hyperplaneId)
	    {
	      hyperplanes.push_back(new SparseFuzzyTube());
	    }
	}
    }
  else
    {
      const vector<unsigned int>::const_iterator nbOfElementsInNextDiIt = cardinalityIt + 1;
      for (unsigned int hyperplaneId = 0; hyperplaneId != cardinality; ++hyperplaneId)
	{
	  hyperplanes.push_back(new Trie(nbOfElementsInNextDiIt, cardinalityEnd));
	}
    }
}

Trie::~Trie()
{
  for (AbstractData* hyperplane : hyperplanes)
    {
      if (hyperplane != &noNoiseTube)
	{
	  delete hyperplane;
	}
    }
}

Trie* Trie::clone() const
{
  return new Trie(*this);
}

Trie& Trie::operator=(const Trie& otherTrie)
{
  copy(otherTrie);
  return *this;
}

Trie& Trie::operator=(Trie&& otherTrie)
{
  hyperplanes = otherTrie.hyperplanes;
  return *this;
}

ostream& operator<<(ostream& out, const Trie& trie)
{
  vector<unsigned int> prefix;
  prefix.reserve(trie.depth());
  trie.print(prefix, out);
  return out;
}

void Trie::copy(const Trie& otherTrie)
{
  hyperplanes.reserve(otherTrie.hyperplanes.size());
  for (const AbstractData* hyperplane : otherTrie.hyperplanes)
    {
      hyperplanes.push_back(hyperplane->clone());
    }
}

const unsigned int Trie::depth() const
{
  if (hyperplanes.empty())
    {
      return 0;
    }
  return 1 + hyperplanes.front()->depth();
}

void Trie::print(vector<unsigned int>& prefix, ostream& out) const
{
  unsigned int hyperplaneId = 0;
  for (AbstractData* hyperplane : hyperplanes)
    {
      prefix.push_back(hyperplaneId++);
      hyperplane->print(prefix, out);
      prefix.pop_back();
    }
}

void Trie::setHyperplane(const unsigned int hyperplaneOldId, const unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator begin, const unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator end, const vector<unsigned int>& attributeOrder, const vector<unordered_map<unsigned int, unsigned int>>& oldIds2NewIds, vector<Attribute*>& attributes)
{
  const vector<Attribute*>::iterator nextAttributeIt = attributes.begin() + 1;
  const unsigned int hyperplaneId = oldIds2NewIds.front().at(hyperplaneOldId);
  AbstractData* hyperplane = hyperplanes[hyperplaneId];
  for (unordered_map<vector<unsigned int>, double, vector_hash<unsigned int>>::const_iterator tupleIt = begin; tupleIt != end; ++tupleIt)
    {
      vector<vector<vector<unsigned int>>::iterator> intersectionIts;
      intersectionIts.reserve(attributeOrder.size());
      intersectionIts.push_back(attributes.front()->getIntersectionsBeginWithPotentialValues(hyperplaneId));
      const unsigned int membership = ceil(tupleIt->second * Attribute::noisePerUnit); // ceil to guarantee that every pattern to be returned is returned
      if (hyperplane->setTuple(tupleIt->first, membership, attributeOrder.begin(), ++oldIds2NewIds.begin(), nextAttributeIt, intersectionIts))
	{
	  AbstractData* newHyperplane;
	  if (isCrisp)
	    {
	      newHyperplane = new DenseCrispTube(static_cast<SparseCrispTube&>(*hyperplane), (*nextAttributeIt)->sizeOfPotential());
	    }
	  else
	    {
	      newHyperplane = new DenseFuzzyTube(static_cast<SparseFuzzyTube&>(*hyperplane), (*nextAttributeIt)->sizeOfPotential());
	    }
	  delete hyperplane;
	  hyperplane = newHyperplane;
	  hyperplanes[hyperplaneId] = hyperplane;
	}
      attributes.front()->substractPotentialNoise(hyperplaneId, membership);
    }
}

void Trie::setSelfLoops(const unsigned int firstSymmetricAttributeId, const unsigned int lastSymmetricAttributeId, vector<Attribute*>& attributes)
{
  vector<vector<vector<unsigned int>>::iterator> intersectionIts;
  intersectionIts.reserve(Attribute::lastAttributeId());
  setSelfLoopsBeforeSymmetricAttributes(firstSymmetricAttributeId, lastSymmetricAttributeId, attributes.begin(), intersectionIts, 0);
}

void Trie::setPresent(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeBegin) const
{
  vector<vector<vector<unsigned int>>::iterator> intersectionIts;
  intersectionIts.reserve(Attribute::lastAttributeId());
  setPresent(presentAttributeIt, presentValue, attributeBegin, intersectionIts);
}

void Trie::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeBegin) const
{
  vector<vector<vector<unsigned int>>::iterator> intersectionIts;
  intersectionIts.reserve(Attribute::lastAttributeId());
  setAbsent(absentAttributeIt, absentValueOriginalIds, attributeBegin, intersectionIts);
}

const bool Trie::setTuple(const vector<unsigned int>& tuple, const unsigned int membership, vector<unsigned int>::const_iterator attributeIdIt, vector<unordered_map<unsigned int, unsigned int>>::const_iterator oldIds2NewIdsIt, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts)
{
  const unsigned int element = oldIds2NewIdsIt->at(tuple[*attributeIdIt]);
  (*attributeIt)->substractPotentialNoise(element, membership);
  for (vector<vector<unsigned int>>::iterator& intersectionIt : intersectionIts)
    {
      (*intersectionIt)[element] -= membership;
      ++intersectionIt;
    }
  intersectionIts.push_back((*attributeIt)->getIntersectionsBeginWithPotentialValues(element));
  AbstractData* hyperplane = hyperplanes[element];
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  if (hyperplane->setTuple(tuple, membership, ++attributeIdIt, ++oldIds2NewIdsIt, nextAttributeIt, intersectionIts))
    {
      AbstractData* newHyperplane;
      if (isCrisp)
	{
	  newHyperplane = new DenseCrispTube(static_cast<SparseCrispTube&>(*hyperplane), (*nextAttributeIt)->sizeOfPotential());
	}
      else
	{
	  newHyperplane = new DenseFuzzyTube(static_cast<SparseFuzzyTube&>(*hyperplane), (*nextAttributeIt)->sizeOfPotential());
	}
      delete hyperplane;
      hyperplanes[element] = newHyperplane;
    }
  return false;
}

void Trie::setCrisp()
{
  isCrisp = true;
}

// PERF: The amount of noise in every counter could be set at the construction of the attributes (but the time to insert the self loops is negligible)
const unsigned int Trie::setSelfLoopsBeforeSymmetricAttributes(const unsigned int firstSymmetricAttributeId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId)
{
  unsigned int noiseInSelfLoops = 0;
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  vector<AbstractData*>::iterator hyperplaneIt = hyperplanes.begin();
  for (unsigned int hyperplaneId = 0; hyperplaneId != hyperplanes.size(); ++hyperplaneId)
    {
      nextIntersectionIts.push_back((*attributeIt)->getIntersectionsBeginWithPotentialValues(hyperplaneId));
      unsigned int noiseInSelfLoopsInHyperplane;
      if (dimensionId == firstSymmetricAttributeId)
	{
	  noiseInSelfLoopsInHyperplane = (*hyperplaneIt)->setSelfLoopsInSymmetricAttribute(hyperplaneId, lastSymmetricAttributeId, nextAttributeIt, nextIntersectionIts, dimensionId + 1);
	}
      else
	{
	  noiseInSelfLoopsInHyperplane = (*hyperplaneIt)->setSelfLoopsBeforeSymmetricAttributes(firstSymmetricAttributeId, lastSymmetricAttributeId, nextAttributeIt, nextIntersectionIts, dimensionId + 1);
	}
      ++hyperplaneIt;
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[hyperplaneId] -= noiseInSelfLoopsInHyperplane;
	}
      (*attributeIt)->substractPotentialNoise(hyperplaneId, noiseInSelfLoopsInHyperplane);
      noiseInSelfLoops += noiseInSelfLoopsInHyperplane;
      nextIntersectionIts.pop_back();
    }
  return noiseInSelfLoops;
}

// PERF: The amount of noise in every counter could be set at the construction of the attributes (but the time to insert the self loops is negligible)
const unsigned int Trie::setSelfLoopsInSymmetricAttribute(const unsigned int hyperplaneId, const unsigned int lastSymmetricAttributeId, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, const unsigned int dimensionId)
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  AbstractData* hyperplane = hyperplanes[hyperplaneId];
  nextIntersectionIts.push_back((*attributeIt)->getIntersectionsBeginWithPotentialValues(hyperplaneId));
  unsigned int noiseInSelfLoopsInHyperplane;
  if (dimensionId == lastSymmetricAttributeId)
    {
      noiseInSelfLoopsInHyperplane = hyperplane->setSelfLoopsAfterSymmetricAttributes(nextAttributeIt, nextIntersectionIts);
      if (noiseInSelfLoopsInHyperplane == 0)
	{
	  // **hyperplaneIt is a tube that would be full of 0-noise values: turn it into a NoNoiseTube
	  delete hyperplane;
	  hyperplanes[hyperplaneId] = &noNoiseTube;
	  noiseInSelfLoopsInHyperplane = Attribute::noisePerUnit * (*nextAttributeIt)->sizeOfPotential();
	}
    }
  else
    {
      noiseInSelfLoopsInHyperplane = hyperplane->setSelfLoopsInSymmetricAttribute(hyperplaneId, lastSymmetricAttributeId, nextAttributeIt, nextIntersectionIts, dimensionId + 1);
    }
  for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
    {
      (*intersectionIt)[hyperplaneId] -= noiseInSelfLoopsInHyperplane;
    }
  (*attributeIt)->substractPotentialNoise(hyperplaneId, noiseInSelfLoopsInHyperplane);
  nextIntersectionIts.pop_back();
  return noiseInSelfLoopsInHyperplane;
}

// PERF: The amount of noise in every counter could be set at the construction of the attributes (but the time to insert the self loops is negligible)
const unsigned int Trie::setSelfLoopsAfterSymmetricAttributes(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts)
{
  unsigned int noiseInSelfLoops = 0;
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  vector<AbstractData*>::iterator hyperplaneIt = hyperplanes.begin();
  for (unsigned int hyperplaneId = 0; hyperplaneId != hyperplanes.size(); ++hyperplaneId)
    {
      nextIntersectionIts.push_back((*attributeIt)->getIntersectionsBeginWithPotentialValues(hyperplaneId));
      unsigned int noiseInSelfLoopsInHyperplane = (*hyperplaneIt)->setSelfLoopsAfterSymmetricAttributes(nextAttributeIt, nextIntersectionIts);
      if (noiseInSelfLoopsInHyperplane == 0)
	{
	  // **hyperplaneIt is a tube that would be full of 0-noise values: turn it into a NoNoiseTube
	  delete *hyperplaneIt;
	  hyperplanes[hyperplaneId] = &noNoiseTube;
	  noiseInSelfLoopsInHyperplane = Attribute::noisePerUnit * (*nextAttributeIt)->sizeOfPotential();
	}
      ++hyperplaneIt;
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[hyperplaneId] -= noiseInSelfLoopsInHyperplane;
	}
      (*attributeIt)->substractPotentialNoise(hyperplaneId, noiseInSelfLoopsInHyperplane);
      noiseInSelfLoops += noiseInSelfLoopsInHyperplane;
      nextIntersectionIts.pop_back();
    }
  return noiseInSelfLoops;
}

const unsigned int Trie::setPresent(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  if (attributeIt == presentAttributeIt)
    {
      nextIntersectionIts.push_back(presentValue.getIntersectionsBeginWithPresentValues());
      const unsigned int newNoiseInHyperplane = hyperplanes[presentValue.getOriginalId()]->setPresentAfterPresentValueMet(nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      return newNoiseInHyperplane;
    }
  presentFixPotentialValues(**attributeIt, presentAttributeIt, presentValue, nextAttributeIt, intersectionIts);
  presentFixAbsentValues(**attributeIt, presentAttributeIt, presentValue, nextAttributeIt, intersectionIts);
  return presentFixPresentValues(**attributeIt, presentAttributeIt, presentValue, nextAttributeIt, nextIntersectionIts);
}

const unsigned int Trie::setPresentAfterPresentValueMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  presentFixPotentialValuesAfterPresentValueMet(**attributeIt, nextAttributeIt, intersectionIts);
  presentFixAbsentValuesAfterPresentValueMet(**attributeIt, nextAttributeIt, intersectionIts);
  return presentFixPresentValuesAfterPresentValueMet(**attributeIt, nextAttributeIt, nextIntersectionIts);
}

const unsigned int Trie::setPresentAfterPotentialOrAbsentUsed(const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  if (attributeIt == presentAttributeIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[presentValue.getOriginalId()]->setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(attributeIt + 1, potentialOrAbsentValueIntersectionIt + 1);
      (*potentialOrAbsentValueIntersectionIt)[presentValue.getId()] += newNoiseInHyperplane;
      return newNoiseInHyperplane;
    }
  return presentFixPresentValuesAfterPotentialOrAbsentUsed(**attributeIt, presentAttributeIt, presentValue, attributeIt + 1, potentialOrAbsentValueIntersectionIt, potentialOrAbsentValueIntersectionIt + 1);
}

const unsigned int Trie::setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt) const
{
  return presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(**attributeIt, attributeIt + 1, potentialOrAbsentValueIntersectionIt, potentialOrAbsentValueIntersectionIt + 1);
}

const unsigned int Trie::presentFixPresentValues(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentValues());
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setPresent(presentAttributeIt, presentValue, nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

const unsigned int Trie::presentFixPresentValuesAfterPresentValueMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentValues());
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setPresentAfterPresentValueMet(nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

const unsigned int Trie::presentFixPresentValuesAfterPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextPotentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setPresentAfterPotentialOrAbsentUsed(presentAttributeIt, presentValue, nextAttributeIt, nextPotentialOrAbsentValueIntersectionIt);
      (*potentialOrAbsentValueIntersectionIt)[(*valueIt)->getId()] += newNoiseInHyperplane;
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

const unsigned int Trie::presentFixPresentValuesAfterPresentValueMetAndPotentialOrAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator potentialOrAbsentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextPotentialOrAbsentValueIntersectionIt) const
{
  unsigned int newNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(nextAttributeIt, nextPotentialOrAbsentValueIntersectionIt);
      (*potentialOrAbsentValueIntersectionIt)[(*valueIt)->getId()] += newNoiseInHyperplane;
      newNoise += newNoiseInHyperplane;
    }
  return newNoise;
}

void Trie::presentFixPotentialValues(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setPresentAfterPotentialOrAbsentUsed(presentAttributeIt, presentValue, nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentValues());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
    }
}

void Trie::presentFixAbsentValues(Attribute& currentAttribute, const vector<Attribute*>::iterator presentAttributeIt, Value& presentValue, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setPresentAfterPotentialOrAbsentUsed(presentAttributeIt, presentValue, nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentValues());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
    }
}

void Trie::presentFixPotentialValuesAfterPresentValueMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentValues());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
    }
}

void Trie::presentFixAbsentValuesAfterPresentValueMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      const unsigned int newNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setPresentAfterPresentValueMetAndPotentialOrAbsentUsed(nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentValues());
      (*valueIt)->addPresentNoise(newNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] += newNoiseInHyperplane;
	}
    }
}

const unsigned int Trie::setAbsent(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  if (attributeIt == absentAttributeIt)
    {
      unsigned int oldNoise = 0;
      for (const unsigned int absentValueOriginalId : absentValueOriginalIds)
	{
	  oldNoise += hyperplanes[absentValueOriginalId]->setAbsentAfterAbsentValuesMet(nextAttributeIt, nextIntersectionIts);
	}
      return oldNoise;
    }
  absentFixAbsentValues(**attributeIt, absentAttributeIt, absentValueOriginalIds, nextAttributeIt, intersectionIts);
  return absentFixPresentValues(**attributeIt, absentAttributeIt, absentValueOriginalIds, nextAttributeIt, intersectionIts, nextIntersectionIts) + absentFixPotentialValues(**attributeIt, absentAttributeIt, absentValueOriginalIds, nextAttributeIt, intersectionIts, nextIntersectionIts);
}

const unsigned int Trie::setAbsentAfterAbsentValuesMet(const vector<Attribute*>::iterator attributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  vector<vector<vector<unsigned int>>::iterator> nextIntersectionIts(incrementIterators(intersectionIts));
  absentFixAbsentValuesAfterAbsentValuesMet(**attributeIt, nextAttributeIt, intersectionIts);
  return absentFixPresentValuesAfterAbsentValuesMet(**attributeIt, nextAttributeIt, intersectionIts, nextIntersectionIts) + absentFixPotentialValuesAfterAbsentValuesMet(**attributeIt, nextAttributeIt, intersectionIts, nextIntersectionIts);
}

const unsigned int Trie::setAbsentAfterAbsentUsed(const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt = absentValueIntersectionIt + 1;
  if (attributeIt == absentAttributeIt)
    {
      unsigned int oldNoise = 0;
      for (const unsigned int absentValueOriginalId : absentValueOriginalIds)
	{
	  oldNoise += hyperplanes[absentValueOriginalId]->setAbsentAfterAbsentValuesMetAndAbsentUsed(nextAttributeIt, nextAbsentValueIntersectionIt);
	}
      return oldNoise;
    }
  return absentFixPresentValuesAfterAbsentUsed(**attributeIt, absentAttributeIt, absentValueOriginalIds, nextAttributeIt, absentValueIntersectionIt, nextAbsentValueIntersectionIt) + absentFixPotentialValuesAfterAbsentUsed(**attributeIt, absentAttributeIt, absentValueOriginalIds, nextAttributeIt, absentValueIntersectionIt, nextAbsentValueIntersectionIt);
}

const unsigned int Trie::setAbsentAfterAbsentValuesMetAndAbsentUsed(const vector<Attribute*>::iterator attributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt) const
{
  const vector<Attribute*>::iterator nextAttributeIt = attributeIt + 1;
  const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt = absentValueIntersectionIt + 1;
  return absentFixPresentValuesAfterAbsentValuesMetAndAbsentUsed(**attributeIt, nextAttributeIt, absentValueIntersectionIt, nextAbsentValueIntersectionIt) + absentFixPotentialValuesAfterAbsentValuesMetAndAbsentUsed(**attributeIt, nextAttributeIt, absentValueIntersectionIt, nextAbsentValueIntersectionIt);
}

const unsigned int Trie::absentFixPresentValues(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsent(absentAttributeIt, absentValueOriginalIds, nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
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

const unsigned int Trie::absentFixPotentialValues(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsent(absentAttributeIt, absentValueOriginalIds, nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
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

const unsigned int Trie::absentFixPresentValuesAfterAbsentValuesMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsentAfterAbsentValuesMet(nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
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

const unsigned int Trie::absentFixPotentialValuesAfterAbsentValuesMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts, vector<vector<vector<unsigned int>>::iterator>& nextIntersectionIts) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      nextIntersectionIts.push_back((*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsentAfterAbsentValuesMet(nextAttributeIt, nextIntersectionIts);
      nextIntersectionIts.pop_back();
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

const unsigned int Trie::absentFixPresentValuesAfterAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsentAfterAbsentUsed(absentAttributeIt, absentValueOriginalIds, nextAttributeIt, nextAbsentValueIntersectionIt);
      (*absentValueIntersectionIt)[(*valueIt)->getId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

const unsigned int Trie::absentFixPotentialValuesAfterAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsentAfterAbsentUsed(absentAttributeIt, absentValueOriginalIds, nextAttributeIt, nextAbsentValueIntersectionIt);
      (*absentValueIntersectionIt)[(*valueIt)->getId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

const unsigned int Trie::absentFixPresentValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.presentBegin(); valueIt != currentAttribute.presentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsentAfterAbsentValuesMetAndAbsentUsed(nextAttributeIt, nextAbsentValueIntersectionIt);
      (*absentValueIntersectionIt)[(*valueIt)->getId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

const unsigned int Trie::absentFixPotentialValuesAfterAbsentValuesMetAndAbsentUsed(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, const vector<vector<unsigned int>>::iterator absentValueIntersectionIt, const vector<vector<unsigned int>>::iterator nextAbsentValueIntersectionIt) const
{
  unsigned int oldNoise = 0;
  for (vector<Value*>::iterator valueIt = currentAttribute.potentialBegin(); valueIt != currentAttribute.potentialEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsentAfterAbsentValuesMetAndAbsentUsed(nextAttributeIt, nextAbsentValueIntersectionIt);
      (*absentValueIntersectionIt)[(*valueIt)->getId()] -= oldNoiseInHyperplane;
      oldNoise += oldNoiseInHyperplane;
    }
  return oldNoise;
}

void Trie::absentFixAbsentValues(Attribute& currentAttribute, const vector<Attribute*>::iterator absentAttributeIt, const vector<unsigned int>& absentValueOriginalIds, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsentAfterAbsentUsed(absentAttributeIt, absentValueOriginalIds, nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      (*valueIt)->substractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

void Trie::absentFixAbsentValuesAfterAbsentValuesMet(Attribute& currentAttribute, const vector<Attribute*>::iterator nextAttributeIt, vector<vector<vector<unsigned int>>::iterator>& intersectionIts) const
{
  for (vector<Value*>::iterator valueIt = currentAttribute.absentBegin(); valueIt != currentAttribute.absentEnd(); ++valueIt)
    {
      const unsigned int oldNoiseInHyperplane = hyperplanes[(*valueIt)->getOriginalId()]->setAbsentAfterAbsentValuesMetAndAbsentUsed(nextAttributeIt, (*valueIt)->getIntersectionsBeginWithPresentAndPotentialValues());
      (*valueIt)->substractPotentialNoise(oldNoiseInHyperplane);
      const unsigned int valueId = (*valueIt)->getId();
      for (vector<vector<unsigned int>>::iterator intersectionIt : intersectionIts)
	{
	  (*intersectionIt)[valueId] -= oldNoiseInHyperplane;
	}
    }
}

void Trie::countNoise(vector<vector<Element>>& nSet) const
{
  const vector<vector<Element>>::iterator nextDimensionIt = nSet.begin() + 1;
  for (Element& element : nSet.front())
    {
      element.addNoise(hyperplanes[element.getId()]->countNoise(nextDimensionIt));
    }
}

const unsigned int Trie::countNoise(const vector<vector<Element>>::iterator dimensionIt) const
{
  unsigned int noise = 0;
  const vector<vector<Element>>::iterator nextDimensionIt = dimensionIt + 1;
  for (Element& element : *dimensionIt)
    {
      const unsigned int noiseInHyperplane = hyperplanes[element.getId()]->countNoise(nextDimensionIt);
      noise += noiseInHyperplane;
      element.addNoise(noiseInHyperplane);
    }
  return noise;
}

const bool Trie::lessNoisyNSet(const vector<unsigned int>& noiseThresholds, vector<vector<Element>>& nSet, vector<vector<Element>::iterator>& tuple) const
{
  const vector<unsigned int>::const_iterator noiseThresholdIt = noiseThresholds.begin();
  const vector<vector<Element>>::iterator dimensionIt = nSet.begin();
  const vector<vector<Element>::iterator>::iterator tupleIt = tuple.begin();
  const vector<unsigned int>::const_iterator nextNoiseThresholdIt = noiseThresholdIt + 1;
  const vector<vector<Element>>::iterator nextDimensionIt = dimensionIt + 1;
  const vector<vector<Element>::iterator>::iterator nextTupleIt = tupleIt + 1;
  for (; *tupleIt != dimensionIt->end(); ++*tupleIt)
    {
      const pair<unsigned int, const bool> noiseInHyperplaneAndIsPause = hyperplanes[(*tupleIt)->getId()]->countNoiseUpToThresholds(nextNoiseThresholdIt, nextDimensionIt, nextTupleIt);
      (*tupleIt)->addNoise(noiseInHyperplaneAndIsPause.first);
      if (noiseInHyperplaneAndIsPause.second)
	{
	  if (*nextTupleIt == nextDimensionIt->end())
	    {
	      *nextTupleIt = nextDimensionIt->begin();
	      ++*tupleIt;
	    }
	  return false;
	}
      if ((*tupleIt)->getNoise() > *noiseThresholdIt)
	{
	  ++*tupleIt;
	  return false;
	}
    }
  return true;
}

pair<unsigned int, const bool> Trie::countNoiseUpToThresholds(const vector<unsigned int>::const_iterator noiseThresholdIt, const vector<vector<Element>>::iterator dimensionIt, const vector<vector<Element>::iterator>::iterator tupleIt) const
{
  unsigned int noise = 0;
  const vector<unsigned int>::const_iterator nextNoiseThresholdIt = noiseThresholdIt + 1;
  const vector<vector<Element>>::iterator nextDimensionIt = dimensionIt + 1;
  const vector<vector<Element>::iterator>::iterator nextTupleIt = tupleIt + 1;
  for (; *tupleIt != dimensionIt->end(); ++*tupleIt)
    {
      pair<unsigned int, const bool> noiseInHyperplaneAndIsPause = hyperplanes[(*tupleIt)->getId()]->countNoiseUpToThresholds(nextNoiseThresholdIt, nextDimensionIt, nextTupleIt);
      (*tupleIt)->addNoise(noiseInHyperplaneAndIsPause.first);
      if (noiseInHyperplaneAndIsPause.second)
	{
	  if (*nextTupleIt == nextDimensionIt->end())
	    {
	      *nextTupleIt = nextDimensionIt->begin();
	      ++*tupleIt;
	    }
	  noiseInHyperplaneAndIsPause.first += noise;
	  return noiseInHyperplaneAndIsPause;
	}
      if ((*tupleIt)->getNoise() > *noiseThresholdIt)
	{
	  ++*tupleIt;
	  return pair<unsigned int, const bool>(noiseInHyperplaneAndIsPause.first + noise, true);
	}
      noise += noiseInHyperplaneAndIsPause.first;
    }
  *tupleIt = dimensionIt->begin();
  return pair<unsigned int, const bool>(noise, false);
}

#ifdef ASSERT
const unsigned int Trie::countNoiseOnPresent(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  const vector<Attribute*>::const_iterator nextAttributeIt = attributeIt + 1;
  if (attributeIt == valueAttributeIt)
    {
      return hyperplanes[value.getOriginalId()]->countNoiseOnPresent(valueAttributeIt, value, nextAttributeIt);
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      noise += hyperplanes[(*valueIt)->getOriginalId()]->countNoiseOnPresent(valueAttributeIt, value, nextAttributeIt);
    }
  return noise;
}

const unsigned int Trie::countNoiseOnPresentAndPotential(const vector<Attribute*>::const_iterator valueAttributeIt, const Value& value, const vector<Attribute*>::const_iterator attributeIt) const
{
  const vector<Attribute*>::const_iterator nextAttributeIt = attributeIt + 1;
  if (attributeIt == valueAttributeIt)
    {
      return hyperplanes[value.getOriginalId()]->countNoiseOnPresentAndPotential(valueAttributeIt, value, nextAttributeIt);
    }
  unsigned int noise = 0;
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->presentBegin(); valueIt != (*attributeIt)->presentEnd(); ++valueIt)
    {
      noise += hyperplanes[(*valueIt)->getOriginalId()]->countNoiseOnPresentAndPotential(valueAttributeIt, value, nextAttributeIt);
    }
  for (vector<Value*>::const_iterator valueIt = (*attributeIt)->potentialBegin(); valueIt != (*attributeIt)->potentialEnd(); ++valueIt)
    {
      noise += hyperplanes[(*valueIt)->getOriginalId()]->countNoiseOnPresentAndPotential(valueAttributeIt, value, nextAttributeIt);
    }
  return noise;
}
#endif

vector<vector<vector<unsigned int>>::iterator> Trie::incrementIterators(const vector<vector<vector<unsigned int>>::iterator>& iterators)
{
  vector<vector<vector<unsigned int>>::iterator> nextIterators;
  nextIterators.reserve(iterators.size());
  for (const vector<vector<unsigned int>>::iterator it : iterators)
    {
      nextIterators.push_back(it + 1);
    }
  return nextIterators;
}
