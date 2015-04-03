// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "Attribute.h"

const bool lessNoisy(const Value* value, const Value* otherValue)
{
  return *value < *otherValue;
}

unsigned int Attribute::noisePerUnit;

#if defined VERBOSE_DIM_CHOICE || defined DEBUG || defined ASSERT
vector<unsigned int> Attribute::internal2ExternalAttributeOrder;
#endif

#ifdef DETAILED_TIME
double Attribute::propagationCheckingDuration = 0;
double Attribute::closednessCheckingDuration = 0;
double Attribute::absentCleaningDuration = 0;
#endif

unsigned int Attribute::maxId = 0;
vector<unsigned int> Attribute::epsilonVector;
vector<vector<string>> Attribute::labelsVector;
bool Attribute::isDensestValuePreferred;

string Attribute::outputElementSeparator;
string Attribute::emptySetString;
string Attribute::elementNoiseSeparator;
bool Attribute::isNoisePrinted;

Attribute::Attribute(): id(0), potential(), absent(), present()
{
}

Attribute::Attribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon): id(maxId++), potential(), absent(), present()
{
  const vector<unsigned int>::const_iterator nbOfValuesInThisAttributeIt = nbOfValuesPerAttribute.begin() + id;
  unsigned int sizeOfAValue = noisePerUnit;
  for (vector<unsigned int>::const_iterator nbOfValuesInAttributeIt = nbOfValuesPerAttribute.begin(); nbOfValuesInAttributeIt != nbOfValuesPerAttribute.end(); ++nbOfValuesInAttributeIt)
    {
      if (nbOfValuesInAttributeIt != nbOfValuesInThisAttributeIt)
	{
	  sizeOfAValue *= *nbOfValuesInAttributeIt;
	}
    }
  vector<unsigned int> noisesInIntersections(nbOfValuesPerAttribute.size() - id - 1, noisePerUnit);
  vector<unsigned int>::const_iterator attributeIt = nbOfValuesInThisAttributeIt + 1;
  for (unsigned int& noiseInIntersection : noisesInIntersections)
    {
      for (vector<unsigned int>::const_iterator nbOfValuesInAttributeIt = nbOfValuesPerAttribute.begin(); nbOfValuesInAttributeIt != nbOfValuesPerAttribute.end(); ++nbOfValuesInAttributeIt)
	{
	  if (nbOfValuesInAttributeIt != attributeIt && nbOfValuesInAttributeIt != nbOfValuesInThisAttributeIt)
	    {
	      noiseInIntersection *= *nbOfValuesInAttributeIt;
	    }
	}
      ++attributeIt;
    }
  const vector<unsigned int>::const_iterator nbOfValuesInNextAttributeIt = nbOfValuesInThisAttributeIt + 1;
  potential.reserve(*nbOfValuesInThisAttributeIt);
  for (unsigned int valueId = 0; valueId != *nbOfValuesInThisAttributeIt; ++valueId)
    {
      potential.push_back(new Value(valueId, sizeOfAValue, nbOfValuesInNextAttributeIt, nbOfValuesPerAttribute.end(), noisesInIntersections));
    }
  absent.reserve(*nbOfValuesInThisAttributeIt);
  present.reserve(*nbOfValuesInThisAttributeIt);
  epsilonVector.push_back(epsilon * noisePerUnit);
}

Attribute::Attribute(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd): id((*parentAttributeIt)->id), potential(), absent(), present()
{
  const vector<Attribute*>::const_iterator parentNextAttributeIt = parentAttributeIt + 1;
  const Attribute& parentAttribute = **parentAttributeIt;
  unsigned int newId = 0;
  // The order (potential, absent, present) seems to provide the best performances
  potential.reserve(parentAttribute.potential.size());
  for (const Value* potentialValue : parentAttribute.potential)
    {
      potential.push_back(createChildValue(*potentialValue, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
    }
  absent.reserve(parentAttribute.absent.size() + parentAttribute.potential.size());
  for (const Value* absentValue : parentAttribute.absent)
    {
      absent.push_back(createChildValue(*absentValue, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
    }
  present.reserve(parentAttribute.present.size() + parentAttribute.potential.size());
  for (const Value* presentValue : parentAttribute.present)
    {
      present.push_back(createChildValue(*presentValue, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
    }
}

Attribute::~Attribute()
{
  for (const Value* potentialValue : potential)
    {
      delete potentialValue;
    }
  for (const Value* absentValue : absent)
    {
      delete absentValue;
    }
  for (const Value* presentValue : present)
    {
      delete presentValue;
    }
}

Attribute* Attribute::clone(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd) const
{
  return new Attribute(parentAttributeIt, parentAttributeEnd, sizeOfAttributeIt, sizeOfAttributeEnd);
}

ostream& operator<<(ostream& out, const Attribute& attribute)
{
  attribute.printValues(attribute.present, out);
  return out;
}

Value* Attribute::createChildValue(const Value& parentValue, const unsigned int newId, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd, const vector<Attribute*>::const_iterator parentNextAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd) const
{
  Value* childValue = new Value(parentValue, newId, sizeOfAttributeIt, sizeOfAttributeEnd);
  vector<vector<unsigned int>>::iterator noiseInIntersectionWithPresentValuesIt = childValue->getIntersectionsBeginWithPresentValues();
  vector<vector<unsigned int>>::iterator noiseInIntersectionWithPresentAndPotentialValuesIt = childValue->getIntersectionsBeginWithPresentAndPotentialValues();
  vector<vector<unsigned int>>::const_iterator parentNoiseInIntersectionWithPresentValuesIt = parentValue.getIntersectionsBeginWithPresentValues();
  vector<vector<unsigned int>>::const_iterator parentNoiseInIntersectionWithPresentAndPotentialValuesIt = parentValue.getIntersectionsBeginWithPresentAndPotentialValues();
  for (vector<Attribute*>::const_iterator attributeIt = parentNextAttributeIt; attributeIt != parentAttributeEnd; ++attributeIt)
    {
      (*attributeIt)->setChildValueIntersections(*parentNoiseInIntersectionWithPresentValuesIt++, *parentNoiseInIntersectionWithPresentAndPotentialValuesIt++, *noiseInIntersectionWithPresentValuesIt++, *noiseInIntersectionWithPresentAndPotentialValuesIt++);
    }
  return childValue;
}

void Attribute::setChildValueIntersections(const vector<unsigned int>& noiseInIntersectionWithPresentValues, const vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& childNoiseInIntersectionWithPresentValues, vector<unsigned int>& childNoiseInIntersectionWithPresentAndPotentialValues) const
{
  // The order (potential, absent, present) seems to provide the best performances
  for (const Value* potentialValue : potential)
    {
      const unsigned int valueId = potentialValue->getId();
      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
    }
  for (const Value* absentValue : absent)
    {
      const unsigned int valueId = absentValue->getId();
      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
    }
  for (const Value* presentValue : present)
    {
      const unsigned int valueId = presentValue->getId();
      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
    }
}

const unsigned int Attribute::getId() const
{
  return id;
}

vector<unsigned int> Attribute::getPresentOriginalIds() const
{
  vector<unsigned int> originalIds;
  originalIds.reserve(present.size());
  for (const Value* presentValue : present)
    {
      originalIds.push_back(presentValue->getOriginalId());
    }
  return originalIds;
}

vector<Element> Attribute::getElements() const
{
  vector<Element> patternDimension;
  patternDimension.reserve(present.size());
  for (const Value* value : present)
    {
      patternDimension.push_back(Element(value->getOriginalId(), value->getPresentAndPotentialNoise()));
    }
  sort(patternDimension.begin(), patternDimension.end(), Element::smallerId);
  return patternDimension;
}

vector<Value*>::iterator Attribute::presentBegin()
{
  return present.begin();
}

vector<Value*>::iterator Attribute::presentEnd()
{
  return present.end();
}

vector<Value*>::iterator Attribute::potentialBegin()
{
  return potential.begin();
}

vector<Value*>::iterator Attribute::potentialEnd()
{
  return potential.end();
}

vector<Value*>::iterator Attribute::absentBegin()
{
  return absent.begin();
}

vector<Value*>::iterator Attribute::absentEnd()
{
  return absent.end();
}

const bool Attribute::presentEmpty() const
{
  return present.empty();
}

const unsigned int Attribute::sizeOfPresent() const
{
  return present.size();
}

const bool Attribute::potentialEmpty() const
{
  return potential.empty();
}

const unsigned int Attribute::sizeOfPotential() const
{
  return potential.size();
}

const unsigned int Attribute::globalSize() const
{
  return present.size() + potential.size() + absent.size();
}

unordered_map<unsigned int, unsigned int> Attribute::setLabels(unordered_map<string, unsigned int>& labels2Ids) const
{
  unordered_map<unsigned int, unsigned int> oldId2NewIds;
  vector<string> labels;
  labels.reserve(potential.size());
  for (pair<const string, unsigned int>& label2Id : labels2Ids)
    {
      if (label2Id.second != numeric_limits<unsigned int>::max())
	{
	  oldId2NewIds[label2Id.second] = labels.size();
	  label2Id.second = labels.size();
	  labels.push_back(label2Id.first);
	}
    }
  labelsVector.push_back(labels);
  return oldId2NewIds;
}

vector<Value*>::iterator Attribute::valueItToEnumerate()
{
  if (isDensestValuePreferred)
    {
      return min_element(potential.begin(), potential.end(), lessNoisy);
    }
  return max_element(potential.begin(), potential.end(), lessNoisy);
}

Value* Attribute::moveValueFromPotentialToPresent(const unsigned int valueOriginalId)
{
  vector<Value*>::iterator valueIt = potential.begin();
  for (; (*valueIt)->getOriginalId() != valueOriginalId; ++valueIt)
    {
    }
  present.push_back(*valueIt);
  *valueIt = potential.back();
  potential.pop_back();
  return present.back();
}

Value* Attribute::moveValueFromPotentialToAbsent(const vector<Value*>::iterator valueIt)
{
  absent.push_back(*valueIt);
  *valueIt = potential.back();
  potential.pop_back();
  return absent.back();
}

// It should be called after initialization only (all values ordered in potential)
vector<vector<unsigned int>>::iterator Attribute::getIntersectionsBeginWithPotentialValues(const unsigned int valueId)
{
  return potential[valueId]->getIntersectionsBeginWithPresentAndPotentialValues();
}

// It should be called after initialization only (all values ordered in potential)
void Attribute::substractPotentialNoise(const unsigned int valueId, const unsigned int noise)
{
  potential[valueId]->substractPotentialNoise(noise);
}

void Attribute::printValues(const vector<Value*>& v, ostream& out) const
{
  bool isFirstElement = true;
  vector<string>& labels = labelsVector[id];
  for (const Value* value : v)
    {
      if (isFirstElement)
	{
	  isFirstElement = false;
	}
      else
	{
	  out << outputElementSeparator;
	}
      out << labels[value->getOriginalId()];
      if (isNoisePrinted)
	{
	  out << elementNoiseSeparator << static_cast<float>(value->getPresentAndPotentialNoise()) / static_cast<float>(noisePerUnit);
	}
    }
  if (isFirstElement)
    {
      out << emptySetString;
    }
}

void Attribute::printValueFromOriginalId(const unsigned int valueOriginalId, ostream& out) const
{
  out << labelsVector[id][valueOriginalId];
}

#ifdef DEBUG
void Attribute::printPresent(ostream& out) const
{
  printValues(present, out);
}

void Attribute::printPotential(ostream& out) const
{
  printValues(potential, out);
}

void Attribute::printAbsent(ostream& out) const
{
  printValues(absent, out);
}

void Attribute::printPotentialValuesFromIds(const vector<unsigned int>& potentialValueIds, ostream& out) const
{
  out << "in attribute " << internal2ExternalAttributeOrder[id] << ": ";
  vector<string>& labels = labelsVector[id];
  bool isFirst = true;
  for (const unsigned int potentialValueId : potentialValueIds)
    {
      if (isFirst)
	{
	  isFirst = false;
	}
      else
	{
	  out << outputElementSeparator;
	}
      out << labels[potential[potentialValueId]->getOriginalId()];
    }
  if (isFirst)
    {
      out << emptySetString;
    }
}
#endif

#if defined DEBUG || defined ASSERT
void Attribute::printValue(const Value& value, ostream& out) const
{
  out << labelsVector[id][value.getOriginalId()] << " in attribute " << internal2ExternalAttributeOrder[id];
}

void Attribute::setInternal2ExternalAttributeOrder(const vector<unsigned int>& internal2ExternalAttributeOrderParam)
{
  internal2ExternalAttributeOrder = internal2ExternalAttributeOrderParam;
}
#endif

const double Attribute::getAppeal(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
#if ENUMERATION_PROCESS == 0
  unsigned int appeal = 0;
  for (vector<Attribute*>::const_iterator potentialAttributeIt = attributeBegin; potentialAttributeIt != attributeEnd; ++potentialAttributeIt)
    {
      const unsigned int potentialAttributeId = (*potentialAttributeIt)->id;
      if (potentialAttributeId != id)
      	{
      	  unsigned int yesFactor = (*potentialAttributeIt)->potential.size();
      	  for (vector<Attribute*>::const_iterator presentAttributeIt = attributeBegin; presentAttributeIt != attributeEnd; ++presentAttributeIt)
      	    {
	      const unsigned int presentAttributeId = (*presentAttributeIt)->id;
      	      if (presentAttributeId != id && presentAttributeId != potentialAttributeId)
      		{
      		  yesFactor *= (*presentAttributeIt)->present.size();
      		}
      	    }
      	  appeal += yesFactor;
      	}
    }

  // for (vector<Attribute*>::const_iterator potentialAttributeIt = attributeBegin; potentialAttributeIt != attributeEnd; ++potentialAttributeIt)
  //   {
  //     unsigned int yesFactor = (*potentialAttributeIt)->potential.size();
  //     const unsigned int potentialAttributeId = (*potentialAttributeIt)->id;
  //     if (potentialAttributeId == id)
  // 	{
  // 	  --yesFactor;
  // 	}
  //     for (vector<Attribute*>::const_iterator presentAttributeIt = attributeBegin; presentAttributeIt != attributeEnd; ++presentAttributeIt)
  // 	{
  // 	  const unsigned int presentAttributeId = (*presentAttributeIt)->id;
  // 	  if (presentAttributeId != id && presentAttributeId != potentialAttributeId)
  // 	    {
  // 	      yesFactor *= (*presentAttributeIt)->present.size();
  // 	    }
  // 	}
  //     appeal += yesFactor;
  //   }

  // for (vector<Attribute*>::const_iterator potentialAttributeIt = attributeBegin; potentialAttributeIt != attributeEnd; ++potentialAttributeIt)
  //   {
  //     unsigned int yesFactor = 0;
  //     const unsigned int potentialAttributeId = (*potentialAttributeIt)->id;
  //     if (potentialAttributeId == id)
  // 	{
  // 	  yesFactor = ((*potentialAttributeIt)->potential.size() - 1) * ((*attributeIt)->present.size() + 2);
  // 	  unsigned int orthogonalTerm = 0;
  // 	  for (vector<Attribute*>::const_iterator orthogonalAttributeIt = attributeBegin; orthogonalAttributeIt != attributeEnd; ++orthogonalAttributeIt)
  // 	    {
  // 	      const unsigned int orthogonalAttributeId = (*orthogonalAttributeIt)->id;
  // 	      if (orthogonalAttributeId != potentialAttributeId)
  // 		{
  // 		  unsigned int presentFactor = 1;
  // 		  for (vector<Attribute*>::const_iterator presentAttributeIt = attributeBegin; presentAttributeIt != attributeEnd; ++presentAttributeIt)
  // 		    {
  // 		      const unsigned int presentAttributeId = (*presentAttributeIt)->id;
  // 		      if (presentAttributeId != potentialAttributeId && presentAttributeId != orthogonalAttributeId)
  // 			{
  // 			  presentFactor *= (*presentAttributeIt)->present.size();
  // 			}
  // 		    }
  // 		  orthogonalTerm += presentFactor;
  // 		}
  // 	    }
  // 	}
  //     else
  // 	{
  // 	  yesFactor = (*potentialAttributeIt)->potential.size() * ((*attributeIt)->present.size() + 1);
  // 	  unsigned int orthogonalTerm = 0;
  // 	  for (vector<Attribute*>::const_iterator orthogonalAttributeIt = attributeBegin; orthogonalAttributeIt != attributeEnd; ++orthogonalAttributeIt)
  // 	    {
  // 	      const unsigned int orthogonalAttributeId = (*orthogonalAttributeIt)->id;
  // 	      if (orthogonalAttributeId != id && orthogonalAttributeId != potentialAttributeId)
  // 		{
  // 		  unsigned int presentFactor = 1;
  // 		  for (vector<Attribute*>::const_iterator presentAttributeIt = attributeBegin; presentAttributeIt != attributeEnd; ++presentAttributeIt)
  // 		    {
  // 		      const unsigned int presentAttributeId = (*presentAttributeIt)->id;
  // 		      if (presentAttributeId != id && presentAttributeId != potentialAttributeId && presentAttributeId != orthogonalAttributeId)
  // 			{
  // 			  presentFactor *= (*presentAttributeIt)->present.size();
  // 			}
  // 		    }
  // 		  orthogonalTerm += presentFactor * ((*potentialAttributeIt)->present.size() + 1);
  // 		}
  // 	    }
  // 	  unsigned int presentFactor = 1;
  // 	  for (vector<Attribute*>::const_iterator presentAttributeIt = attributeBegin; presentAttributeIt != attributeEnd; ++presentAttributeIt)
  // 	    {
  // 	      const unsigned int presentAttributeId = (*presentAttributeIt)->id;
  // 	      if (presentAttributeId != id && presentAttributeId != potentialAttributeId)
  // 		{
  // 		  presentFactor *= (*presentAttributeIt)->present.size();
  // 		}
  // 	    }
  // 	  yesFactor *= (orthogonalTerm + presentFactor);
  // 	}
  //     appeal += yesFactor;
  //   }

  if (appeal != 0 || present.empty())
    {
      return appeal;
    }
  return -1;
#else
  unsigned int numerator = 0;
  unsigned int presentAndPotentialArea = 1;
  unsigned int presentArea = 1;
  for (vector<Attribute*>::const_iterator potentialAttributeIt = attributeBegin; potentialAttributeIt != attributeEnd; ++potentialAttributeIt)
    {
      const unsigned int potentialAttributeId = (*potentialAttributeIt)->id;
      if (potentialAttributeId != id)
	{
	  const unsigned int sizeOfPresent = (*potentialAttributeIt)->present.size();
	  presentArea *= sizeOfPresent;
	  unsigned int sizeOfPotential = (*potentialAttributeIt)->potential.size();
	  presentAndPotentialArea *= (sizeOfPresent + sizeOfPotential);
	  for (vector<Attribute*>::const_iterator presentAttributeIt = attributeBegin; presentAttributeIt != attributeEnd; ++presentAttributeIt)
	    {
	      const unsigned int presentAttributeId = (*presentAttributeIt)->id;
	      if (presentAttributeId != id && presentAttributeId != potentialAttributeId)
		{
		  sizeOfPotential *= (*presentAttributeIt)->present.size();
		}
	    }
	  numerator += sizeOfPotential;
	}
    }
  if (numerator != 0)
    {
      const unsigned int potentialArea = presentAndPotentialArea - presentArea;
      if (potentialArea == 0)
	{
	  // This is the only attribute with a non-empty potential set
	  return 0;
	}
      return static_cast<double>(numerator) / static_cast<double>(potentialArea);
    }
  // At least one other attribute has an empty present set
  if (present.empty())
    {
      return 0;
    }
  return -1;
#endif
}

pair<bool, vector<unsigned int>> Attribute::findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd, IrrelevantValueIds& irrelevantValueIds)
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  vector<unsigned int> newIrrelevantValueOriginalIds;
  newIrrelevantValueOriginalIds.reserve(potential.size());
  list<unsigned int>::iterator irrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.begin();
  // These potential values are irrelevant if at least one previous present values is not extensible with it
  for (const Value* potentialValue : potential)
    {
      if (irrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.end() && *irrelevantValueIdIt == potentialValue->getId())
	{
	  ++irrelevantValueIdIt;
	}
      else
	{
	  if (valueDoesNotExtendPresent(*potentialValue, attributeBegin, attributeEnd))
	    {
#ifdef DEBUG
	      cout << labelsVector[id][potentialValue->getOriginalId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never be present nor prevent the closedness of any future pattern" << endl;
#endif
	      irrelevantValueIds.irrelevantValueIds.insert(irrelevantValueIdIt, potentialValue->getId());
	      newIrrelevantValueOriginalIds.push_back(potentialValue->getOriginalId());
	    }
	}
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return pair<bool, vector<unsigned int>>(false, newIrrelevantValueOriginalIds);
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
const bool Attribute::presentAndPotentialIrrelevant(const unsigned int presentAndPotentialIrrelevancyThreshold) const
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  vector<Value*>::const_iterator presentValueIt = present.begin();
  for (; presentValueIt != present.end() && (*presentValueIt)->getPresentAndPotentialNoise() <= presentAndPotentialIrrelevancyThreshold; ++presentValueIt)
    {
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef DEBUG
  if (presentValueIt != present.end())
    {
      cout << labelsVector[id][(*presentValueIt)->getOriginalId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " contains too much potential noise given the size constraints -> Prune!" << endl;
    }
#endif
  return presentValueIt != present.end();
}

// CLEAN: Like findIrrelevantValuesAndCheckConstraints but presentAndPotentialIrrelevantValue replaces irrelevantValue (factorize?)
pair<bool, vector<unsigned int>> Attribute::findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity(const unsigned int presentAndPotentialIrrelevancyThreshold, IrrelevantValueIds& irrelevantValueIds)
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  vector<unsigned int> newIrrelevantValueOriginalIds;
  newIrrelevantValueOriginalIds.reserve(potential.size());
  list<unsigned int>::iterator irrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.begin();
  // These potential values are irrelevant if they contain too much noise in any extension satisfying the minimal size constraints
  for (const Value* potentialValue : potential)
    {
      if (irrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.end() && *irrelevantValueIdIt == potentialValue->getId())
	{
	  ++irrelevantValueIdIt;
	}
      else
	{
	  if (presentAndPotentialIrrelevantValue(*potentialValue, presentAndPotentialIrrelevancyThreshold))
	    {
	      irrelevantValueIds.irrelevantValueIds.insert(irrelevantValueIdIt, potentialValue->getId());
	      newIrrelevantValueOriginalIds.push_back(potentialValue->getOriginalId());
	    }
	}
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return pair<bool, vector<unsigned int>>(false, newIrrelevantValueOriginalIds);
}

const bool Attribute::presentAndPotentialIrrelevantValue(const Value& value, const unsigned int presentAndPotentialIrrelevancyThreshold) const
{
#ifdef DEBUG
  if (value.getPresentAndPotentialNoise() > presentAndPotentialIrrelevancyThreshold)
    {
      cout << "Given the minimal size constraints, " << labelsVector[id][value.getOriginalId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never be present nor prevent the closedness of any future pattern" << endl;
    }
#endif
  return value.getPresentAndPotentialNoise() > presentAndPotentialIrrelevancyThreshold;
}

void Attribute::presentAndPotentialCleanAbsent(const unsigned int presentAndPotentialIrrelevancyThreshold)
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  for (vector<Value*>::iterator absentValueIt = absent.begin(); absentValueIt != absent.end(); )
    {
      if ((*absentValueIt)->getPresentAndPotentialNoise() > presentAndPotentialIrrelevancyThreshold)
	{
	  absentValueToCleanFound(absentValueIt);
	}
      else
	{
	  ++absentValueIt;
	}
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
}
#endif

const bool Attribute::valueDoesNotExtendPresent(const Value& value, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<unsigned int>::const_iterator thisEpsilonIt = epsilonVector.begin() + id;
  if (value.getPresentNoise() > *thisEpsilonIt)
    {
      return true;
    }
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin() + (*attributeIt)->id;
  for (unsigned int intersectionIndex = id; epsilonIt != thisEpsilonIt && value.extendsPastPresent((*attributeIt)->present.begin(), (*attributeIt)->present.end(), *epsilonIt, --intersectionIndex); ++epsilonIt)
    {
      ++attributeIt;
    }
  if (epsilonIt != thisEpsilonIt)
    {
      return true;
    }
  for (unsigned int reverseAttributeIndex = maxId - id; ++attributeIt != attributeEnd && value.extendsFuturePresent((*attributeIt)->present.begin(), (*attributeIt)->present.end(), *(++epsilonIt), --reverseAttributeIndex); )
    {
    }
  return attributeIt != attributeEnd;
}

vector<unsigned int> Attribute::erasePotentialValues(const list<unsigned int>& potentialValueIds)
{
  vector<unsigned int> erasedValueOriginalIds;
  erasedValueOriginalIds.reserve(potentialValueIds.size());
  list<unsigned int>::const_iterator potentialValueIdIt = --(potentialValueIds.end());
  for (vector<Value*>::iterator potentialIt = --(potential.end()); ; --potentialIt)
    {
      if ((*potentialIt)->getId() == *potentialValueIdIt)
	{
	  erasedValueOriginalIds.push_back((*potentialIt)->getOriginalId());
	  delete *potentialIt;
	  potentialIt = potential.erase(potentialIt);
	  if (potentialValueIdIt == potentialValueIds.begin())
	    {
	      return erasedValueOriginalIds;
	    }
	  --potentialValueIdIt;
	}
    }
  return erasedValueOriginalIds;
}

const bool Attribute::closed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  vector<Value*>::const_iterator valueIt = absent.begin();
  for (; valueIt != absent.end() && valueDoesNotExtendPresentAndPotential(**valueIt, attributeBegin, attributeEnd); ++valueIt)
    {
    }
#ifdef DETAILED_TIME
  closednessCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef DEBUG
  if (valueIt != absent.end())
    {
      cout << labelsVector[id][(*valueIt)->getOriginalId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " extends any future pattern -> Prune!" << endl;
    }
#endif
  return valueIt == absent.end();
}

const bool Attribute::valueDoesNotExtendPresentAndPotential(const Value& value, const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
  const vector<unsigned int>::const_iterator thisEpsilonIt = epsilonVector.begin() + id;
  if (value.getPresentAndPotentialNoise() > *thisEpsilonIt)
    {
      return true;
    }
  vector<Attribute*>::const_iterator attributeIt = attributeBegin;
  vector<unsigned int>::const_iterator epsilonIt = epsilonVector.begin() + (*attributeIt)->id;
  for (unsigned int intersectionIndex = id; epsilonIt != thisEpsilonIt && value.extendsPastPresentAndPotential((*attributeIt)->present.begin(), (*attributeIt)->present.end(), *epsilonIt, --intersectionIndex) && value.extendsPastPresentAndPotential((*attributeIt)->potential.begin(), (*attributeIt)->potential.end(), *epsilonIt, intersectionIndex); ++epsilonIt)
    {
      ++attributeIt;
    }
  if (epsilonIt != thisEpsilonIt)
    {
      return true;
    }
  for (unsigned int reverseAttributeIndex = maxId - id; ++attributeIt != attributeEnd && value.extendsFuturePresentAndPotential((*attributeIt)->present.begin(), (*attributeIt)->present.end(), *(++epsilonIt), --reverseAttributeIndex) && value.extendsFuturePresentAndPotential((*attributeIt)->potential.begin(), (*attributeIt)->potential.end(), *epsilonIt, reverseAttributeIndex); )
    {
    }
  return attributeIt != attributeEnd;
}

void Attribute::cleanAbsent(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd)
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  for (vector<Value*>::iterator absentIt = absent.begin(); absentIt != absent.end(); )
    {
      if (valueDoesNotExtendPresent(**absentIt, attributeBegin, attributeEnd))
	{
	  absentValueToCleanFound(absentIt);
	}
      else
	{
	  ++absentIt;
	}
    }
#ifdef DETAILED_TIME
  absentCleaningDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
}

void Attribute::absentValueToCleanFound(vector<Value*>::iterator& absentValueIt)
{
#ifdef DEBUG
  cout << labelsVector[id][(*absentValueIt)->getOriginalId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never extend any future pattern" << endl;
#endif
  delete *absentValueIt;
  *absentValueIt = absent.back();
  absent.pop_back();
}

pair<const bool, vector<unsigned int>> Attribute::tauFarValueOriginalValueIdsAndCheckConstraints(const Value* absentValue)
{
  return pair<const bool, vector<unsigned int>>(false, vector<unsigned int>());
}

const bool Attribute::finalizable() const
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  const unsigned int epsilon = epsilonVector[id];
  vector<Value*>::const_iterator valueIt = potential.begin();
  for (; valueIt != potential.end() && (*valueIt)->getPresentAndPotentialNoise() <= epsilon; ++valueIt)
    {
    }
  if (valueIt == potential.end())
    {
      for (valueIt = present.begin(); valueIt != present.end() && (*valueIt)->getPresentAndPotentialNoise() <= epsilon; ++valueIt)
	{
	}
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return valueIt == present.end();
}

vector<unsigned int> Attribute::finalize()
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  vector<unsigned int> originalIdsOfValuesSetPresent;
  originalIdsOfValuesSetPresent.reserve(potential.size());
  for (Value* potentialValue : potential)
    {
      present.push_back(potentialValue);
      originalIdsOfValuesSetPresent.push_back(potentialValue->getOriginalId());
    }
  potential.clear();
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return originalIdsOfValuesSetPresent;
}

const unsigned int Attribute::lastAttributeId()
{
  return maxId - 1;
}

const vector<unsigned int>& Attribute::getEpsilonVector()
{
  return epsilonVector;
}

void Attribute::setDensityPrecedenceAndOutputFormat(const bool isDensestValuePreferredParam, const char* outputElementSeparatorParam, const char* emptySetStringParam, const char* elementNoiseSeparatorParam, const bool isNoisePrintedParam)
{
  isDensestValuePreferred = isDensestValuePreferredParam;
  outputElementSeparator = outputElementSeparatorParam;
  emptySetString = emptySetStringParam;
  elementNoiseSeparator = elementNoiseSeparatorParam;
  isNoisePrinted = isNoisePrintedParam;
}

const bool Attribute::noisePrinted()
{
  return isNoisePrinted;
}

void Attribute::printOutputElementSeparator(ostream& out)
{
  out << outputElementSeparator;
}

void Attribute::printNoise(const float noise, ostream& out)
{
  out << elementNoiseSeparator << noise;
}

#ifdef DETAILED_TIME
void Attribute::printDurations(ostream& out)
{
#ifdef GNUPLOT
  out << closednessCheckingDuration << '\t' << propagationCheckingDuration << '\t' << absentCleaningDuration;
#else
  out << "Time spent checking closedness: " << closednessCheckingDuration << 's' << endl << "Time spent searching for irrelevant values: " << propagationCheckingDuration << 's' << endl << "Time spent cleaning the set of absent values: " << absentCleaningDuration << 's';
#endif
}
#endif

#ifdef ASSERT
vector<Value*>::const_iterator Attribute::presentBegin() const
{
  return present.begin();
}

vector<Value*>::const_iterator Attribute::presentEnd() const
{
  return present.end();
}

vector<Value*>::const_iterator Attribute::potentialBegin() const
{
  return potential.begin();
}

vector<Value*>::const_iterator Attribute::potentialEnd() const
{
  return potential.end();
}

vector<Value*>::const_iterator Attribute::absentBegin() const
{
  return absent.begin();
}

vector<Value*>::const_iterator Attribute::absentEnd() const
{
  return absent.end();
}
#endif
