// Copyright 2013,2014 Matheus Marzano

// This file is part of multidupehack.

// multidupehack is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef ELEMENT_H_
#define ELEMENT_H_

#include <vector>

using namespace std;

class Element
{
 public:
  Element(const unsigned int id, const unsigned int noise = 0);

  const bool operator<(const Element& otherElement) const;
  const unsigned int getId() const;
  const unsigned int getNoise() const;
  void addNoise(const unsigned int n);

  static const bool smallerId(const Element& e1, const Element& e2);
  static vector<unsigned int> idVectorUnion(const vector<Element>& v1,const vector<Element>& v2);

 protected:
  unsigned int id;
  unsigned int noise; 
};

#endif	/*ELEMENT_H*/
