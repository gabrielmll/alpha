// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "SymmetricAttribute.h"

unsigned int SymmetricAttribute::firstSymmetricAttributeId;

SymmetricAttribute::SymmetricAttribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon): Attribute(nbOfValuesPerAttribute, epsilon)
{
  firstSymmetricAttributeId = id - 1;
}

SymmetricAttribute::SymmetricAttribute(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd): Attribute(parentAttributeIt, parentAttributeEnd, sizeOfAttributeIt, sizeOfAttributeEnd)
{
}

SymmetricAttribute* SymmetricAttribute::clone(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd) const
{
  return new SymmetricAttribute(parentAttributeIt, parentAttributeEnd, sizeOfAttributeIt, sizeOfAttributeEnd);
}

unordered_map<unsigned int, unsigned int> SymmetricAttribute::setLabels(unordered_map<string, unsigned int>& labels2Ids) const
{
  const unordered_map<unsigned int, unsigned int> oldId2NewIds = Attribute::setLabels(labels2Ids);
  while (labelsVector.size() <= id)
    {
      labelsVector.push_back(labelsVector.back());
    }
  return oldId2NewIds;
}

Value* SymmetricAttribute::moveValueFromPotentialToPresent(const unsigned int valueOriginalId)
{
  vector<Value*>::iterator valueIt = potential.begin();
  for (; (*valueIt)->getOriginalId() != valueOriginalId; ++valueIt)
    {
    }
  present.push_back(*valueIt);
  potential.erase(valueIt);
  return present.back();
}

Value* SymmetricAttribute::moveValueFromPotentialToAbsent(const vector<Value*>::iterator valueIt)
{
  absent.push_back(*valueIt);
  potential.erase(valueIt);
  return absent.back();
}

Value* SymmetricAttribute::moveSymmetricValueFromPotentialToPresent(const Value& symmetricValue)
{
  // symmetricValue has the same id, which is the index in potential too, as the value to move
  vector<Value*>::iterator valueIt = potential.begin() + symmetricValue.getId();
  present.push_back(*valueIt);
  potential.erase(valueIt);
  return present.back();
}

void SymmetricAttribute::moveSymmetricValueFromPotentialToAbsent(const Value* value)
{
  vector<Value*>::iterator symValueIt = lower_bound(potential.begin(), potential.end(), value, Value::smallerId);
  Value* symValue = *symValueIt;
  potential.erase(symValueIt);
  absent.push_back(symValue);
}

pair<bool, vector<unsigned int>> SymmetricAttribute::findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd, IrrelevantValueIds& irrelevantValueIds)
{
  const vector<Attribute*>::const_iterator symmetricAttributeBegin = attributeBegin + firstSymmetricAttributeId;
  const vector<Attribute*>::const_iterator symmetricAttributeEnd = symmetricAttributeBegin + 2;
  if (id == firstSymmetricAttributeId)
    {
      return Attribute::findIrrelevantValuesAndCheckTauContiguity(symmetricAttributeBegin, symmetricAttributeEnd, irrelevantValueIds);
    }
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  vector<unsigned int> newIrrelevantValueOriginalIds;
  newIrrelevantValueOriginalIds.reserve(potential.size());
  vector<Value*>::const_iterator symmetricValueIt = (*symmetricAttributeBegin)->potentialBegin();
  list<unsigned int>::iterator irrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.begin();
  for (const Value* potentialValue : potential)
    {
      if (irrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.end() && *irrelevantValueIdIt == potentialValue->getId())
	{
	  ++irrelevantValueIdIt;
	}
      else
	{
	  if (symmetricValuesDoNotExtendPresent(*potentialValue, **symmetricValueIt, attributeBegin, symmetricAttributeEnd))
	    {
#ifdef DEBUG
	      cout << labelsVector[id][potentialValue->getOriginalId()] << " in attributes " << internal2ExternalAttributeOrder[id - 1] << " and " << internal2ExternalAttributeOrder[id] << " will never be present nor prevent the closedness of any future pattern" << endl;
#endif
	      irrelevantValueIds.irrelevantValueIds.insert(irrelevantValueIdIt, potentialValue->getId());
	      newIrrelevantValueOriginalIds.push_back(potentialValue->getOriginalId());
	    }
	}
      ++symmetricValueIt;
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return pair<bool, vector<unsigned int>>(false, newIrrelevantValueOriginalIds);
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
void SymmetricAttribute::presentAndPotentialCleanAbsent(const unsigned int presentAndPotentialIrrelevancyThreshold, const vector<Attribute*>::iterator attributeIt)
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  Attribute* symmetricAttribute;
  if (id == firstSymmetricAttributeId)
    {
      symmetricAttribute = *(attributeIt + 1);
    }
  else
    {
      symmetricAttribute = *(attributeIt - 1);
    }
  vector<Value*>::iterator symmetricValueIt = symmetricAttribute->absentBegin();
  for (vector<Value*>::iterator absentValueIt = absent.begin(); absentValueIt != absent.end(); )
    {
      if ((*absentValueIt)->getPresentAndPotentialNoise() > presentAndPotentialIrrelevancyThreshold)
	{
	  symmetricAttribute->absentValueToCleanFound(symmetricValueIt);
	  absentValueToCleanFound(absentValueIt);
	}
      else
	{
	  ++symmetricValueIt;
	  ++absentValueIt;
	}
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
}
#endif

const bool SymmetricAttribute::symmetricValuesDoNotExtendPresent(const Value& value, const Value& symmetricValue, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<unsigned int>::const_iterator symmetricEpsilonIt = epsilonVector.begin() + id - 1;
  if (value.getPresentNoise() > *symmetricEpsilonIt)
    {
      return true;
    }
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin();
  for (unsigned int intersectionIndex = id; epsilonIt != symmetricEpsilonIt && value.symmetricValuesExtendPastPresent((*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *epsilonIt, --intersectionIndex); ++epsilonIt)
    {
      ++attributeIt;
    }
  if (!(epsilonIt == symmetricEpsilonIt && value.extendsPastPresent((*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *(++epsilonIt), 0)))
    {
      return true;
    }
  ++attributeIt;
  for (unsigned int reverseAttributeIndex = maxId - id; ++attributeIt != attributeEnd && value.symmetricValuesExtendFuturePresent(symmetricValue, (*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *(++epsilonIt), --reverseAttributeIndex); )
    {
    }
  return attributeIt != attributeEnd;
}

const bool SymmetricAttribute::closed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  if (id == firstSymmetricAttributeId)
    {
      return true;
    }
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  const vector<Attribute*>::const_iterator symmetricAttributeBegin = attributeBegin + firstSymmetricAttributeId;
  const vector<Attribute*>::const_iterator symmetricAttributeEnd = symmetricAttributeBegin + 2;
  vector<Value*>::const_iterator valueIt = absent.begin();
  for (vector<Value*>::const_iterator symmetricValueIt = (*symmetricAttributeBegin)->absentBegin(); valueIt != absent.end() && ((*symmetricAttributeBegin)->valueDoesNotExtendPresentAndPotential(**symmetricValueIt, symmetricAttributeBegin, symmetricAttributeEnd) || symmetricValuesDoNotExtendPresentAndPotential(**valueIt, **symmetricValueIt, attributeBegin, attributeEnd)); ++valueIt)
    {
      ++symmetricValueIt;
    }
#ifdef DETAILED_TIME
  closednessCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef DEBUG
  if (valueIt != absent.end())
    {
      cout << labelsVector[id][(*valueIt)->getOriginalId()] << " in attributes " << internal2ExternalAttributeOrder[id - 1] << " and " << internal2ExternalAttributeOrder[id] << " extend the pattern -> Prune!" << endl;
    }
#endif
  return valueIt == absent.end();
}

const bool SymmetricAttribute::symmetricValuesDoNotExtendPresentAndPotential(const Value& value, const Value& symmetricValue, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<unsigned int>::const_iterator symmetricEpsilonIt = epsilonVector.begin() + id - 1;
  if (value.getPresentAndPotentialNoise() > *symmetricEpsilonIt)
    {
      return true;
    }
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin();
  for (unsigned int intersectionIndex = id; epsilonIt != symmetricEpsilonIt && value.symmetricValuesExtendPastPresentAndPotential((*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *epsilonIt, --intersectionIndex) && value.symmetricValuesExtendPastPresentAndPotential((*attributeIt)->potentialBegin(), (*attributeIt)->potentialEnd(), *epsilonIt, intersectionIndex); ++epsilonIt)
    {
      ++attributeIt;
    }
  if (!(epsilonIt == symmetricEpsilonIt && value.extendsPastPresentAndPotential((*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *(++epsilonIt), 0)))
    {
      return true;
    }
  ++attributeIt;
  for (unsigned int reverseAttributeIndex = maxId - id; ++attributeIt != attributeEnd && value.symmetricValuesExtendFuturePresentAndPotential(symmetricValue, (*attributeIt)->presentBegin(), (*attributeIt)->presentEnd(), *(++epsilonIt), --reverseAttributeIndex) && value.symmetricValuesExtendFuturePresentAndPotential(symmetricValue, (*attributeIt)->potentialBegin(), (*attributeIt)->potentialEnd(), *epsilonIt, reverseAttributeIndex); )
    {
    }
  return attributeIt != attributeEnd;
}

void SymmetricAttribute::cleanAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  const vector<Attribute*>::const_iterator symmetricAttributeBegin = attributeBegin + firstSymmetricAttributeId;
  const vector<Attribute*>::const_iterator symmetricAttributeEnd = symmetricAttributeBegin + 2;
  if (id == firstSymmetricAttributeId)
    {
      Attribute& symmetricAttribute = **(symmetricAttributeBegin + 1);
      vector<Value*>::iterator symmetricValueIt = symmetricAttribute.absentBegin();
      for (vector<Value*>::iterator absentIt = absent.begin(); absentIt != absent.end(); )
	{
	  if (valueDoesNotExtendPresent(**absentIt, symmetricAttributeBegin, symmetricAttributeEnd))
	    {
	      symmetricAttribute.absentValueToCleanFound(symmetricValueIt);
	      absentValueToCleanFound(absentIt);
	    }
	  else
	    {
	      ++symmetricValueIt;
	      ++absentIt;
	    }
	}
#ifdef DETAILED_TIME
      absentCleaningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
      return;
    }
  vector<Value*>::iterator symmetricValueIt = (*symmetricAttributeBegin)->absentBegin();
  for (vector<Value*>::iterator absentIt = absent.begin(); absentIt != absent.end(); )
    {
      if (symmetricValuesDoNotExtendPresent(**absentIt, **symmetricValueIt, attributeBegin, attributeEnd))
	{
	  (*symmetricAttributeBegin)->absentValueToCleanFound(symmetricValueIt);
	  absentValueToCleanFound(absentIt);
	}
      else
	{
	  ++symmetricValueIt;
	  ++absentIt;
	}
    }
#ifdef DETAILED_TIME
  absentCleaningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
}
