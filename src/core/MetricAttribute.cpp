// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "MetricAttribute.h"

vector<double> MetricAttribute::tauVector;
vector<vector<double>> MetricAttribute::timestampsVector;

MetricAttribute::MetricAttribute(const vector<unsigned int>& nbOfValuesPerAttribute, const double epsilon, const double tau): Attribute(nbOfValuesPerAttribute, epsilon)
{
  tauVector.resize(id + 1);
  tauVector.back() = tau;
}

MetricAttribute::MetricAttribute(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd): Attribute()
{
  const vector<Attribute*>::const_iterator parentNextAttributeIt = parentAttributeIt + 1;
  const MetricAttribute& parentMetricAttribute = static_cast<MetricAttribute&>(**parentAttributeIt);
  id = parentMetricAttribute.id;
  const unsigned int presentAndPotentialSize = parentMetricAttribute.present.size() + parentMetricAttribute.potential.size();
  unsigned int newId = 0;
  potential.reserve(parentMetricAttribute.potential.size());
  present.reserve(presentAndPotentialSize);
  absent.reserve(parentMetricAttribute.absent.size() + parentMetricAttribute.potential.size());
  vector<Value*>::const_iterator potentialValueIt = parentMetricAttribute.potential.begin();
  vector<Value*>::const_iterator presentValueIt = parentMetricAttribute.present.begin();
  vector<Value*>::const_iterator absentValueIt = parentMetricAttribute.absent.begin();
  while (potentialValueIt != parentMetricAttribute.potential.end() || presentValueIt != parentMetricAttribute.present.end() || absentValueIt != parentMetricAttribute.absent.end())
    {
      if (potentialValueIt == parentMetricAttribute.potential.end())
	{
	  if (presentValueIt == parentMetricAttribute.present.end())
	    {
	      absent.push_back(createChildValue(**absentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
	    }
	  else
	    {
	      if (absentValueIt == parentMetricAttribute.absent.end())
		{
		  present.push_back(createChildValue(**presentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
		}
	      else
		{
		  if ((*presentValueIt)->getId() < (*absentValueIt)->getId())
		    {
		      present.push_back(createChildValue(**presentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
		    }
		  else
		    {
		      absent.push_back(createChildValue(**absentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
		    }
		}
	    }
	}
      else
	{
	  if (presentValueIt == parentMetricAttribute.present.end())
	    {
	      if (absentValueIt == parentMetricAttribute.absent.end())
		{
		  potential.push_back(createChildValue(**potentialValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
		}
	      else
		{
		  if ((*potentialValueIt)->getId() < (*absentValueIt)->getId())
		    {
		      potential.push_back(createChildValue(**potentialValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
		    }
		  else
		    {
		      absent.push_back(createChildValue(**absentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
		    }
		}
	    }
	  else
	    {
	      if (absentValueIt == parentMetricAttribute.absent.end())
		{
		  if ((*potentialValueIt)->getId() < (*presentValueIt)->getId())
		    {
		      potential.push_back(createChildValue(**potentialValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
		    }
		  else
		    {
		      present.push_back(createChildValue(**presentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
		    }
		}
	      else
		{
		  if ((*potentialValueIt)->getId() < (*presentValueIt)->getId())
		    {
		      if ((*potentialValueIt)->getId() < (*absentValueIt)->getId())
			{
			  potential.push_back(createChildValue(**potentialValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
			}
		      else
			{
			  absent.push_back(createChildValue(**absentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
			}
		    }
		  else
		    {
		      if ((*presentValueIt)->getId() < (*absentValueIt)->getId())
			{
			  present.push_back(createChildValue(**presentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
			}
		      else
			{
			  absent.push_back(createChildValue(**absentValueIt++, newId++, sizeOfAttributeIt, sizeOfAttributeEnd, parentNextAttributeIt, parentAttributeEnd));
			}
		    }
		}
	    }
	}
    }
}

MetricAttribute* MetricAttribute::clone(const vector<Attribute*>::const_iterator parentAttributeIt, const vector<Attribute*>::const_iterator parentAttributeEnd, const vector<unsigned int>::const_iterator sizeOfAttributeIt, const vector<unsigned int>::const_iterator sizeOfAttributeEnd) const
{
  return new MetricAttribute(parentAttributeIt, parentAttributeEnd, sizeOfAttributeIt, sizeOfAttributeEnd);
}

unordered_map<unsigned int, unsigned int> MetricAttribute::setLabels(unordered_map<string, unsigned int>& labels2Ids) const
{
  timestampsVector.resize(id + 1);
  vector<double>& timestampsOfThisAttribute = timestampsVector.back();
  timestampsOfThisAttribute.reserve(potential.size());
  for (pair<const string, unsigned int>& label2Id : labels2Ids)
    {
      if (label2Id.second != numeric_limits<unsigned int>::max())
	{
	  timestampsOfThisAttribute.push_back(lexical_cast<double>(label2Id.first));
	}
    }
  sort(timestampsOfThisAttribute.begin(), timestampsOfThisAttribute.end());
  unordered_map<unsigned int, unsigned int> oldId2NewIds;
  vector<string> labels;
  labels.reserve(potential.size());
  for (double timestamp : timestampsOfThisAttribute)
    {
      const string label = lexical_cast<string>(timestamp);
      pair<const string, unsigned int>& label2Id = *labels2Ids.find(label);
      oldId2NewIds[label2Id.second] = labels.size();
      label2Id.second = labels.size();
      labels.push_back(label);
    }
  labelsVector.push_back(labels);
  return oldId2NewIds;
}

void MetricAttribute::setChildValueIntersections(const vector<unsigned int>& noiseInIntersectionWithPresentValues, const vector<unsigned int>& noiseInIntersectionWithPresentAndPotentialValues, vector<unsigned int>& childNoiseInIntersectionWithPresentValues, vector<unsigned int>& childNoiseInIntersectionWithPresentAndPotentialValues) const
{
  vector<Value*>::const_iterator presentValueIt = present.begin();
  vector<Value*>::const_iterator absentValueIt = absent.begin();
  vector<Value*>::const_iterator potentialValueIt = potential.begin();
  while (potentialValueIt != potential.end() || presentValueIt != present.end() || absentValueIt != absent.end())
    {
      if (potentialValueIt == potential.end())
	{
	  if (presentValueIt == present.end())
	    {
	      const unsigned int valueId = (*absentValueIt++)->getId();
	      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
	      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
	    }
	  else
	    {
	      if (absentValueIt == absent.end())
		{
		  const unsigned int valueId = (*presentValueIt++)->getId();
		  childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
		  childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
		}
	      else
		{
		  if ((*presentValueIt)->getId() < (*absentValueIt)->getId())
		    {
		      const unsigned int valueId = (*presentValueIt++)->getId();
		      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
		      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
		    }
		  else
		    {
		      const unsigned int valueId = (*absentValueIt++)->getId();
		      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
		      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
		    }
		}
	    }
	}
      else
	{
	  if (presentValueIt == present.end())
	    {
	      if (absentValueIt == absent.end())
		{
		  const unsigned int valueId = (*potentialValueIt++)->getId();
		  childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
		  childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
		}
	      else
		{
		  if ((*potentialValueIt)->getId() < (*absentValueIt)->getId())
		    {
		      const unsigned int valueId = (*potentialValueIt++)->getId();
		      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
		      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
		    }
		  else
		    {
		      const unsigned int valueId = (*absentValueIt++)->getId();
		      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
		      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
		    }
		}
	    }
	  else
	    {
	      if (absentValueIt == absent.end())
		{
		  if ((*potentialValueIt)->getId() < (*presentValueIt)->getId())
		    {
		      const unsigned int valueId = (*potentialValueIt++)->getId();
		      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
		      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
		    }
		  else
		    {
		      const unsigned int valueId = (*presentValueIt++)->getId();
		      childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
		      childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
		    }
		}
	      else
		{
		  if ((*potentialValueIt)->getId() < (*presentValueIt)->getId())
		    {
		      if ((*potentialValueIt)->getId() < (*absentValueIt)->getId())
			{
			  const unsigned int valueId = (*potentialValueIt++)->getId();
			  childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
			  childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
			}
		      else
			{
			  const unsigned int valueId = (*absentValueIt++)->getId();
			  childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
			  childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
			}
		    }
		  else
		    {
		      if ((*presentValueIt)->getId() < (*absentValueIt)->getId())
			{
			  const unsigned int valueId = (*presentValueIt++)->getId();
			  childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
			  childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
			}
		      else
			{
			  const unsigned int valueId = (*absentValueIt++)->getId();
			  childNoiseInIntersectionWithPresentValues.push_back(noiseInIntersectionWithPresentValues[valueId]);
			  childNoiseInIntersectionWithPresentAndPotentialValues.push_back(noiseInIntersectionWithPresentAndPotentialValues[valueId]);
			}
		    }
		}
	    }
	}
    }
}

Value* MetricAttribute::moveValueFromPotentialToPresent(const unsigned int valueOriginalId)
{
  vector<Value*>::iterator valueIt = potential.begin();
  for (; (*valueIt)->getOriginalId() != valueOriginalId; ++valueIt)
    {
    }
  Value* value = *(present.insert(lower_bound(present.begin(), present.end(), *valueIt, Value::smallerId), *valueIt));
  potential.erase(valueIt);
  return value;
}

Value* MetricAttribute::moveValueFromPotentialToAbsent(const vector<Value*>::iterator valueIt)
{
  Value* value = *(absent.insert(lower_bound(absent.begin(), absent.end(), *valueIt, Value::smallerId), *valueIt));
  potential.erase(valueIt);
  return value;
}

// PERF: decrease of the absent intervals on which binary searches are performed (use of the last iterator returned as a new bound)
pair<bool, vector<unsigned int>> MetricAttribute::findIrrelevantValuesAndCheckTauContiguity(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd, IrrelevantValueIds& irrelevantValueIds)
{
  if (present.empty())
    {
      return Attribute::findIrrelevantValuesAndCheckTauContiguity(attributeBegin, attributeEnd, irrelevantValueIds);
    }
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  if (potential.empty())
    {
#ifdef DETAILED_TIME
      propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
      return pair<bool, vector<unsigned int>>(false, vector<unsigned int>());
    }
  vector<unsigned int> newIrrelevantValueOriginalIds;
  newIrrelevantValueOriginalIds.reserve(potential.size());
  const double tau = tauVector[id];
  const vector<double>& timestamps = timestampsVector[id];
  vector<Value*>::iterator lowerPotentialBorderIt = lower_bound(potential.begin(), potential.end(), present.front(), Value::smallerId);
  vector<Value*>::const_iterator upperPotentialBorderIt = lower_bound(potential.begin(), potential.end(), present.back(), Value::smallerId);
  if (lowerPotentialBorderIt != potential.end())
    {
      // There are some potential values that are greater than the smallest present value
      list<unsigned int>::iterator irrelevantValueIdIt = lower_bound(irrelevantValueIds.irrelevantValueIds.begin(), irrelevantValueIds.irrelevantValueIds.end(), (*lowerPotentialBorderIt)->getId());
      // Values between the lowest and the greatest present timestamps
      vector<Value*>::const_iterator presentIt = present.begin();
      double presentTimestamp = timestamps[(*presentIt)->getOriginalId()];
      double scopeTimestamp = presentTimestamp;
      for (vector<Value*>::const_iterator potentialIt = lowerPotentialBorderIt; potentialIt != upperPotentialBorderIt; )
	{
	  // Increase scopeTimestamp w.r.t. present timestamps within scope
	  while (presentTimestamp <= scopeTimestamp && presentIt != present.end())
	    {
	      scopeTimestamp = presentTimestamp + tau;
	      if (++presentIt != present.end())
		{
		  presentTimestamp = timestamps[(*presentIt)->getOriginalId()];
		}
	    }
	  const unsigned int potentialId = (*potentialIt)->getId();
	  const unsigned int potentialOriginalId = (*potentialIt)->getOriginalId();
	  if (irrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.end() && *irrelevantValueIdIt == potentialId)
	    {
	      ++potentialIt;
	      ++irrelevantValueIdIt;
	    }
	  else
	    {
	      if (valueDoesNotExtendPresent(**potentialIt, attributeBegin, attributeEnd))
		{
		  if (++potentialIt == upperPotentialBorderIt && presentIt != present.end())
		    {
#ifdef DETAILED_TIME
		      propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef DEBUG
		      cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!" << endl;
#endif
		      return pair<bool, vector<unsigned int>>(true, vector<unsigned int>());
		    }
		  irrelevantValueIds.irrelevantValueIds.insert(irrelevantValueIdIt, potentialId);
		  newIrrelevantValueOriginalIds.push_back(potentialOriginalId);
		}
	      else
		{
		  // Check tau-contiguity
		  const double potentialTimestamp = timestamps[potentialOriginalId];
		  if (potentialTimestamp > scopeTimestamp)
		    {
#ifdef DETAILED_TIME
		      propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef DEBUG
		      cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!" << endl;
#endif
		      return pair<bool, vector<unsigned int>>(true, vector<unsigned int>());
		    }
		  // Increase (if possible) scopeTimestamp w.r.t. potentialTimestamp
		  const double scopePotentialTimestamp = potentialTimestamp + tau;
		  if (scopePotentialTimestamp > scopeTimestamp)
		    {
		      scopeTimestamp = scopePotentialTimestamp;
		    }
		  ++potentialIt;
		}
	    }
	}
    }
  // Values beyond the greatest present timestamp
  if (upperPotentialBorderIt != potential.end())
    {
      // There are some potential values that are greater than the largest present value
      list<unsigned int>::iterator upperIrrelevantValueIdIt = lower_bound(irrelevantValueIds.irrelevantValueIds.begin(), irrelevantValueIds.irrelevantValueIds.end(), (*upperPotentialBorderIt)->getId());
      double scopeTimestamp = timestamps[present.back()->getOriginalId()] + tau;
      for (; upperPotentialBorderIt != potential.end(); ++upperPotentialBorderIt)
	{
	  // Check tau-contiguity
	  const double nextTimestamp = timestamps[(*upperPotentialBorderIt)->getOriginalId()];
	  if (nextTimestamp > scopeTimestamp)
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beyond " << scopeTimestamp << " is irrelevant" << endl;
#endif
	      for (vector<Value*>::const_iterator tauFarValueIt = upperPotentialBorderIt; tauFarValueIt != potential.end(); ++tauFarValueIt)
		{
		  if (upperIrrelevantValueIdIt == irrelevantValueIds.irrelevantValueIds.end() || *upperIrrelevantValueIdIt != (*tauFarValueIt)->getId())
		    {
		      upperIrrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.insert(upperIrrelevantValueIdIt, (*tauFarValueIt)->getId());
		      newIrrelevantValueOriginalIds.push_back((*tauFarValueIt)->getOriginalId());
		    }
		  ++upperIrrelevantValueIdIt;
		}
	      break;
	    }
	  const unsigned int upperPotentialBorderId = (*upperPotentialBorderIt)->getId();
	  if (upperIrrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.end() && *upperIrrelevantValueIdIt == upperPotentialBorderId)
	    {
	      ++upperIrrelevantValueIdIt;
	    }
	  else
	    {
	      if (valueDoesNotExtendPresent(**upperPotentialBorderIt, attributeBegin, attributeEnd))
		{
		  irrelevantValueIds.irrelevantValueIds.insert(upperIrrelevantValueIdIt, upperPotentialBorderId);
		  newIrrelevantValueOriginalIds.push_back((*upperPotentialBorderIt)->getOriginalId());
		}
	      else
		{
		  scopeTimestamp = nextTimestamp + tau;
		}
	    }
	}
      eraseAbsentValuesBeyondTimestamp(scopeTimestamp);
    }
  // Values beneath the lowest present timestamp
  if (lowerPotentialBorderIt != potential.begin())
    {
      // There are some potential values that are lower than the smallest present value
      list<unsigned int>::iterator lowerIrrelevantValueIdIt;
      if (lowerPotentialBorderIt == potential.end())
	{
	  lowerIrrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.end();
	}
      else
	{
	  lowerIrrelevantValueIdIt = lower_bound(irrelevantValueIds.irrelevantValueIds.begin(), irrelevantValueIds.irrelevantValueIds.end(), (*lowerPotentialBorderIt)->getId()); // cannot be computed before (case: all values in irrelevantValueIds, if any, were smaller than present.front() at the call)
	}
      if (lowerIrrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.begin())
	{
	  --lowerIrrelevantValueIdIt;
	}
      double scopeTimestamp = timestamps[present.front()->getOriginalId()] - tau;
      do
	{
	  // Check tau-contiguity
	  const double nextTimestamp = timestamps[(*(--lowerPotentialBorderIt))->getOriginalId()];
	  if (nextTimestamp < scopeTimestamp)
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beneath " << scopeTimestamp << " is irrelevant" << endl;
#endif
	      list<unsigned int>::iterator insertIt = irrelevantValueIds.irrelevantValueIds.begin();
	      for (vector<Value*>::const_iterator tauFarValueIt = potential.begin(); tauFarValueIt != lowerPotentialBorderIt + 1; ++tauFarValueIt)
		{
		  const unsigned int tauFarValueId = (*tauFarValueIt)->getId();
		  if (insertIt == irrelevantValueIds.irrelevantValueIds.end() || *insertIt != tauFarValueId)
		    {
		      insertIt = irrelevantValueIds.irrelevantValueIds.insert(insertIt, tauFarValueId);
		      newIrrelevantValueOriginalIds.push_back((*tauFarValueIt)->getOriginalId());
		    }
		  ++insertIt;
		}
	      break;
	    }
	  if (lowerIrrelevantValueIdIt == irrelevantValueIds.irrelevantValueIds.end())
	    {
	      if (valueDoesNotExtendPresent(**lowerPotentialBorderIt, attributeBegin, attributeEnd))
		{
		  lowerIrrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.insert(lowerIrrelevantValueIdIt, (*lowerPotentialBorderIt)->getId());
		  newIrrelevantValueOriginalIds.push_back((*lowerPotentialBorderIt)->getOriginalId());
		}
	      else
		{
		  scopeTimestamp = nextTimestamp - tau;
		}
	    }
	  else
	    {
	      const unsigned int lowerPotentialBorderId = (*lowerPotentialBorderIt)->getId();
	      if (lowerPotentialBorderId < *lowerIrrelevantValueIdIt)
		{
		  // Here lowerIrrelevantValueIdIt == irrelevantValueIds.irrelevantValueIds.begin() holds
		  if (valueDoesNotExtendPresent(**lowerPotentialBorderIt, attributeBegin, attributeEnd))
		    {
		      lowerIrrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.insert(lowerIrrelevantValueIdIt, lowerPotentialBorderId);
		      newIrrelevantValueOriginalIds.push_back((*lowerPotentialBorderIt)->getOriginalId());
		    }
		  else
		    {
		      scopeTimestamp = nextTimestamp - tau;
		    }
		}
	      else
		{
		  if (lowerPotentialBorderId == *lowerIrrelevantValueIdIt)
		    {
		      if (lowerIrrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.begin())
			{
			  --lowerIrrelevantValueIdIt;
			}
		    }
		  else
		    {
		      if (valueDoesNotExtendPresent(**lowerPotentialBorderIt, attributeBegin, attributeEnd))
			{
			  lowerIrrelevantValueIdIt = --(irrelevantValueIds.irrelevantValueIds.insert(++lowerIrrelevantValueIdIt, lowerPotentialBorderId));
			  newIrrelevantValueOriginalIds.push_back((*lowerPotentialBorderIt)->getOriginalId());
			}
		      else
			{
			  scopeTimestamp = nextTimestamp - tau;
			}
		    }
		}
	    }
	} while (lowerPotentialBorderIt != potential.begin());
      eraseAbsentValuesBeneathTimestamp(scopeTimestamp);
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return pair<bool, vector<unsigned int>>(false, newIrrelevantValueOriginalIds);
}

#ifdef MIN_SIZE_ELEMENT_PRUNING
// CLEAN: Like findIrrelevantValuesAndCheckConstraints but presentAndPotentialIrrelevantValue replaces irrelevantValue (factorize?)
pair<bool, vector<unsigned int>> MetricAttribute::findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity(const unsigned int presentAndPotentialIrrelevancyThreshold, IrrelevantValueIds& irrelevantValueIds)
{
  if (present.empty())
    {
      return Attribute::findPresentAndPotentialIrrelevantValuesAndCheckTauContiguity(presentAndPotentialIrrelevancyThreshold, irrelevantValueIds);
    }
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  if (potential.empty())
    {
#ifdef DETAILED_TIME
      propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
      return pair<bool, vector<unsigned int>>(false, vector<unsigned int>());
    }
  vector<unsigned int> newIrrelevantValueOriginalIds;
  newIrrelevantValueOriginalIds.reserve(potential.size());
  const double tau = tauVector[id];
  const vector<double>& timestamps = timestampsVector[id];
  vector<Value*>::iterator lowerPotentialBorderIt = lower_bound(potential.begin(), potential.end(), present.front(), Value::smallerId);
  vector<Value*>::const_iterator upperPotentialBorderIt = lower_bound(potential.begin(), potential.end(), present.back(), Value::smallerId);
  if (lowerPotentialBorderIt != potential.end())
    {
      // There are some potential values that are greater than the smallest present value
      list<unsigned int>::iterator irrelevantValueIdIt = lower_bound(irrelevantValueIds.irrelevantValueIds.begin(), irrelevantValueIds.irrelevantValueIds.end(), (*lowerPotentialBorderIt)->getId());
      // Values between the lowest and the greatest present timestamps
      vector<Value*>::const_iterator presentIt = present.begin();
      double presentTimestamp = timestamps[(*presentIt)->getOriginalId()];
      double scopeTimestamp = presentTimestamp;
      for (vector<Value*>::const_iterator potentialIt = lowerPotentialBorderIt; potentialIt != upperPotentialBorderIt; )
	{
	  // Increase scopeTimestamp w.r.t. present timestamps within scope
	  while (presentTimestamp <= scopeTimestamp && presentIt != present.end())
	    {
	      scopeTimestamp = presentTimestamp + tau;
	      if (++presentIt != present.end())
		{
		  presentTimestamp = timestamps[(*presentIt)->getOriginalId()];
		}
	    }
	  const unsigned int potentialId = (*potentialIt)->getId();
	  const unsigned int potentialOriginalId = (*potentialIt)->getOriginalId();
	  if (irrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.end() && *irrelevantValueIdIt == potentialId)
	    {
	      ++potentialIt;
	      ++irrelevantValueIdIt;
	    }
	  else
	    {
	      if (presentAndPotentialIrrelevantValue(**potentialIt, presentAndPotentialIrrelevancyThreshold))
		{
		  if (++potentialIt == upperPotentialBorderIt && presentIt != present.end())
		    {
#ifdef DETAILED_TIME
		      propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef DEBUG
		      cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!" << endl;
#endif
		      return pair<bool, vector<unsigned int>>(true, vector<unsigned int>());
		    }
		  irrelevantValueIds.irrelevantValueIds.insert(irrelevantValueIdIt, potentialId);
		  newIrrelevantValueOriginalIds.push_back(potentialOriginalId);
		}
	      else
		{
		  // Check tau-contiguity
		  const double potentialTimestamp = timestamps[potentialOriginalId];
		  if (potentialTimestamp > scopeTimestamp)
		    {
#ifdef DETAILED_TIME
		      propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef DEBUG
		      cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!" << endl;
#endif
		      return pair<bool, vector<unsigned int>>(true, vector<unsigned int>());
		    }
		  // Increase (if possible) scopeTimestamp w.r.t. potentialTimestamp
		  const double scopePotentialTimestamp = potentialTimestamp + tau;
		  if (scopePotentialTimestamp > scopeTimestamp)
		    {
		      scopeTimestamp = scopePotentialTimestamp;
		    }
		  ++potentialIt;
		}
	    }
	}
    }
  // Values beyond the greatest present timestamp
  if (upperPotentialBorderIt != potential.end())
    {
      // There are some potential values that are greater than the largest present value
      list<unsigned int>::iterator upperIrrelevantValueIdIt = lower_bound(irrelevantValueIds.irrelevantValueIds.begin(), irrelevantValueIds.irrelevantValueIds.end(), (*upperPotentialBorderIt)->getId());
      double scopeTimestamp = timestamps[present.back()->getOriginalId()] + tau;
      for (; upperPotentialBorderIt != potential.end(); ++upperPotentialBorderIt)
	{
	  // Check tau-contiguity
	  const double nextTimestamp = timestamps[(*upperPotentialBorderIt)->getOriginalId()];
	  if (nextTimestamp > scopeTimestamp)
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential or absent value beyond " << scopeTimestamp << " is irrelevant" << endl;
#endif
	      for (vector<Value*>::const_iterator tauFarValueIt = upperPotentialBorderIt; tauFarValueIt != potential.end(); ++tauFarValueIt)
		{
		  const unsigned int tauFarValueId = (*tauFarValueIt)->getId();
		  if (upperIrrelevantValueIdIt == irrelevantValueIds.irrelevantValueIds.end() || *upperIrrelevantValueIdIt != tauFarValueId)
		    {
		      upperIrrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.insert(upperIrrelevantValueIdIt, tauFarValueId);
		      newIrrelevantValueOriginalIds.push_back((*tauFarValueIt)->getOriginalId());
		    }
		  ++upperIrrelevantValueIdIt;
		}
	      break;
	    }
	  const unsigned int upperPotentialBorderId = (*upperPotentialBorderIt)->getId();
	  if (upperIrrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.end() && *upperIrrelevantValueIdIt == upperPotentialBorderId)
	    {
	      ++upperIrrelevantValueIdIt;
	    }
	  else
	    {
	      if (presentAndPotentialIrrelevantValue(**upperPotentialBorderIt, presentAndPotentialIrrelevancyThreshold))
		{
		  irrelevantValueIds.irrelevantValueIds.insert(upperIrrelevantValueIdIt, upperPotentialBorderId);
		  newIrrelevantValueOriginalIds.push_back((*upperPotentialBorderIt)->getOriginalId());
		}
	      else
		{
		  scopeTimestamp = nextTimestamp + tau;
		}
	    }
	}
      eraseAbsentValuesBeyondTimestamp(scopeTimestamp);
    }
  // Values beneath the lowest present timestamp
  if (lowerPotentialBorderIt != potential.begin())
    {
      // There are some potential values that are lower than the smallest present value
      list<unsigned int>::iterator lowerIrrelevantValueIdIt;
      if (lowerPotentialBorderIt == potential.end())
	{
	  lowerIrrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.end();
	}
      else
	{
	  lowerIrrelevantValueIdIt = lower_bound(irrelevantValueIds.irrelevantValueIds.begin(), irrelevantValueIds.irrelevantValueIds.end(), (*lowerPotentialBorderIt)->getId()); // cannot be computed before (case: all values in irrelevantValueIds, if any, were smaller than present.front() at the call)
	}
      if (lowerIrrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.begin())
	{
	  --lowerIrrelevantValueIdIt;
	}
      double scopeTimestamp = timestamps[present.front()->getOriginalId()] - tau;
      do
	{
	  // Check tau-contiguity
	  const double nextTimestamp = timestamps[(*(--lowerPotentialBorderIt))->getOriginalId()];
	  if (nextTimestamp < scopeTimestamp)
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential or absent value beneath " << scopeTimestamp << " is irrelevant" << endl;
#endif
	      list<unsigned int>::iterator insertIt = irrelevantValueIds.irrelevantValueIds.begin();
	      for (vector<Value*>::const_iterator tauFarValueIt = potential.begin(); tauFarValueIt != lowerPotentialBorderIt + 1; ++tauFarValueIt)
		{
		  const unsigned int tauFarValueId = (*tauFarValueIt)->getId();
		  if (insertIt == irrelevantValueIds.irrelevantValueIds.end() || *insertIt != tauFarValueId)
		    {
		      insertIt = irrelevantValueIds.irrelevantValueIds.insert(insertIt, tauFarValueId);
		      newIrrelevantValueOriginalIds.push_back((*tauFarValueIt)->getOriginalId());
		    }
		  ++insertIt;
		}
	      break;
	    }
	  if (lowerIrrelevantValueIdIt == irrelevantValueIds.irrelevantValueIds.end())
	    {
	      if (presentAndPotentialIrrelevantValue(**lowerPotentialBorderIt, presentAndPotentialIrrelevancyThreshold))
		{
		  lowerIrrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.insert(lowerIrrelevantValueIdIt, (*lowerPotentialBorderIt)->getId());
		  newIrrelevantValueOriginalIds.push_back((*lowerPotentialBorderIt)->getOriginalId());
		}
	      else
		{
		  scopeTimestamp = nextTimestamp - tau;
		}
	    }
	  else
	    {
	      const unsigned int lowerPotentialBorderId = (*lowerPotentialBorderIt)->getId();
	      if (lowerPotentialBorderId < *lowerIrrelevantValueIdIt)
		{
		  // Here lowerIrrelevantValueIdIt == irrelevantValueIds.irrelevantValueIds.begin() holds
		  if (presentAndPotentialIrrelevantValue(**lowerPotentialBorderIt, presentAndPotentialIrrelevancyThreshold))
		    {
		      lowerIrrelevantValueIdIt = irrelevantValueIds.irrelevantValueIds.insert(lowerIrrelevantValueIdIt, lowerPotentialBorderId);
		      newIrrelevantValueOriginalIds.push_back((*lowerPotentialBorderIt)->getOriginalId());
		    }
		  else
		    {
		      scopeTimestamp = nextTimestamp - tau;
		    }
		}
	      else
		{
		  if (lowerPotentialBorderId == *lowerIrrelevantValueIdIt)
		    {
		      if (lowerIrrelevantValueIdIt != irrelevantValueIds.irrelevantValueIds.begin())
			{
			  --lowerIrrelevantValueIdIt;
			}
		    }
		  else
		    {
		      if (presentAndPotentialIrrelevantValue(**lowerPotentialBorderIt, presentAndPotentialIrrelevancyThreshold))
			{
			  lowerIrrelevantValueIdIt = --(irrelevantValueIds.irrelevantValueIds.insert(++lowerIrrelevantValueIdIt, lowerPotentialBorderId));
			  newIrrelevantValueOriginalIds.push_back((*lowerPotentialBorderIt)->getOriginalId());
			}
		      else
			{
			  scopeTimestamp = nextTimestamp - tau;
			}
		    }
		}
	    }
	} while (lowerPotentialBorderIt != potential.begin());
      eraseAbsentValuesBeneathTimestamp(scopeTimestamp);
    }
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return pair<bool, vector<unsigned int>>(false, newIrrelevantValueOriginalIds);
}
#endif

void MetricAttribute::eraseAbsentValuesBeneathTimestamp(const double timestamp)
{
#ifdef DEBUG
  cout << "In the " << tauVector[id] << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every absent value beneath " << timestamp << " is removed" << endl;
#endif
  const vector<double>& timestamps = timestampsVector[id];
  vector<Value*>::iterator absentValueIt = absent.begin();
  for (; absentValueIt != absent.end() && timestamps[(*absentValueIt)->getOriginalId()] < timestamp; ++absentValueIt)
    {
      delete *absentValueIt;
    }
  absent.erase(absent.begin(), absentValueIt);
}

void MetricAttribute::eraseAbsentValuesBeyondTimestamp(const double timestamp)
{
#ifdef DEBUG
  cout << "In the " << tauVector[id] << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every absent value beyond " << timestamp << " is removed" << endl;
#endif
  if (!absent.empty())
    {
      const vector<double>& timestamps = timestampsVector[id];
      if (timestamps[absent.front()->getOriginalId()] > timestamp)
	{
	  for (Value* absentValue : absent)
	    {
	      delete absentValue;
	    }
	  absent.clear();
	  return;
	}
      vector<Value*>::iterator absentValueIt = --(absent.end());
      for (; timestamps[(*absentValueIt)->getOriginalId()] > timestamp; --absentValueIt)
	{
	  delete *absentValueIt;
	}
      absent.erase(absentValueIt + 1, absent.end());
    }
}

// CLEAN: Like Attribute::closed but the absent range is restricted at the beginning (factorize?)
// WARNING: the symmetric absent sets must contain the same original ids in the same order
// WARNING: This must be the last symmetric attribute
const bool MetricAttribute::closed(const vector<Attribute*>::const_iterator attributeBegin, const vector<Attribute*>::const_iterator attributeEnd) const
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  if (present.empty() || absent.empty())
    {
#ifdef DETAILED_TIME
      closednessCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
      return true;
    }
  const double tau = tauVector[id];
  const vector<double>& timestamps = timestampsVector[id];
  double scopeTimestamp = timestamps[present.back()->getOriginalId()] + tau;
  vector<Value*>::const_iterator tauCloseAbsentEnd = lower_bound(absent.begin(), absent.end(), present.back(), Value::smallerId);
  for (; tauCloseAbsentEnd != absent.end() && timestamps[(*tauCloseAbsentEnd)->getOriginalId()] <= scopeTimestamp; ++tauCloseAbsentEnd)
    {
    }
  scopeTimestamp = timestamps[present.front()->getOriginalId()] - tau;
  vector<Value*>::const_iterator tauCloseAbsentBegin;
  if (timestamps[absent.front()->getOriginalId()] < scopeTimestamp)
    {
      tauCloseAbsentBegin = lower_bound(absent.begin(), absent.end(), present.front(), Value::smallerId);
      while (timestamps[(*(--tauCloseAbsentBegin))->getOriginalId()] >= scopeTimestamp)
	{
	}
      ++tauCloseAbsentBegin;
    }
  else
    {
      tauCloseAbsentBegin = absent.begin();
    }
  for (; tauCloseAbsentBegin != tauCloseAbsentEnd && valueDoesNotExtendPresentAndPotential(**tauCloseAbsentBegin, attributeBegin, attributeEnd); ++tauCloseAbsentBegin)
    {
    }
#ifdef DETAILED_TIME
  closednessCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
#ifdef DEBUG
  if (tauCloseAbsentBegin != tauCloseAbsentEnd)
    {
      cout << labelsVector[id][(*tauCloseAbsentBegin)->getOriginalId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " extends any future pattern -> Prune!" << endl;
    }
#endif
  return tauCloseAbsentBegin == tauCloseAbsentEnd;
}

void MetricAttribute::absentValueToCleanFound(vector<Value*>::iterator& absentValueIt)
{
#ifdef DEBUG
  cout << labelsVector[id][(*absentValueIt)->getOriginalId()] << " in attribute " << internal2ExternalAttributeOrder[id] << " will never extend any future pattern" << endl;
#endif
  delete *absentValueIt;
  absentValueIt = absent.erase(absentValueIt);
}

pair<const bool, vector<unsigned int>> MetricAttribute::tauFarValueOriginalValueIdsAndCheckConstraints(const Value* absentValue)
{
  if (present.empty())
    {
      return pair<const bool, vector<unsigned int>>(false, vector<unsigned int>());
    }
  const double tau = tauVector[id];
  const vector<double>& timestamps = timestampsVector[id];
  const vector<Value*>::iterator lowerPotentialBorderIt = lower_bound(potential.begin(), potential.end(), absentValue, Value::smallerId);
  const unsigned int absentValueId = absentValue->getId();
  const unsigned int minPresentId = present.front()->getId();
  if (minPresentId > absentValueId)
    {
      // absentValue is beneath the present range
      vector<unsigned int> irrelevantValueOriginalIds;
      irrelevantValueOriginalIds.reserve(potential.size());
      double scopeTimestamp;
      if (lowerPotentialBorderIt != potential.end() && (*lowerPotentialBorderIt)->getId() < minPresentId)
	{
	  scopeTimestamp = timestamps[(*lowerPotentialBorderIt)->getOriginalId()] - tau;
	}
      else
	{
	  scopeTimestamp = timestamps[present.front()->getOriginalId()] - tau;
	}
      if (lowerPotentialBorderIt == potential.begin())
	{
	  eraseAbsentValuesBeneathTimestamp(scopeTimestamp);
	}
      else
	{
	  if (scopeTimestamp > timestamps[(*(lowerPotentialBorderIt - 1))->getOriginalId()])
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beneath " << scopeTimestamp << " is irrelevant" << endl;
#endif
	      // Potential values with too small timestamps are irrelevant
	      for (vector<Value*>::iterator tauFarPotentialValueIt = potential.begin(); tauFarPotentialValueIt != lowerPotentialBorderIt; ++tauFarPotentialValueIt)
		{
		  irrelevantValueOriginalIds.push_back((*tauFarPotentialValueIt)->getOriginalId());
		  delete *tauFarPotentialValueIt;
		}
	      potential.erase(potential.begin(), lowerPotentialBorderIt);
	      eraseAbsentValuesBeneathTimestamp(scopeTimestamp);
	    }
	}
      return pair<const bool, vector<unsigned int>>(false, irrelevantValueOriginalIds);
    }
  const unsigned int maxPresentId = present.back()->getId();
  if (maxPresentId < absentValueId)
    {
      // absentValue is beyond the present range
      vector<unsigned int> irrelevantValueOriginalIds;
      irrelevantValueOriginalIds.reserve(potential.size());
      if (lowerPotentialBorderIt == potential.end())
	{
	  if (potential.empty() || potential.back()->getId() < maxPresentId)
	    {
	      eraseAbsentValuesBeyondTimestamp(timestamps[present.back()->getOriginalId()] + tau);
	    }
	  else
	    {
	      eraseAbsentValuesBeyondTimestamp(timestamps[potential.back()->getOriginalId()] + tau);
	    }
	}
      else
	{
	  double scopeTimestamp;
	  if (lowerPotentialBorderIt == potential.begin())
	    {
	      scopeTimestamp = timestamps[present.back()->getOriginalId()] + tau;
	    }
	  else
	    {
	      if ((*(lowerPotentialBorderIt - 1))->getId() < maxPresentId)
		{
		  scopeTimestamp = timestamps[present.back()->getOriginalId()] + tau;
		}
	      else
		{
		  scopeTimestamp = timestamps[(*(lowerPotentialBorderIt - 1))->getOriginalId()] + tau;
		}
	    }
	  if (scopeTimestamp < timestamps[(*lowerPotentialBorderIt)->getOriginalId()])
	    {
#ifdef DEBUG
	      cout << "In the " << tau << "-contiguous attribute " << internal2ExternalAttributeOrder[id] << ", every potential value beyond " << scopeTimestamp << " is irrelevant" << endl;
#endif
	      // Potential values with too great timestamps are irrelevant
	      for (vector<Value*>::iterator tauFarPotentialValueIt = lowerPotentialBorderIt; tauFarPotentialValueIt != potential.end(); ++tauFarPotentialValueIt)
		{
		  irrelevantValueOriginalIds.push_back((*tauFarPotentialValueIt)->getOriginalId());
		  delete *tauFarPotentialValueIt;
		}
	      potential.erase(lowerPotentialBorderIt, potential.end());
	      eraseAbsentValuesBeyondTimestamp(scopeTimestamp);
	    }
	}
      return pair<const bool, vector<unsigned int>>(false, irrelevantValueOriginalIds);
    }
  // absentValue is inside the present range
  const vector<Value*>::iterator lowerPresentBorderIt = lower_bound(present.begin(), present.end(), absentValue, Value::smallerId);
  double previousTimestamp;
  if (lowerPotentialBorderIt == potential.begin())
    {
      previousTimestamp = timestamps[(*(lowerPresentBorderIt - 1))->getOriginalId()];
    }
  else
    {
      previousTimestamp = timestamps[max((*(lowerPresentBorderIt - 1))->getOriginalId(), (*(lowerPotentialBorderIt - 1))->getOriginalId())];
    }
  double nextTimestamp;
  if (lowerPotentialBorderIt == potential.end())
    {
      nextTimestamp = timestamps[(*lowerPresentBorderIt)->getOriginalId()];
    }
  else
    {
      nextTimestamp = timestamps[min((*lowerPresentBorderIt)->getOriginalId(), (*lowerPotentialBorderIt)->getOriginalId())];
    }
  if (previousTimestamp + tau < nextTimestamp)
    {
#ifdef DEBUG
      cout << tau << "-contiguity constraint on attribute " << internal2ExternalAttributeOrder[id] << " not verified -> Prune!" << endl;
#endif
      return pair<const bool, vector<unsigned int>>(true, vector<unsigned int>());
    }
  return pair<const bool, vector<unsigned int>>(false, vector<unsigned int>());
}

const bool MetricAttribute::finalizable() const
{
  return (!present.empty() || potential.empty()) && Attribute::finalizable();
}

vector<unsigned int> MetricAttribute::finalize()
{
#ifdef DETAILED_TIME
  const steady_clock::time_point startingPoint = steady_clock::now();
#endif
  vector<unsigned int> originalIdsOfValuesSetPresent;
  originalIdsOfValuesSetPresent.reserve(potential.size());
  // WARNING: present and potential must be ordered
  for (Value* potentialValue : potential)
    {
      present.insert(lower_bound(present.begin(), present.end(), potentialValue, Value::smallerId), potentialValue);
      originalIdsOfValuesSetPresent.push_back(potentialValue->getOriginalId());
    }
  potential.clear();
#ifdef DETAILED_TIME
  propagationCheckingDuration += duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#endif
  return originalIdsOfValuesSetPresent;
}
