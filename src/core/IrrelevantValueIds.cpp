// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include "IrrelevantValueIds.h"
#include "Attribute.h"

IrrelevantValueIds::IrrelevantValueIds(): irrelevantValueIds(), attributeIt()
{
}

IrrelevantValueIds::IrrelevantValueIds(const vector<Attribute*>::iterator attributeItParam): irrelevantValueIds(), attributeIt(attributeItParam)
{
}

IrrelevantValueIds::IrrelevantValueIds(const IrrelevantValueIds& otherIrrelevantValueIds, const vector<Attribute*>::iterator attributeItParam): irrelevantValueIds(otherIrrelevantValueIds.irrelevantValueIds), attributeIt(attributeItParam)
{
}

const bool IrrelevantValueIds::operator<(const IrrelevantValueIds& otherIrrelevantValueIds) const
{
  return irrelevantValueIds.size() * (*(otherIrrelevantValueIds.attributeIt))->globalSize() > otherIrrelevantValueIds.irrelevantValueIds.size() * (*attributeIt)->globalSize();
}

vector<Attribute*>::iterator IrrelevantValueIds::getAttributeIt() const
{
  return attributeIt;
}

void IrrelevantValueIds::setAttributeIt(const vector<Attribute*>::iterator attributeItParam)
{
  attributeIt = attributeItParam;
}

const bool IrrelevantValueIds::someIrrelevantElement() const
{
  return !irrelevantValueIds.empty();
}

void IrrelevantValueIds::clear()
{
  irrelevantValueIds.clear();
}

void IrrelevantValueIds::mergeWith(const IrrelevantValueIds& otherIrrelevantValueIds)
{
  list<unsigned int>::const_iterator otherIrrelevantValueIdsIt = otherIrrelevantValueIds.irrelevantValueIds.begin();
  for (list<unsigned int>::iterator irrelevantValueIdsIt = irrelevantValueIds.begin(); irrelevantValueIdsIt != irrelevantValueIds.end() && otherIrrelevantValueIdsIt != otherIrrelevantValueIds.irrelevantValueIds.end(); ++irrelevantValueIdsIt)
    {
      if (*otherIrrelevantValueIdsIt < *irrelevantValueIdsIt)
	{
	  irrelevantValueIdsIt = irrelevantValueIds.insert(irrelevantValueIdsIt, *otherIrrelevantValueIdsIt++);
	}
      else
	{
	  if (*otherIrrelevantValueIdsIt == *irrelevantValueIdsIt)
	    {
	      ++otherIrrelevantValueIdsIt;
	    }
	}
    }
  irrelevantValueIds.insert(irrelevantValueIds.end(), otherIrrelevantValueIdsIt, otherIrrelevantValueIds.irrelevantValueIds.end());
}

void IrrelevantValueIds::newIrrelevantValues(const IrrelevantValueIds& otherIrrelevantValueIds)
{
  list<unsigned int>::const_iterator otherIrrelevantValueIdsIt = otherIrrelevantValueIds.irrelevantValueIds.begin();
  for (list<unsigned int>::iterator irrelevantValueIdsIt = irrelevantValueIds.begin(); irrelevantValueIdsIt != irrelevantValueIds.end() && otherIrrelevantValueIdsIt != otherIrrelevantValueIds.irrelevantValueIds.end(); ++otherIrrelevantValueIdsIt)
    {
      if (*otherIrrelevantValueIdsIt == *irrelevantValueIdsIt)
	{
	  irrelevantValueIdsIt = irrelevantValueIds.erase(irrelevantValueIdsIt);
	}
      else
	{
	  irrelevantValueIdsIt = irrelevantValueIds.insert(irrelevantValueIdsIt, *otherIrrelevantValueIdsIt);
	  ++irrelevantValueIdsIt;
	}
    }
  irrelevantValueIds.insert(irrelevantValueIds.end(), otherIrrelevantValueIdsIt, otherIrrelevantValueIds.irrelevantValueIds.end());
}
