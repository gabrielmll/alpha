// Copyright 2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "GroupMeasure.h"

unsigned int GroupMeasure::measureId = 0;
unsigned int GroupMeasure::nbOfMeasures;
vector<GroupCovers*> GroupMeasure::groupCovers;
bool GroupMeasure::isSomeMeasureMonotone = false;
bool GroupMeasure::isSomeMeasureAntiMonotone = false;

GroupMeasure::GroupMeasure()
{
  ++measureId;
}

GroupMeasure::GroupMeasure(const GroupMeasure& otherGroupMeasure)
{
  if (measureId == 0)
    {
      groupCovers.push_back(new GroupCovers(*(groupCovers.back())));
    }
  incrementMeasureId();
}

GroupMeasure::GroupMeasure(const GroupMeasure&& otherGroupMeasure)
{
}

GroupMeasure::~GroupMeasure()
{
  if (measureId == 0)
    {
      delete groupCovers.back();
      groupCovers.pop_back();
    }
  incrementMeasureId();
}

GroupMeasure& GroupMeasure::operator=(const GroupMeasure& otherGroupMeasure)
{
  if (measureId == 0)
    {
      groupCovers.push_back(new GroupCovers(*(groupCovers.back())));
    }
  incrementMeasureId();
  return *this;  
}

GroupMeasure& GroupMeasure::operator=(const GroupMeasure&& otherGroupMeasure)
{
  return *this;  
}

void GroupMeasure::initGroups(const vector<string>& groupFileNames, const char* groupElementSeparator, const char* groupDimensionElementsSeparator, const vector<unsigned int>& cardinalities, const vector<unordered_map<string, unsigned int>>& labels2Ids, const vector<unsigned int>& dimensionOrder)
{
  unsigned int totalNbOfElements = 0;
  for (const unsigned int cardinality : cardinalities)
    {
      totalNbOfElements += cardinality;
    }
  groupCovers.reserve(totalNbOfElements);
  groupCovers.push_back(new GroupCovers(groupFileNames, groupElementSeparator, groupDimensionElementsSeparator, cardinalities, labels2Ids, dimensionOrder));
}

void GroupMeasure::allMeasuresSet()
{
  nbOfMeasures = measureId;
  measureId = 0;
  // If unnecessary, clear the maximal group covers (in this way, do not copy them at every copy of the group cover measures)
  if (!isSomeMeasureMonotone)
    {
      groupCovers.back()->clearMaxCovers();
    }
  // If unnecessary, clear the minimal group covers (in this way, do not copy them at every copy of the group cover measures)
  if (!isSomeMeasureAntiMonotone)
    {
      groupCovers.back()->clearMinCovers();
    }
}

unsigned int GroupMeasure::minCoverOfGroup(const unsigned int groupId)
{
  return groupCovers.back()->minCoverOfGroup(groupId);
}

unsigned int GroupMeasure::maxCoverOfGroup(const unsigned int groupId)
{
  return groupCovers.back()->maxCoverOfGroup(groupId);
}

const bool GroupMeasure::violationAfterAdding(const unsigned int dimensionIdOfElementsSetPresent, const vector<unsigned int>& elementsSetPresent)
{
  if (isSomeMeasureAntiMonotone)
    {
      if (measureId == 0)
	{
	  groupCovers.back()->add(dimensionIdOfElementsSetPresent, elementsSetPresent);
	}
      if (violationAfterAdding())
	{
	  measureId = 0;
	  return true;
	}
      incrementMeasureId();
    }
  return false;
}

const bool GroupMeasure::violationAfterRemoving(const unsigned int dimensionIdOfElementsSetAbsent, const vector<unsigned int>& elementsSetAbsent)
{
  if (isSomeMeasureMonotone)
    {
      if (measureId == 0)
	{
	  groupCovers.back()->remove(dimensionIdOfElementsSetAbsent, elementsSetAbsent);
	}
      if (violationAfterRemoving())
	{
	  measureId = 0;
	  return true;
	}
      incrementMeasureId();
    }
  return false;
}

const bool GroupMeasure::violationAfterAdding() const
{
  return false;
}

const bool GroupMeasure::violationAfterRemoving() const
{
  return false;
}

void GroupMeasure::incrementMeasureId() const
{
  if (++measureId == nbOfMeasures)
    {
      measureId = 0;
    }
}
