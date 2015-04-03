// Copyright 2007,2008,2009,2010,2011,2012,2013,2014 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef IRRELEVANTVALUEIDS_H_
#define IRRELEVANTVALUEIDS_H_

#include "../../Parameters.h"

#include <vector>
#include <list>
#include <ostream>

class Attribute;

using namespace std;

class IrrelevantValueIds
{
 public:
  /* PERF: a list<vector<Value*>::iterator> would avoid browsing potential when removing the irrelevant values (they should then be removed backwards) */
  list<unsigned int> irrelevantValueIds;

  IrrelevantValueIds();
  IrrelevantValueIds(const vector<Attribute*>::iterator attributeIt);
  IrrelevantValueIds(const IrrelevantValueIds& otherIrrelevantValueIds, const vector<Attribute*>::iterator attributeIt);

  const bool operator<(const IrrelevantValueIds& otherIrrelevantValueIds) const;

  vector<Attribute*>::iterator getAttributeIt() const;
  const bool someIrrelevantElement() const;
  void clear();

  void setAttributeIt(const vector<Attribute*>::iterator attributeIt);  
  void mergeWith(const IrrelevantValueIds& otherIrrelevantValueIds);
  void newIrrelevantValues(const IrrelevantValueIds& otherIrrelevantValueIds);

 protected:
  vector<Attribute*>::iterator attributeIt;
};

#endif /*IRRELEVANTVALUEIDS_H_*/
